#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "producto.h"
#include "producto_buttons.h"

extern producto_t producto;

static void check_buttons_task(void *arg);
static void check_buttons(void);
static bool debounce_button(button_t *button);

TaskHandle_t check_buttons_task_handle = NULL;
xQueueHandle button_queue;

static bool debounce_button(button_t *button)
{
    bool retVal = false;
    uint8_t level = gpio_get_level(button->gpio);

    /* Latch level if appears stable */
    if (button->level != level)
    {
	button->level_count += 1;

	if (button->level_count > 3)
	{
	    button->level_count = 0;
	    button->level = level;
	    retVal = true;
	}
    }

    /* Reset the count if spurious */
    else
    {
	button->level_count = 0;
    }

    return retVal;
}

static void check_buttons(void)
{
    button_t *button;
    
    for ( uint8_t i = 0; i < PRODUCTO_NUM_TIMERS; i++ )
    {
	button = &producto.buttons[i];
	
	if (debounce_button(button)
	    && (( button->edge_type == BUTTON_EDGE_BOTH )
	    	|| ( button->level == 0 && button->edge_type == BUTTON_EDGE_NEG )
	    	|| ( button->level == 1 && button->edge_type == BUTTON_EDGE_POS ))
	    ) {
	    xQueueSend( button_queue, button, (TickType_t) 0 );
	}
    }
}

static void check_buttons_task(void *arg)
{
    while (1) {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
	/* printf("\nCHECKING BUTTONS\n"); */
	check_buttons();
    }
}

void buttons_init(void)
{
    uint64_t gpio_mask = 0;
    
    for ( uint8_t i = 0; i < PRODUCTO_NUM_TIMERS; i++ )
    {
	gpio_mask |= (1ULL << producto.buttons[i].gpio);
    }
    
    /* GPIO Configuration */
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = gpio_mask;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    button_queue = xQueueCreate(10, sizeof(button_t));
    
    xTaskCreate(check_buttons_task, "check_buttons_task", 2048, NULL, 5, &check_buttons_task_handle);
}
