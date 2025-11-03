#include "main.h"
#include "rx8130ce.h"
#include "i2c.h"

static uint8_t bin2bcd(uint8_t n) {
  return (((n / 10) << 4) + n % 10);
}

static uint8_t bcd2bin(uint8_t n) {
  return (n & 0x0f) + (n >> 4) * 10;
}

static void rx8130ce_software_reset(void)
{
	LL_I2C_Disable(I2C2);
	I2C2_Start(); 

    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x1E, 0);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x1E, 0x80);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x50, 0x6C);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x53, 0x01);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x66, 0x03);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x6B, 0x02);
    wireWriteRegister(RX8130CE_WRITE_ADDR, 0x6B, 0x01);
    HAL_Delay(125);
}

static void rx8130ce_dummy_read(void)
{
	LL_I2C_Disable(I2C2);
	I2C2_Start(); 

    wireReadRegister(RX8130CE_READ_ADDR, 0x1D);
}

static uint8_t rx8130ce_read_vlf(void)
{
    uint8_t res;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    res = wireReadRegister(RX8130CE_READ_ADDR, 0x1D);

    if (res & 0x02) {
        return 1;
    }
    else {
        return 0;
    }
}

static uint8_t rx8130ce_set_vlf(uint8_t value)
{
    uint8_t res;
    uint32_t timeout = 0;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    res = wireReadRegister(RX8130CE_READ_ADDR, 0x1D);

    timeout = 0;
    if (value) {
        res |= 0x02;
    }
    else {
        res &= ~0x02;
    }
    res = wireWriteRegister(RX8130CE_WRITE_ADDR, 0x1D, res);
    while (res) {
        res = wireWriteRegister(RX8130CE_WRITE_ADDR, 0x1D, res);
        timeout++;
        if (timeout > 100)
            return 0;
    } 
    return 1;   
}

// 修正函数名拼写错误（Check 而非 Cheack）
int Check_date(int w_year, int w_month, int w_date) {
    // 月份有效性检查（1-12）
    if (w_month < 1 || w_month > 12) {
        return 0;
    }

    // 各月份天数表（2月初始为28天）
    unsigned char Month_buf[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // 处理闰年2月
    if (w_month == 2) {
        int is_leap = ((w_year % 4 == 0 && w_year % 100 != 0) || (w_year % 400 == 0));
        Month_buf[1] = is_leap ? 29 : 28;
    }

    // 日期有效性检查（1~Month_buf[w_month-1]）
    if (w_date < 1 || w_date > Month_buf[w_month-1]) {
        return 0;
    }

    return 1;
}

static uint8_t rx8130ce_initialization(void)
{
    uint8_t res = 0;
    uint8_t return_data = 0;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    res += wireWriteRegister(RX8130CE_WRITE_ADDR, 0x30, 0x00);  
    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_EXT);
    return_data &= (~(RX8130_BIT_EXT_TE));
    return_data |= 0x03;
    res += wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_EXT, return_data);
    rx8130ce_set_vlf(0);
    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
    return_data &= (~(RX8130_BIT_CTRL_TEST));
    return_data &= (~(RX8130_BIT_CTRL_AIE));
    return_data &= (~(RX8130_BIT_CTRL_TIE));
    return_data &= (~(RX8130_BIT_CTRL_UIE));
    res += wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);
    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL1);
    return_data |= (1 << 4);
    res += wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL1, return_data);

    if (res)
    	return 0;
    else
    	return 1;
}

uint8_t rx8130ce_set_time(rtc_time_t time)
{
    uint8_t return_data;
    uint8_t date[7];
    uint8_t res, res1, res2;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    if (Check_date(time.tm_year+2000, time.tm_mon, time.tm_mday)) {
        return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
        return_data |= RX8130_BIT_CTRL_STOP;
        res = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);

        date[RX8130_REG_SEC-0x10] = bin2bcd(time.tm_sec);
        date[RX8130_REG_MIN-0x10] = bin2bcd(time.tm_min);
        date[RX8130_REG_HOUR-0x10] = bin2bcd(time.tm_hour);		//only 24hr time

        date[RX8130_REG_MDAY-0x10] = bin2bcd(time.tm_mday);
        date[RX8130_REG_MONTH-0x10] = bin2bcd(time.tm_mon);
        date[RX8130_REG_YEAR-0x10] = bin2bcd((time.tm_year + 1900) % 100);
        date[RX8130_REG_WDAY-0x10] = bin2bcd(time.tm_wday);    

        res1 = I2C_Write_Bytes(RX8130CE_WRITE_ADDR, RX8130_REG_SEC, date, 7, I2C_TIMOUT_MS);

        return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
        return_data &= (~(RX8130_BIT_CTRL_STOP));
        res2 = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);    

        if (!res && res1 && !res2)
            return 1;
        else
            return 0;
    }
    else {
        return 0;
    }
}

