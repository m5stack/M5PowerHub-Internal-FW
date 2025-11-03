/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ws2812.h
  * @brief   This file contains all the function prototypes for
  *          the ws2812.c file
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
#ifndef __ws2812_H__
#define __ws2812_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define PIXEL_MAX 8
#define RGB_BUFFER_SIZE 10
#define BIT_1 28
#define BIT_0 10

#define RING_BUFFER_SIZE  8

typedef struct {
    uint32_t led_color[PIXEL_MAX];
} led_command_t;

typedef struct {
    led_command_t buffer[RING_BUFFER_SIZE];
    volatile uint32_t head;  // 生产位置（ISR修改）
    volatile uint32_t tail;  // 消费位置（主循环修改）
} led_ringbuffer_t;

typedef struct Node {
    uint32_t data[PIXEL_MAX];
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} FIFOBuffer;

/* USER CODE END Private defines */
extern uint8_t brightness_index[PIXEL_MAX];
extern uint32_t *color_buf;
extern uint32_t *lastest_rgb_color;
extern FIFOBuffer* rgb_buffer;
extern uint8_t rgb_show_lock;

void sk6812_init(uint8_t num);
void neopixel_set_color(uint8_t num, uint32_t color);
void ws2812_show(void);
FIFOBuffer* createBuffer();
void enqueue(FIFOBuffer* buffer, uint32_t newData[PIXEL_MAX]);
int8_t dequeue(FIFOBuffer* buffer, uint32_t result[PIXEL_MAX]);
uint32_t* getQueueRear(FIFOBuffer* buffer);

void led_buffer_init(void);
uint8_t led_buffer_peek_latest(uint32_t* output_array);
uint8_t led_buffer_pop(led_command_t* output);
uint8_t led_buffer_push(const uint32_t* color_data);
void reset_led_buffer(void);

/* USER CODE BEGIN Prototypes */
typedef struct
{
  const uint16_t head[3];           //先发送3个0等待DMA稳定
  uint16_t data[24 * PIXEL_MAX];    //真正的数据
  const uint16_t tail;              //发送最后1个0，保证DMA结束后，PWM输出低
} frame_buf;

/* USER CODE END Prototypes */

#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
