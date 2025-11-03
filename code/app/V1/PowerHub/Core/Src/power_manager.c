#include "main.h"
#include "power_manager.h"
#include "ws2812.h"
#include "gpio.h"
#include "iwdg.h"
#include "i2c.h"

static void main_power_off_without_standby(void)
{
    memset((uint8_t *)&sys_power_status, 0, sizeof(sys_power_status));
    LL_GPIO_ResetOutputPin(MPWR_EN_GPIO_Port, MPWR_EN_Pin);
    disable_usb_power();
    disable_led_power();
}

static void power_led_set_to_color_temp(uint32_t color)
{
    for(int i = 0; i < 8; i++) {
        if (i >= 6)
            neopixel_set_color(i, color);
        else
            neopixel_set_color(i, 0);
    }
    ws2812_show();      
}

static void power_led_init(void)
{
    for (int i = 0; i < 8; i++) {
        neopixel_set_color(i, 0);
    }

    ws2812_show();      
}

void set_standby_mode(void)
{
  HAL_PWREx_EnablePullUpPullDownConfig();
  HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_B, LL_PWR_GPIO_BIT_0);
  if (wakeup_source_status.nstby_wkup) {
    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_B, LL_PWR_GPIO_BIT_5);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6_LOW); // PB5 nSTBY_WKUP
  }
  if (wakeup_source_status.vin_det) {
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_HIGH); // PA0 VIN_DET
  }
  if (wakeup_source_status.vusb_det) {
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW); // PA4 VUSB_DET
  }
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF6);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
  HAL_PWR_EnterSTANDBYMode();  
}

void clear_pwr_flag(void)
{
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    /* Check and Clear the Wakeup flag */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF6) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF6);
    }
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF1) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
    }
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF2) != RESET)
    {
      // is_usb_wakeup = 1;
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
    }
  }  
}

void power_led_set_to_color(uint32_t color)
{
    led_config.led_color[6] = color;
    led_config.led_color[7] = color;
}

void power_led_show_before(void)
{
    for(int i = 0; i < 8; i++) {
        neopixel_set_color(i, led_config.led_color[i]);
    }

    ws2812_show();      
}

void main_power_on(void)
{
    HAL_PWREx_DisablePullUpPullDownConfig();
    memset((uint8_t *)&sys_power_status, 0, sizeof(sys_power_status));
    memset((uint8_t *)&sc8721_config, 0, sizeof(sc8721_config));
    LL_GPIO_SetOutputPin(xGPIO0_BOOT_GPIO_Port, xGPIO0_BOOT_Pin);
    LL_GPIO_SetOutputPin(MPWR_EN_GPIO_Port, MPWR_EN_Pin);
    enable_led_power();
    sys_power_status.main_power_status = 1;

    power_led_init();
    power_led_set_to_color_temp(0x00FF00);
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x000000);    
    HAL_Delay(100);  
    HAL_IWDG_Refresh(&hiwdg);  
    power_led_set_to_color_temp(0x00FF00);
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x000000);    
    HAL_Delay(100);  
    HAL_IWDG_Refresh(&hiwdg);  
    power_led_show_before();
    
}

void main_power_off(void)
{
    memset((uint8_t *)&sys_power_status, 0, sizeof(sys_power_status));
    LL_GPIO_ResetOutputPin(MPWR_EN_GPIO_Port, MPWR_EN_Pin);
    disable_usb_power();
    disable_led_power();
    set_standby_mode();
}

void reset_esp32(void)
{
    LL_GPIO_ResetOutputPin(USB_CON_LV1_GPIO_Port, USB_CON_LV1_Pin);
    LL_GPIO_ResetOutputPin(USB_CON_LV2_GPIO_Port, USB_CON_LV2_Pin);    
    main_power_off_without_standby();
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(100);
    main_power_on();
    reset_led_buffer(); 
    led_update_flag = 0;    

    user_i2c_init();
    i2c1_it_enable();     
}

void set_esp32_download_mode(void)
{
    LL_I2C_DeInit(I2C1);
    LL_I2C_Disable(I2C1);
    i2c1_all_it_disable();
    i2c_stop_timeout_flag = 0;
    i2c_stop_timeout_counter = 0;
    i2c_timeout_counter = 0;       
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    LL_GPIO_ResetOutputPin(USB_CON_LV1_GPIO_Port, USB_CON_LV1_Pin);
    LL_GPIO_ResetOutputPin(USB_CON_LV2_GPIO_Port, USB_CON_LV2_Pin);    
    LL_GPIO_ResetOutputPin(xGPIO0_BOOT_GPIO_Port, xGPIO0_BOOT_Pin);
    LL_GPIO_ResetOutputPin(MPWR_EN_GPIO_Port, MPWR_EN_Pin);
  
    power_led_set_to_color_temp(0x0000ff);
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x000000);    
    HAL_Delay(100);   
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x0000ff);   
    HAL_Delay(100); 
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x000000);    
    HAL_Delay(100);  
    HAL_IWDG_Refresh(&hiwdg);        
    power_led_set_to_color_temp(0x0000ff);    
    HAL_Delay(100);  
    HAL_IWDG_Refresh(&hiwdg);        
    LL_GPIO_SetOutputPin(MPWR_EN_GPIO_Port, MPWR_EN_Pin);
    power_led_set_to_color_temp(0x000000);   
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    LL_GPIO_SetOutputPin(xGPIO0_BOOT_GPIO_Port, xGPIO0_BOOT_Pin);
    power_led_set_to_color_temp(0x0000ff);
    HAL_Delay(100);
    HAL_IWDG_Refresh(&hiwdg);
    power_led_set_to_color_temp(0x000000);    
    HAL_Delay(100); 
    HAL_IWDG_Refresh(&hiwdg);  
    power_led_set_to_color_temp(0x0000ff);   
    HAL_Delay(100); 
    HAL_IWDG_Refresh(&hiwdg);
    power_led_init();
    reset_led_buffer(); 
    led_update_flag = 0;
    HAL_Delay(100);  
    HAL_IWDG_Refresh(&hiwdg);   
}

