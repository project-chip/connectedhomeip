/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AppTask.h"
#include "AppConfig.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "qrcodegen.h"
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/ASR/NetworkCommissioningDriver.h>

#include "init_Matter.h"
#ifdef CONFIG_ENABLE_CHIP_SHELL
#include "matter_shell.h"
#endif
#include <app-common/zap-generated/attributes/Accessors.h>

namespace {
lega_thread_t sAppTaskHandle;
} // namespace

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

AppTask AppTask::sAppTask;

namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::ASRWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

static DeviceCallbacks EchoCallbacks;

CHIP_ERROR AppTask::StartAppTask()
{
    // Start App task.
    OSStatus result = lega_rtos_create_thread(&sAppTaskHandle, 2, APP_TASK_NAME, (lega_thread_function_t) AppTaskMain,
                                              APP_TASK_STACK_SIZE, (lega_thread_arg_t) this);
    return (result == kNoErr) ? CHIP_NO_ERROR : CHIP_APPLICATION_ERROR(0x02);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    ASR_LOG("App Task started");

    if (MatterInitializer::Init_Matter_Stack(MATTER_DEVICE_NAME) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    if (deviceMgr.Init(&EchoCallbacks) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    if (MatterInitializer::Init_Matter_Server() != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    NetWorkCommissioningInstInit();

    ASR_LOG("Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(RendezvousInformationFlag(RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(RendezvousInformationFlag(RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    /* Delete task */
    lega_rtos_delete_thread(NULL);
}
