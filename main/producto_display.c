#include <string.h>
#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "tftspi.h"
#include "tft.h"

#include "producto.h"
#include "producto_buttons.h"
#include "producto_display.h"

extern producto_t producto;

static void tft_init(void);
static void display_task(void *arg);

static void update_activity()
{
    producto_activity_t activity = producto.activities[producto.current_activity];
    
    TFT_fillRect(0,0,240,135/2, TFT_BLACK);
    TFT_print(activity.name, 0, 0);
}

static void update_time()
{
    producto_activity_t activity = producto.activities[producto.current_activity];
    
    uint8_t hr = activity.seconds / 3600 % 24;
    uint8_t min = activity.seconds / 60 % 60;
    uint8_t sec = activity.seconds % 60;
    
    char time_str[10];
    sprintf(time_str, "\r%02d:%02d:%02d", hr, min, sec);
    /* TFT_fillRect(0,135/2,240,135/2, TFT_BLACK); */
    TFT_print(time_str, 0, 135/2);
}

static void display_log_tail(int num, char *outbuf)
{
    producto_log_buffer_t *log_buffer = &producto.log_buffer;
    uint8_t valid_num = 0, before_wrap = 0, after_wrap = 0, zero_val = 0;
    char *outbuf_ptr = outbuf;

    /* Circular buffer has not wrapped */
    if (log_buffer->last > log_buffer->first)
    {
	valid_num = (( log_buffer->last - log_buffer->first ) >= num )
	    ? num
	    : log_buffer->last - log_buffer->first;

	for(int i = log_buffer->last - valid_num; i < log_buffer->last; i++)
	{
	    outbuf_ptr += sprintf(outbuf_ptr, "%s\n", log_buffer->buf[i]);
	}
    }

    /* Circular buffer has wrapped */
    else if (log_buffer->last < log_buffer->first)
    {
	/* Tail doesn't read over wrap bounds */
	if (log_buffer->last >= num)
	{
	    after_wrap = log_buffer->last;
	    before_wrap = log_buffer->buflen;
	    zero_val = after_wrap - num;
	}

	/* Tail reads over wrap bounds */
	else
	{
	    after_wrap = log_buffer->last;
	    before_wrap = log_buffer->buflen - (num - after_wrap);
	    zero_val = 0;
	}

	/* Print first side of the wrap */
	for (int i = before_wrap; i < log_buffer->buflen; i++)
	{
	    outbuf_ptr += sprintf(outbuf_ptr, "%s\n", log_buffer->buf[i]);
	}

	/* Print other side of the wrap */
	for (int i = zero_val; i < after_wrap; i++)
	{
	    outbuf_ptr += sprintf(outbuf_ptr, "%s\n", log_buffer->buf[i]);
	}
    }

    /* Circular buffer empty */
    else
    {
	
    }

    TFT_fillScreen(TFT_BLACK);
    TFT_print(outbuf, 0, 0);
}


static void list_activities()
{
    TFT_fillScreen(TFT_BLACK);
    TFT_print("LIST", CENTER, CENTER);
}

static void display_task(void *arg)
{
    display_evt_t display_evt;
    static char local_log_buf[10 * PRODUCTO_LOG_MAX_LINE_LENGTH];
    
    while(1)
    {
	xQueueReceive(producto.display_evt_queue, &display_evt, portMAX_DELAY);

	switch (display_evt.type)
	{
	case DISPLAY_EVT_UPDATE_TIMER:
	    update_time();
	    break;

	case DISPLAY_EVT_UPDATE_ACTIVITY:
	    update_activity();
	    update_time();
	    break;

	case DISPLAY_EVT_LIST_ACTIVITIES:
	    list_activities();
	    break;

	case DISPLAY_EVT_LOG:
	    display_log_tail(10, local_log_buf);
	    break;
	    
	default:
	    break;
	}
    }
}

void display_init(void)
{
    tft_init();
    
    xTaskCreate(display_task, "display_task", 2048, NULL, 5, NULL);
}

static void tft_init(void)
{
    esp_err_t ret;
    tft_max_rdclock = 8000000;
    
    TFT_PinsInit();

    // ====  CONFIGURE SPI DEVICES(s)  ====================================================================================

    spi_lobo_device_handle_t spi;
	
    spi_lobo_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,				// set SPI MISO pin
        .mosi_io_num=PIN_NUM_MOSI,				// set SPI MOSI pin
        .sclk_io_num=PIN_NUM_CLK,				// set SPI CLK pin
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
	.max_transfer_sz = 6*1024,
    };
    spi_lobo_device_interface_config_t devcfg={
        .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=-1,                       // we will use external CS pin
	.spics_ext_io_num=PIN_NUM_CS,           // external CS pin
	.flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    vTaskDelay(500 / portTICK_RATE_MS);

    // ====================================================================================================================


    // ==================================================================
    // ==== Initialize the SPI bus and attach the LCD to the SPI bus ====

    ret=spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &spi);
    assert(ret==ESP_OK);
    printf("SPI: display device added to spi bus (%d)\r\n", TFT_HSPI_HOST);
    tft_disp_spi = spi;

    // ==== Test select/deselect ====
    ret = spi_lobo_device_select(spi, 1);
    assert(ret==ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret==ESP_OK);

    printf("SPI: attached display device, speed=%u\r\n", spi_lobo_get_speed(spi));
    printf("SPI: bus uses native pins: %s\r\n", spi_lobo_uses_native_pins(spi) ? "true" : "false");

    // ================================
    // ==== Initialize the Display ====

    printf("SPI: display init...\r\n");
    TFT_display_init();
    TFT_invertDisplay(1);
    printf("OK\r\n");
	
    // ---- Detect maximum read speed ----
    tft_max_rdclock = find_rd_speed();
    printf("SPI: Max rd speed = %u\r\n", tft_max_rdclock);

    // ==== Set SPI clock used for display operations ====
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    printf("SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));

    tft_font_rotate = 0;
    tft_text_wrap = 0;
    tft_font_transparent = 0;
    tft_font_forceFixed = 0;
    tft_gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(LANDSCAPE_FLIP);
    TFT_setFont(DEFAULT_FONT, NULL);
    TFT_resetclipwin();
    
    tft_fg = TFT_CYAN;
    printf("\r\n");
}
