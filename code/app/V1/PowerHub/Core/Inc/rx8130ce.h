#ifndef __RX8130CE_H__
#define __RX8130CE_H__

#define RX8130CE_WRITE_ADDR 0x64
#define RX8130CE_READ_ADDR 0x65

// RX-8130 Register definitions
#define RX8130_REG_SEC		0x10
#define RX8130_REG_MIN		0x11
#define RX8130_REG_HOUR		0x12
#define RX8130_REG_WDAY		0x13
#define RX8130_REG_MDAY		0x14
#define RX8130_REG_MONTH	0x15
#define RX8130_REG_YEAR		0x16

#define RX8130_REG_ALMIN	0x17
#define RX8130_REG_ALHOUR	0x18
#define RX8130_REG_ALWDAY	0x19
#define RX8130_REG_TCOUNT0	0x1A
#define RX8130_REG_TCOUNT1	0x1B
#define RX8130_REG_EXT		0x1C
#define RX8130_REG_FLAG		0x1D
#define RX8130_REG_CTRL0	0x1E
#define RX8130_REG_CTRL1	0x1F

#define RX8130_REG_END		0x23

// Extension Register (1Ch) bit positions
#define RX8130_BIT_EXT_TSEL		(7 << 0)
#define RX8130_BIT_EXT_WADA		(1 << 3)
#define RX8130_BIT_EXT_TE		(1 << 4)
#define RX8130_BIT_EXT_USEL		(1 << 5)
#define RX8130_BIT_EXT_FSEL		(3 << 6)

// Flag Register (1Dh) bit positions
#define RX8130_BIT_FLAG_VLF		(1 << 1)
#define RX8130_BIT_FLAG_AF		(1 << 3)
#define RX8130_BIT_FLAG_TF		(1 << 4)
#define RX8130_BIT_FLAG_UF		(1 << 5)

// Control 0 Register (1Еh) bit positions
#define RX8130_BIT_CTRL_TSTP	(1 << 2)
#define RX8130_BIT_CTRL_AIE		(1 << 3)
#define RX8130_BIT_CTRL_TIE		(1 << 4)
#define RX8130_BIT_CTRL_UIE		(1 << 5)
#define RX8130_BIT_CTRL_STOP	(1 << 6)
#define RX8130_BIT_CTRL_TEST	(1 << 7)

typedef struct
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
} rtc_time_t;

typedef struct
{
  int tm_min;
  int tm_hour;
  int tm_mday;
  unsigned char is_alarm_enable;
} alarm_time_t;

uint8_t rx8130ce_init(void);
uint8_t rx8130ce_set_time(rtc_time_t time);
uint8_t rx8130ce_get_time(rtc_time_t *time);
uint8_t rx8130ce_set_alarm_enable(void);
uint8_t rx8130ce_set_alarm_disable(void);
uint8_t rx8130ce_set_alarm_time(alarm_time_t alarm_time);

#endif
