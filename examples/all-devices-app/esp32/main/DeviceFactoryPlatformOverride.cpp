/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DeviceFactoryPlatformOverride.h"
#include "AppDeviceFactory.h"
#include "sdkconfig.h"
#include <app_config/enabled_devices.h>

#if CONFIG_HAVE_SPEAKER && ALL_DEVICES_ENABLE_CHIME
#if CONFIG_DEVICE_TYPE_M5STACK_CORES3
#include "CoreS3Chime.h"
using BoardChime = chip::app::CoreS3Chime;
#elif CONFIG_DEVICE_TYPE_ESP_MOSAICO
#include "MosaicoChime.h"
using BoardChime = chip::app::MosaicoChime;
#endif
#endif

#if CONFIG_ENABLE_ESP32_COMMISSIONING_PROXY && ALL_DEVICES_ENABLE_COMMISSIONING_PROXY
#include "CommissioningProxyBleAdapter.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h>
#include <app/server/Server.h>
#include <device/types/commissioning-proxy/CommissioningProxy.h>
#include <lib/support/logging/CHIPLogging.h>

#include <memory>
#endif

namespace chip {
namespace app {

#if CONFIG_ENABLE_ESP32_COMMISSIONING_PROXY && ALL_DEVICES_ENABLE_COMMISSIONING_PROXY
namespace {

// The adapter and transport outlive every device the factory creates, because a
// registered transport holds a pointer back to the device's cluster. The transport
// drives the single BLE scanner, so only one proxy device is ever created.
CommissioningProxyBleAdapter sBleProxyAdapter;

void RegisterCommissioningProxyDevice(TimerDelegate & timerDelegate)
{
    using namespace Clusters::CommissioningProxy;

    static CommissioningProxyBleTransport sBleProxyTransport(sBleProxyAdapter, timerDelegate);

    const CommissioningProxy::Context context{ Server::GetInstance().GetFabricTable(), timerDelegate };

    // The BLE transport implements the background scan commands, which is what the
    // feature advertises. There is no Wi-Fi PAF transport, so no WiFiBand.
    const CommissioningProxyCluster::Config config(BitMask<Feature>(Feature::kBackgroundScan), BitMask<WiFiBandBitmap>());

    AppDeviceFactory::GetInstance().RegisterCreator("commissioning-proxy", [context, config]() {
        static bool sProxyDeviceCreated = false;
        if (sProxyDeviceCreated)
        {
            ChipLogError(AppServer, "Only one commissioning-proxy device is supported: its transport drives a single radio");
            return AppDeviceFactory::DeviceRegistrationEntry{};
        }
        sProxyDeviceCreated = true;

        auto device = std::make_unique<CommissioningProxy>(context, config);
        device->AddTransport(sBleProxyTransport);
        auto * rawDevice = device.get();
        return AppDeviceFactory::DeviceRegistrationEntry{
            std::move(device), AppDeviceFactory::MakeAfterDeviceRegistrationCallback<CommissioningProxy>(rawDevice),
            AppDeviceFactory::MakeBeforeDeviceUnregistrationCallback<CommissioningProxy>(rawDevice)
        };
    });
}

} // namespace
#endif // CONFIG_ENABLE_ESP32_COMMISSIONING_PROXY && ALL_DEVICES_ENABLE_COMMISSIONING_PROXY

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate)
{
#if CONFIG_HAVE_SPEAKER && ALL_DEVICES_ENABLE_CHIME
    AppDeviceFactory::GetInstance().RegisterCreator(
        "chime", [&timerDelegate]() { return AppDeviceFactory::MakeDevice<BoardChime>(timerDelegate); });
#endif

#if CONFIG_ENABLE_ESP32_COMMISSIONING_PROXY && ALL_DEVICES_ENABLE_COMMISSIONING_PROXY
    RegisterCommissioningProxyDevice(timerDelegate);
#endif
}

} // namespace app
} // namespace chip
