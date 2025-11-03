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
#include <string.h>
#include "rx8130ce.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
  uint8_t main_power_status;
  uint8_t led_power_status;
  uint8_t usb_power_status;
  uint8_t grove_r_power_status;
  uint8_t grove_b_power_status;
  uint8_t pdcdc_power_status;
  uint8_t pdcdc_output_status;
  uint8_t pd_power_direct;
  uint8_t vameter_power_status;
  uint8_t charge_power_status;
  uint8_t charge_status;
} power_status_t;

typedef struct
{
  uint16_t output_voltage;
  uint8_t current_limit_value;
  uint16_t iout_limit;
  uint8_t power_output_control;
} sc8721_config_t;

typedef enum
{
  CHARGE_NO          = 0x00U,
  CHARGE_START       = 0x01U,
  CHARGE_ERROR       = 0x02U,
} charge_status_t;

typedef struct
{
  uint32_t led_color[8];
  uint32_t led_brightness[8];
} led_config_t;

typedef struct
{
  uint8_t nstby_wkup;
  uint8_t vin_det;
  uint8_t vusb_det;
} wakeup_source_status_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern power_status_t sys_power_status;
extern volatile sc8721_config_t sc8721_config;
extern alarm_time_t alarm_time;
extern volatile led_config_t led_config;
extern wakeup_source_status_t wakeup_source_status;
extern uint8_t i2c_address[1];
extern volatile uint8_t charge_start_flag;
extern volatile uint8_t charge_stop_flag;
extern volatile uint32_t charge_pulse_time;
extern volatile uint8_t charge_status;
extern volatile uint8_t led_update_flag;
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
