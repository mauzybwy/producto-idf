#ifndef _PRODUCTO_H_
#define _PRODUCTO_H_

#include "producto_buttons.h"

#define PRODUCTO_TASK_0_BTN 32
#define PRODUCTO_TASK_1_BTN 17
#define PRODUCTO_TASK_2_BTN 2
#define PRODUCTO_TASK_3_BTN 15
#define PRODUCTO_TASK_4_BTN 13
#define PRODUCTO_TASK_5_BTN 12

#define PRODUCTO_NUM_TIMERS (6U)
#define PRODUCTO_NUM_SPECIAL_BTNS (0U)
#define PRODUCTO_NUM_BUTTONS (PRODUCTO_NUM_TIMERS + PRODUCTO_NUM_SPECIAL_BTNS)

typedef struct {
    char name[32];
} producto_timer_t;

typedef struct {
    button_t buttons[PRODUCTO_NUM_BUTTONS];
    producto_timer_t timers[PRODUCTO_NUM_TIMERS];
} producto_t;

producto_t producto;

#endif /* _PRODUCTO_H_ */
