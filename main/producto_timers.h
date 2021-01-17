#ifndef _PRODUCTO_TIMERS_H_
#define _PRODUCTO_TIMERS_H_

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

#endif /* _PRODUCTO_TIMERS_H_ */
