/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023, 2025 NXP
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
 *          for RW61x platform using the NXP RW610 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform_coex.h"
#include <crypto/CHIPCryptoPAL.h>
#include <platform/FreeRTOS/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#include <stdlib.h>

#include <lwip/tcpip.h>

#include "els_pkc_mbedtls.h"

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OtaSupport.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "fwk_platform_ot.h"
#include "ot_platform_common.h"
#endif

extern "C" void BOARD_InitHardware(void);
extern "C" void otPlatSetResetFunction(void (*fp)(void));
extern "C" void initiateResetInIdle(void);

extern "C" {
#include "osa.h"
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

#include "wlan_bt_fw.h"

extern "C" {
#include "wlan.h"
#include "wm_net.h"
}

#ifdef gPlatformMonolithicApp_d
extern const uint32_t fw_cpu1[];
#define WIFI_FW_BIN_ADDRESS (uint32_t) & fw_cpu1[0]
#else
#define WIFI_FW_BIN_ADDRESS wlan_fw_bin
#endif

#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

extern "C" void vApplicationMallocFailedHook(void)
{
    ChipLogError(DeviceLayer, "Malloc Failure");
}

extern "C" void __wrap_exit(int __status)
{
    ChipLogError(DeviceLayer, "======> error exit function !!!");
    assert(0);
}

/* wlan_event_callback should be defined here because it is
 * referenced in wlan.c but only defined in main.c of sdk examples */
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
extern "C" int wlan_event_callback(enum wlan_event_reason reason, void * data)
{
    return 0;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

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

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
CHIP_ERROR PlatformManagerImpl::WiFiInterfaceInit(void)
{
    CHIP_ERROR result = CHIP_NO_ERROR;
    int wifi_status   = WM_SUCCESS;

    ChipLogProgress(DeviceLayer, "Initialize WLAN");

    wifi_status = wlan_init((uint8_t *) WIFI_FW_BIN_ADDRESS, wlan_fw_bin_len);
    if (wifi_status != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "WLAN initialization failed");
        result = CHIP_ERROR_INTERNAL;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "WLAN initialized");
    }

    return result;
}
#endif

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    int osError;

    /* Mask of combined controllers to initialize */
    uint8_t controllerMask = 0U;

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

    /*
     * Initialize controllers here before initializing BLE/OT/WIFI,
     * this will load the firmware in CPU1/CPU2 depending on the
     * connectivity used
     */

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD || CHIP_DEVICE_CONFIG_ENABLE_ZIGBEE
    controllerMask |= conn802_15_4_c;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD || CHIP_DEVICE_CONFIG_ENABLE_ZIGBEE */
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    controllerMask |= connBle_c;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    controllerMask |= connWlan_c;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

    PLATFORM_InitControllers(controllerMask);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    /* Initialize platform services */
    err = ServiceInit();
    SuccessOrExit(err);

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

    osError = OSA_SetupIdleFunction(chip::DeviceLayer::PlatformManagerImpl::IdleHook);
    if (osError != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to setup idle function");
        err = CHIP_ERROR_NO_MEMORY;
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    err = WiFiInterfaceInit();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer,
                        "Wi-Fi module initialization failed. Make sure the Wi-Fi/BLE module is properly configured and connected "
                        "with the board and start again!");
        chipDie();
    }
    ChipLogProgress(DeviceLayer, "Wi-Fi module initialization done.");

    /* Initialize platform services */
    err = ServiceInit();
    SuccessOrExit(err);

#endif

#if CONFIG_CHIP_ETHERNET
    /* Initialize platform services */
    err = ServiceInit();
    SuccessOrExit(err);
#endif // CONFIG_CHIP_ETHERNET

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

void PlatformManagerImpl::StopBLEConnectivity(void) {}

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
