idf_component_register(
    SRCS "LEDWidget.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio
    PRIV_REQUIRES esp_driver_ledc esp_timer
)
