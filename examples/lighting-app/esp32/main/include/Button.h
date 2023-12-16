#ifndef ESP32_BUTTON_H
#define ESP32_BUTTON_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#ifndef CONFIG_ESP32_BUTTON_LONG_PRESS_DURATION_MS
#define CONFIG_ESP32_BUTTON_LONG_PRESS_DURATION_MS (5000)
#endif

#ifndef CONFIG_ESP32_BUTTON_LONG_PRESS_REPEAT_MS
#define CONFIG_ESP32_BUTTON_LONG_PRESS_REPEAT_MS (1000)
#endif

#ifndef CONFIG_ESP32_BUTTON_QUEUE_SIZE
#define CONFIG_ESP32_BUTTON_QUEUE_SIZE (4)
#endif

#ifndef CONFIG_ESP32_BUTTON_TASK_STACK_SIZE
#define CONFIG_ESP32_BUTTON_TASK_STACK_SIZE 3072
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_BIT(x) (1ULL<<x)
#define BUTTON_DOWN (1)
#define BUTTON_UP (2)
#define BUTTON_HELD (3)

typedef struct {
    uint8_t pin;
    uint8_t event;
} button_event_t;

class ButtonTask {
    public:
        QueueHandle_t button_init(unsigned long long pin_select);
        QueueHandle_t pulled_button_init(unsigned long long pin_select, gpio_pull_mode_t pull_mode);
};

#ifdef __cplusplus
}
#endif

#endif

