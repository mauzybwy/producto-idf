#ifndef _PRODUCTO_H_
#define _PRODUCTO_H_

#include <time.h>
#include "freertos/queue.h"
#include "producto_buttons.h"
#include "producto_activities.h"

#define PRODUCTO_ACTY_0_BTN 32
#define PRODUCTO_ACTY_1_BTN 17
#define PRODUCTO_ACTY_2_BTN 2
#define PRODUCTO_ACTY_3_BTN 15
#define PRODUCTO_ACTY_4_BTN 13
#define PRODUCTO_ACTY_5_BTN 12

#define PRODUCTO_PAUSE_BTN  0
#define PRODUCTO_LIST_BTN   35

#define PRODUCTO_NUM_ACTIVITY (6U)
#define PRODUCTO_NUM_SPECIAL_BTNS (2U)
#define PRODUCTO_NUM_BUTTONS (PRODUCTO_NUM_ACTIVITY + PRODUCTO_NUM_SPECIAL_BTNS)

#define PRODUCTO_ACTIVITY_PAUSED  (0U)
#define PRODUCTO_ACTIVITY_RUNNING (1U)

#define PRODUCTO_BUTTON_TYPE_ACTIVITY (0U)
#define PRODUCTO_BUTTON_TYPE_PAUSE    (1U)
#define PRODUCTO_BUTTON_TYPE_LIST     (2U)

#define PRODUCTO_LOG_MAX_LINE_LENGTH (32U)
#define PRODUCTO_LOG_MAX_ENTRIES     (64U)

#define PRODUCTO_SCREEN_LOG      (0U)
#define PRODUCTO_SCREEN_ACTIVITY (1U)
#define PRODUCTO_SCREEN_LIST     (2U)

#pragma pack(1)
typedef struct {
    char buf[PRODUCTO_LOG_MAX_ENTRIES][PRODUCTO_LOG_MAX_LINE_LENGTH];
    uint8_t first;
    uint8_t last;
    uint8_t buflen;
} producto_log_buffer_t;

#pragma pack(1)
typedef struct {
    producto_log_buffer_t log_buffer;
    button_t buttons[PRODUCTO_NUM_BUTTONS];
    producto_activity_t activities[PRODUCTO_NUM_BUTTONS];
    xQueueHandle display_evt_queue;
    xQueueHandle button_evt_queue;
    xQueueHandle activity_evt_queue;
    uint8_t current_activity;
    uint8_t current_screen;
    time_t start_time;
} producto_t;

producto_t producto;

void producto_log(char *log);

#endif /* _PRODUCTO_H_ */
