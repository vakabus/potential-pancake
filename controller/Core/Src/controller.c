#include "controller.h"
#include "main.h"
#include "log.h"
#include "pid.h"
#include "tim.h"
#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef enum {
	BOOT = 0,
	INIT = 1,
	WAITING = 2,
	MOVING = 3,
	PUNCHING = 4,
	FAILED = 5,
	TEST = 6,
} main_state_t;

typedef enum {
	X,
	Y
} axis_t;

static const char* STATE_NAMES[] = {
		[BOOT] = "BOOT",
		[INIT] = "INIT",
		[WAITING] = "WAITING",
		[MOVING] = "MOVING",
		[PUNCHING] = "PUNCHING",
		[FAILED] = "FAILED",
		[TEST] = "TEST",
};

static main_state_t state = BOOT;
static int32_t head_position[] = {100000, 100000};
static bool block_interrupt_errors = false;


static void transition(main_state_t new) {
	if (state == new) {
		log_err("ehm wtf, you are trying to transition from state %s to state %s", STATE_NAMES[new], STATE_NAMES[new]);
		return;
	}

	log_info("transitioning to state %s", STATE_NAMES[new]);

	/* finally, change the state */
	state = new;
}

static inline bool is_blue_button_pressed() {
	GPIO_PinState state = HAL_GPIO_ReadPin(BUTTON_BLUE_GPIO_Port, BUTTON_BLUE_Pin);
	return state == 1;
}

static inline void configure_motor(axis_t axis, uint16_t power, bool forward) {
	if (power < 0 || power > 100) {
		log_error("dutyCycle %d out of valid range 0-100", power);
		return;
	}

	/* reconfigure direction & duty cycle */
	if (axis == X) {
		TIM4->CCR2 = forward ? power : (100 - power);
		HAL_GPIO_WritePin(DIR_X_GPIO_Port, DIR_X_Pin, forward);
	} else {
		TIM4->CCR1 = forward ? power : (100 - power);
		HAL_GPIO_WritePin(DIR_Y_GPIO_Port, DIR_Y_Pin, forward);
	}
}

static void set_motor_power(int16_t xpower, int16_t ypower) {
	bool xForward = xpower > 0;
	bool yForward = ypower > 0;
	configure_motor(X, abs(xpower), xForward);
	configure_motor(Y, abs(ypower), yForward);
}

static bool in_range(int32_t val, int32_t lowerBound, int32_t upperBound) {
	return lowerBound < val && val < upperBound;
}