void enable_led_power(void)
{
    sys_power_status.led_power_status = 1;
    LL_GPIO_ResetOutputPin(nLED_EN_GPIO_Port, nLED_EN_Pin);
}

void disable_led_power(void)
{
    sys_power_status.led_power_status = 0;
    LL_GPIO_SetOutputPin(nLED_EN_GPIO_Port, nLED_EN_Pin);
}

void enable_usb_power(void)
{
    sys_power_status.usb_power_status = 1;
    LL_GPIO_SetOutputPin(OEN_USB_GPIO_Port, OEN_USB_Pin);
}

void disable_usb_power(void)
{
    sys_power_status.usb_power_status = 0;
    LL_GPIO_ResetOutputPin(OEN_USB_GPIO_Port, OEN_USB_Pin);
}

void enable_grove_r_power(void)
{
    sys_power_status.grove_r_power_status = 1;
    LL_GPIO_SetOutputPin(OEN_GRV_R_GPIO_Port, OEN_GRV_R_Pin);
}

void disable_grove_r_power(void)
{
    sys_power_status.grove_r_power_status = 0;
    LL_GPIO_ResetOutputPin(OEN_GRV_R_GPIO_Port, OEN_GRV_R_Pin);
}

void enable_grove_b_power(void)
{
    sys_power_status.grove_b_power_status = 1;
    LL_GPIO_SetOutputPin(OEN_GRV_B_GPIO_Port, OEN_GRV_B_Pin);
}

void disable_grove_b_power(void)
{
    sys_power_status.grove_b_power_status = 0;
    LL_GPIO_ResetOutputPin(OEN_GRV_B_GPIO_Port, OEN_GRV_B_Pin);
}

void enable_pdcdc_power(void)
{
    sys_power_status.pdcdc_power_status = 1;
    LL_GPIO_SetOutputPin(PDCDC_EN_GPIO_Port, PDCDC_EN_Pin);
}

void disable_pdcdc_power(void)
{
    sys_power_status.pdcdc_power_status = 0;
    LL_GPIO_ResetOutputPin(PDCDC_EN_GPIO_Port, PDCDC_EN_Pin);
}

void enable_pdcdc_output(void)
{
    sys_power_status.pdcdc_output_status = 1;
    LL_GPIO_SetOutputPin(OEN_PWROUT_GPIO_Port, OEN_PWROUT_Pin);
}

void disable_pdcdc_output(void)
{
    sys_power_status.pdcdc_output_status = 0;
    LL_GPIO_ResetOutputPin(OEN_PWROUT_GPIO_Port, OEN_PWROUT_Pin);
}

void enable_vameter_power_output(void)
{
    sys_power_status.vameter_power_status = 1;
    LL_GPIO_ResetOutputPin(nVA_EN_GPIO_Port, nVA_EN_Pin);
}

void disable_vameter_power_output(void)
{
    sys_power_status.vameter_power_status = 0;
    LL_GPIO_SetOutputPin(nVA_EN_GPIO_Port, nVA_EN_Pin);
}

void set_pd_power_input(void)
{
    disable_pdcdc_power();
    disable_pdcdc_output();
    LL_GPIO_SetOutputPin(PDCDC_REFLOW_GPIO_Port, PDCDC_REFLOW_Pin);
    sys_power_status.pd_power_direct = 0;
}

void set_pd_power_output(void)
{
    enable_pdcdc_power();
    enable_pdcdc_output();
    LL_GPIO_ResetOutputPin(PDCDC_REFLOW_GPIO_Port, PDCDC_REFLOW_Pin);
    sys_power_status.pd_power_direct = 1;
}

void enable_charge_power(void)
{
    sys_power_status.charge_power_status = 1;
    LL_GPIO_SetOutputPin(CHG_EN_GPIO_Port, CHG_EN_Pin);
}

void disable_charge_power(void)
{
    sys_power_status.charge_power_status = 0;
    LL_GPIO_ResetOutputPin(CHG_EN_GPIO_Port, CHG_EN_Pin);
}
