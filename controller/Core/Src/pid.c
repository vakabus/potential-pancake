#include "pid.h"

void pid_vars_init(pid_vars_t* vars, double Kp, double Ki, double Kd, double output_max, double output_min) {
	vars->Kd = Kd;
	vars->Ki = Ki;
	vars->Kp = Kp;

	for (int i = 0; i < PID_VARS_ERROR_ARR_SIZE; i++) vars->errors[i] = 0;
	vars->errors_index = 0;
	vars->integral_sum = 0;

	vars->output_max = output_max;
	vars->output_min = output_min;

	vars->dt = 1;
}

double pid(pid_vars_t* vars, double e) {
	/* update the integral array */
	double prev_err = vars->errors[vars->errors_index];
	vars->integral_sum += e * (vars->dt);
	vars->integral_sum -= prev_err * (vars->dt);
	vars->errors_index = (vars->errors_index + 1) % PID_VARS_ERROR_ARR_SIZE;
	vars->errors[vars->errors_index] = e;

	double output = (vars->Kp)*e + (vars->Ki)*(vars->integral_sum) + (vars->Kd)* ((e - prev_err)/(vars->dt));

	/* limit output within output_min and output_max */
	if (output>(vars->output_max))
	output = vars->output_max;
	else if (output<(vars->output_min))
	output = vars->output_min;

	return output;
}
