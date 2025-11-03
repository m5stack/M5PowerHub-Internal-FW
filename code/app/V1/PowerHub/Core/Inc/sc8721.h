#ifndef __SC8721_H__
#define __SC8721_H__

#define SC8721_WRITE_ADDR 0xC4
#define SC8721_READ_ADDR 0xC5
#define SC8721_7BIT_ADDR 0x62

#define SC8721_REG_CSO 0x01
#define SC8721_REG_SLOPE_COMP 0x02
#define SC8721_REG_VOUT_SET_MSB 0x03
#define SC8721_REG_VOUT_SET_LSB 0x04
#define SC8721_REG_GLOBAL_CTRL 0x05
#define SC8721_REG_SYS_SET 0x06
#define SC8721_REG_RESERVED 0x07
#define SC8721_REG_FREQ_SET 0x08
#define SC8721_REG_STATUS_1 0x09
#define SC8721_REG_STATUS_2 0x0A

uint8_t sc8721_set_voltage(uint16_t voltage_mv);
uint8_t sc8721_set_current_limit(uint16_t current_limit_mv);
uint8_t sc8721_set_output_switch(void);
void sc8721_set_output_switch_phy(void);

#endif