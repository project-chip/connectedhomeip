/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2022 Silabs.
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

#include "AppConfig.h"
#include "OTAConfig.h"
#include <matter_config.h>

#include <FreeRTOS.h>

#include <mbedtls/platform.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef SL_WIFI
#include "wfx_host_events.h"
#endif /* SL_WIFI */
#ifdef __cplusplus
}
#endif

#if PW_RPC_ENABLED
#include "Rpc.h"
#endif

#ifdef ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

#include <crypto/CHIPCryptoPAL.h>
// If building with the SiWx917-provided crypto backend, we can use the

#include "SiWx917DeviceDataProvider.h"

#if EFR32_OTA_ENABLED
void SI917MatterConfig::InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
#if 0 // TODO : OTA is not planned now for CCP
    OTAConfig::Init();
#endif
}
#endif

void SI917MatterConfig::ConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg){
    // Initialize OTA only when Thread or WiFi connectivity is established
    /*if (((event->Type == DeviceEventType::kThreadConnectivityChange) &&
         (event->ThreadConnectivityChange.Result == kConnectivity_Established)) ||
        ((event->Type == DeviceEventType::kInternetConnectivityChange) &&
         (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)))
    {
        SILABS_LOG("Scheduling OTA Requestor initialization")
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                    InitOTARequestorHandler, nullptr);
    }*/
    SILABS_LOG("Scheduling OTA Requestor initialization")
}

CHIP_ERROR SI917MatterConfig::InitMatter(const char * appName)
{
    CHIP_ERROR err;

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    SILABS_LOG("==================================================");
    SILABS_LOG("%s starting", appName);
    SILABS_LOG("==================================================");

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif
    SetDeviceInstanceInfoProvider(&EFR32::EFR32DeviceDataProvider::GetDeviceDataProvider());
    SetCommissionableDataProvider(&EFR32::EFR32DeviceDataProvider::GetDeviceDataProvider());

    //==============================================
    // Init Matter Stack
    //==============================================
    SILABS_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    ReturnErrorOnFailure(chip::Platform::MemoryInit());

    SILABS_LOG("Init RSI 911x Platform");
    int32_t deviceInit = wfx_rsi_platform();
    if (deviceInit != SL_STATUS_OK)
    {
        SILABS_LOG("RSI init failed");
        return CHIP_ERROR_INTERNAL;
    }
    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(appName);

    // Stop Matter event handling while setting up resources
    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Create initParams with SDK example defaults here
    static chip::CommonCaseDeviceServerInitParams initParams;

    // Initialize the remaining (not overridden) providers to the SDK example defaults
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    // Init Matter Server and Start Event Loop
    err = chip::Server::GetInstance().Init(initParams);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    ReturnErrorOnFailure(err);

    // OTA Requestor initialization will be triggered by the connectivity events
    // TODO
    // PlatformMgr().AddEventHandler(ConnectivityEventCallback, reinterpret_cast<intptr_t>(nullptr));

    SILABS_LOG("Starting Platform Manager Event Loop");
    ReturnErrorOnFailure(PlatformMgr().StartEventLoopTask());

#ifdef SL_WIFI
    InitWiFi();
#endif

#ifdef ENABLE_CHIP_SHELL
    chip::startShellTask();
#endif

    return CHIP_NO_ERROR;
}

#ifdef SL_WIFI
void SI917MatterConfig::InitWiFi(void)
{
#ifdef RS911X_WIFI
    /*
     * Start up any RSI interface stuff
     * (Not required) - Note that wfx_wifi_start will deal with
     * starting up a rsi task - which will initialize the SPI interface.
     */
#endif
}
#endif // SL_WIFI

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback

    // Check CHIP Config nvm3 and repack flash if necessary.
    Internal::SILABSConfig::RepackNvm3Flash();
}
