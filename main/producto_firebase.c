#include <stdio.h>
#include <string.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#include "producto.h"
#include "producto_http.h"
#include "producto_firebase.h"

#include "cJSON.h"

/* extern producto_t producto; */

#define MAX_HTTP_OUTPUT_BUFFER 2048


void firebase_write(char *path, cJSON *json_root)
{
    char *post_data = cJSON_Print(json_root);
    http_patch("https://producto-1cba1-default-rtdb.firebaseio.com/timers.json", post_data);
    free(post_data);
}

cJSON* firebase_read(char *path)
{
    static char urlbuf[64] = {0};
    static char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

    sprintf(urlbuf, "https://producto-1cba1-default-rtdb.firebaseio.com/%s.json", path);
    
    http_get(urlbuf, local_response_buffer);

    return cJSON_Parse(local_response_buffer);
}