static void head_goto(int32_t xCoord, int32_t yCoord) {
	pid_vars_t varsX;
	pid_vars_t varsY;

	pd_vars_init(&varsX, 0.25, 40, 100, -100);
	pd_vars_init(&varsY, 0.25, 40, 100, -100);

	int16_t xpower = 1;
	int16_t ypower = 1;

	int hitCount = 0;
	/* at least 250ms stationary */
	while (hitCount < 250) {
		if (in_range(head_position[0], xCoord*4-1, xCoord*4+1) && in_range(head_position[1],yCoord*4 - 1, yCoord*4+1)) hitCount++;
		else hitCount = 0;

		int32_t  ex = (xCoord*4) - head_position[0];
		int32_t ey = (yCoord*4) - head_position[1];
		xpower = (int16_t) pd_controller(&varsX, ex);
		ypower = (int16_t) pd_controller(&varsY, ey);
		set_motor_power(xpower, ypower);

		HAL_Delay(1);

		if (state == FAILED) {
			log_err("looks like we messed up while trying to go to coordinates [%ld, %ld]", xCoord, yCoord);
			return;
		}
	}
	set_motor_power(0,0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == FAIL_Pin) {
		log_error("simulator triggered device failure");
		transition(FAILED);
	}
	// 00 01 11 10 -> forward
	static uint8_t last_enc_x_state = 255;
	if (GPIO_Pin == ENC_X1_Pin || GPIO_Pin == ENC_X2_Pin) {
		GPIO_PinState x1 = HAL_GPIO_ReadPin(ENC_X1_GPIO_Port, ENC_X1_Pin);
		GPIO_PinState x2 = HAL_GPIO_ReadPin(ENC_X2_GPIO_Port, ENC_X2_Pin);
		uint8_t state = x1 << 1 | x2;
		if (last_enc_x_state == 255) {
			// not yet initialized, saving last state and not doing anything
			last_enc_x_state = state;
		} else {
			//log_info("last state %d, now %d", last_enc_x_state, state);
			state = last_enc_x_state << 2 | state;
			switch (state) {
				case 0b0001:
				case 0b0111:
				case 0b1110:
				case 0b1000:
					head_position[0] += 1;
					break;
				case 0b1011:
				case 0b1101:
				case 0b0100:
				case 0b0010:
					head_position[0] -= 1;
					break;
				default: {
					if (!block_interrupt_errors) {
						log_err("invalid X encoder change, packed state is %d", state);
						transition(FAILED);
					}
				}
			}
			last_enc_x_state = state & 0b11;
		}
	}

	// 00 01 11 10 -> forward
	static uint8_t last_enc_y_state = 255;
	if (GPIO_Pin == ENC_Y1_Pin || GPIO_Pin == ENC_Y2_Pin) {
		GPIO_PinState y1 = HAL_GPIO_ReadPin(ENC_Y1_GPIO_Port, ENC_Y1_Pin);
		GPIO_PinState y2 = HAL_GPIO_ReadPin(ENC_Y2_GPIO_Port, ENC_Y2_Pin);
		uint8_t state = y1 << 1 | y2;
		if (last_enc_y_state == 255) {
			// not yet initialized, saving last state and not doing anything
			last_enc_y_state = state;
		} else {
			//log_info("last state %d, now %d", last_enc_y_state, state);
			state = last_enc_y_state << 2 | state;
			switch (state) {
				case 0b0001:
				case 0b0111:
				case 0b1110:
				case 0b1000:
					head_position[1] += 1;
					break;
				case 0b1011:
				case 0b1101:
				case 0b0100:
				case 0b0010:
					head_position[1] -= 1;
					break;
				default: {
					if (!block_interrupt_errors) {
						log_err("invalid Y encoder change, packed state is %d", state);
						transition(FAILED);
					}
				}
			}
			last_enc_y_state = state & 0b11;
		}
	}

	if (GPIO_Pin == SAFE_T_Pin) {
		head_position[1] = -1;
	}
	if (GPIO_Pin == SAFE_B_Pin) {
		head_position[1] = 4*1000 - 1;
	}
	if (GPIO_Pin == SAFE_L_Pin) {
		head_position[0] = -1;
	}
	if (GPIO_Pin == SAFE_R_Pin) {
		head_position[0] = 4*1500 - 1;
	}
}

#define WAIT_FOR_BUTTON 1

static void handle_boot(){
	/* make sure the log from previous reset is not on the same line */
	printf("\r\n");

	/* wait for the simulator to boot */
	log_info("booting simulator");

	/* verify, that we can handle this */
	log_info("verifying performance characteristics");
	const pin_count = 8;
	const int16_t pins[] = {SAFE_T_Pin, SAFE_R_Pin, SAFE_L_Pin, SAFE_B_Pin, ENC_X1_Pin, ENC_X2_Pin, ENC_Y1_Pin, ENC_Y2_Pin};
	int32_t invocation = 0;
	block_interrupt_errors = true;
	HAL_Delay(1); // wait until next tick
	uint32_t startTick = HAL_GetTick();
	while (startTick + 100 > HAL_GetTick()) {
		HAL_GPIO_EXTI_Callback(pins[invocation % 8]);
		invocation++;
	}
	head_position[0] = 100000; //reset x coord
	head_position[1] = 100000; //reset y coord
	block_interrupt_errors = false;
	log_info("managed to call interrupt handler %d times within 100ms", invocation);
	log_info("   => max controllable speed at most %d mm/s", invocation * 10 / 4);
	// if we assume, we can cross the working area in 1 second, we would need to handle at least 1500*4 interrupts/s
	// and doubling that to make sure we have enough cycles for additional computations
	log_info("   => max %d interrupts per second, required is at least %d", invocation * 10, 1500*4 * 2);
	if (invocation * 10 > 1500*4 * 2)
		log_info("   => system is safe");
	else {
		log_error("   => system is not performant enough to handle ludicrous head speeds, refusing to continue");
		transition(FAILED);
		return;
	}
	pid_vars_t vars;
	pd_vars_init(&vars, 1.1, 1.2, 100, -100);
	int pd_invocation = 0;
	volatile int32_t blackhole = 0;
	HAL_Delay(1);
	startTick = HAL_GetTick();
	while (startTick + 100 > HAL_GetTick()) {
		blackhole = pd_controller(&vars, pd_invocation);
		pd_invocation++;
	}
	log_info("managed to call pd_controller %d times within 100ms", pd_invocation);


	// technically not necessary due to the test above, but it does not hurt
	log_info("waiting for hardware initialization");
	HAL_Delay(2); // this is definitely more than 1ms

#if WAIT_FOR_BUTTON
	/* busy wait for the blue button */
	log_info("waiting for blue button...");
	while (!is_blue_button_pressed()) {};
	/* wait until it's released */
	while(is_blue_button_pressed()) {};
#endif

	/* start motor control PWM */
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	set_motor_power(0,0);

	/* transition into INIT state */
	transition(INIT);
};
static void handle_init() {
	log_info("starting location system calibration...");
	int16_t dir[] = {45, 45};
	while (dir[0] != 0 || dir[1] != 0) {
		if (head_position[0] >= 1500*4 && head_position[0] < 1500*5) {
			dir[0] = -100;
		}
		if (1500*1 < head_position[0] && head_position[0] < 1500*3) {
			dir[0] = 0;
		}
		if (head_position[1] >= 1000*4 && head_position[1] < 1000*5) {
			dir[1] = -100;
		}
		if (1000*1 < head_position[1] && head_position[1] < 1000*3) {
			dir[1] = 0;
		}
		set_motor_power(dir[0], dir[1]);

		if (state == FAILED) {
			log_err("we messed up calibration, sorry");
			return;
		}
	}
	log_info("location system calibrated, current position is [%d, %d]", head_position[0], head_position[1]);

	/* transition into WAITING state */
	transition(WAITING);
};

