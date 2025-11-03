/**
  ******************************************************************************
  * @file    ws2812.c
  * @brief   This file provides code for the configuration
  *          of the PWM instances.
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
#include "ws2812.h"
#include <stdlib.h>

/* USER CODE BEGIN 0 */
#include <string.h>
#include "tim.h"
/* USER CODE END 0 */

uint32_t *color_buf = NULL;
uint8_t rled[PIXEL_MAX] = {0};
uint8_t gled[PIXEL_MAX] = {0};
uint8_t bled[PIXEL_MAX] = {0};
uint8_t brightness_index[PIXEL_MAX] = {0};
uint32_t *lastest_rgb_color = NULL;
FIFOBuffer* rgb_buffer;
uint8_t rgb_show_lock = 0;

frame_buf frame = { 
	.head[0] = 0,
	.head[1] = 0,
	.head[2] = 0,
	.tail = 0,
};

// 全局缓冲区实例
static led_ringbuffer_t g_led_buffer = {0};

FIFOBuffer* createBuffer() {
    FIFOBuffer* buffer = (FIFOBuffer*)malloc(sizeof(FIFOBuffer));
    buffer->front = buffer->rear = NULL;
    return buffer;
}

void enqueue(FIFOBuffer* buffer, uint32_t newData[PIXEL_MAX]) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    memcpy(newNode->data, newData, PIXEL_MAX * sizeof(uint32_t));
    newNode->next = NULL;
    
    if (buffer->rear == NULL) {
        buffer->front = buffer->rear = newNode;
        return;
    }
    
    buffer->rear->next = newNode;
    buffer->rear = newNode;
}

int8_t dequeue(FIFOBuffer* buffer, uint32_t result[PIXEL_MAX]) {
    if (buffer->front == NULL) {
        return 0;
    }

    Node* temp = buffer->front;
    memcpy(result, temp->data, PIXEL_MAX * sizeof(uint32_t));
    buffer->front = buffer->front->next;

    if (buffer->front == NULL) {
        buffer->rear = NULL;
    }

    free(temp);

    return 1;
}

uint32_t* getQueueRear(FIFOBuffer* buffer) {
    if (buffer->rear != NULL) {
        return buffer->rear->data;
    } else {
        return NULL; // 如果队列为空，返回 NULL
    }
}

// 初始化缓冲区（系统启动时调用）
void led_buffer_init(void)
{
    memset(&g_led_buffer, 0, sizeof(g_led_buffer));
}

// 写入新颜色配置（I2C中断上下文调用）
uint8_t led_buffer_push(const uint32_t* color_data)
{
    uint32_t next_head = (g_led_buffer.head + 1) % RING_BUFFER_SIZE;
    
    // 缓冲区满检测（保留一个空位用于区分满/空状态）
    if(next_head == g_led_buffer.tail) {
        return 0; // 缓冲区满，丢弃新数据或根据需求处理
    }
    
    // 安全拷贝数据（建议关闭中断或使用互斥量保护）
    memcpy(g_led_buffer.buffer[g_led_buffer.head].led_color, 
          color_data, 
          sizeof(uint32_t)*PIXEL_MAX);
    
    g_led_buffer.head = next_head;
    return 1;
}

// 获取待处理颜色配置（主循环调用）
uint8_t led_buffer_pop(led_command_t* output)
{
    if(g_led_buffer.tail == g_led_buffer.head) {
        return 0; // 缓冲区空
    }
    
    // 拷贝数据到输出结构
    memcpy(output->led_color, 
          g_led_buffer.buffer[g_led_buffer.tail].led_color,
          sizeof(uint32_t)*PIXEL_MAX);
    
    // 更新消费位置（原子操作）
    g_led_buffer.tail = (g_led_buffer.tail + 1) % RING_BUFFER_SIZE;
    return 1;
}

uint8_t led_buffer_peek_latest(uint32_t* output_array) 
{   
    // 判断缓冲区是否为空
    if(g_led_buffer.head == g_led_buffer.tail) {
        return 0;
    }
    
    // 计算最新有效数据位置
    uint32_t last_valid_index = (g_led_buffer.head == 0) ? 
                               (RING_BUFFER_SIZE - 1) : 
                               (g_led_buffer.head - 1);
    
    // 安全拷贝最新数据
    memcpy(output_array, 
          g_led_buffer.buffer[last_valid_index].led_color,
          sizeof(uint32_t)*PIXEL_MAX);
    
    return 1;
}

void reset_led_buffer(void) 
{   
    g_led_buffer.head = g_led_buffer.tail;
}

void sk6812_init(uint8_t num) {
  color_buf = (uint32_t *)calloc(num, sizeof(uint32_t));
  lastest_rgb_color = (uint32_t *)calloc(num, sizeof(uint32_t));
  rgb_buffer = createBuffer();
  memset((uint8_t *)&led_config, 0, sizeof(led_config));
  for (int i = 0; i < PIXEL_MAX; i++) {
    led_config.led_color[i] = 0x000000;
    led_config.led_brightness[i] = 20;
  }
}

void neopixel_set_color(uint8_t num, uint32_t color) {
  uint16_t r, g, b;
  if (num >= PIXEL_MAX)
    return;
  
  if (led_config.led_brightness[num] < 10 && led_config.led_brightness[num])
    led_config.led_brightness[num] = 10;

  r = ((color >> 16) & 0xff) * ((float)led_config.led_brightness[num] / 255.0f);
  g = ((color >> 8) & 0xff) * ((float)led_config.led_brightness[num] / 255.0f);
  b = (color & 0xff) * ((float)led_config.led_brightness[num] / 255.0f);

  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
	rled[num] = r;
	gled[num] = g;
	bled[num] = b;
	color_buf[num] = color;
}

void ws2812_show(void)
{
  if (sys_power_status.led_power_status) {
    HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_2);
    uint8_t i, j;

    for(i = 0; i < PIXEL_MAX; i++) {
      for(j = 0; j < 8; j++) {														// G->R->B
        frame.data[24 * i + j] = (gled[i] & (0x80 >> j)) ? BIT_1 : BIT_0; 			// 将高低位扩展到16bit
        frame.data[24 * i + j + 8]   = (rled[i] & (0x80 >> j)) ? BIT_1 : BIT_0;	
        frame.data[24 * i + j + 16]  = (bled[i] & (0x80 >> j)) ? BIT_1 : BIT_0;
      }
    }

    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_2, (uint32_t *)&frame, 3 + 24 * PIXEL_MAX + 1);
  }
}
/* USER CODE BEGIN 2 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_2);
  rgb_show_lock = 0;
}
/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
