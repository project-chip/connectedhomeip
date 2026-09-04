/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * BL616 WiFi+BLE low-power support for contact-sensor-app.
 *
 * Mirrors the pattern from bouffalo_sdk/examples/pmu/wl_ble_lp:
 *   - app_pre_matter_init() : initialises LP hardware before Matter can use
 *                      the SEC SHA engine.
 *   - app_pds_init() : called by AppTask.cpp, registers GPIO wakeup handling.
 *   - lp_exit()      : restores clocks (set_cpu_bclk_80M_and_gate_clk),
 *                      calls board_recovery(), re-enables FreeRTOS tick and
 *                      UART shell IRQ, then dispatches on wake reason.
 *   - bflb_pm_app_check() : required hook called by PM framework; checks
 *                      pbuf chain activity via pm_pbufc_check().
 *   - GPIO handling  : regular bflb_gpio IRQ for active (non-PDS) operation;
 *                      bl_lp_io_wakeup_cfg + bl_lp_wakeup_io_int_register for
 *                      wakeup from PDS.
 *   - hosal_gpio_input_get() : compat shim for AppTask.cpp (iot_sdk API),
 *                      implemented via bflb_gpio_read().
 *
 * vApplicationSleep / tickless_enter / tickless_exit are provided by
 * components/os/power_mgmt/tickless.c (linked via _power_mgmt source_set).
 */

#include <lib/support/logging/CHIPLogging.h>

#include <FreeRTOS.h>
#include <task.h>

extern "C" {
#if defined(BL616CL)
#include <bl616cl_glb.h>
#include <bl616cl_hbn.h>
#include <bl616cl_pm.h>
#else
#include <bl616_glb.h>
#include <bl616_hbn.h>
#include <bl616_pm.h>
#endif
#include <bflb_clock.h>
#include <bflb_gpio.h>
#include <bflb_irq.h>
#include <bflb_rtc.h>
#include <bflb_sec_sha.h>
#include <bflb_uart.h>
#include <bl_lp.h>
#include <board.h>
#include <clock_manager.h>
#include <easyflash.h>
#include <pm_manager.h>
}

#include "mboard.h"

static struct bflb_device_s * s_gpio_dev;
static bl_lp_io_cfg_t s_io_wakeup_cfg;

static void (*s_pin_handler)(int, bool) = NULL;
#if CHIP_DETAIL_LOGGING
static struct bflb_device_s * s_rtc_dev = NULL;
static uint64_t s_sleep_enter_rtc       = 0;
#endif
static struct bflb_device_s * s_sha_dev = NULL;

static void app_lp_config_gpio(void);
static void app_lp_config_wakup_gpio(void);

/* -------------------------------------------------------------------------- */
/* GPIO interrupt handlers                                                    */
/* -------------------------------------------------------------------------- */
static void gpio_isr(uint8_t pin)
{
    if (s_pin_handler)
    {
        s_pin_handler(pin, bflb_gpio_read(s_gpio_dev, pin) > 0);
    }
}

