/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <FreeRTOS.h>
#include <portable.h>
#include <portmacro.h>
#include <task.h>

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <bl_dma.h>
#include <bl_irq.h>
#include <bl_rtc.h>
#include <bl_sec.h>
#include <bl_sys.h>
#include <bl_uart.h>
#include <blog.h>
#include <easyflash.h>
#include <event_device.h>
#include <hal_board.h>
#include <hal_boot2.h>
#include <hal_button.h>
#include <hal_gpio.h>
#include <hal_sys.h>
#include <hosal_pwm.h>
#include <wifi_mgmr_ext.h>

#define IOT_DVK_3S 1

#if BOARD_ID == IOT_DVK_3S
hosal_pwm_dev_t pwmR = { .port = 0xff };
#else
hosal_pwm_dev_t pwmR, pwmG, pwmB;
#endif

static void (*Button_FactoryResetEventHandler)(void)   = nullptr;
static void (*Button_LightingActionEventHandler)(void) = nullptr;
void Button_Configure_FactoryResetEventHandler(void (*callback)(void))
{
    Button_FactoryResetEventHandler = callback;
}

void Button_Configure_LightingActionEventHandler(void (*callback)(void))
{
    Button_LightingActionEventHandler = callback;
}

static void Platform_Light_Init(void)
{
#if BOARD_ID == IOT_DVK_3S
    if (pwmR.port == 0xff)
    {
        pwmR.port              = 0;
        pwmR.config.pin        = 0;
        pwmR.config.duty_cycle = 0; // duty_cycle range is 0~10000 correspond to 0~100%
        pwmR.config.freq       = 1000;
        hosal_pwm_init(&pwmR);
        vTaskDelay(50);
        hosal_pwm_start(&pwmR);
    }
#else
    pwmR.port              = 0;
    pwmR.config.pin        = 20;
    pwmR.config.duty_cycle = 0; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmR.config.freq       = 1000;
    hosal_pwm_init(&pwmR);

    pwmG.port              = 1;
    pwmG.config.pin        = 21;
    pwmG.config.duty_cycle = 0; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmG.config.freq       = 1000;
    hosal_pwm_init(&pwmG);

    pwmB.port              = 2;
    pwmB.config.pin        = 17;
    pwmB.config.duty_cycle = 0; // duty_cycle range is 0~10000 correspond to 0~100%
    pwmB.config.freq       = 1000;
    hosal_pwm_init(&pwmB);

    vTaskDelay(50);
    hosal_pwm_start(&pwmR);
    hosal_pwm_start(&pwmG);
    hosal_pwm_start(&pwmB);
#endif
}

void BL602_LightState_Update(uint8_t red, uint8_t green, uint8_t blue)
{
#if BOARD_ID == IOT_DVK_3S
    uint32_t level = (red * 10000) / UINT8_MAX;
    log_info("red level: %d\r\n", level);
    hosal_pwm_duty_set(&pwmR, level);
#else
    uint32_t r_level = (red * 10000) / UINT8_MAX;
    uint32_t g_level = (green * 10000) / UINT8_MAX;
    uint32_t b_level = (blue * 10000) / UINT8_MAX;
    hosal_pwm_duty_set(&pwmR, r_level);
    hosal_pwm_duty_set(&pwmG, g_level);
    hosal_pwm_duty_set(&pwmB, b_level);
#endif
}
void user_vAssertCalled(void) __attribute__((weak, alias("vAssertCalled")));
void vAssertCalled(void)
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
    while (ulSetTo1ToExitFunction != 1)
    {
        __asm volatile("NOP");
    }
}

void vApplicationMallocFailedHook(void)
{
    printf("Memory Allocate Failed. Current left size is %d bytes\r\n", xPortGetFreeHeapSize());
    while (true)
    {
        /*empty here*/
    }
}

void vApplicationIdleHook(void)
{
    __asm volatile("   wfi     ");
    /*empty*/
}

