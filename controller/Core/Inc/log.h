#ifndef INC_LOG_H_
#define INC_LOG_H_

#include "stm32f4xx_hal.h"
#include <stdio.h>

#define log_info(format,...) printf("[info] " format "\r\n", ##__VA_ARGS__)
#define log_debug(format,...) printf("[debu] " format "\r\n", ##__VA_ARGS__ )
#define log_warn(format,...) printf("[warn] " format "\r\n", ##__VA_ARGS__ )
#define log_err(format,...) printf("[erro] " format "\r\n", ##__VA_ARGS__ )
#define log_error(...) log_err(__VA_ARGS__)

#endif /* INC_LOG_H_ */
