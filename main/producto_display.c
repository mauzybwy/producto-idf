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
extern xQueueHandle button_queue;

static void tft_init(void);
static void display_task(void *arg);

static char tmp_buff[64];

static void display_task(void *arg)
{
    button_t button;
    
    while(1)
    {
	xQueueReceive(button_queue, &button, portMAX_DELAY);
	printf("BUTTON: %s\n", producto.timers[button.id].name);

	TFT_fillScreen(TFT_BLACK);
	sprintf(tmp_buff, "%s", producto.timers[button.id].name);
	TFT_print(tmp_buff, CENTER, CENTER);
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
