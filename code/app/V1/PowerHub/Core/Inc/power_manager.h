#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__

void power_led_set_to_color(uint32_t color);
void main_power_on(void);
void main_power_off(void);
void enable_led_power(void);
void disable_led_power(void);
void reset_esp32(void);
void set_esp32_download_mode(void);
void clear_pwr_flag(void);
void set_standby_mode(void);
void enable_usb_power(void);
void disable_usb_power(void);
void enable_grove_r_power(void);
void disable_grove_r_power(void);
void enable_grove_b_power(void);
void disable_grove_b_power(void);
void enable_pdcdc_power(void);
void disable_pdcdc_power(void);
void enable_pdcdc_output(void);
void disable_pdcdc_output(void);
void set_pd_power_output(void);
void set_pd_power_input(void);
void enable_vameter_power_output(void);
void disable_vameter_power_output(void);
void enable_charge_power(void);
void disable_charge_power(void);
void power_led_show_before(void);

#endif