#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "producto.h"
#include "producto_buttons.h"
#include "producto_timers.h"
#include "producto_display.h"

producto_t producto = {
    .timers = {
	{
	    .name = "Busy",
	},
	{
	    .name = "Getting Busy",
	},
	{
	    .name = "Wowza",
	},
	{
	    .name = "Working",
	},
	{
	    .name = "Pooping",
	},
	{
	    .name = "Peeing",
	},

    },
    .buttons = {
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_0_BTN,
	    .id = 0,
	},
	{
	    
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_1_BTN,
	    .id = 1,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_2_BTN,
	    .id = 2,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_3_BTN,
	    .id = 3,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_4_BTN,
	    .id = 4,
	},
	{
	    .level_count = 0,
	    .edge_type = BUTTON_EDGE_NEG,
	    .level = 1,
	    .gpio = PRODUCTO_TASK_5_BTN,
	    .id = 5,
	},
    }
};

void app_main(void)
{
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    buttons_init();
    init_and_start_timers();
    display_init();
}
