#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

void init_and_start_timers(void);