static int32_t targetX, targetY;

static int32_t read_positive_number() {
	char buf;
	int32_t res = 0;
	bool something = false;
	while(true) {
		_read(0, &buf, 1);
		if (48 <= buf && buf <= 58) {
			something = true;
			res *= 10;
			res += (buf - 48);
		} else if (something) {
			return res;
		}
	}
}

static void handle_waiting(){
	log_info("waiting for next punch coordinates (in format: \"<x> <y>\\n\")");
	log_info("please don't mess it up, the parsing is not really robust");

	targetX = read_positive_number();
	targetY = read_positive_number();

	log_info("new goal set to [%d, %d]", targetX, targetY);
	transition(MOVING);
};
static void handle_moving(){
	log_info("moving");
	head_goto(targetX, targetY);

	if (state != FAILED)
		transition(PUNCHING);
};
static void handle_punching(){
	log_info("punching");
	HAL_GPIO_WritePin(PUNCH_GPIO_Port, PUNCH_Pin, GPIO_PIN_SET);
	HAL_Delay(2); // at least 1ms
	HAL_GPIO_WritePin(PUNCH_GPIO_Port, PUNCH_Pin, GPIO_PIN_RESET);
	HAL_Delay(2); // at least 1ms
	while(HAL_GPIO_ReadPin(HEAD_UP_GPIO_Port, HEAD_UP_Pin) != GPIO_PIN_RESET) {};

	transition(WAITING);
};
static void handle_failed(){
	log_info("entered failed state, can't automatically recover");

	/* fancy reset log */
	_write(0, "Device will reset in 3", sizeof("Device will reset in 3"));
	HAL_Delay(500);
	_write(0,"\b2",2);
	HAL_Delay(500);
	_write(0,"\b1",2);
	HAL_Delay(500);
	_write(0, "\b\b\b\bnow! Good bye!\r\n", sizeof("\b\b\b\bnow! Good bye!\r\n"));
	NVIC_SystemReset();
};


static void handle_test() {
	head_goto(0, 1000);
	log_info("Done...");
	HAL_Delay(5000);
	head_goto(1, 1000);
	log_info("Done...");
	HAL_Delay(5000);
	head_goto(2, 999);
	log_info("Done...");
	HAL_Delay(5000);
	transition(INIT);
}

static const void (*STATE_HANDLERS[])() = {
		[BOOT] = &handle_boot,
		[INIT] = &handle_init,
		[WAITING] = &handle_waiting,
		[MOVING] = &handle_moving,
		[PUNCHING] = &handle_punching,
		[FAILED] = &handle_failed,
		[TEST] = &handle_test,
};

void handle_main_loop() {
	/* jump to appropriate handler based on current state */
	STATE_HANDLERS[state]();
}
