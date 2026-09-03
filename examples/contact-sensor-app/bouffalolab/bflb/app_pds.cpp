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

#include <lwip/ip4_addr.h>
#include <lwip/inet_chksum.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <lwip/prot/ip4.h>
#include <lwip/prot/ip6.h>

#include "app_pds.h"

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

namespace {

constexpr uint8_t kActiveDtim              = 1;
constexpr uint8_t kIdleDtim                = 10;
constexpr uint32_t kDtimActivityDurationMs = 3000;
constexpr uint32_t kDtimActivityNotification = 1U;

StaticTask_t sDtimActivityTaskStorage;
StackType_t sDtimActivityTaskStack[configMINIMAL_STACK_SIZE];
TaskHandle_t sDtimActivityTaskHandle = nullptr;
uint8_t sCurrentDtim                 = 0;

void SetDtim(uint8_t dtim)
{
    if (sCurrentDtim == dtim)
    {
        return;
    }

    set_dtim_config(dtim);
    bl_lp_fw_bcn_loss_cfg_dtim_default(dtim);
    sCurrentDtim = dtim;
    ChipLogDetail(DeviceLayer, "[LP] DTIM switched to %u", dtim);
}

void DtimActivityTask(void * arg)
{
    (void) arg;

    bool dtimActive = true;
    for (;;)
    {
        uint32_t notificationValue = 0;
        const TickType_t waitTime  = dtimActive ? pdMS_TO_TICKS(kDtimActivityDurationMs) : portMAX_DELAY;
        BaseType_t notificationReceived =
            xTaskNotifyWait(0, kDtimActivityNotification, &notificationValue, waitTime);

        if (notificationReceived == pdTRUE && (notificationValue & kDtimActivityNotification) != 0)
        {
            SetDtim(kActiveDtim);
            dtimActive = true;
        }
        else
        {
            SetDtim(kIdleDtim);
            dtimActive = false;
        }
    }
}

} // namespace

static void app_lp_config_gpio(void);
static void app_lp_config_wakup_gpio(void);

void app_dtim_activity_notify(void)
{
    if (sDtimActivityTaskHandle == nullptr)
    {
        return;
    }

    BaseType_t notifyResult;
    if (xPortIsInsideInterrupt())
    {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        notifyResult = xTaskNotifyFromISR(sDtimActivityTaskHandle, kDtimActivityNotification, eSetBits,
                                          &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
    else
    {
        notifyResult = xTaskNotify(sDtimActivityTaskHandle, kDtimActivityNotification, eSetBits);
    }

    if (notifyResult != pdPASS)
    {
        ChipLogError(DeviceLayer, "[LP] Failed to notify DTIM activity task");
    }
}

bool IsLocalIPv4Destination(const ip4_addr_t * destination, const struct netif * input_netif)
{
    return netif_is_up(input_netif) && !ip4_addr_isany(netif_ip4_addr(input_netif)) &&
        ip4_addr_cmp(destination, netif_ip4_addr(input_netif));
}

extern "C" int app_dtim_ip4_input(struct pbuf * pbuf, struct netif * input_netif)
{
    if (pbuf != nullptr && input_netif != nullptr && pbuf->payload != nullptr && pbuf->len >= IP_HLEN &&
        pbuf->tot_len >= IP_HLEN)
    {
        const auto * header = static_cast<const ip_hdr *>(pbuf->payload);
        const uint16_t headerLength = IPH_HL_BYTES(header);
        const uint16_t packetLength = lwip_ntohs(IPH_LEN(header));

        if (headerLength >= IP_HLEN && headerLength <= pbuf->len && packetLength >= headerLength &&
            packetLength <= pbuf->tot_len)
        {
#if CHECKSUM_CHECK_IP
            IF__NETIF_CHECKSUM_ENABLED(input_netif, NETIF_CHECKSUM_CHECK_IP)
            {
                if (inet_chksum(header, headerLength) != 0)
                {
                    return 0;
                }
            }
#endif

            ip4_addr_t destination;
            memcpy(&destination.addr, &header->dest.addr, sizeof(destination.addr));
            if (IsLocalIPv4Destination(&destination, input_netif))
            {
                app_dtim_activity_notify();
            }
        }
    }

    return 0;
}

extern "C" int app_dtim_ip6_input(struct pbuf * pbuf, struct netif * input_netif)
{
    if (pbuf != nullptr && input_netif != nullptr && pbuf->payload != nullptr && pbuf->len >= IP6_HLEN &&
        pbuf->tot_len >= IP6_HLEN)
    {
        const auto * header          = static_cast<const ip6_hdr *>(pbuf->payload);
        const uint16_t payloadLength = lwip_ntohs(IP6H_PLEN(header));
        if (payloadLength > pbuf->tot_len - IP6_HLEN)
        {
            return 0;
        }

        ip6_addr_t destination;
        ip6_addr_t source;
        ip6_addr_copy_from_packed(destination, header->dest);
        ip6_addr_copy_from_packed(source, header->src);
        if (!ip6_addr_isipv4mappedipv6(&source) && !ip6_addr_ismulticast(&source) && !ip6_addr_isany(&source) &&
            !ip6_addr_ismulticast(&destination) && netif_is_up(input_netif) &&
            netif_get_ip6_addr_match(input_netif, &destination) >= 0)
        {
            app_dtim_activity_notify();
        }
    }

    return 0;
}

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

    sDtimActivityTaskHandle = xTaskCreateStatic(DtimActivityTask, "DtimActivity", configMINIMAL_STACK_SIZE, nullptr,
                                                configMAX_PRIORITIES - 2, sDtimActivityTaskStack,
                                                &sDtimActivityTaskStorage);
    if (sDtimActivityTaskHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "[LP] Failed to create DTIM activity task");
    }
    app_dtim_activity_notify();

    app_clock_init();

    app_lp_config_gpio();

    app_lp_config_wakup_gpio();
    bl_lp_wakeup_io_int_register(wakeup_io_callback);
}
