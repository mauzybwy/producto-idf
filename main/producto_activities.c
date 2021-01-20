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
#include "producto_firebase.h"

#include "cJSON.h"

extern producto_t producto;

static char strbuf[16] = {0};
void update_session_in_firebase(void)
{
    producto_activity_t *activity;
    cJSON *sessions_root = cJSON_CreateObject();
    cJSON *session_tasks = cJSON_CreateObject();

    for (int i=0; i < PRODUCTO_NUM_ACTIVITY; i++)
    {
    	activity = &producto.activities[i];
    	cJSON_AddNumberToObject(session_tasks, activity->name, activity->seconds);
    }
    
    sprintf(strbuf, "%ld", producto.start_time);
    cJSON_AddItemToObject(sessions_root, strbuf, session_tasks);
    
    firebase_write("sessions", sessions_root);

    cJSON_Delete(sessions_root);
}

static void set_timers_from_firebase(void)
{
    cJSON *timers_root = NULL, *current_element = NULL;;
    timers_root = firebase_read("timers");

    for (int i = 0 ; i < cJSON_GetArraySize(timers_root) ; i++)
    {
	current_element = cJSON_GetArrayItem(timers_root, i);
	if (cJSON_IsString(current_element)) {
	    strcpy(producto.activities[i].name, current_element->valuestring);
	    printf("%s\n", current_element->valuestring);
	}
    }

    cJSON_Delete(timers_root);
}

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
	    producto.current_screen = PRODUCTO_SCREEN_ACTIVITY;
	    display_evt.type = DISPLAY_EVT_UPDATE_ACTIVITY;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}
	break;
		
    case PRODUCTO_BUTTON_TYPE_PAUSE:
	/* set_timers_from_firebase(); */
	activity = &producto.activities[producto.current_activity];
	activity->status = ( (activity->status == PRODUCTO_ACTIVITY_PAUSED)
			     ? PRODUCTO_ACTIVITY_RUNNING
			     : PRODUCTO_ACTIVITY_PAUSED );
	break;

    case PRODUCTO_BUTTON_TYPE_LIST:
	if (producto.current_screen != PRODUCTO_SCREEN_LIST)
	{
	    producto.current_screen = PRODUCTO_SCREEN_LIST;
	    display_evt.type = DISPLAY_EVT_LIST_ACTIVITIES;
	    xQueueSend(producto.display_evt_queue, &display_evt, (TickType_t) 0);
	}

	else
	{
	    producto.current_screen = PRODUCTO_SCREEN_ACTIVITY;
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

	if(producto.current_screen == PRODUCTO_SCREEN_ACTIVITY)
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

	case ACTIVITY_EVT_UPDATE_FIREBASE:
	    update_session_in_firebase();
	    break;

	case ACTIVITY_EVT_SYNC_TIMER_NAMES:
	    set_timers_from_firebase();
	    break;

	default:
	    break;

	}
    }
}

void activities_init(void)
{
    xTaskCreate(activities_task, "activities_task", 4096, NULL, 5, NULL);
}
