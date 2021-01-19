#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "producto.h"
#include "producto_activities.h"

#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

#include "producto.h"
#include "producto_activities.h"
#include "producto_display.h"
#include "producto_buttons.h"

extern producto_t producto;

static void handle_button_press(button_evt_t button_evt)
{
    display_evt_t display_evt;
    producto_activity_t *activity;
    
    switch (button_evt.button.type)
    {
    case PRODUCTO_BUTTON_TYPE_ACTIVITY:
	if (producto.current_activity != button_evt.button.id)
	{
	    producto.current_activity = button_evt.button.id;
	    activity = &producto.activities[producto.current_activity];
	    activity->status = PRODUCTO_ACTIVITY_RUNNING;
	    producto.hide_activity = false;
	    display_evt.type = DISPLAY_EVT_UPDATE_ACTIVITY;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}
	break;
		
    case PRODUCTO_BUTTON_TYPE_PAUSE:
	activity = &producto.activities[producto.current_activity];
	activity->status = ( (activity->status == PRODUCTO_ACTIVITY_PAUSED)
			     ? PRODUCTO_ACTIVITY_RUNNING
			     : PRODUCTO_ACTIVITY_PAUSED );
	break;

    case PRODUCTO_BUTTON_TYPE_LIST:
	if (!producto.hide_activity)
	{
	    producto.hide_activity = true;
	    display_evt.type = DISPLAY_EVT_LIST_ACTIVITIES;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}

	else
	{
	    producto.hide_activity = false;
	    display_evt.type = DISPLAY_EVT_UPDATE_ACTIVITY;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}
	break;
    }
}

static void handle_second_tick(void)
{
    display_evt_t display_evt;
    producto_activity_t *activity;
    activity = &producto.activities[producto.current_activity];

    if(activity->status == PRODUCTO_ACTIVITY_RUNNING)
    {
	activity->seconds++;

	if(!producto.hide_activity)
	{
	    display_evt.type = DISPLAY_EVT_UPDATE_TIMER;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}
    }
}

static void activities_task(void *arg)
{
    activity_evt_t activity_evt;
    
    while(1)
    {
	xQueueReceive(producto.activity_evt_queue, &activity_evt, portMAX_DELAY);

	switch (activity_evt.type)
	{
	case ACTIVITY_EVT_BUTTON_PRESSED:
	    handle_button_press(*(button_evt_t*)(activity_evt.data));
	    break;

	case ACTIVITY_EVT_SECOND_TICK:
	    handle_second_tick();
	    break;

	default:
	    break;

	}
    }
}

void activities_init(void)
{
    xTaskCreate(activities_task, "activities_task", 2048, NULL, 5, NULL);
}
