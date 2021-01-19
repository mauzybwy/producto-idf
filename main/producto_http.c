#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include "cJSON.h"

#include "producto_http.h"


#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "producto_http";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

void http_get(char* url, char *response_buffer)
{
    /* static char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0}; */
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     */
    esp_http_client_config_t config = {
	.url = url,
        /* .host = "httpbin.org", */
        /* .path = "/get", */
        /* .query = "esp", */
        .event_handler = _http_event_handler,
        .user_data = response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    /* ESP_LOG_BUFFER_HEX(TAG, response_buffer, strlen(response_buffer)); */
    printf("%s\n", response_buffer);
}

void http_patch(char *url, char *patch_data)
{
    esp_http_client_config_t config = {
	.url = url,
        .event_handler = _http_event_handler,
        .user_data = NULL,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
	.method = HTTP_METHOD_PATCH,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    
    /* esp_http_client_set_method(client, HTTP_METHOD_PATCH); */
    /* esp_http_client_set_header(client, "Content-Type", "application/json"); */
    esp_http_client_set_post_field(client, patch_data, strlen(patch_data));

    esp_err_t err = esp_http_client_perform(client);
    err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
}

/* void http_init(void) */
/* { */
/*     esp_http_client_config_t config = { */
/* 	.url = "https://producto-1cba1-default-rtdb.firebaseio.com/timers.json", */
/* 	.event_handler = _http_event_handle, */
/*     }; */
    
/*     esp_http_client_handle_t client = esp_http_client_init(&config); */
/*     esp_err_t err = esp_http_client_perform(client); */

/*     if (err == ESP_OK) { */
/* 	ESP_LOGI(TAG, "Status = %d, content_length = %d", */
/* 		 esp_http_client_get_status_code(client), */
/* 		 esp_http_client_get_content_length(client)); */
/*     } */
/*     else { */
/* 	ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err)); */
/*     } */
/* } */

/* void http_init(void) */
/* {        */
/*     esp_http_client_config_t config = { */
/* 	.url = "https://producto-1cba1-default-rtdb.firebaseio.com/timers.json", */
/* 	.event_handler = _http_event_handle, */
/*     }; */
    
/*     esp_http_client_handle_t client = esp_http_client_init(&config); */
/*     esp_err_t err = esp_http_client_perform(client); */

/*     if (err == ESP_OK) { */
/* 	ESP_LOGI(TAG, "Status = %d, content_length = %d", */
/* 		 esp_http_client_get_status_code(client), */
/* 		 esp_http_client_get_content_length(client)); */
/*     } */
/*     else { */
/* 	ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err)); */
/*     } */
    
/*     cJSON *json_root; */
/*     json_root = cJSON_CreateObject(); */
/*     /\* cJSON_AddStringToObject(json_root, "string", "fdsa"); *\/ */
/*     /\* cJSON_AddStringToObject(json_root, "field1", "butthole"); *\/ */
/*     /\* cJSON_AddNumberToObject(json_root, "number", 69); *\/ */
/*     /\* cJSON_AddTrueToObject(json_root, "flag_true"); *\/ */
/*     /\* cJSON_AddFalseToObject(json_root, "flag_false"); *\/ */

/*     /\* cJSON *array = cJSON_CreateArray(); *\/ */
/*     /\* cJSON *jstr = cJSON_CreateString("asdf"); *\/ */
/*     /\* cJSON_AddItemToArray(array, jstr); *\/ */
/*     /\* jstr = cJSON_CreateString("fdsa"); *\/ */
/*     /\* cJSON_AddItemToArray(array, jstr); *\/ */
/*     /\* cJSON_AddItemToObject(json_root, "test_array", array); *\/ */
/*     /\* /\\* const char *my_json_string = cJSON_Print(json_root); *\\/ *\/ */
/*     /\* /\\* ESP_LOGI(TAG, "my_json_string\n%s",my_json_string); *\\/ *\/ */

/*     /\* cJSON *firebase_array = cJSON_CreateObject(); *\/ */
/*     /\* cJSON_AddStringToObject(json_root, "0", "hmmm"); *\/ */
/*     cJSON_AddStringToObject(json_root, "1", "definitely"); */
/*     /\* cJSON_AddStringToObject(json_root, "2", "this"); *\/ */
/*     /\* cJSON_AddStringToObject(json_root, "3", "works"); *\/ */

/*     /\* cJSON_AddItemToObject(json_root, "firebase_array", firebase_array); *\/ */

/*     const char *post_data = cJSON_Print(json_root);; */
/*     esp_http_client_set_url(client, "https://producto-1cba1-default-rtdb.firebaseio.com/test/butts/firebase_array.json"); */
/*     esp_http_client_set_method(client, HTTP_METHOD_PATCH); */
/*     esp_http_client_set_header(client, "Content-Type", "application/json"); */
/*     esp_http_client_set_post_field(client, post_data, strlen(post_data)); */
/*     err = esp_http_client_perform(client); */
/*     if (err == ESP_OK) { */
/*         ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d", */
/*                 esp_http_client_get_status_code(client), */
/*                 esp_http_client_get_content_length(client)); */
/*     } else { */
/*         ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err)); */
/*     } */

/*     esp_http_client_cleanup(client); */
/*     cJSON_Delete(json_root); */
/* } */
