
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
static hosal_gpio_dev_t gpio_key                    = { .port = CHIP_RESET_PIN, .config = INPUT_PULL_DOWN, .priv = NULL };
static hosal_gpio_dev_t gpio_contact                = { .port = CHIP_CONTACT_PIN, .config = INPUT_PULL_DOWN, .priv = NULL };
static hosal_gpio_irq_handler_t app_pds_irq_handler = NULL;
static int app_pds_wakeup_source                    = -1;
static uint32_t app_pds_wakeup_pin                  = -1;

extern "C" void btble_pds_fastboot_done_callback(void);

uint64_t wakeup_time        = 0;
uint64_t sleep_calling_time = 0;
uint64_t sleep_time         = 0;

extern "C" void vApplicationSleep(TickType_t xExpectedIdleTime)
{
    uint64_t sleep_before = bl_rtc_get_timestamp_ms();

    sleep_calling_time = bl_rtc_get_timestamp_ms();

    btble_vApplicationSleepExt(xExpectedIdleTime);

    extern BaseType_t TrapNetCounter, *pTrapNetCounter;
    if (app_pds_wakeup_source == PDS_WAKEUP_BY_RTC)
    {
    }
    else if (app_pds_wakeup_source == PDS_WAKEUP_BY_GPIO)
    {
        if (((1 << CHIP_RESET_PIN) & app_pds_wakeup_pin) && app_pds_irq_handler)
        {
            app_pds_irq_handler(&gpio_key);
        }

        if (((1 << CHIP_CONTACT_PIN) & app_pds_wakeup_pin) && app_pds_irq_handler)
        {
            app_pds_irq_handler(&gpio_contact);
        }
    }

    app_pds_wakeup_source = -1;
    app_pds_wakeup_pin    = -1;
}

void app_pds_config_pin(void)
{
    uint8_t wakeup_pin[] = { gpio_key.port, gpio_contact.port };

    hosal_gpio_init(&gpio_key);
    hosal_gpio_init(&gpio_contact);
    hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE, app_pds_irq_handler, &gpio_key);
    hosal_gpio_irq_set(&gpio_contact, HOSAL_IRQ_TRIG_SYNC_FALLING_RISING_EDGE, app_pds_irq_handler, &gpio_contact);

    bl_pds_gpio_wakeup_cfg(wakeup_pin, sizeof(wakeup_pin) / sizeof(wakeup_pin[0]), PDS_GPIO_EDGE_BOTH);
}

void app_pds_fastboot_done_callback(void)
{
    wakeup_time = bl_rtc_get_timestamp_ms();

#if CHIP_PROGRESS_LOGGING || CHIP_ERROR_LOGGING
    extern hosal_uart_dev_t uart_stdio;
    bl_uart_init(uart_stdio.config.uart_id, uart_stdio.config.tx_pin, uart_stdio.config.rx_pin, uart_stdio.config.cts_pin,
                 uart_stdio.config.rts_pin, uart_stdio.config.baud_rate);
#endif

    btble_pds_fastboot_done_callback();

    bl_psram_init();

    // app_pds_config_pin();

    app_pds_wakeup_source = bl_pds_get_wakeup_source();
    app_pds_wakeup_pin    = bl_pds_get_wakeup_gpio();
}

int app_pds_before_sleep_callback(void)
{
    if (otr_isStackIdle())
    {

        bl_pds_set_psram_retention(1);
        lmac154_sleepStoreRegs(low_power_pds_lmac154_backup);

        sleep_time = bl_rtc_get_timestamp_ms();

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

        zb_timer_cfg(bl_rtc_get_counter() * (32768 >> LMAC154_US_PER_SYMBOL_BITS));

        zb_timer_restore_events(true);

        bl_irq_enable(M154_IRQn);
    }
    bl_sec_init();
}

void app_pds_init(hosal_gpio_irq_handler_t pinHandler)
{
    btble_pds_init(&app_pds_conf);

    btble_set_before_sleep_callback(app_pds_before_sleep_callback);
    btble_set_after_sleep_callback(app_pds_after_sleep_callback);

    bl_pds_register_fastboot_done_callback(app_pds_fastboot_done_callback);

    app_pds_irq_handler = pinHandler;
    app_pds_config_pin();

    btble_pds_enable(1);
}
