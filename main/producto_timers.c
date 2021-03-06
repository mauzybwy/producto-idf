#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "producto.h"
#include "producto_timers.h"
#include "producto_buttons.h"
#include "producto_display.h"
#include "producto_activities.h"

#define TIMER_DIVIDER         (16) /*  Hardware timer clock divider */
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER) /* convert counter value to seconds */
#define TIMER_INTERVAL0_SEC   (0.01)  /* sample test interval for the first timer */
#define TIMER_INTERVAL1_SEC   (10.0)  /* sample test interval for the second timer */

#define CHECK_BUTTONS         (0U)  /* time to poll the buttons */
#define TEST_WITH_RELOAD      (1U)  /* testing will be done with auto reload */
#define UPDATE_FIREBASE_TASKS (3U)  /* update the tasks in firebase */

extern producto_t producto;

static xQueueHandle timer_group0_queue;

/*
 * Dispatch timer events
 */
static void timer_evt_task(void *arg)
{
    timer_event_t evt;
    
    while (1) {
        xQueueReceive(timer_group0_queue, &evt, portMAX_DELAY);

	/* Dipatch button checking */
	if (evt.type == CHECK_BUTTONS)
	{
            /* printf("\n    Check them buttons\n"); */
	    xTaskNotifyGive(check_buttons_task_handle);
        }

	/* Dispatch example timer */
	else if (evt.type == TEST_WITH_RELOAD)
	{
            /* printf("\n    Example timer with auto reload\n"); */
        }

	/* Received unknown timer event... */
	else {
            printf("\n    UNKNOWN EVENT TYPE\n");
	    printf("Group[%d], timer[%d] alarm event\n", evt.timer_group, evt.timer_idx);
        }
    }
}

static void second_ticker_callback(void* arg)
{
    static activity_evt_t activity_evt;
    activity_evt.type = ACTIVITY_EVT_SECOND_TICK;
    
    xQueueSend(producto.activity_evt_queue, &activity_evt, (TickType_t) 0);
    
}

static void update_firebase_callback(void* arg)
{
    static activity_evt_t activity_evt;
    activity_evt.type = ACTIVITY_EVT_UPDATE_FIREBASE;
    
    xQueueSend(producto.activity_evt_queue, &activity_evt, (TickType_t) 0);
    
}

/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
    timer_spinlock_take(TIMER_GROUP_0);
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t timer_intr = timer_group_get_intr_status_in_isr(TIMER_GROUP_0);
    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, timer_idx);

    /* Prepare basic event data
       that will be then sent back to the main program task */
    static timer_event_t evt;
    evt.timer_group = 0;
    evt.timer_idx = timer_idx;
    evt.timer_counter_value = timer_counter_value;

    /* Handle Group0 Timer0: CHECK_BUTTONS */
    if (timer_intr & TIMER_INTR_T0)
    {
        evt.type = CHECK_BUTTONS;
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    }

    /* Handle Group0 Timer1: TEST_WITH_RELOAD */
    else if (timer_intr & TIMER_INTR_T1)
    {
        evt.type = TEST_WITH_RELOAD;
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
    }

    /* Received an unsupported timer interrupt */
    else {
        evt.type = -1; // not supported even type
    }

    /* After the alarm has been triggered
       we need enable it again, so it is triggered the next time */
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, timer_idx);

    /* Now just send the event data back to the main program task */
    xQueueSendFromISR(timer_group0_queue, &evt, NULL);
    timer_spinlock_give(TIMER_GROUP_0);
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_sec - the interval of alarm to set
 */
static void timer_group0_init(int timer_idx,
			      bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
                       (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

void init_and_start_timers(void)
{
    /* Timer Group #0  Config*/
    timer_group0_queue = xQueueCreate(10, sizeof(timer_event_t));
    timer_group0_init(TIMER_0, CHECK_BUTTONS, TIMER_INTERVAL0_SEC);
    timer_group0_init(TIMER_1, TEST_WITH_RELOAD, TIMER_INTERVAL1_SEC);

    /* Second-tick */
    const esp_timer_create_args_t second_ticker_args = {
	.callback = &second_ticker_callback,
	.name = "second_ticker"
    };

    esp_timer_handle_t second_ticker;
    ESP_ERROR_CHECK(esp_timer_create(&second_ticker_args, &second_ticker));

    /* Update Firebase Tasks */
    const esp_timer_create_args_t update_firebase_args = {
	.callback = &update_firebase_callback,
	.name = "update_firebase"
    };

    esp_timer_handle_t update_firebase;
    ESP_ERROR_CHECK(esp_timer_create(&update_firebase_args, &update_firebase));

    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_periodic(second_ticker, 1000000));
    ESP_ERROR_CHECK(esp_timer_start_periodic(update_firebase, 60000000));

    /* Timer Event Task */
    xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
}
