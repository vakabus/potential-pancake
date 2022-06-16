/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DIR_X_Pin GPIO_PIN_0
#define DIR_X_GPIO_Port GPIOC
#define DIR_Y_Pin GPIO_PIN_1
#define DIR_Y_GPIO_Port GPIOC
#define PUNCH_Pin GPIO_PIN_2
#define PUNCH_GPIO_Port GPIOC
#define ENC_X1_Pin GPIO_PIN_3
#define ENC_X1_GPIO_Port GPIOC
#define ENC_X1_EXTI_IRQn EXTI3_IRQn
#define BUTTON_BLUE_Pin GPIO_PIN_0
#define BUTTON_BLUE_GPIO_Port GPIOA
#define ENC_X2_Pin GPIO_PIN_4
#define ENC_X2_GPIO_Port GPIOC
#define ENC_X2_EXTI_IRQn EXTI4_IRQn
#define ENC_Y1_Pin GPIO_PIN_5
#define ENC_Y1_GPIO_Port GPIOC
#define ENC_Y1_EXTI_IRQn EXTI9_5_IRQn
#define LED_GREEN_Pin GPIO_PIN_12
#define LED_GREEN_GPIO_Port GPIOD
#define LED_ORANGE_Pin GPIO_PIN_13
#define LED_ORANGE_GPIO_Port GPIOD
#define LED_RED_Pin GPIO_PIN_14
#define LED_RED_GPIO_Port GPIOD
#define LED_BLUE_Pin GPIO_PIN_15
#define LED_BLUE_GPIO_Port GPIOD
#define ENC_Y2_Pin GPIO_PIN_6
#define ENC_Y2_GPIO_Port GPIOC
#define ENC_Y2_EXTI_IRQn EXTI9_5_IRQn
#define SAFE_L_Pin GPIO_PIN_7
#define SAFE_L_GPIO_Port GPIOC
#define SAFE_L_EXTI_IRQn EXTI9_5_IRQn
#define SAFE_R_Pin GPIO_PIN_8
#define SAFE_R_GPIO_Port GPIOC
#define SAFE_R_EXTI_IRQn EXTI9_5_IRQn
#define SAFE_T_Pin GPIO_PIN_9
#define SAFE_T_GPIO_Port GPIOC
#define SAFE_T_EXTI_IRQn EXTI9_5_IRQn
#define SAFE_B_Pin GPIO_PIN_10
#define SAFE_B_GPIO_Port GPIOC
#define SAFE_B_EXTI_IRQn EXTI15_10_IRQn
#define HEAD_UP_Pin GPIO_PIN_11
#define HEAD_UP_GPIO_Port GPIOC
#define FAIL_Pin GPIO_PIN_12
#define FAIL_GPIO_Port GPIOC
#define FAIL_EXTI_IRQn EXTI15_10_IRQn
#define PWM_Y_Pin GPIO_PIN_6
#define PWM_Y_GPIO_Port GPIOB
#define PWM_X_Pin GPIO_PIN_7
#define PWM_X_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
