/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023-2024 NXP
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for RT1170 platforms using the NXP RT1170 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform_coex.h"
#include "ksdk_mbedtls.h"
#include <crypto/CHIPCryptoPAL.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <stdlib.h>

#include <lwip/tcpip.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OtaSupport.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "ot_platform_common.h"
#include <openthread/platform/entropy.h>
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#include "lib/spinel/spinel.h"
#define OT_NXP_SPINEL_PROP_VENDOR_BLE_STATE SPINEL_PROP_VENDOR__BEGIN
#endif /* CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED */
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_ENABLE_WPA

#include "board.h"
#include "clock_config.h"
#include "fsl_phy.h"
#include "pin_mux.h"

#include "fsl_enet_mdio.h"
#include "fsl_phyksz8081.h"

#include "enet_ethernetif.h"
#include "fsl_debug_console.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"

//----LWIP defines----//
/* MAC address configuration. */
#define configMAC_ADDR                                                                                                             \
    {                                                                                                                              \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11                                                                                         \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* MDIO operations. */
#define EXAMPLE_MDIO_OPS enet_ops

/* PHY operations. */
#define EXAMPLE_PHY_OPS phyksz8081_ops

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetFreq(kCLOCK_IpgClk)

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

static mdio_handle_t mdioHandle = { .ops = &EXAMPLE_MDIO_OPS };
static phy_handle_t phyHandle   = { .phyAddr = EXAMPLE_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &EXAMPLE_PHY_OPS };

#endif

extern "C" void BOARD_InitHardware(void);
extern "C" void otPlatSetResetFunction(void (*fp)(void));
extern "C" void initiateResetInIdle(void);

#include "fwk_platform_ot.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

#include "fsl_adapter_gpio.h"
#include "fsl_os_abstraction.h"

extern "C" {
#include "wlan.h"
#include "wm_net.h"
}

#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

extern "C" void vApplicationMallocFailedHook(void)
{
    ChipLogError(DeviceLayer, "Malloc Failure");
}

#if WIFI_PTA_ENABLED && (CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_ENABLE_OPENTHREAD)
#ifdef SD8801
#define HOSTCMD_RESP_BUFF_SIZE 1024
/* 8801 command buffer to enable External Coexistence.
 * Source: SDK 2.11.1, wifi_cli demo, wlan_tests.c file. */
const uint8_t ext_coex_8801_cmd_buf[] = { 0xe0, 0,    0x1d, 0, 0x17, 0,    0,    0,    0x01, 0,    0,    0,    0x2f, 0x02, 0x0d,
                                          0x00, 0x01, 0,    0, 0x03, 0x01, 0x02, 0x01, 0x01, 0x00, 0x28, 0x00, 0x3c, 0x00 };
#endif

static CHIP_ERROR EnableWiFiCoexistence(void)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

#ifdef SD8801
    int status        = WM_FAIL;
    uint8_t * rspBuff = NULL;
    uint32_t reqd_len = 0;

    rspBuff = (uint8_t *) pvPortMalloc(HOSTCMD_RESP_BUFF_SIZE);
    if (rspBuff != NULL)
    {
        status = wlan_send_hostcmd((void *) ext_coex_8801_cmd_buf, sizeof(ext_coex_8801_cmd_buf) / sizeof(u8_t), rspBuff,
                                   HOSTCMD_RESP_BUFF_SIZE, &reqd_len);

        if (status == WM_SUCCESS)
        {
            ChipLogProgress(DeviceLayer, "8801 Coexistence enabled");
        }
        else
        {
            ChipLogError(DeviceLayer, "8801 Coexistence enabling failed = %d", status);
            ret = CHIP_ERROR_INTERNAL;
        }

        vPortFree(rspBuff);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to allocate memory for Wi-Fi coexistence response buffer");
    }
#else
    ChipLogError(DeviceLayer, "This chip does not support Wi-Fi and OpenThread coexistence");
    ret = CHIP_ERROR_INTERNAL;
#endif

    return ret;
}
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_WPA
extern "C" void vApplicationIdleHook(void)
{
    chip::DeviceLayer::PlatformManagerImpl::IdleHook();
}
#endif

extern "C" void __wrap_exit(int __status)
{
    ChipLogError(DeviceLayer, "======> error exit function !!!");
    assert(0);
}

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

void PlatformManagerImpl::HardwareInit(void)
{
    BOARD_InitHardware();
    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
}

