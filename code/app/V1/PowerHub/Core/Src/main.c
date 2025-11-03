/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "power_manager.h"
#include "ws2812.h"
#include "button.h"
#include "sc8721.h"
#include "ina226.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_ADDRESS 0x50
#define FIRMWARE_VERSION 1
#define APPLICATION_ADDRESS     ((uint32_t)0x08001800) 
#define BOOTLOADER_ADDRESS     ((uint32_t)0x08000000) 
#define BOOTLOADER_VER_ADDR ((uint32_t)0x08001800 - 1)
#define FLASH_DATA_SIZE 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t bootloader_version;
typedef  void (*pFunction)(void);
volatile uint32_t JumpAddress;
volatile pFunction JumpToApplication;
volatile uint8_t iap_jump_flag;
uint8_t i2c_address[1] = {0};
uint8_t flash_data[FLASH_DATA_SIZE] = {0};
volatile uint32_t i2c_stop_timeout_delay = 0;
power_status_t sys_power_status;
volatile sc8721_config_t sc8721_config;
wakeup_source_status_t wakeup_source_status;
uint8_t sc8721_reg_data[10] = {0};
volatile uint8_t get_sc8721_flag = 0;
volatile led_config_t led_config;
volatile uint8_t sc8721_update_flag = 0;
static volatile uint8_t sc8721_delay_flag = 0;
static volatile uint32_t sc8721_update_delay = 0;
volatile uint8_t led_update_flag = 0;
volatile uint8_t alarm_update_flag = 0;
volatile uint8_t vameter_update_flag = 0;
volatile uint8_t usb_mode = 0;
volatile uint8_t charge_start_flag = 0;
volatile uint8_t charge_stop_flag = 0;
volatile uint32_t charge_pulse_time = 0;
volatile uint8_t charge_status = 0;
volatile uint8_t fm_version = FIRMWARE_VERSION;
volatile uint8_t is_flash_write_back = 0;
volatile uint8_t is_power_off_flag = 0;
volatile uint8_t is_reset_flag = 0;
volatile uint8_t is_download_flag = 0;
INA226 ina226_bat;
INA226 ina226_pwrcan;
INA226 ina226_pwr485;
INA226 ina226_usb;
INA226 ina226_grove_r;
INA226 ina226_grove_b;
rtc_time_t rtc_time;
alarm_time_t alarm_time;
volatile uint32_t rtc_update_delay = 0;

//static uint32_t test_delay = 0;
//static uint32_t test_voltage = 3000;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void init_flash_data(void) 
{   
  uint32_t flash_write_timeout = 0;

  if (!(readPackedMessageFromFlash(flash_data, FLASH_DATA_SIZE))) {
    i2c_address[0] = I2C_ADDRESS;
    flash_data[0] = i2c_address[0];
    wakeup_source_status.nstby_wkup = 1;
    wakeup_source_status.vin_det = 1;
    wakeup_source_status.vusb_det = 1;
    flash_data[1] = wakeup_source_status.nstby_wkup;
    flash_data[2] = wakeup_source_status.vin_det;
    flash_data[3] = wakeup_source_status.vusb_det;

    // retry 20 times, if still fail, reset system
    while(!writeMessageToFlash(flash_data , FLASH_DATA_SIZE)) {
      flash_write_timeout++;
      if (flash_write_timeout > 20) {
        HAL_NVIC_SystemReset();
      }
    }
  } else {
    i2c_address[0] = flash_data[0];
    wakeup_source_status.nstby_wkup = flash_data[1];
    wakeup_source_status.vin_det = flash_data[2];
    wakeup_source_status.vusb_det = flash_data[3];
  }
}

uint8_t flash_data_write_back(void)
{
  uint32_t flash_write_timeout = 0;

  if (readPackedMessageFromFlash(flash_data, FLASH_DATA_SIZE)) {
    flash_data[0] = i2c_address[0];
    flash_data[1] = wakeup_source_status.nstby_wkup;
    flash_data[2] = wakeup_source_status.vin_det;
    flash_data[3] = wakeup_source_status.vusb_det;    
    // retry 20 times, if still fail, reset system
    while(!writeMessageToFlash(flash_data , FLASH_DATA_SIZE)) {
      flash_write_timeout++;
      if (flash_write_timeout > 20) {
        flash_write_timeout = 0;
        return 0;
      }
    }
    // write success, return 1
    return 1;    
  }
  else {
    return 0;
  }     
}

