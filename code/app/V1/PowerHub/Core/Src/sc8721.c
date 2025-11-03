#include "main.h"
#include "sc8721.h"
#include "power_manager.h"
#include "i2c.h"

static uint16_t voltage_setting;
static uint16_t current_limit_setting;

uint8_t sc8721_set_voltage(uint16_t voltage_mv)
{
    uint16_t voltage_offset = 0;
    uint8_t fb_dir = 0;
    uint8_t res, res1, res2, res3;
    uint8_t current_over_limit_flag = 0;

	LL_I2C_Disable(I2C2);
	I2C2_Start();    

    if (voltage_mv < 3000)
        voltage_mv = 3000;
    else if (voltage_mv > 20000)
        voltage_mv = 20000;

    voltage_setting = voltage_mv;

    if (voltage_mv >= 5000) {
        voltage_offset = (voltage_mv - 5000) / 20;
    }
    else {
        fb_dir = 1;
        voltage_offset = (5000 - voltage_mv) / 20;
    }

    res = wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_VOUT_SET_MSB, voltage_offset >> 2);

    if (fb_dir == 0) {
        res1 = wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_VOUT_SET_LSB, (voltage_offset & 0x03) | 0x18);
    }
    else {
    	res1 = wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_VOUT_SET_LSB, (voltage_offset & 0x03) | 0x1C);
    }

    if (voltage_setting * current_limit_setting > 20000000) {
        uint16_t current_limit_mv = 20000000 / voltage_setting;
        uint8_t cso_set = current_limit_mv * 0.62f / 8 - 1;
        res3 = wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_CSO, cso_set);
        current_over_limit_flag = 1; 
    }

    res2 = sc8721_set_output_switch();

    if (!current_over_limit_flag) {
        if (!res && !res1 && !res2)
            return 1;
        else
            return 0;
    }
    else {
        if (!res && !res1 && !res2 && !res3)
            return 1;
        else
            return 0;
    }
}

uint8_t sc8721_set_current_limit(uint16_t current_limit_mv)
{
    
    uint8_t cso_set = 0;
    uint8_t res, res1;

	LL_I2C_Disable(I2C2);
	I2C2_Start();      

    if (current_limit_mv > 3000)
        current_limit_mv = 3000;

    current_limit_setting = current_limit_mv;

    if ((uint32_t)voltage_setting * current_limit_mv > 20000000) {
        current_limit_mv = 20000000 / voltage_setting;
        // 再次确保不超过最大值
        if (current_limit_mv > 3000) {
            current_limit_mv = 3000;
        }
        cso_set =  (uint8_t)(current_limit_mv * 0.62f / 8.0f - 1);
    }
    else {
        cso_set = sc8721_config.current_limit_value;
    }

    res = wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_CSO, cso_set);

    res1 = sc8721_set_output_switch();

    if (!res && !res1)
    	return 1;
    else
    	return 0;
}

uint8_t sc8721_set_output_switch(void)
{
	LL_I2C_Disable(I2C2);
	I2C2_Start();      
    
    if (sc8721_config.power_output_control) {
        enable_pdcdc_output();
        return wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_GLOBAL_CTRL, 0x02);
    }
    else {
        disable_pdcdc_output();
        return  wireWriteRegister(SC8721_WRITE_ADDR, SC8721_REG_GLOBAL_CTRL, 0x06);
    }
}

void sc8721_set_output_switch_phy(void)
{
    if (sc8721_config.power_output_control) {
        enable_pdcdc_output();
    }
    else {
        disable_pdcdc_output();
    }
}