CHIP_ERROR PlatformManagerImpl::ServiceInit(void)
{
    status_t status;
    CHIP_ERROR chipRes = CHIP_NO_ERROR;

    status = CRYPTO_InitHardware();

    if (status != 0)
    {
        chipRes = CHIP_ERROR_INTERNAL;
        ChipLogError(DeviceLayer, "Crypto hardware init error");
    }

    return chipRes;
}

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_ENABLE_WPA
CHIP_ERROR PlatformManagerImpl::EthernetInterfaceInit()
{

    static struct netif netif;
    ethernetif_config_t enet_config = {
        .phyHandle  = &phyHandle,
        .macAddress = configMAC_ADDR,
    };

    mdioHandle.resource.csrClock_Hz = EXAMPLE_CLOCK_FREQ;

    netifapi_netif_add(&netif, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);

    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(netif_default);
    LOCK_TCPIP_CORE();
    netif_create_ip6_linklocal_address(netif_default, 1);
    UNLOCK_TCPIP_CORE();

    vTaskDelay(pdMS_TO_TICKS(1500));

    for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        const char * str_ip = "-";
        if (ip6_addr_isvalid(netif_ip6_addr_state(netif_default, i)))
        {
            str_ip = ip6addr_ntoa(netif_ip6_addr(netif_default, i));
        }
        PRINTF(" IPv6 Address%d    : %s\r\n", i, str_ip);
    }
    PRINTF("************************************************\r\n");

    if (netif_is_up(&netif))
    {
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NOT_CONNECTED;
}
#endif

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    hal_gpio_status_t status_gpio;
    hal_gpio_pin_config_t resetPinConfig = {
        .direction = kHAL_GpioDirectionOut,
        .level     = 0U,
        .port      = 3,
        .pin       = 9,
    };

    GPIO_HANDLE_DEFINE(otGpioResetHandle);

#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    int osError;

    /* Initialize platform services */
    err = ServiceInit();
    SuccessOrExit(err);

    // Initialize the configuration system.
    err = Internal::NXPConfig::Init();
    SuccessOrExit(err);

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    /* Initialize LwIP via Wi-Fi layer. */
    net_ipv4stack_init();
#else
    /* Initialize LwIP directly. */
    tcpip_init(NULL, NULL);
#endif

    /* Init the controller by giving as an arg the connectivity supported */
    PLATFORM_InitControllers(connBle_c
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                             | conn802_15_4_c
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                             | connWlan_c
#endif
    );
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    PLATFORM_InitOt();
    /*
     * Initialize the RCP here: the WiFi initialization requires to enable/disable
     * coexistence through the Spinel interface; as such, the otPlatRadioInit() call
     * will fail if done afterwards
     */
    otPlatLogInit();
    otPlatRadioInit();
    otPlatSetResetFunction(initiateResetInIdle);
    otPlatRandomInit();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

    osError = OSA_SetupIdleFunction(chip::DeviceLayer::PlatformManagerImpl::IdleHook);
    if (osError != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to setup idle function");
        err = CHIP_ERROR_NO_MEMORY;
        goto exit;
    }

    ChipLogProgress(DeviceLayer, "Wi-Fi module initialization done.");
#elif !CHIP_DEVICE_CONFIG_ENABLE_THREAD
    err = EthernetInterfaceInit();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer,
                        "LWIP interface initialization failed. Make sure there is internet conectivity on the board and reset!");
        chipDie();
    }
    ChipLogProgress(DeviceLayer, "LWIP interface initialization done");
#endif

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    err = System::Clock::InitClock_RealTime();
    SuccessOrExit(err);

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

exit:
    return err;
}

void PlatformManagerImpl::SaveSettings(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    otPlatSaveSettingsIdle();
#endif
}

void PlatformManagerImpl::IdleHook(void)
{
    bool isResetScheduled = PlatformMgrImpl().GetResetInIdleValue();
    if (isResetScheduled)
    {
        /*
         * In case a reset in IDLE has been scheduled
         * Disable IRQs so that the idle task won't be preempted until the reset
         */
        OSA_InterruptDisable();
    }

    chip::DeviceLayer::Internal::NXPConfig::RunSystemIdleTask();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    SaveSettings();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* Resume OTA Transactions in Idle task */
    OTA_TransactionResume();
#endif

    /*
     * In case a reset in idle operation has been posted,
     * reset the device after having run the idle function
     */
    if (isResetScheduled)
    {
        PlatformMgrImpl().Reset();
    }
}

void PlatformManagerImpl::Reset(void)
{
    ChipLogProgress(DeviceLayer, "System restarting");
    // Restart the system.
    NVIC_SystemReset();
    while (1)
    {
    }
}

void PlatformManagerImpl::ScheduleResetInIdle(void)
{
    resetInIdle = true;
}

bool PlatformManagerImpl::GetResetInIdleValue(void)
{
    return resetInIdle;
}

extern "C" void initiateResetInIdle(void)
{
    PlatformMgr().Shutdown();
    PlatformMgrImpl().ScheduleResetInIdle();
}

extern "C" void scheduleResetInIdle(void)
{
    PlatformMgrImpl().ScheduleResetInIdle();
}

extern "C" bool getResetInIdleValue(void)
{
    return PlatformMgrImpl().GetResetInIdleValue();
}

void PlatformManagerImpl::StopBLEConnectivity(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    otPlatRadioSendSetPropVendorUint8Cmd(OT_NXP_SPINEL_PROP_VENDOR_BLE_STATE, 0);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD */
}

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    /* Handle the server shutting down & emit the ShutDown event*/
    PlatformMgr().HandleServerShuttingDown();
    /* Shutdown all layers */
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip

extern "C" void mt_wipe(void)
{
    chip::DeviceLayer::Internal::NXPConfig::FactoryResetConfig();
}
