/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <assert.h>

#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/mt793x/NetworkCommissioningWiFiDriver.h>

#include "AppConfig.h"
#include "AppTask.h"
#include "qrcodegen.h"

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10
#define EXAMPLE_VENDOR_ID 0xcafe

#ifdef portYIELD_FROM_ISR
#define OS_YIELD_FROM_ISR(yield) portYIELD_FROM_ISR(yield)
#elif portEND_SWITCHING_ISR
#define OS_YIELD_FROM_ISR(yield) portEND_SWITCHING_ISR(yield)
#else
#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif

namespace {

TaskHandle_t sAppTaskHandle;

bool sIsWiFiProvisioned = false;
bool sIsWiFiEnabled     = false;
bool sIsWiFiAttached    = false;

StackType_t appStack[APP_TASK_STACK_SIZE * 2 / sizeof(StackType_t)];
StaticTask_t appTaskStruct;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
using namespace chip::DeviceLayer::NetworkCommissioning;
chip::app::Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */,
                                                                                      &GenioWiFiDriver::GetInstance());
#endif

} // namespace

using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

constexpr chip::EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

CHIP_ERROR AppTask::StartAppTask()
{
    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, MATTER_ARRAY_SIZE(appStack), NULL, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
        return APP_ERROR_CREATE_TASK_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    // Wait for the WiFi to be initialized
    MT793X_LOG("APP: Wait WiFi Init");
    vTaskDelay(1000); // TODO
    MT793X_LOG("APP: Done WiFi Init");

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    sWiFiNetworkCommissioningInstance.Init();
#endif

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = chip::app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    MT793X_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kSoftAP));

    return error;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    CHIP_ERROR error = sAppTask.Init();
    if (error != CHIP_NO_ERROR)
    {
        MT793X_LOG("AppTask.Init() failed");
        appError(error);
    }

    MT793X_LOG("AppTask started");

    while (true)
    {
        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiAttached    = ConnectivityMgr().IsWiFiStationConnected();

            PlatformMgr().UnlockChipStack();
        }
    }
}
