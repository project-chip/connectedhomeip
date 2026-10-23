/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

extern "C" {
#include <bl_irq.h>
#include <bl_pds.h>
#include <bl_psram.h>
#include <bl_sec.h>
#include <bl_uart.h>
#include <hal_pds.h>
#include <hosal_gpio.h>
#include <hosal_uart.h>

#include <btble_lib_api.h>
#include <btble_pds.h>

#include <lmac154.h>
#include <lmac154_lp.h>
#include <mboard.h>
#include <openthread/thread.h>
#include <openthread_port.h>
#include <ot_radio_trx.h>
#include <zb_timer.h>
}

#define PDS_TOLERANCE_TIME_32768CYCLE (164)
#define PDS_MIN_TIME_32768CYCLE (PDS_WARMUP_LATENCY_CNT + 33)
#define PDS_SLEEP_MAX_MS 3600000

btble_app_conf_t app_pds_conf = {
    .print_enable     = 0, // 1: enable uart print in library; 0: disable uart print in library
    .gpio_irq_restore = 1, // 1: restore gpio irq after pds wakeup; 0: do not restore gpio irq after pds wakeup
    .gpio_num         = 2, // 3,
    .gpio_index       = { CHIP_RESET_PIN, CHIP_CONTACT_PIN },
    .pull_type        = { INPUT_PULL_DOWN, INPUT_PULL_DOWN },
    .trigger_type     = { HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE, HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE },
};

static uint32_t low_power_pds_lmac154_backup[72];
static hosal_gpio_dev_t gpio_key              = { .port = CHIP_RESET_PIN, .config = INPUT_PULL_DOWN, .priv = NULL };
static hosal_gpio_dev_t gpio_contact          = { .port = CHIP_CONTACT_PIN, .config = INPUT_PULL_DOWN, .priv = NULL };
static void (*app_pds_irq_handler)(int, bool) = NULL;
static int app_pds_wakeup_source              = -1;
static uint32_t app_pds_wakeup_pin            = -1;

extern "C" void btble_pds_fastboot_done_callback(void);
#if CHIP_DETAIL_LOGGING
static bool app_pds_is_wakup_from_sleep = false;
#endif

static void gpio_isr(void * arg)
{
    uint8_t value           = 0;
    hosal_gpio_dev_t * gpio = (hosal_gpio_dev_t *) arg;

    if (app_pds_irq_handler)
    {
        hosal_gpio_input_get(gpio, &value);
        app_pds_irq_handler(gpio->port, value);
    }
}

static inline uint32_t app_pds_get_lmac154_symbol_counter(void)
{
    constexpr uint32_t kRtcFrequencyBits = 15;
    constexpr uint32_t kRtcCounterMask   = (1UL << kRtcFrequencyBits) - 1;
    constexpr uint32_t kSymbolsPerSecond = 1000000UL >> LMAC154_US_PER_SYMBOL_BITS;

    uint64_t rtcCounter = bl_rtc_get_counter();
    uint32_t seconds    = static_cast<uint32_t>(rtcCounter >> kRtcFrequencyBits);
    uint32_t remainder  = static_cast<uint32_t>(rtcCounter & kRtcCounterMask);

    return (seconds * kSymbolsPerSecond) +
        (((remainder * kSymbolsPerSecond) + (1UL << (kRtcFrequencyBits - 1))) >> kRtcFrequencyBits);
}

static inline uint32_t app_pds_zb_timer_ticks_to_ms(uint32_t ticks)
{
    return (ticks * 16 + 999) / 1000;
}

static inline TickType_t app_pds_get_sleep_time(TickType_t xExpectedIdleTime)
{
    uint32_t nextTimerMs = 0;
    uint32_t remainingTicks;
    uint32_t remainingMs;

    remainingTicks = zb_timer_get_remaining_time(ZB_TIMER_THREAD_MS);
    if (remainingTicks != 0)
    {
        nextTimerMs = app_pds_zb_timer_ticks_to_ms(remainingTicks);
    }

    remainingTicks = zb_timer_get_remaining_time(ZB_TIMER_THREAD_US);
    if (remainingTicks != 0)
    {
        remainingMs = app_pds_zb_timer_ticks_to_ms(remainingTicks);
        nextTimerMs = (nextTimerMs == 0 || remainingMs < nextTimerMs) ? remainingMs : nextTimerMs;
    }

    TickType_t nextTimerTicks = pdMS_TO_TICKS(nextTimerMs);

    if (nextTimerTicks != 0 && nextTimerTicks < xExpectedIdleTime)
    {
        return nextTimerTicks;
    }

    return xExpectedIdleTime;
}

