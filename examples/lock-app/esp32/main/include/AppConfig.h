#pragma once

#include "driver/gpio.h"

// ---- Lock Example App Config ----

#define APP_TASK_NAME "APP"
#define SYSTEM_STATE_LED GPIO_NUM_26
#define LOCK_STATE_LED GPIO_NUM_2

#define PB0 0
#define PB1 1


#define APP_LOCK_BUTTON PB1
#define APP_FUNCTION_BUTTON PB0
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000
