idf_component_register(
  SRCS "producto_main.c" "producto_timers.c" "producto_buttons.c" "producto_display.c"
  INCLUDE_DIRS "."
  REQUIRES
    tft
    spiffs
  PRIV_REQUIRES
    nvs_flash
)