uint8_t rx8130ce_set_alarm_time(alarm_time_t alarm_time)
{
    uint8_t return_data;
    uint8_t alarm_data[3];
    uint8_t res, res1, res2, res3, res4;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
    return_data &= ~(RX8130_BIT_CTRL_AIE | RX8130_BIT_CTRL_UIE);
    res = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);

	alarm_data[0] = bin2bcd(alarm_time.tm_min);
	alarm_data[1] = bin2bcd(alarm_time.tm_hour);
	alarm_data[2] = bin2bcd(alarm_time.tm_mday);    

    res1 = I2C_Write_Bytes(RX8130CE_WRITE_ADDR, RX8130_REG_ALMIN, alarm_data, 2, I2C_TIMOUT_MS);  

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_EXT);
    return_data |= RX8130_BIT_EXT_WADA;
    res3 = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_EXT, return_data);

	if (alarm_data[2] == 0) {
		alarm_data[2] |= 0x80;	//turn on AE bit to ignore day of month (no zero day)
        res2 = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_ALWDAY, alarm_data[2]);
	}
	else {
		res2 = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_ALWDAY, alarm_data[2]);
	}      

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_FLAG);
    return_data &= ~(RX8130_BIT_FLAG_AF | RX8130_BIT_FLAG_UF);
    res4 = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_FLAG, return_data);

    if (!res && !res1 && !res2 && !res3 && !res4) {
        return 1;
    }
    else {
        return 0;
    }
}

uint8_t rx8130ce_clear_alarm_flag(void)
{
    uint8_t return_data;
    uint8_t res;    

	LL_I2C_Disable(I2C2);
	I2C2_Start(); 

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_FLAG);
    return_data &= ~(RX8130_BIT_FLAG_AF | RX8130_BIT_FLAG_UF);
    res = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_FLAG, return_data);    

    if (!res) {
        return 1;
    }
    else {
        return 0;
    }    
}

uint8_t rx8130ce_set_alarm_enable(void)
{
    uint8_t return_data;
    uint8_t res;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
    return_data |= (RX8130_BIT_CTRL_AIE);
    res = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);

    if (!res) {
        return 1;
    }
    else {
        return 0;
    }
}

uint8_t rx8130ce_set_alarm_disable(void)
{
    uint8_t return_data;
    uint8_t res;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    return_data = wireReadRegister(RX8130CE_READ_ADDR, RX8130_REG_CTRL0);
    return_data &= ~(RX8130_BIT_CTRL_AIE | RX8130_BIT_CTRL_UIE);
    res = wireWriteRegister(RX8130CE_WRITE_ADDR, RX8130_REG_CTRL0, return_data);

    if (!res) {
        return 1;
    }
    else {
        return 0;
    }
}

uint8_t rx8130ce_get_time(rtc_time_t *time)
{
    uint8_t date[7];
    uint8_t res;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    res = I2C_Read_Bytes(RX8130CE_READ_ADDR, RX8130_REG_SEC, date, 7, I2C_TIMOUT_MS);

    if (!res) {
        time->tm_sec = bcd2bin(date[RX8130_REG_SEC-0x10]);
        time->tm_min = bcd2bin(date[RX8130_REG_MIN-0x10]);
        time->tm_hour = bcd2bin(date[RX8130_REG_HOUR-0x10]);
        time->tm_mday = bcd2bin(date[RX8130_REG_MDAY-0x10]);
        time->tm_mon = bcd2bin(date[RX8130_REG_MONTH-0x10]);
        time->tm_year = bcd2bin(date[RX8130_REG_YEAR-0x10]);
        time->tm_wday = bcd2bin(date[RX8130_REG_WDAY-0x10]);
    }

    if (!res)
        return 1;
    else
        return 0;
}

uint8_t rx8130ce_init(void)
{
    uint8_t res = 255;
    uint32_t timeout = 0;
    uint32_t init_timeout = 0;

	LL_I2C_Disable(I2C2);
	I2C2_Start();     

    HAL_Delay(30);
    rx8130ce_dummy_read();
    alarm_time.is_alarm_enable = 0;
init_step:
    res = rx8130ce_read_vlf();
    if (res == 1) {
        rx8130ce_software_reset();
        if (!rx8130ce_set_vlf(0))
            return 0;
        res = rx8130ce_read_vlf();
        while (res)
        {
            if (!rx8130ce_set_vlf(0))
                return 0;
            res = rx8130ce_read_vlf();
            timeout++;
            if (timeout > 100)
                return 0;
        }
        rx8130ce_initialization();
        return 1;
    }
    else if (res == 0) {
        rx8130ce_set_alarm_disable();
        goto finish_step;
    }
    else {
        init_timeout++;
        if (init_timeout > 100)
            return 0;        
        goto init_step;
    }
finish_step:
    return 1;
}
