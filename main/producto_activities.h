#ifndef _PRODUCTO_ACTIVITIES_H_
#define _PRODUCTO_ACTIVITIES_H_

void activities_init(void);

#define ACTIVITY_EVT_BUTTON_PRESSED (0U)
#define ACTIVITY_EVT_SECOND_TICK (1U)

xQueueHandle activities_queue;

#pragma pack(1)
typedef struct {
    char name[32];
    uint32_t seconds;
    uint8_t status;
} producto_activity_t;

#pragma pack(1)
typedef struct {
    uint8_t data[64];
    uint8_t type;
} activity_evt_t;

#endif /* _PRODUCTO_ACTIVITIES_H_ */
