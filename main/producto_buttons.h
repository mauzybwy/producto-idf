#ifndef _PRODUCTO_BUTTONS_H_
#define _PRODUCTO_BUTTONS_H_

#include "freertos/queue.h"

#define BUTTON_EDGE_POS  (0U)
#define BUTTON_EDGE_NEG  (1U)
#define BUTTON_EDGE_BOTH (2U)

#define BUTTON_MAX_STRING   (32U)

#define BUTTON_EVT_CLICK (0);

#pragma pack(1)
typedef struct {
    uint8_t level_count;
    uint8_t edge_type;
    uint8_t level;
    uint8_t gpio;
    uint8_t id;
    uint8_t type;
} button_t;

#pragma pack(1)
typedef struct {
    button_t button;
    uint8_t type;
} button_evt_t;

void buttons_init(void);
/* void check_buttons_task(void *arg); */
TaskHandle_t check_buttons_task_handle;
xQueueHandle button_queue;

#endif /* _PRODUCTO_BUTTONS_H_ */
