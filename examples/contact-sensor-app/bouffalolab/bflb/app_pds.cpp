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

#include <app/server/Server.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

#include <atomic>
#include <cstring>

#include <FreeRTOS.h>
#include <task.h>

#include "app_pds.h"
#include <plat.h>
extern "C" {
#include <lwip/def.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <lwip/prot/ethernet.h>
#include <lwip/prot/ip.h>
#include <lwip/prot/ip4.h>
#include <lwip/prot/ip6.h>
#include <lwip/prot/udp.h>
#include <wifi_pkt_hooks.h>
}

extern "C" void bflb_connectivity_manager_set_endpoint_queue_filter(void);

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
constexpr uint16_t kMdnsPort               = 5353;
constexpr ip_addr_t kMdnsIpv4Address       = IPADDR4_INIT_BYTES(224, 0, 0, 251);
constexpr ip_addr_t kMdnsIpv6Address       = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0xFB);
uint8_t sCurrentDtim                       = 0;
std::atomic<uint32_t> sDtimHoldMask{ APP_DTIM_HOLD_STARTUP };
std::atomic<uint32_t> sDtimUnicastDeadline{ 0 };
std::atomic<uint32_t> sDtimMdnsDeadline{ 0 };
std::atomic<bool> sHasIpv4Address{ false };
std::atomic<bool> sHasIpv6Address{ false };
bool sNetworkHooksInstalled = false;

uint32_t CurrentTick(void)
{
    return static_cast<uint32_t>(xTaskGetTickCount());
}
bool DeadlineActive(uint32_t now, uint32_t deadline)
{
    return deadline != 0 && static_cast<int32_t>(deadline - now) > 0;
}
bool IsMdnsPacket(const struct pbuf * packet, uint16_t etherType)
{
    constexpr u16_t kIpHeaderOffset = sizeof(struct eth_hdr);
    struct udp_hdr udpHeader;

    if (etherType == ETHTYPE_IP)
    {
        struct ip_hdr ipHeader;
        if (pbuf_copy_partial(packet, &ipHeader, sizeof(ipHeader), kIpHeaderOffset) != sizeof(ipHeader) || IPH_V(&ipHeader) != 4 ||
            IPH_PROTO(&ipHeader) != IP_PROTO_UDP)
        {
            return false;
        }

        const u8_t ipHeaderLength = IPH_HL_BYTES(&ipHeader);
        if (ipHeaderLength < sizeof(struct ip_hdr) || ipHeaderLength > IP_HLEN_MAX ||
            pbuf_copy_partial(packet, &udpHeader, sizeof(udpHeader), kIpHeaderOffset + ipHeaderLength) != sizeof(udpHeader))
        {
            return false;
        }

        return ip4_addr_cmp(&ipHeader.dest, ip_2_ip4(&kMdnsIpv4Address)) && lwip_ntohs(udpHeader.dest) == kMdnsPort;
    }

    if (etherType == ETHTYPE_IPV6)
    {
        struct ip6_hdr ipHeader;
        if (pbuf_copy_partial(packet, &ipHeader, sizeof(ipHeader), kIpHeaderOffset) != sizeof(ipHeader) || IP6H_V(&ipHeader) != 6 ||
            IP6H_NEXTH(&ipHeader) != IP6_NEXTH_UDP ||
            pbuf_copy_partial(packet, &udpHeader, sizeof(udpHeader), kIpHeaderOffset + sizeof(ipHeader)) != sizeof(udpHeader))
        {
            return false;
        }

        return ip6_addr_cmp_packed(ip_2_ip6(&kMdnsIpv6Address), &ipHeader.dest, IP6_NO_ZONE) &&
            lwip_ntohs(udpHeader.dest) == kMdnsPort;
    }

    return false;
}

bool IsUnicastDestination(const struct eth_addr & destination)
{
    return (destination.addr[0] & 0x01U) == 0;
}

extern "C" void * app_dtim_wifi_output_hook(bool isSta, void * packet, void * arg);

void InstallNetworkActivityHooks(void)
{
    if (sNetworkHooksInstalled)
    {
        return;
    }

    bflb_connectivity_manager_set_endpoint_queue_filter();
    bl_pkt_eth_output_hook_register(app_dtim_wifi_output_hook, nullptr);
    sNetworkHooksInstalled = true;
}

bool ConsumeActiveDeadline(std::atomic<uint32_t> & deadline, uint32_t now)
{
    uint32_t value = deadline.load(std::memory_order_acquire);
    for (;;)
    {
        if (DeadlineActive(now, value))
        {
            return true;
        }
        if (value == 0 || deadline.compare_exchange_weak(value, 0, std::memory_order_acq_rel))
        {
            return false;
        }
    }
}