#if (configUSE_TICKLESS_IDLE != 0)
void vApplicationSleep(TickType_t xExpectedIdleTime_ms)
{
#if defined(CFG_BLE_PDS)
    int32_t bleSleepDuration_32768cycles = 0;
    int32_t expectedIdleTime_32768cycles = 0;
    eSleepModeStatus eSleepStatus;
    bool freertos_max_idle = false;

    if (pds_start == 0)
        return;

    if (xExpectedIdleTime_ms + xTaskGetTickCount() == portMAX_DELAY)
    {
        freertos_max_idle = true;
    }
    else
    {
        xExpectedIdleTime_ms -= 1;
        expectedIdleTime_32768cycles = 32768 * xExpectedIdleTime_ms / 1000;
    }

    if ((!freertos_max_idle) && (expectedIdleTime_32768cycles < TIME_5MS_IN_32768CYCLE))
    {
        return;
    }

    /*Disable mtimer interrrupt*/
    *(volatile uint8_t *) configCLIC_TIMER_ENABLE_ADDRESS = 0;

    eSleepStatus = eTaskConfirmSleepModeStatus();
    if (eSleepStatus == eAbortSleep || ble_controller_sleep_is_ongoing())
    {
        /*A task has been moved out of the Blocked state since this macro was
        executed, or a context siwth is being held pending.Restart the tick
        and exit the critical section. */
        /*Enable mtimer interrrupt*/
        *(volatile uint8_t *) configCLIC_TIMER_ENABLE_ADDRESS = 1;
        // printf("%s:not do ble sleep\r\n", __func__);
        return;
    }

    bleSleepDuration_32768cycles = ble_controller_sleep();

    if (bleSleepDuration_32768cycles < TIME_5MS_IN_32768CYCLE)
    {
        /*BLE controller does not allow sleep.  Do not enter a sleep state.Restart the tick
        and exit the critical section. */
        /*Enable mtimer interrrupt*/
        // printf("%s:not do pds sleep\r\n", __func__);
        *(volatile uint8_t *) configCLIC_TIMER_ENABLE_ADDRESS = 1;
    }
    else
    {
        printf("%s:bleSleepDuration_32768cycles=%ld\r\n", __func__, bleSleepDuration_32768cycles);
        if (eSleepStatus == eStandardSleep &&
            ((!freertos_max_idle) && (expectedIdleTime_32768cycles < bleSleepDuration_32768cycles)))
        {
            hal_pds_enter_with_time_compensation(1, expectedIdleTime_32768cycles - 40); // 40);//20);
        }
        else
        {
            hal_pds_enter_with_time_compensation(1, bleSleepDuration_32768cycles - 40); // 40);//20);
        }
    }
#endif
}
#endif

void vApplicationGetIdleTaskMemory(StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer,
                                   uint32_t * pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    // static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
    static StackType_t uxIdleTaskStack[512];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    //*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    *pulIdleTaskStackSize = 512; // size 512 words is For ble pds mode, otherwise stack overflow of idle task will happen.
}

static void event_cb_key_event(input_event_t * event, void * private_data)
{
    switch (event->code)
    {
    case KEY_1: {
        log_info("[KEY_1] [EVT] INIT DONE %lld\r\n", aos_now_ms());
        log_info("short press \r\n");

        if (Button_LightingActionEventHandler != nullptr)
        {
            (*Button_LightingActionEventHandler)();
        }
    }
    break;
    case KEY_2: {
        log_info("[KEY_2] [EVT] INIT DONE %lld\r\n", aos_now_ms());
        log_info("long press \r\n");
    }
    break;
    case KEY_3: {
        log_info("[KEY_3] [EVT] INIT DONE %lld\r\n", aos_now_ms());
        log_info("longlong press \r\n");
        if (Button_FactoryResetEventHandler != nullptr)
        {
            (*Button_FactoryResetEventHandler)();
        }
    }
    break;
    default: {
        log_info("[KEY] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
        /*nothing*/
    }
    }
}

extern int get_dts_addr(const char * name, uint32_t * start, uint32_t * off);

void InitPlatform(void)
{
    bl_sys_init();

    hal_button_module_init(8, 1000, 4800, 5000);
    Platform_Light_Init();
    aos_register_event_filter(EV_KEY, event_cb_key_event, NULL);
    wifi_td_diagnosis_init();
}

#ifdef __cplusplus
}
#endif