extern "C" void vApplicationSleep(TickType_t xExpectedIdleTime)
{
#if CHIP_DETAIL_LOGGING
    uint64_t sleep_before       = bl_rtc_get_timestamp_ms();
    app_pds_is_wakup_from_sleep = false;
#endif

    TickType_t sleepTime = app_pds_get_sleep_time(xExpectedIdleTime);
    btble_vApplicationSleepExt(sleepTime);

    extern BaseType_t TrapNetCounter, *pTrapNetCounter;
    if (app_pds_wakeup_source == PDS_WAKEUP_BY_RTC)
    {
    }
    else if (app_pds_wakeup_source == PDS_WAKEUP_BY_GPIO)
    {
        if (((1 << CHIP_RESET_PIN) & app_pds_wakeup_pin))
        {
            gpio_isr(&gpio_key);
        }

        if (((1 << CHIP_CONTACT_PIN) & app_pds_wakeup_pin))
        {
            gpio_isr(&gpio_contact);
        }
    }

#if CHIP_DETAIL_LOGGING
    if (app_pds_is_wakup_from_sleep)
    {
        uint32_t sleep_time = static_cast<unsigned long>(bl_rtc_get_timestamp_ms() - sleep_before);
        ChipLogDetail(NotSpecified, "app sleep: %lu, %lu", static_cast<unsigned long>(sleepTime), sleep_time);
    }
#endif

    app_pds_wakeup_source = -1;
    app_pds_wakeup_pin    = -1;
}

void app_pds_config_pin(void)
{
    uint8_t wakeup_pin[] = { gpio_key.port, gpio_contact.port };

    hosal_gpio_init(&gpio_key);
    hosal_gpio_init(&gpio_contact);
    hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE, gpio_isr, &gpio_key);
    hosal_gpio_irq_set(&gpio_contact, HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE, gpio_isr, &gpio_contact);

    bl_pds_gpio_wakeup_cfg(wakeup_pin, sizeof(wakeup_pin) / sizeof(wakeup_pin[0]), PDS_GPIO_EDGE_BOTH);
}

void app_pds_fastboot_done_callback(void)
{
#if CHIP_PROGRESS_LOGGING || CHIP_DETAIL_LOGGING || CHIP_ERROR_LOGGING
    extern hosal_uart_dev_t uart_stdio;
    bl_uart_init(uart_stdio.config.uart_id, uart_stdio.config.tx_pin, uart_stdio.config.rx_pin, uart_stdio.config.cts_pin,
                 uart_stdio.config.rts_pin, uart_stdio.config.baud_rate);
#endif

    btble_pds_fastboot_done_callback();

    bl_psram_init();

    app_pds_wakeup_source = bl_pds_get_wakeup_source();
    app_pds_wakeup_pin    = bl_pds_get_wakeup_gpio();
}

static inline bool app_pds_is_thread_stack_idle(void)
{
    bool is_trx_frames_empty = false;
    otRadioState radioState  = OT_RADIO_STATE_DISABLED;

    if (ot_radio_ctx.instance == nullptr)
    {
        return true;
    }

    uint32_t tag        = otrEnterCrit();
    is_trx_frames_empty = (utils_dlist_empty(&ot_radio_ctx.rx_frame_list)) && (ot_radio_ctx.tx_frame == NULL);
    if (otThreadGetDeviceRole(ot_radio_ctx.instance) != OT_DEVICE_ROLE_DISABLED)
    {
        radioState = otPlatRadioGetState(ot_radio_ctx.instance);
    }
    otrExitCrit(tag);

    if (is_trx_frames_empty && (radioState == OT_RADIO_STATE_DISABLED || radioState == OT_RADIO_STATE_SLEEP))
    {
        return true;
    }

    return false;
}

int app_pds_before_sleep_callback(void)
{
    if (app_pds_is_thread_stack_idle())
    {
        bl_pds_set_psram_retention(1);
        zb_timer_store();
        lmac154_sleepStoreRegs(low_power_pds_lmac154_backup);
        L1C_Cache_Flush();

        return 0;
    }

    return -1;
}

void app_pds_after_sleep_callback(void)
{
    if (lmac154_isDisabled())
    {
        lmac154_sleepRestoreRegs(low_power_pds_lmac154_backup);
        lmac154_disableRx();

        zb_timer_cfg(app_pds_get_lmac154_symbol_counter());

        zb_timer_restore_events(true);

        lmac154_enableCoex();
        bl_irq_enable(M154_IRQn);
    }
    bl_sec_init();

#if CHIP_DETAIL_LOGGING
    app_pds_is_wakup_from_sleep = true;
#endif
}

void app_pds_init(void (*pinHandler)(int, bool))
{
    btble_pds_init(&app_pds_conf);

    btble_set_before_sleep_callback(app_pds_before_sleep_callback);
    btble_set_after_sleep_callback(app_pds_after_sleep_callback);

    bl_pds_register_fastboot_done_callback(app_pds_fastboot_done_callback);

    app_pds_irq_handler = pinHandler;
    app_pds_config_pin();

    btble_pds_enable(1);
}
