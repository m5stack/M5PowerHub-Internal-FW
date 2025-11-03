/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "i2c_ex.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define I2C_TIMOUT_MS     10
/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);

/* USER CODE BEGIN Prototypes */
void user_i2c_init(void);
void I2C2_Start(void);
uint8_t I2C2_TransmitData(uint8_t device_id ,uint8_t *pdata, uint8_t size, uint16_t timeout);
uint8_t I2C2_TransmitData_RepeatedStart(uint8_t device_id ,uint8_t *pdata, uint8_t size, uint16_t timeout);
uint8_t I2C2_ReceiveData(uint8_t device_id , uint8_t *pdata, uint8_t size, uint16_t timeout);
uint8_t I2C_Write_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t TxLen,uint16_t Timeout);
uint8_t I2C_Read_Bytes(uint8_t DeviceAddress, uint8_t MemAddress, uint8_t *pData, uint8_t RxLen,uint16_t Timeout);
uint8_t wireWriteRegister(uint8_t write_addr, uint8_t reg, uint8_t data);
uint8_t wireReadRegister(uint8_t read_addr, uint8_t reg);
uint8_t check_i2c_device(uint8_t i2c_address);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

