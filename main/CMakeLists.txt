idf_component_register(
  SRCS "producto_main.c" "producto_timers.c" "producto_buttons.c" "producto_display.c" "producto_activities.c" "producto_wifi.c" "producto_http.c" "producto_firebase.c"
  INCLUDE_DIRS "."
  REQUIRES
    tft
    spiffs
  PRIV_REQUIRES
    nvs_flash
    esp_http_client
    json
    esp-tls
)
