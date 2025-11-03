/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_gpio.h"

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
#define OEN_GRV_R_Pin LL_GPIO_PIN_14
#define OEN_GRV_R_GPIO_Port GPIOC
#define OEN_GRV_B_Pin LL_GPIO_PIN_15
#define OEN_GRV_B_GPIO_Port GPIOC
#define VIN_DET_Pin LL_GPIO_PIN_0
#define VIN_DET_GPIO_Port GPIOA
#define nVA_EN_Pin LL_GPIO_PIN_1
#define nVA_EN_GPIO_Port GPIOA
#define PMU_SW_Pin LL_GPIO_PIN_2
#define PMU_SW_GPIO_Port GPIOA
#define MPWR_EN_Pin LL_GPIO_PIN_3
#define MPWR_EN_GPIO_Port GPIOA
#define VUSB_DET_Pin LL_GPIO_PIN_4
#define VUSB_DET_GPIO_Port GPIOA
#define xGPIO0_BOOT_Pin LL_GPIO_PIN_5
#define xGPIO0_BOOT_GPIO_Port GPIOA
#define CHG_STAT_Pin LL_GPIO_PIN_6
#define CHG_STAT_GPIO_Port GPIOA
#define CHG_STAT_EXTI_IRQn EXTI4_15_IRQn
#define LED_DAT_Pin LL_GPIO_PIN_7
#define LED_DAT_GPIO_Port GPIOA
#define CHG_EN_Pin LL_GPIO_PIN_0
#define CHG_EN_GPIO_Port GPIOB
#define OEN_PWROUT_Pin LL_GPIO_PIN_1
#define OEN_PWROUT_GPIO_Port GPIOB
#define PDCDC_REFLOW_Pin LL_GPIO_PIN_8
#define PDCDC_REFLOW_GPIO_Port GPIOA
#define PDCDC_EN_Pin LL_GPIO_PIN_6
#define PDCDC_EN_GPIO_Port GPIOC
#define USB_CON_LV2_Pin LL_GPIO_PIN_15
#define USB_CON_LV2_GPIO_Port GPIOA
#define USB_CON_LV1_Pin LL_GPIO_PIN_3
#define USB_CON_LV1_GPIO_Port GPIOB
#define nLED_EN_Pin LL_GPIO_PIN_4
#define nLED_EN_GPIO_Port GPIOB
#define nSTBY_WKUP_Pin LL_GPIO_PIN_5
#define nSTBY_WKUP_GPIO_Port GPIOB
#define OEN_USB_Pin LL_GPIO_PIN_8
#define OEN_USB_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
