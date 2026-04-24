idf_component_register(
    SRCS "LEDWidget.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio
    PRIV_REQUIRES driver esp_timer
)