/* -------------------------------------------------------------------------- */
/* LP IO wakeup callback (fires after PDS wakeup via GPIO)                    */
/* -------------------------------------------------------------------------- */
static void wakeup_io_callback(uint64_t wake_io_bits)
{
    if (s_pin_handler)
    {
        if (wake_io_bits & (1 << CHIP_RESET_PIN))
        {
            s_pin_handler(CHIP_RESET_PIN, bl_lp_wakeup_io_get_mode(CHIP_RESET_PIN) == BL_LP_IO_WAKEUP_MODE_RISING);
        }

        if (wake_io_bits & (1 << CHIP_CONTACT_PIN))
        {
            s_pin_handler(CHIP_CONTACT_PIN, bl_lp_wakeup_io_get_mode(CHIP_CONTACT_PIN) == BL_LP_IO_WAKEUP_MODE_RISING);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* LPFW clock / peripheral restore on wakeup                                  */
/* -------------------------------------------------------------------------- */

static void set_cpu_bclk_80M_and_gate_clk(void)
{
    uint32_t tmpVal = 0;

    GLB_Set_MCU_System_CLK_Div(0, 3);
    CPU_Set_MTimer_CLK(ENABLE, BL_MTIMER_SOURCE_CLOCK_MCU_CLK, Clock_System_Clock_Get(BL_SYSTEM_CLOCK_MCU_CLK) / 1000000 - 1);

    /* Keep clocks for: CPU, DMA, SEC, SDU */
    tmpVal = 0;
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_M_CPU, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_M_DMA, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_M_SEC, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_M_SDU, 1);
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG0, tmpVal);

    tmpVal = 0;
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_EF_CTRL, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_SF_CTRL, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_DMA, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1A_UART0, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1A_UART1, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_SEC_ENG, 1);
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG1, tmpVal);

    tmpVal = 0;
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S2_WIFI, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_EXT_EMI_MISC, 1);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, GLB_CGEN_S1_EXT_PIO, 1);
    BL_WR_REG(GLB_BASE, GLB_CGEN_CFG2, tmpVal);
}

/* -------------------------------------------------------------------------- */
/* LPFW system callbacks                                                       */
/* -------------------------------------------------------------------------- */

extern "C" void vPortSetupTimerInterrupt(void);

static int lp_enter(void * arg)
{
    (void) arg;
#if CHIP_DETAIL_LOGGING
    if (s_rtc_dev)
    {
        s_sleep_enter_rtc = bflb_rtc_get_time(s_rtc_dev);
    }
#endif
    app_lp_config_wakup_gpio();
    return 0;
}