void UpdateActivityDeadline(uint8_t kind, uint32_t now)
{
    std::atomic<uint32_t> * deadline = nullptr;
    switch (kind)
    {
    case APP_DTIM_ACTIVITY_UNICAST:
        deadline = &sDtimUnicastDeadline;
        break;
    case APP_DTIM_ACTIVITY_MDNS:
        deadline = &sDtimMdnsDeadline;
        break;
    default:
        return;
    }

    uint32_t requestedDeadline = now + pdMS_TO_TICKS(kDtimActivityDurationMs);
    if (requestedDeadline == 0)
    {
        requestedDeadline = 1;
    }
    uint32_t currentDeadline = deadline->load(std::memory_order_acquire);
    while ((currentDeadline == 0 || static_cast<int32_t>(requestedDeadline - currentDeadline) > 0) &&
           !deadline->compare_exchange_weak(currentDeadline, requestedDeadline, std::memory_order_acq_rel))
    {
    }
}

extern "C" void * app_dtim_wifi_output_hook(bool isSta, void * packet, void * arg)
{
    (void) isSta;
    (void) arg;

    const struct pbuf * packetBuffer = static_cast<const struct pbuf *>(packet);
    struct eth_hdr header;
    if (packetBuffer == nullptr || pbuf_copy_partial(packetBuffer, &header, sizeof(header), 0) != sizeof(header))
    {
        return packet;
    }

    const uint16_t etherType = lwip_ntohs(header.type);
    if (etherType != ETHTYPE_IP && etherType != ETHTYPE_IPV6)
    {
        return packet;
    }

    if (IsMdnsPacket(packetBuffer, etherType))
    {
        app_dtim_activity_notify(APP_DTIM_ACTIVITY_MDNS);
    }
    else if (IsUnicastDestination(header.dest))
    {
        app_dtim_activity_notify(APP_DTIM_ACTIVITY_UNICAST);
    }
    return packet;
}

void SetDtim(uint8_t dtim)
{
    if (sCurrentDtim == dtim)
    {
        return;
    }

    set_dtim_config(dtim);
    bl_lp_fw_bcn_loss_cfg_dtim_default(dtim);
    sCurrentDtim = dtim;
}

} // namespace

static void app_lp_config_gpio(void);
static void app_lp_config_wakup_gpio(void);

extern "C" void app_dtim_activity_notify(uint8_t kind)
{
    const uint32_t now = xPortIsInsideInterrupt() ? static_cast<uint32_t>(xTaskGetTickCountFromISR()) : CurrentTick();
    UpdateActivityDeadline(kind, now);
}

extern "C" void app_dtim_set_hold(enum app_dtim_hold_reason reason, bool hold)
{
    const uint32_t reasonMask = static_cast<uint32_t>(reason);
    uint32_t oldMask          = sDtimHoldMask.load(std::memory_order_acquire);
    for (;;)
    {
        const uint32_t newMask = hold ? oldMask | reasonMask : oldMask & ~reasonMask;
        if (newMask == oldMask || sDtimHoldMask.compare_exchange_weak(oldMask, newMask, std::memory_order_acq_rel))
        {
            if (newMask == oldMask)
            {
                return;
            }
            break;
        }
    }
}

extern "C" int app_dtim_pm_check(void)
{
    const uint32_t now       = CurrentTick();
    const bool unicastActive = ConsumeActiveDeadline(sDtimUnicastDeadline, now);
    const bool mdnsActive    = ConsumeActiveDeadline(sDtimMdnsDeadline, now);
    const bool held          = sDtimHoldMask.load(std::memory_order_acquire) != 0;
    const bool active        = unicastActive || mdnsActive;
    SetDtim(held || active ? kActiveDtim : kIdleDtim);
    return pm_pbufc_check();
}

