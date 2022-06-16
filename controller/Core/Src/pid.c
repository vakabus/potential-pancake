#include "pid.h"
#include "stm32f4xx_hal.h"
#include "log.h"

void pd_vars_init(pid_vars_t* vars, float Kp, float Kd, int32_t output_max, int32_t output_min) {
	vars->Kd = Kd;
	vars->Kp = Kp;

	vars->output_max = output_max;
	vars->output_min = output_min;

	vars->last_e = 0;
	vars->last_last_e = 0;
	vars->last_output = 0;
	vars->last_filtered_e = 0;
	vars->last_timestamp = HAL_GetTick();
}

int32_t pd_controller(pid_vars_t* vars, int32_t e) {
	uint32_t timestamp = HAL_GetTick();

	/* we refuse to run more often than once every tick due to division by zero */
	if (vars->last_timestamp == timestamp) return vars->last_output;

	/* calculate dt */
	uint32_t dt = timestamp - vars->last_timestamp;
	vars->last_timestamp = timestamp;

	/* calculate PD controller */
	int32_t filtered_e = (e + vars->last_e + vars->last_last_e) / 3;
	float derivative = (((float)(filtered_e - vars->last_filtered_e)) / (float)dt);
	float o = (vars->Kp)*e + (vars->Kd) * derivative;
	o = o + ((o > 0) - (o < 0))*10; // make sure that we can overcome static friction
	int32_t output = round(o);

	/* limit output within output_min and output_max */
	if (output>(vars->output_max))
	output = vars->output_max;
	else if (output<(vars->output_min))
	output = vars->output_min;

	vars->last_output = output;
	vars->last_last_e = vars->last_e;
	vars->last_e = e;
	vars->last_filtered_e = filtered_e;
	return output;
}
