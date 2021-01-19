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
#include "producto_activities.h"

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
    .hide_activity = false,
};

#define MAX_HTTP_OUTPUT_BUFFER 2048
void app_main(void)
{
    wifi_init();

    static char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    http_get("https://producto-1cba1-default-rtdb.firebaseio.com/timers.json", local_response_buffer);
	
    /* printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size()); */

    /* producto.button_evt_queue = xQueueCreate(10, sizeof(button_evt_t)); */
    /* producto.display_evt_queue = xQueueCreate(10, sizeof(display_evt_t)); */
    /* producto.activity_evt_queue = xQueueCreate(10, sizeof(activity_evt_t)); */

    /* buttons_init(); */
    /* display_init(); */
    /* init_and_start_timers(); */
    /* activities_init(); */
}
