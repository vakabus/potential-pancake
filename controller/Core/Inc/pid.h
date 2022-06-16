#ifndef INC_PID_H_
#define INC_PID_H_

#include "stm32f4xx_hal.h"

#define PID_VARS_ERROR_ARR_SIZE 10

typedef struct {
	/* parameters */
	float Kp;
	float Kd;

	/* max output limits for the PID controller */
	int32_t output_max;
	int32_t output_min;

	/* below are session variables for the PID controller */
	int32_t last_e;
	int32_t last_last_e;
	int32_t	last_filtered_e;
	int32_t last_output;
	uint32_t last_timestamp;
} pid_vars_t;

void pd_vars_init(pid_vars_t* vars, float Kp, float Kd, int32_t output_max, int32_t output_min);
int32_t pd_controller(pid_vars_t* vars, int32_t e);

#endif /* INC_PID_H_ */