void Reset_AllPeriph(void)
{
  LL_I2C_DeInit(I2C1);
  LL_I2C_DisableAutoEndMode(I2C1);
  LL_I2C_Disable(I2C1);
  LL_I2C_DisableIT_ADDR(I2C1);
  SysTick->CTRL=0;
  SYSCFG->CFGR1 &= SYSCFG_CFGR1_MEM_MODE;
  NVIC->ICER[0] = 0xFFFFFFFF;
  NVIC->ICPR[0] = 0xFFFFFFFF;
  /* Set EXTICRx registers to reset value */
  EXTI->EXTICR[0] = 0;
  EXTI->EXTICR[1] = 0;
  EXTI->EXTICR[2] = 0;
  EXTI->EXTICR[3] = 0;
  /* Set CFGR2 register to reset value: clear SRAM parity error flag */
  SYSCFG->CFGR2 |= (uint32_t) SYSCFG_CFGR2_SRAM_PE; 
  LL_RCC_DeInit();   
  __HAL_SYSCFG_CLEAR_FLAG();
//	 HAL_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;  
}

void Jump_Bootloader(void)
{
  if (((*(__IO uint32_t*)BOOTLOADER_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
  {
    Reset_AllPeriph();
    SCB->VTOR = BOOTLOADER_ADDRESS;
    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (BOOTLOADER_ADDRESS +4);
    JumpToApplication = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) BOOTLOADER_ADDRESS);
    JumpToApplication();
  }
}

void Slave_Complete_Callback(uint8_t *rx_data, uint16_t len)
{
  uint8_t buf[48];
  uint8_t rx_buf[48];
  uint8_t rx_mark[48] = {0}; 

  if (len > 1) {
    if (rx_data[0] <= 0x06) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        if (rx_buf[0])
          enable_led_power();
        else
          disable_led_power();
      }
      if (rx_mark[1]) {
        if (rx_buf[1])
          enable_usb_power();
        else
          disable_usb_power();
      }
      if (rx_mark[2]) {
        if (rx_buf[2])
          enable_grove_r_power();
        else
          disable_grove_r_power();
      }
      if (rx_mark[3]) {
        if (rx_buf[3])
          enable_grove_b_power();
        else
          disable_grove_b_power();
      }
      if (rx_mark[4]) {
        if (rx_buf[4]) {
          enable_pdcdc_power();
          sc8721_update_delay = HAL_GetTick();
          sc8721_delay_flag = 1;
        }
        else {
          sc8721_delay_flag = 0;
          sc8721_update_flag = 0;
          disable_pdcdc_power();
        }
      }
      if (rx_mark[5]) {
        if (rx_buf[5]) {
          enable_vameter_power_output();
          vameter_update_flag = 1;
        }
        else {
          disable_vameter_power_output();
          vameter_update_flag = 0;
        }
      }
      if (rx_mark[6]) {
        if (rx_buf[6])
          enable_charge_power();
        else
          disable_charge_power();
      }
      
    }     
    else if (rx_data[0] == 0x10) {
      usb_mode = rx_data[1];
      switch (usb_mode)
      {
      case 0:
        LL_GPIO_ResetOutputPin(USB_CON_LV1_GPIO_Port, USB_CON_LV1_Pin);
        LL_GPIO_ResetOutputPin(USB_CON_LV2_GPIO_Port, USB_CON_LV2_Pin);
        break;
      case 1:
        LL_GPIO_SetOutputPin(USB_CON_LV1_GPIO_Port, USB_CON_LV1_Pin);
        LL_GPIO_ResetOutputPin(USB_CON_LV2_GPIO_Port, USB_CON_LV2_Pin);
        break;
      case 2:
        LL_GPIO_SetOutputPin(USB_CON_LV1_GPIO_Port, USB_CON_LV1_Pin);
        LL_GPIO_SetOutputPin(USB_CON_LV2_GPIO_Port, USB_CON_LV2_Pin);
        break;
      
      default:
        break;
      }
    }     
    else if ((rx_data[0] >= 0x20) && (rx_data[0] <= 0x24)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0x20+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        sc8721_config.output_voltage &= 0xff00;
        sc8721_config.output_voltage |= rx_buf[0];
        sc8721_update_flag = 1;
      }
      if (rx_mark[1]) {
        sc8721_config.output_voltage &= 0x00ff;
        sc8721_config.output_voltage |= (rx_buf[1] << 8);
        sc8721_update_flag = 1;
      }
      if (rx_mark[2]) {
        if (rx_buf[2] <= 232) {
          sc8721_config.current_limit_value = rx_buf[2];
          sc8721_config.iout_limit = (sc8721_config.current_limit_value + 1) * 8 / 0.62f;
          sc8721_update_flag = 1;
        }
      }
      if (rx_mark[3]) {
        sc8721_config.power_output_control = rx_buf[3];
        sc8721_set_output_switch_phy();
        sc8721_update_flag = 1;
        
      }
      if (rx_mark[4]) {
        if (rx_buf[4]) {
          set_pd_power_output();
        }
        else {
          set_pd_power_input();
        }
        
      }
    }     
    else if ((rx_data[0] >= 0x60) && (rx_data[0] <= 0x7F)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0x60+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      led_buffer_peek_latest(&led_config.led_color[0]);

      if (rx_mark[0]) {
        led_config.led_color[0] &= 0xffffff00;
        led_config.led_color[0] |= rx_buf[0];
        
      }
      if (rx_mark[1]) {
        led_config.led_color[0] &= 0xffff00ff;
        led_config.led_color[0] |= (rx_buf[1] << 8);
        
      }
      if (rx_mark[2]) {
        led_config.led_color[0] &= 0xff00ffff;
        led_config.led_color[0] |= (rx_buf[2] << 16);
        
      }
      if (rx_mark[3]) {
        led_config.led_color[0] &= 0x00ffffff;
        led_config.led_color[0] |= (rx_buf[3] << 24);
        
      }

      if (rx_mark[4]) {
        led_config.led_color[1] &= 0xffffff00;
        led_config.led_color[1] |= rx_buf[4];
        
      }
      if (rx_mark[5]) {
        led_config.led_color[1] &= 0xffff00ff;
        led_config.led_color[1] |= (rx_buf[5] << 8);
        
      }
      if (rx_mark[6]) {
        led_config.led_color[1] &= 0xff00ffff;
        led_config.led_color[1] |= (rx_buf[6] << 16);
        
      }
      if (rx_mark[7]) {
        led_config.led_color[1] &= 0x00ffffff;
        led_config.led_color[1] |= (rx_buf[7] << 24);
        
      }

      if (rx_mark[8]) {
        led_config.led_color[2] &= 0xffffff00;
        led_config.led_color[2] |= rx_buf[8];
        
      }
      if (rx_mark[9]) {
        led_config.led_color[2] &= 0xffff00ff;
        led_config.led_color[2] |= (rx_buf[9] << 8);
        
      }
      if (rx_mark[10]) {
        led_config.led_color[2] &= 0xff00ffff;
        led_config.led_color[2] |= (rx_buf[10] << 16);
        
      }
      if (rx_mark[11]) {
        led_config.led_color[2] &= 0x00ffffff;
        led_config.led_color[2] |= (rx_buf[11] << 24);
        
      }

      if (rx_mark[12]) {
        led_config.led_color[3] &= 0xffffff00;
        led_config.led_color[3] |= rx_buf[12];
        
      }
      if (rx_mark[13]) {
        led_config.led_color[3] &= 0xffff00ff;
        led_config.led_color[3] |= (rx_buf[13] << 8);
        
      }
      if (rx_mark[14]) {
        led_config.led_color[3] &= 0xff00ffff;
        led_config.led_color[3] |= (rx_buf[14] << 16);
        
      }
      if (rx_mark[15]) {
        led_config.led_color[3] &= 0x00ffffff;
        led_config.led_color[3] |= (rx_buf[15] << 24);
        
      }

      if (rx_mark[16]) {
        led_config.led_color[4] &= 0xffffff00;
        led_config.led_color[4] |= rx_buf[16];
        
      }
      if (rx_mark[17]) {
        led_config.led_color[4] &= 0xffff00ff;
        led_config.led_color[4] |= (rx_buf[17] << 8);
        
      }
      if (rx_mark[18]) {
        led_config.led_color[4] &= 0xff00ffff;
        led_config.led_color[4] |= (rx_buf[18] << 16);
        
      }
      if (rx_mark[19]) {
        led_config.led_color[4] &= 0x00ffffff;
        led_config.led_color[4] |= (rx_buf[19] << 24);
        
      }

      if (rx_mark[20]) {
        led_config.led_color[5] &= 0xffffff00;
        led_config.led_color[5] |= rx_buf[20];
        
      }
      if (rx_mark[21]) {
        led_config.led_color[5] &= 0xffff00ff;
        led_config.led_color[5] |= (rx_buf[21] << 8);
        
      }
      if (rx_mark[22]) {
        led_config.led_color[5] &= 0xff00ffff;
        led_config.led_color[5] |= (rx_buf[22] << 16);
        
      }
      if (rx_mark[23]) {
        led_config.led_color[5] &= 0x00ffffff;
        led_config.led_color[5] |= (rx_buf[23] << 24);
        
      }

      if (rx_mark[24]) {
        led_config.led_color[6] &= 0xffffff00;
        led_config.led_color[6] |= rx_buf[24];
        
      }
      if (rx_mark[25]) {
        led_config.led_color[6] &= 0xffff00ff;
        led_config.led_color[6] |= (rx_buf[25] << 8);
        
      }
      if (rx_mark[26]) {
        led_config.led_color[6] &= 0xff00ffff;
        led_config.led_color[6] |= (rx_buf[26] << 16);
        
      }
      if (rx_mark[27]) {
        led_config.led_color[6] &= 0x00ffffff;
        led_config.led_color[6] |= (rx_buf[27] << 24);
        
      }

      if (rx_mark[28]) {
        led_config.led_color[7] &= 0xffffff00;
        led_config.led_color[7] |= rx_buf[28];
        
      }
      if (rx_mark[29]) {
        led_config.led_color[7] &= 0xffff00ff;
        led_config.led_color[7] |= (rx_buf[29] << 8);
        
      }
      if (rx_mark[30]) {
        led_config.led_color[7] &= 0xff00ffff;
        led_config.led_color[7] |= (rx_buf[30] << 16);
        
      }
      if (rx_mark[31]) {
        led_config.led_color[7] &= 0x00ffffff;
        led_config.led_color[7] |= (rx_buf[31] << 24);
        
      }

      led_buffer_push(&led_config.led_color[0]);
    }     
    else if ((rx_data[0] >= 0x80) && (rx_data[0] <= 0x87)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0x80+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      for (int i = 0; i < 8; i++) {
        if (rx_mark[i]) {
          led_config.led_brightness[i] = rx_buf[i];
          led_update_flag = 1;
        }
      }
    }     
    else if ((rx_data[0] >= 0xB0) && (rx_data[0] <= 0xB2)) {
      uint8_t is_data_change = 0;

      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0xB0+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        if (rx_buf[0] != wakeup_source_status.nstby_wkup) {
          is_data_change = 1;
          wakeup_source_status.nstby_wkup = rx_buf[0];
        }
      }
      if (rx_mark[1]) {
        if (rx_buf[1] != wakeup_source_status.vin_det) {
          is_data_change = 1;
          wakeup_source_status.vin_det = rx_buf[1];
        }
      }
      if (rx_mark[2]) {
        if (rx_buf[2] != wakeup_source_status.vusb_det) {
          is_data_change = 1;
          wakeup_source_status.vusb_det = rx_buf[2];
        }
      }
      if (is_data_change)
        is_flash_write_back = 1;
    }     
    else if ((rx_data[0] >= 0xC0) && (rx_data[0] <= 0xC6)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0xC0+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        if (rx_buf[0] < 60)
          rtc_time.tm_sec = rx_buf[0];
      }
      if (rx_mark[1]) {
        if (rx_buf[1] < 60)
          rtc_time.tm_min = rx_buf[1];
      }
      if (rx_mark[2]) {
        if (rx_buf[2] < 24)
          rtc_time.tm_hour = rx_buf[2];
      }
      if (rx_mark[3]) {
        if (rx_buf[3] < 32)
          rtc_time.tm_mday = rx_buf[3];
      }
      if (rx_mark[4]) {
        if (rx_buf[4] < 13)
          rtc_time.tm_mon = rx_buf[4];
      }
      if (rx_mark[5]) {
        rtc_time.tm_year = rx_buf[5];
      }
      if (rx_mark[6]) {
        rtc_time.tm_wday = rx_buf[6];
      }
      rx8130ce_set_time(rtc_time);
    }     
    else if ((rx_data[0] >= 0xD0) && (rx_data[0] <= 0xD3)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0xD0+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        if (rx_buf[0] < 60)
          alarm_time.tm_min = rx_buf[0];
      }
      if (rx_mark[1]) {
        if (rx_buf[1] < 60)
          alarm_time.tm_hour = rx_buf[1];
      }
      if (rx_mark[2]) {
        if (rx_buf[2] < 31)
          alarm_time.tm_mday = rx_buf[2];
      }
      if (rx_mark[3]) {
        if (rx_buf[3]) {
          alarm_time.is_alarm_enable = 1;
        }
        else {
          alarm_time.is_alarm_enable = 0;
        }
      }
      alarm_update_flag = 1;
    }     
    else if ((rx_data[0] >= 0xE0) && (rx_data[0] <= 0xE2)) {
      for(int i = 0; i < len - 1; i++) {
        uint8_t rx_index = rx_data[0]-0xE0+i;
        
        rx_buf[rx_index] = rx_data[1+i];
        rx_mark[rx_index] = 1;
      }

      if (rx_mark[0]) {
        if (rx_buf[0]) {
          is_power_off_flag = 1;
        }
      }
      if (rx_mark[1]) {
        if (rx_buf[1]) {
          is_reset_flag = 1;
        }
      }
      if (rx_mark[2]) {
        if (rx_buf[2]) {
          is_download_flag = 1;
        }
      }
    }         
    else if (rx_data[0] == 0x90) {
      if (rx_data[1]) {
        get_sc8721_flag = 1;
      }
    }        
    else if (rx_data[0] == 0xFD) {
      if (rx_data[1] == 1) {
        iap_jump_flag = 1;
      }
    }        
  }
  else if (len == 1) {
    if (rx_data[0] <= 0x06) {
      buf[0] = sys_power_status.led_power_status;
      buf[1] = sys_power_status.usb_power_status;
      buf[2] = sys_power_status.grove_r_power_status;
      buf[3] = sys_power_status.grove_b_power_status;
      buf[4] = sys_power_status.pdcdc_power_status;
      buf[5] = sys_power_status.vameter_power_status;
      buf[6] = sys_power_status.charge_power_status;
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x00], 0x06-rx_data[0]+1);
    }
    else if (rx_data[0] == 0x10) {
      i2c1_set_send_data((uint8_t *)&usb_mode, 1);
    }     
    else if ((rx_data[0] >= 0x20) && (rx_data[0] <= 0x24)) {
      memcpy(&buf[0], (uint8_t *)&sc8721_config.output_voltage, 2);
      buf[2] = sc8721_config.current_limit_value;
      buf[3] = sc8721_config.power_output_control;
      buf[4] = sys_power_status.pd_power_direct;
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x20], 0x24-rx_data[0]+1);
    }        
    else if ((rx_data[0] >= 0x30) && (rx_data[0] <= 0x47)) {
      uint16_t voltage_temp, current_temp;
      voltage_temp = ina226_bat.Result.BusVoltage_uV / 1000;
      current_temp = ina226_bat.Result.Current_uA / 1000;
      memcpy(&buf[0], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[2], (uint8_t *)&current_temp, 2);
      voltage_temp = ina226_pwrcan.Result.BusVoltage_uV / 1000;
      current_temp = ina226_pwrcan.Result.Current_uA / 1000;
      memcpy(&buf[4], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[6], (uint8_t *)&current_temp, 2);
      voltage_temp = ina226_pwr485.Result.BusVoltage_uV / 1000;
      current_temp = ina226_pwr485.Result.Current_uA / 1000;
      memcpy(&buf[8], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[10], (uint8_t *)&current_temp, 2);
      voltage_temp = ina226_usb.Result.BusVoltage_uV / 1000;
      current_temp = ina226_usb.Result.Current_uA / 1000;
      memcpy(&buf[12], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[14], (uint8_t *)&current_temp, 2);
      voltage_temp = ina226_grove_r.Result.BusVoltage_uV / 1000;
      current_temp = ina226_grove_r.Result.Current_uA / 1000;
      memcpy(&buf[16], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[18], (uint8_t *)&current_temp, 2);
      voltage_temp = ina226_grove_b.Result.BusVoltage_uV / 1000;
      current_temp = ina226_grove_b.Result.Current_uA / 1000;
      memcpy(&buf[20], (uint8_t *)&voltage_temp, 2);
      memcpy(&buf[22], (uint8_t *)&current_temp, 2);

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x30], 0x47-rx_data[0]+1);
    }        
    else if (rx_data[0] >= 0x50 && rx_data[0] <= 0x51) {
      buf[0] = charge_status;
      buf[1] = LL_GPIO_IsInputPinSet(VIN_DET_GPIO_Port, VIN_DET_Pin);
      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x50], 0x51-rx_data[0]+1);
    }        
    else if ((rx_data[0] >= 0x60) && (rx_data[0] <= 0x7F)) {
      memcpy(&buf[0], (uint8_t *)&led_config.led_color[0], 32);

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x60], 0x7F-rx_data[0]+1);
    }  
    else if ((rx_data[0] >= 0x80) && (rx_data[0] <= 0x87)) {
      for (int i = 0; i < 8; i++) {
        buf[i] = led_config.led_brightness[i];
      }

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x80], 0x87-rx_data[0]+1);
    }  
    else if ((rx_data[0] >= 0x90) && (rx_data[0] <= 0x9A)) {
      buf[0] = get_sc8721_flag;
      for (int i = 0; i < 10; i++) {
        buf[i+1] = sc8721_reg_data[i];
      }

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0x90], 0x9A-rx_data[0]+1);
    }  
    else if ((rx_data[0] >= 0xB0) && (rx_data[0] <= 0xB2)) {
      buf[0] = wakeup_source_status.nstby_wkup;
      buf[1] = wakeup_source_status.vin_det;
      buf[2] = wakeup_source_status.vusb_det;

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0xB0], 0xB2-rx_data[0]+1);
    }  
    else if (rx_data[0] == 0xFF) 
    {
      i2c1_set_send_data(i2c_address, 1);
    }    
    else if (rx_data[0] == 0xFE) 
    {
      i2c1_set_send_data((uint8_t *)&fm_version, 1);
    }  
    else if (rx_data[0] == 0xA0)
    {
      uint8_t button_status = 0;
      button_status = (!!(GPIOA->IDR & PMU_SW_Pin));
      i2c1_set_send_data((uint8_t *)&button_status, 1);
    }  
    else if ((rx_data[0] >= 0xC0) && (rx_data[0] <= 0xC6)) {
      buf[0] = rtc_time.tm_sec;
      buf[1] = rtc_time.tm_min;
      buf[2] = rtc_time.tm_hour;
      buf[3] = rtc_time.tm_mday;
      buf[4] = rtc_time.tm_mon;
      buf[5] = rtc_time.tm_year;
      buf[6] = rtc_time.tm_wday;

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0xC0], 0xC6-rx_data[0]+1);
    }     
    else if ((rx_data[0] >= 0xD0) && (rx_data[0] <= 0xD4)) {
      buf[0] = alarm_time.tm_min;
      buf[1] = alarm_time.tm_hour;
      buf[2] = alarm_time.tm_mday;
      buf[3] = alarm_time.is_alarm_enable;
      buf[4] = alarm_update_flag;

      i2c1_set_send_data((uint8_t *)&buf[rx_data[0]-0xD0], 0xD4-rx_data[0]+1);
    }
    else if (rx_data[0] == 0xFC)
    {
      bootloader_version = *(uint8_t*)BOOTLOADER_VER_ADDR;
      i2c1_set_send_data((uint8_t *)&bootloader_version, 1);
    }          
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint32_t flash_writeback_timeout = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  if (HAL_Init() != HAL_OK) {
    HAL_NVIC_SystemReset();
  }

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  iwdg_ob_program();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
//  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  HAL_IWDG_Refresh(&hiwdg);
  clear_pwr_flag();
  sk6812_init(PIXEL_MAX);
  led_buffer_init();
  main_power_on();
  enable_charge_power();
  get_sc8721_flag = 1;
  init_flash_data();
  rx8130ce_init();
  user_i2c_init();
  i2c1_it_enable();
  HAL_IWDG_Refresh(&hiwdg);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_IWDG_Refresh(&hiwdg);
    // judge_button_1();
    handle_button_2();
    
    if (charge_start_flag && !charge_stop_flag && (HAL_GetTick() - charge_pulse_time > 2000)) {
      if (!(!!(GPIOA->IDR & CHG_STAT_Pin))) {
        charge_status = CHARGE_NO;
      }
      else {
        charge_status = CHARGE_START;
      }
      charge_start_flag = 0;
      charge_stop_flag = 0;            
    }

    led_command_t output_temp;

    if (led_buffer_pop(&output_temp)) {
      for(int i = 0; i < 8; i++) {
        neopixel_set_color(i, output_temp.led_color[i]);
      }
      ws2812_show();        
    }
    else {
      if (led_update_flag) {
        for(int i = 0; i < 8; i++) {
          neopixel_set_color(i, led_config.led_color[i]);
        }
        ws2812_show();
      }
    }

    i2c_timeout_counter = 0;
    if (i2c_stop_timeout_flag) {
      if (i2c_stop_timeout_delay < HAL_GetTick()) {
        i2c_stop_timeout_counter++;
        i2c_stop_timeout_delay = HAL_GetTick() + 10;
      }
    }
    if (i2c_stop_timeout_counter > 50) {
      LL_I2C_DeInit(I2C1);
      LL_I2C_DisableAutoEndMode(I2C1);
      LL_I2C_Disable(I2C1);
      LL_I2C_DisableIT_ADDR(I2C1);     
      user_i2c_init();    
      i2c1_it_enable();
      HAL_Delay(500);
    } 

    if (sc8721_delay_flag) {
      if (HAL_GetTick() - sc8721_update_delay >= 100) {
        if (!sc8721_update_flag) {
          sc8721_update_flag = 1;
        }
        sc8721_delay_flag = 0;
      }      
    }

    if (sc8721_update_flag && sys_power_status.pdcdc_power_status) {
      if (check_i2c_device(SC8721_7BIT_ADDR) == 0) {
        sc8721_set_voltage(sc8721_config.output_voltage);
        sc8721_set_current_limit(sc8721_config.iout_limit);
        sc8721_set_output_switch();
        sc8721_update_flag = 0;
      }
    }
    if (vameter_update_flag) {
      uint8_t res, res1, res2, res3, res4, res5;
      res = INA226_Init(&ina226_bat, NULL, 0x46, 0.01, 8);
      res1 = INA226_Init(&ina226_pwrcan, NULL, 0x44, 0.01, 8);
      res2 = INA226_Init(&ina226_pwr485, NULL, 0x45, 0.01, 8);
      res3 = INA226_Init(&ina226_usb, NULL, 0x40, 0.01, 8);
      res4 = INA226_Init(&ina226_grove_r, NULL, 0x42, 0.01, 8);
      res5 = INA226_Init(&ina226_grove_b, NULL, 0x43, 0.01, 8); 
      if (!res && !res1 && !res2 && !res3 && !res4 && !res5) {
        vameter_update_flag = 0;
      }
    }
    if (sys_power_status.vameter_power_status && !vameter_update_flag) {
      INA226_MeasureAll(&ina226_bat);
      ina226_bat.Result.Current_uA *= 2;
      INA226_MeasureAll(&ina226_pwrcan);
      INA226_MeasureAll(&ina226_pwr485);
      INA226_MeasureAll(&ina226_usb);
      INA226_MeasureAll(&ina226_grove_r);
      INA226_MeasureAll(&ina226_grove_b);
    }
    if (get_sc8721_flag) {
      LL_I2C_Disable(I2C2);
      I2C2_Start();       
      if (I2C_Read_Bytes(SC8721_READ_ADDR, 1, sc8721_reg_data, 10, I2C_TIMOUT_MS) == 0) {
        get_sc8721_flag = 0;
      }
    }
    if (is_flash_write_back) {
      __disable_irq();

      // retry 20 times, if still fail, reset system
      flash_writeback_timeout = 0;
      while(!flash_data_write_back()) {
        flash_writeback_timeout++;
        if (flash_writeback_timeout > 20) {
          HAL_NVIC_SystemReset();
        }
      }
      
      __enable_irq(); 
      is_flash_write_back = 0;
    }
    if (iap_jump_flag) {
      Jump_Bootloader();
      iap_jump_flag = 0;
    }
    if (HAL_GetTick() - rtc_update_delay > 100) {
      rx8130ce_get_time(&rtc_time);
      rtc_update_delay = HAL_GetTick();
    }
    if (alarm_update_flag) {
      uint8_t res, res1;
      res = rx8130ce_set_alarm_time(alarm_time);
      if (alarm_time.is_alarm_enable) {
        res1 = rx8130ce_set_alarm_enable();
      }
      else {
        res1 = rx8130ce_set_alarm_disable();
      }
      if (res && res1) {
        alarm_update_flag = 0;
      }
    }

    if (is_power_off_flag) {
      is_power_off_flag = 0;
      main_power_off();
    }

    if (is_reset_flag) {
      is_reset_flag = 0;
      reset_esp32();
    }

    if (is_download_flag) {
      is_download_flag = 0;
      set_esp32_download_mode();
    }
    // if (HAL_GetTick() - test_delay >= 2000) {
    //   sc8721_set_voltage(test_voltage);
    //   test_voltage += 1000;
    //   if (test_voltage > 20000)
    //     test_voltage = 3000;
    //   test_delay = HAL_GetTick();
    // }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  uint32_t clock_setup_timeout = 0;

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
  {
    clock_setup_timeout++;
    if (clock_setup_timeout > 32000) {
      HAL_NVIC_SystemReset();
    }    
  }

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();

  clock_setup_timeout = 0;
  while(LL_RCC_HSI_IsReady() != 1)
  {
    clock_setup_timeout++;
    if (clock_setup_timeout > 32000) {
      HAL_NVIC_SystemReset();
    }    
  }

  /* LSI configuration and activation */
  clock_setup_timeout = 0;
  LL_RCC_LSI_Enable();
  while(LL_RCC_LSI_IsReady() != 1)
  {
    clock_setup_timeout++;
    if (clock_setup_timeout > 32000) {
      HAL_NVIC_SystemReset();
    }    
  }

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_4);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();

  clock_setup_timeout = 0;
  while(LL_RCC_PLL_IsReady() != 1)
  {
    clock_setup_timeout++;
    if (clock_setup_timeout > 32000) {
      HAL_NVIC_SystemReset();
    }    
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  clock_setup_timeout = 0;
  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
    clock_setup_timeout++;
    if (clock_setup_timeout > 32000) {
      HAL_NVIC_SystemReset();
    }    
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_NVIC_SystemReset();
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
