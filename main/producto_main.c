#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "producto.h"
#include "producto_http.h"
#include "producto_wifi.h"
#include "producto_buttons.h"
#include "producto_timers.h"
#include "producto_display.h"
#include "producto_firebase.h"
#include "producto_activities.h"

#include "cJSON.h"

producto_t producto = {
    .activities = {
	{
	    .name = "Busy",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "Getting Busy",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "Wowza",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "Working",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "Pooping",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "Peeing",
	    .seconds = 0,
	    .status = PRODUCTO_ACTIVITY_PAUSED,
	},
	{
	    .name = "PAUSE",
	},
	{
	    .name = "LIST",
	},

    },
    .buttons = {
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_0_BTN,
	    .id = 0,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_1_BTN,
	    .id = 1,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_2_BTN,
	    .id = 2,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_3_BTN,
	    .id = 3,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_4_BTN,
	    .id = 4,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_ACTY_5_BTN,
	    .id = 5,
	    .type = PRODUCTO_BUTTON_TYPE_ACTIVITY,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_PAUSE_BTN,
	    .id = 6,
	    .type = PRODUCTO_BUTTON_TYPE_PAUSE,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_LIST_BTN,
	    .id = 7,
	    .type = PRODUCTO_BUTTON_TYPE_LIST,
	},
	
    },
    .button_evt_queue = NULL,
    .display_evt_queue = NULL,
    .activity_evt_queue = NULL,
    .current_activity = 7,
    .current_screen = PRODUCTO_SCREEN_LOG,
    .log_buffer = {
	.buf = {{0}},
	.first = 0,
	.last = 0,
	.buflen = PRODUCTO_LOG_MAX_ENTRIES,
    },
    .start_time = (time_t)(-1),
};

void producto_log(char *log)
{
    display_evt_t display_evt;
    producto_log_buffer_t *log_buffer = &producto.log_buffer;
    
    strcpy(log_buffer->buf[log_buffer->last], log);

    /* Circular buff it */
    log_buffer->last = (log_buffer->last + 1) % 64;
    
    if(log_buffer->last == log_buffer->first)
    {
	log_buffer->first = (log_buffer->first + 1) % 64;
    }

    if (producto.current_screen == PRODUCTO_SCREEN_LOG)
    {
	display_evt.type = DISPLAY_EVT_LOG;
	xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
    }
}

void set_start_time(void)
{
    setenv("TZ", "GMT+5", 1);
    tzset();
    time(&producto.start_time);
    struct tm * timeinfo;
    timeinfo = localtime ( &producto.start_time );
    printf ( "Current local time and date: %s", asctime (timeinfo) );
}

static void download_timer_names(void)
{
    static activity_evt_t activity_evt;
    activity_evt.type = ACTIVITY_EVT_SYNC_TIMER_NAMES;
    
    xQueueSend(producto.activity_evt_queue, &activity_evt, (TickType_t) 0);
}


#define MAX_HTTP_OUTPUT_BUFFER 2048
void app_main(void)
{
    producto.display_evt_queue = xQueueCreate(10, sizeof(display_evt_t));
    producto.button_evt_queue = xQueueCreate(10, sizeof(button_evt_t));
    producto.activity_evt_queue = xQueueCreate(10, sizeof(activity_evt_t));
    display_init();
    
    producto_log("Initializing wifi...");
    wifi_init();

    set_start_time();

    producto_log("Connecting to firebase...");

    producto_log("Initializing GPIO...");
    buttons_init();

    producto_log("Starting timer interrupts...");
    init_and_start_timers();

    producto_log("Set up activities...");
    activities_init();

    download_timer_names();

    producto_log("READY");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}