static int lp_exit(void * arg)
{
    (void) arg;
    extern TaskHandle_t rxl_process_task_hd;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    set_cpu_bclk_80M_and_gate_clk();
    board_recovery();

    if (s_sha_dev)
    {
        bflb_group0_request_sha_access(s_sha_dev);
        bflb_sha_link_init(s_sha_dev);
    }
    vPortSetupTimerInterrupt();

    int reason = bl_lp_get_wake_reason();
    if (reason & LPFW_WAKEUP_WIFI_BROADCAST)
    {
        vTaskNotifyGiveFromISR(rxl_process_task_hd, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        pm_alloc_mem_reset();
    }
    app_lp_config_gpio();

#if CHIP_DETAIL_LOGGING
    if (s_rtc_dev)
    {
        uint64_t sleep_ticks = bflb_rtc_get_time(s_rtc_dev) - s_sleep_enter_rtc;
        uint32_t rtc_hz      = bflb_clk_get_peripheral_clock(BFLB_DEVICE_TYPE_RTC, 0);
        uint64_t sleep_ms    = (rtc_hz != 0) ? (sleep_ticks * 1000ULL / rtc_hz) : 0;
        ChipLogDetail(NotSpecified, "[LP] wake reason=0x%x sleep=%llu ms", reason, (unsigned long long) sleep_ms);
    }
#endif

    return 0;
}

/* -------------------------------------------------------------------------- */
/* PM framework hook – called by prebuilt LP library                          */
/* -------------------------------------------------------------------------- */

extern "C" int bflb_pm_app_check(void)
{
    return pm_pbufc_check();
}

extern "C" void app_pre_matter_init(void)
{
    uint8_t soc_v;
    uint8_t rt_v;
    uint8_t aon_v;

    hal_pm_ldo11_cfg(PM_PDS_LDO_LEVEL_SOC_DEFAULT, PM_PDS_LDO_LEVEL_RT_DEFAULT, PM_PDS_LDO_LEVEL_AON_DEFAULT);
    hal_pm_ldo11_cfg_get(&soc_v, &rt_v, &aon_v);
    ChipLogProgress(NotSpecified, "[LP] LDO SOC:%d RT:%d AON:%d", soc_v, rt_v, aon_v);

    HBN_Enable_RTC_Counter();
    pm_rc32k_auto_cal_init();
    pm_sys_init();

    bl_lp_init();
    bl_lp_sys_callback_register(lp_enter, NULL, lp_exit, NULL);

    easyflash_init();
    app_set_clock_source(CLOCK_SOURCE_PASSIVE);
#if CHIP_DETAIL_LOGGING
    s_rtc_dev = bflb_device_get_by_name("rtc");
#endif
    s_gpio_dev = bflb_device_get_by_name("gpio");
    s_sha_dev  = bflb_device_get_by_name(BFLB_NAME_SEC_SHA);

    extern int enable_multicast_broadcast;
    enable_multicast_broadcast = true;
}

/* -------------------------------------------------------------------------- */
/* GPIO interrupt configuration                                                */
/* -------------------------------------------------------------------------- */

static void app_lp_config_gpio(void)
{
    if (NULL == s_gpio_dev)
    {
        return;
    }

    bflb_gpio_init(s_gpio_dev, CHIP_RESET_PIN, GPIO_INPUT | GPIO_PULLDOWN);
    bflb_gpio_init(s_gpio_dev, CHIP_CONTACT_PIN, GPIO_INPUT | GPIO_PULLDOWN);

    bflb_gpio_int_init(s_gpio_dev, CHIP_RESET_PIN, GPIO_INT_TRIG_MODE_SYNC_FALLING_RISING_EDGE);
    bflb_gpio_int_init(s_gpio_dev, CHIP_CONTACT_PIN, GPIO_INT_TRIG_MODE_SYNC_FALLING_RISING_EDGE);

    bflb_gpio_irq_attach(CHIP_RESET_PIN, gpio_isr);
    bflb_gpio_irq_attach(CHIP_CONTACT_PIN, gpio_isr);

    bflb_gpio_int_mask(s_gpio_dev, CHIP_RESET_PIN, false);
    bflb_gpio_int_mask(s_gpio_dev, CHIP_CONTACT_PIN, false);

    bflb_irq_enable(s_gpio_dev->irq_num);
}

static void app_lp_config_wakup_gpio(void)
{
    memset(&s_io_wakeup_cfg, 0, sizeof(s_io_wakeup_cfg));
    s_io_wakeup_cfg.io_wakeup_unmask = (1ULL << CHIP_RESET_PIN) | (1ULL << CHIP_CONTACT_PIN);
    s_io_wakeup_cfg.io_0_15_ie       = BL_LP_IO_INPUT_ENABLE;
    s_io_wakeup_cfg.io_20_34_ie      = BL_LP_IO_INPUT_ENABLE;
    if (bflb_gpio_read(s_gpio_dev, CHIP_RESET_PIN))
    {
        s_io_wakeup_cfg.io_0_7_pds_trig_mode = BL_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE;
    }
    else
    {
        s_io_wakeup_cfg.io_0_7_pds_trig_mode = BL_LP_PDS_IO_TRIG_SYNC_RISING_EDGE;
    }
    if (bflb_gpio_read(s_gpio_dev, CHIP_CONTACT_PIN))
    {
        s_io_wakeup_cfg.io_20_27_pds_trig_mode = BL_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE;
    }
    else
    {
        s_io_wakeup_cfg.io_20_27_pds_trig_mode = BL_LP_PDS_IO_TRIG_SYNC_RISING_EDGE;
    }
    s_io_wakeup_cfg.io_0_15_res  = BL_LP_IO_RES_PULL_DOWN;
    s_io_wakeup_cfg.io_20_34_res = BL_LP_IO_RES_PULL_DOWN;

    bl_lp_io_wakeup_cfg(&s_io_wakeup_cfg);
}

/* -------------------------------------------------------------------------- */
/* Public entry point – same interface as bl702l/app_pds.cpp                  */
/* -------------------------------------------------------------------------- */

void app_pds_init(void (*pinHandler)(int, bool))
{
    s_pin_handler = pinHandler;

    app_clock_init();

    app_lp_config_gpio();

    app_lp_config_wakup_gpio();
    bl_lp_wakeup_io_int_register(wakeup_io_callback);
}
