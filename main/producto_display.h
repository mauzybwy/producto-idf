#ifndef _PRODUCTO_DISPLAY_H_
#define _PRODUCTO_DISPLAY_H_

#include "freertos/queue.h"

void display_init(void);
xQueueHandle display_evt_queue;

#define DISPLAY_EVT_UPDATE_TIMER    (0U)
#define DISPLAY_EVT_UPDATE_ACTIVITY (1U)

#pragma pack(1)
typedef struct {
    uint8_t type;
} display_evt_t;

#endif /* _PRODUCTO_DISPLAY_H_ */
