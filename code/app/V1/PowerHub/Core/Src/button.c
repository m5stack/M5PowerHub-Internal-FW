#include "main.h"
#include "button.h"
#include "power_manager.h"
#include "iwdg.h"

#define POWER_ON_TIME 2000
#define POWER_OFF_TIME 5000
#define RESET_TIME 3000
#define CLICK_FILTER_TIME 50

// 宏定义及全局变量
#define DEBOUNCE_TIME    20    // 消抖时间20ms（需根据实际测试调整）
#define CLICK_TIME       500   // 双击间隔500ms（参考系统双击时间设置）
#define LONG_PRESS_TIME  3000  // 长按3秒复位

// 状态变量
typedef struct {
    ButtonState state;
    uint32_t last_change_time;
    uint32_t first_click_time;
    uint8_t click_count;
    uint8_t long_press_flag;
} button_t;

volatile button_t button = {0};

static uint32_t button_press_time = 0;
static uint8_t click_flag = 0;
static uint8_t button_press_flag = 0;
static uint32_t click_time = 0;

static uint8_t is_button_2_change = 0;
static uint32_t button2_press_time = 0;
static uint8_t button2_click_start = 0;

// 按钮事件处理主函数
void handle_button_2(void) 
{
    ButtonState current_state = !(VUSB_DET_GPIO_Port->IDR & VUSB_DET_Pin);

    if (current_state != button.state) {
        if (!is_button_2_change) {
            button2_press_time = HAL_GetTick(); 
            is_button_2_change = 1;
        }
    }
    else {
        is_button_2_change = 0;
    }
     
    if (((HAL_GetTick() - button2_press_time) > DEBOUNCE_TIME) && is_button_2_change) {
        current_state = !(VUSB_DET_GPIO_Port->IDR & VUSB_DET_Pin);

        if (current_state != button.state) {
            button.state = current_state;
            button.last_change_time = HAL_GetTick();

            if (button.state == BUTTON_PRESSED) {
                // 长按检测
                while (1)
                {
                    HAL_IWDG_Refresh(&hiwdg);
                    if (((!(VUSB_DET_GPIO_Port->IDR & VUSB_DET_Pin)) == BUTTON_RELEASED) || ((HAL_GetTick() - button.last_change_time) > LONG_PRESS_TIME)) {
                        break;
                    }
                } 

                if((HAL_GetTick() - button.last_change_time) > LONG_PRESS_TIME) {
                    set_esp32_download_mode();
                    button.click_count = 0;
                    button.long_press_flag = 1;
                }
            }
            else {
                if (!button.long_press_flag) {
                    if (button.click_count == 0) {
                        button2_click_start = 1;
                        button.first_click_time = HAL_GetTick();
                    }
                    button.click_count++;  
                }
                else {
                    button.long_press_flag = 0;
                }
            }            
        }        
    }

    if (button2_click_start && (HAL_GetTick() - button.first_click_time >= CLICK_TIME)) {
        button2_click_start = 0;

        if (button.click_count == 1) {
            reset_esp32();
        }
        else if (button.click_count == 2) {
            if (!wakeup_source_status.nstby_wkup && !wakeup_source_status.vin_det && !wakeup_source_status.vusb_det) {
                button.click_count = 0;
                return;
            }
            if (sys_power_status.main_power_status) {
                sys_power_status.main_power_status = 0;
                main_power_off();
            }            
        }
        button.click_count = 0;
    }
}


void judge_button_1(void)
{
    if (!(!!(GPIOA->IDR & PMU_SW_Pin))) {
        if (!click_flag) {
            click_time = HAL_GetTick();
            click_flag = 1;
        }
        if (!button_press_flag) {
            button_press_time = HAL_GetTick();
            button_press_flag = 1;
        }
        if (HAL_GetTick() - button_press_time >= POWER_OFF_TIME) {
            if (!wakeup_source_status.nstby_wkup && !wakeup_source_status.vin_det && !wakeup_source_status.vusb_det)
                return;
            if (sys_power_status.main_power_status) {
                sys_power_status.main_power_status = 0;
                main_power_off();
            }
        }          
    }
    else {
        button_press_time = 0;
        click_flag = 0;
        button_press_flag = 0;
        click_time = 0;        
    }    
}
