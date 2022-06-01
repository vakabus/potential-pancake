#ifndef INC_PID_H_
#define INC_PID_H_

#define PID_VARS_ERROR_ARR_SIZE 10

typedef struct {
	/* parameters */
	double Kp;
	double Ki;
	double Kd;

	/* max output limits for the PID controller */
	double output_max;
	double output_min;

	/* below are session variables for the PID controller */
	double errors[PID_VARS_ERROR_ARR_SIZE];
	int errors_index;
	double integral_sum;
	double dt;
} pid_vars_t;

void pid_vars_init(pid_vars_t* vars, double Kp, double Ki, double Kd, double output_max, double output_min);
double pid(pid_vars_t* vars, double e);

#endif /* INC_PID_H_ */
