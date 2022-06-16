#include "usart.h"
#include "stm32f4xx_hal.h"
#include <limits.h>

int _read(int file, char *ptr, int len) {
	HAL_UART_Receive(&huart2, (uint8_t*)ptr, len, INT_MAX);
	return len;
}

int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, 100);
	return len;
}