static void app_dtim_platform_event(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event == nullptr)
    {
        return;
    }

    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::kWiFiConnectivityChange:
        if (event->WiFiConnectivityChange.Result == chip::DeviceLayer::kConnectivity_Established)
        {
            InstallNetworkActivityHooks();
            app_dtim_set_hold(APP_DTIM_HOLD_STARTUP, false);
            // Keep APP_DTIM_HOLD_RECOVERY held here; the station is associated but may not yet
            // have an IP address. The IPv4/IPv6 address events below release it once an address
            // is actually assigned.
        }
        else
        {
            sHasIpv4Address.store(false, std::memory_order_release);
            sHasIpv6Address.store(false, std::memory_order_release);
            app_dtim_set_hold(APP_DTIM_HOLD_RECOVERY, true);
        }
        break;
    case chip::DeviceLayer::DeviceEventType::kInternetConnectivityChange:
        if (event->InternetConnectivityChange.IPv4 == chip::DeviceLayer::kConnectivity_Established)
        {
            sHasIpv4Address.store(true, std::memory_order_release);
        }
        else if (event->InternetConnectivityChange.IPv4 == chip::DeviceLayer::kConnectivity_Lost)
        {
            sHasIpv4Address.store(false, std::memory_order_release);
        }
        if (event->InternetConnectivityChange.IPv6 == chip::DeviceLayer::kConnectivity_Established)
        {
            sHasIpv6Address.store(true, std::memory_order_release);
        }
        else if (event->InternetConnectivityChange.IPv6 == chip::DeviceLayer::kConnectivity_Lost)
        {
            sHasIpv6Address.store(false, std::memory_order_release);
        }
        app_dtim_set_hold(APP_DTIM_HOLD_RECOVERY,
                          !(sHasIpv4Address.load(std::memory_order_acquire) || sHasIpv6Address.load(std::memory_order_acquire)));
        break;
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        if (event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV4_Assigned)
        {
            sHasIpv4Address.store(true, std::memory_order_release);
        }
        else if (event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV6_Assigned)
        {
            sHasIpv6Address.store(true, std::memory_order_release);
        }
        else if (event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV4_Lost)
        {
            sHasIpv4Address.store(false, std::memory_order_release);
        }
        else if (event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV6_Lost)
        {
            sHasIpv6Address.store(false, std::memory_order_release);
        }
        app_dtim_set_hold(APP_DTIM_HOLD_RECOVERY,
                          !(sHasIpv4Address.load(std::memory_order_acquire) || sHasIpv6Address.load(std::memory_order_acquire)));
        break;
    case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished:
        app_dtim_set_hold(APP_DTIM_HOLD_BLE, true);
        app_dtim_set_hold(APP_DTIM_HOLD_COMMISSIONING, true);
        break;
    case chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
        app_dtim_set_hold(APP_DTIM_HOLD_BLE, false);
        app_dtim_set_hold(APP_DTIM_HOLD_COMMISSIONING, chip::Server::GetInstance().GetFailSafeContext().IsFailSafeArmed());
        break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        app_dtim_set_hold(APP_DTIM_HOLD_COMMISSIONING, false);
        break;
    default:
        break;
    }
}

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
/* PM sleep check – registered with pm_sleep_check_register(); the tickless    */
/* accurate path invokes it before every PDS entry, which is where the DTIM    */
/* selection must be refreshed (tickless_accurate.c reads dtim_origin when     */
/* handing off to the LP firmware).                                            */
/* -------------------------------------------------------------------------- */

extern "C" int app_dtim_sleep_check(void)
{
    return app_dtim_pm_check();
}

extern "C" void app_pre_matter_init(void)
{
    hal_pm_ldo11_cfg(PM_PDS_LDO_LEVEL_SOC_DEFAULT, PM_PDS_LDO_LEVEL_RT_DEFAULT, PM_PDS_LDO_LEVEL_AON_DEFAULT);

    HBN_Enable_RTC_Counter();
    pm_rc32k_auto_cal_init();
    pm_sys_init();
    pm_sleep_check_register("app_dtim", app_dtim_sleep_check, 10);

    bl_lp_init();
    bl_lp_sys_callback_register(lp_enter, NULL, lp_exit, NULL);

    easyflash_init();
    app_set_clock_source(CLOCK_SOURCE_PASSIVE);
    s_gpio_dev = bflb_device_get_by_name("gpio");
#if CHIP_DETAIL_LOGGING
    s_rtc_dev = bflb_device_get_by_name("rtc");
#endif
    s_sha_dev = bflb_device_get_by_name(BFLB_NAME_SEC_SHA);

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

    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().AddEventHandler(app_dtim_platform_event);
    if (err != CHIP_NO_ERROR)
    {
        // Without this handler, APP_DTIM_HOLD_STARTUP is never released and the network
        // activity hooks are never installed, so DTIM management is permanently broken.
        ChipLogError(DeviceLayer, "[LP] Failed to register DTIM platform event handler");
        appError(err);
    }

    app_clock_init();

    app_lp_config_gpio();

    app_lp_config_wakup_gpio();
    bl_lp_wakeup_io_int_register(wakeup_io_callback);
}
