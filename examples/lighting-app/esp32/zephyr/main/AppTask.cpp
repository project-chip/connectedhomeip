/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <DeviceInfoProviderImpl.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Zephyr/wifi/ZephyrWifiDriver.h>
#endif

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

using namespace ::chip;
using namespace ::chip::DeviceLayer;

namespace {

const struct gpio_dt_spec sLightLed = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

DeviceInfoProviderImpl sExampleDeviceInfoProvider;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
app::Clusters::NetworkCommissioning::Instance sWiFiCommissioningInstance(0 /* endpoint */,
                                                                         &(NetworkCommissioning::ZephyrWifiDriver::Instance()));
#endif

} // namespace

void AppTask::SetOnOffLED(bool on)
{
    if (!gpio_is_ready_dt(&sLightLed))
    {
        return;
    }
    gpio_pin_set_dt(&sLightLed, on ? 1 : 0);
}

CHIP_ERROR AppTask::Init()
{
    LOG_INF("Bringing up Matter stack for ESP32-C6 (Zephyr)");

    ReturnErrorOnFailure(Platform::MemoryInit());
    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ReturnErrorOnFailure(sWiFiCommissioningInstance.Init());
#endif

    if (gpio_is_ready_dt(&sLightLed))
    {
        gpio_pin_configure_dt(&sLightLed, GPIO_OUTPUT_INACTIVE);
    }
    else
    {
        LOG_WRN("led0 not ready, light state will not be shown on a GPIO");
    }

    // Example (test) DAC/PAI/CD. No factory-data partition. Not for production.
    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    static CommonCaseDeviceServerInitParams initParams;
    ReturnErrorOnFailure(initParams.InitializeStaticResourcesBeforeServerInit());
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

    sExampleDeviceInfoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    SetDeviceInfoProvider(&sExampleDeviceInfoProvider);

    ReturnErrorOnFailure(Server::GetInstance().Init(initParams));

    // The rendezvous flag must match the radios actually compiled in.
    ConfigurationMgr().LogDeviceConfig();
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork));
#endif

    // Everything above must be done before the CHIP thread starts.
    ReturnErrorOnFailure(PlatformMgr().StartEventLoopTask());

    LOG_INF("Matter stack initialized");
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartApp()
{
    ReturnErrorOnFailure(Init());

    // No app-level event queue in this minimal build (no buttons or status LED).
    // The CHIP event loop runs on its own thread; park the main thread.
    while (true)
    {
        k_sleep(K_FOREVER);
    }

    return CHIP_NO_ERROR;
}
