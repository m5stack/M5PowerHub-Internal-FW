#ifndef __BUTTON_H__
#define __BUTTON_H__

typedef enum {
    BUTTON_RELEASED,
    BUTTON_PRESSED
} ButtonState;

typedef enum {
    EVENT_NONE,
    EVENT_SINGLE_CLICK,
    EVENT_DOUBLE_CLICK,
    EVENT_LONG_PRESS
} ButtonEvent;

void judge_button_1(void);
void handle_button_2(void);

#endif
