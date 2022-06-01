#include "controller.h"
#include "main.h"
#include "log.h"
#include "pid.h"
#include "tim.h"

#include <stdbool.h>

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
static double head_position[] = {0, 0};


static void transition(main_state_t new) {
	log_info("transitioning to state %s", STATE_NAMES[new]);

	/* finally, change the state */
	state = new;
}

static inline bool is_blue_button_pressed() {
	GPIO_PinState state = HAL_GPIO_ReadPin(BUTTON_BLUE_GPIO_Port, BUTTON_BLUE_Pin);
	return state == 1;
}

static void configure_motor(axis_t axis, uint16_t power, bool forward) {
	static const uint32_t AXIS_CHANNEL_TABLE[] = {
			[X] = TIM_CHANNEL_2,
			[Y] = TIM_CHANNEL_1
	};

	if (power < 0 || power > 100) {
		log_error("dutyCycle %d out of valid range 0-100", power);
		return;
	}

	HAL_StatusTypeDef res;

	/* Reconfigure PWM */
	/* Help can be found here: https://simonmartin.ch/resources/stm32/dl/STM32%20Tutorial%2002%20-%20PWM%20Generation%20using%20HAL%20(and%20FreeRTOS).pdf */

	res = HAL_TIM_PWM_Stop(&htim4, AXIS_CHANNEL_TABLE[axis]); // stop generation of pwm
	if (res != HAL_OK) log_error("failed PWM stop");
	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = forward ? power : (100 - power);
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	res = HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, AXIS_CHANNEL_TABLE[axis]);
	if (res != HAL_OK) log_error("failed PWM ConfigChannel");
	res = HAL_TIM_PWM_Start(&htim4, AXIS_CHANNEL_TABLE[axis]); // start pwm generation
	if (res != HAL_OK) log_error("failed HAL PWM start");

	/* reconfigure direction */
	if (axis == X)
		HAL_GPIO_WritePin(DIR_X_GPIO_Port, DIR_X_Pin, forward);
	else
		HAL_GPIO_WritePin(DIR_Y_GPIO_Port, DIR_Y_Pin, forward);
}

static void move(int16_t dir[2]) {
	bool xForward = dir[0] > 0;
	bool yForward = dir[1] > 0;
	configure_motor(X, abs(dir[0]), xForward);
	configure_motor(Y, abs(dir[1]), yForward);
}


static void handle_boot(){
	/* make sure the log from previous reset is not on the same line */
	printf("\r\n");

	/* wait for the simulator to boot */
	log_info("booting simulator");
	HAL_Delay(10); // FIXME: is this really >1 millisecond?

#if WAIT_FOR_BUTTON
	/* busy wait for the blue button */
	log_info("waiting for blue button...");
	while (!is_blue_button_pressed()) {};
	/* wait until it's released */
	while(is_blue_button_pressed()) {};
#endif

	/* transition into INIT state */
	transition(TEST);
};
static void handle_init(){

};
static void handle_waiting(){};
static void handle_moving(){};
static void handle_punching(){};
static void handle_failed(){
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
	static int direction = 0;
	static bool first_time = true;
	if (first_time) {
		first_time = true;
		//timer_enable_wtf();
		//HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
		//HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

		configure_motor(X, 0, true);
		configure_motor(Y, 0, true);
		//int16_t dir[] = {100,100};
		//move(dir);
	}

	configure_motor(Y, 50, true);
	configure_motor(X, 50, true);
	HAL_Delay(500);
	configure_motor(Y, 50, false);
	configure_motor(X, 50, false);
	HAL_Delay(500);

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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == FAIL_Pin) {
		log_error("simulator triggered device failure");
		transition(FAILED);
	}
}
