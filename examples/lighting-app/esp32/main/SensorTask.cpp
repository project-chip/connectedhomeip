#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "SensorTask.h"
#include "AppTask.h"

/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output (ESP32C2/ESP32H2 uses GPIO8 as the second output pin)
 * GPIO19: output (ESP32C2/ESP32H2 uses GPIO9 as the second output pin)
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Note. These are the default GPIO pins to be used in the example. You can
 * change IO pins in menuconfig.
 *
 * Test:
 * Connect GPIO18(8) with GPIO4
 * Connect GPIO19(9) with GPIO5
 * Generate pulses on GPIO18(8)/19(9), that triggers interrupt on GPIO4/5
 *
 */

#define GPIO_INPUT_IO_0     ((gpio_num_t) 5)
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)
/*
 * Let's say, GPIO_INPUT_IO_0=4, GPIO_INPUT_IO_1=5
 * In binary representation,
 * 1ULL<<GPIO_INPUT_IO_0 is equal to 0000000000000000000000000000000000010000 and
 * 1ULL<<GPIO_INPUT_IO_1 is equal to 0000000000000000000000000000000000100000
 * GPIO_INPUT_PIN_SEL                0000000000000000000000000000000000110000
 * */

#define SENSOR_TASK_NAME "SensorTask"
#define SENSOR_EVENT_QUEUE_SIZE 10
#define SENSOR_TASK_STACK_SIZE (2048)
#define SENSOR_CHECK_DELAY_MS (100)
#define MAX_TIME_TO_WAIT_MS (60000)
#define INITIAL_DELAY_MS (2000)

static const char * TAG = "sensor-task";

namespace {
    TaskHandle_t sSensorTaskHandle;
} // namespace

SensorTask SensorTask::sSensorTask;
TimerHandle_t SensorTask::sDetectionTimer;
bool SensorTask::sForcedPresenceUpdate;

CHIP_ERROR GpioInit(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)1;
    gpio_config(&io_conf);

    return CHIP_NO_ERROR;
}

static void maxDetectionTimerExpired(TimerHandle_t pxTimer) {
    SensorTask::sForcedPresenceUpdate = true;
}

void SendUpdatedPresenceToAppTask(bool personDetected)
{
    AppEvent personDetectedEvent = {};
    personDetectedEvent.Type = AppEvent::kEventType_PersonDetected;
    personDetectedEvent.PersonDetectedEvent.PersonDetected = personDetected;
    personDetectedEvent.mHandler = AppTask::PersonDetectedEventHandler;

    GetAppTask().PostEvent(&personDetectedEvent);
}

uint32_t timeInMillis() {
    return esp_timer_get_time() / 1000;
}

void SensorTask::SensorTaskMain(void * pvParameter)
{
    CHIP_ERROR err = GpioInit();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "SensorTask.Init() failed due to %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    bool personDetected = false;
    xTimerStart(sDetectionTimer, 0);
    vTaskDelay(pdMS_TO_TICKS(INITIAL_DELAY_MS));

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(SENSOR_CHECK_DELAY_MS));
        if ((personDetected != gpio_get_level(GPIO_INPUT_IO_0)) || sForcedPresenceUpdate) {
            personDetected = gpio_get_level(GPIO_INPUT_IO_0);
            ESP_LOGI(TAG, "Presence detection: %s, is forced %d", personDetected ? "true" : "false", sForcedPresenceUpdate);
            SendUpdatedPresenceToAppTask(personDetected);
            sForcedPresenceUpdate = false;
            xTimerStart(sDetectionTimer, 0);
        }
    }
}

#define MAX_DETECTION_UPDATE_TIMER "max_detection_update_timer"

CHIP_ERROR SensorTask::StartSensorTask()
{
    // Start Sensor task
    BaseType_t xReturned;
    sDetectionTimer = xTimerCreate(MAX_DETECTION_UPDATE_TIMER, pdMS_TO_TICKS(MAX_TIME_TO_WAIT_MS), pdFALSE, (void*)0, maxDetectionTimerExpired);
    xReturned = xTaskCreate(SensorTaskMain, SENSOR_TASK_NAME, SENSOR_TASK_STACK_SIZE, NULL, 1, &sSensorTaskHandle);
    return (xReturned == pdPASS) ? CHIP_NO_ERROR : SENSOR_ERROR_CREATE_TASK_FAILED;
}

