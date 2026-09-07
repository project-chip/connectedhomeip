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
#include <DeviceFactoryPlatformOverride.h>
#include <PosixAudioManager.h>
#include <PosixChime.h>
#include <PosixSpeaker.h>
#include <app_config/enabled_devices.h>
#include <device-factory/DeviceFactory.h>
#include <lib/support/logging/CHIPLogging.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <CommissioningProxyBleAdapter.h>
// The commissioning-proxy and ble-transport dependencies in BUILD.gn are conditional on
// chip_config_network_layer_ble, which gn check cannot evaluate, hence the nogncheck.
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h> // nogncheck
#include <device/types/commissioning-proxy/CommissioningProxyDevice.h>              // nogncheck
#endif

namespace chip {
namespace app {

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                    PersistentStorageDelegate * storageDelegate, PosixAudioManager & audioManager)
{
    // Registered only when a transport is compiled in.
#if CONFIG_NETWORK_LAYER_BLE
    if constexpr (ALL_DEVICES_ENABLE_COMMISSIONING_PROXY)
    {
        const CommissioningProxyDevice::Context proxyContext{ fabricTable, timerDelegate };

        // The adapter and its transport driver outlive every device the factory creates,
        // because a registered transport holds a pointer back to the device's cluster.
        // Only one commissioning proxy device is ever created, so one instance of each
        // serves it.
        BitMask<Clusters::CommissioningProxy::Feature> proxyFeatures;

        static CommissioningProxyBleAdapter sBleProxyAdapter;
        static Clusters::CommissioningProxy::CommissioningProxyBleTransport sBleProxyTransport(sBleProxyAdapter, timerDelegate);

        // Every transport driver implements ProxyBackgroundScanStart/Stop, so the
        // feature follows from having any transport at all.
        proxyFeatures.Set(Clusters::CommissioningProxy::Feature::kBackgroundScan);

        const Clusters::CommissioningProxy::CommissioningProxyCluster::Config proxyConfig(proxyFeatures);

        DeviceFactory::GetInstance().RegisterCreator(
            "commissioning-proxy", [proxyContext, proxyConfig]() -> std::unique_ptr<DeviceInterface> {
                // Refuse a second proxy. The driver above is a single instance because
                // the radio it drives is: one BLE scanner. Handing it to a second device
                // would take it over from the first, leaving it registered but
                // unreachable, and two proxies could not both work on one radio anyway.
                static bool sProxyDeviceCreated = false;
                if (sProxyDeviceCreated)
                {
                    ChipLogError(AppServer, "Only one commissioning-proxy device is supported: its transports drive single radios");
                    return nullptr;
                }
                sProxyDeviceCreated = true;

                auto device = std::make_unique<CommissioningProxyDevice>(proxyContext, proxyConfig);
                device->AddTransport(sBleProxyTransport);
                return device;
            });
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if constexpr (ALL_DEVICES_ENABLE_SPEAKER)
    {
        DeviceFactory::GetInstance().RegisterCreator("speaker", [&timerDelegate, &audioManager]() {
            return std::make_unique<PosixSpeaker>(PosixSpeaker::Context{ timerDelegate }, audioManager);
        });
    }

    if constexpr (ALL_DEVICES_ENABLE_CHIME)
    {
        DeviceFactory::GetInstance().RegisterCreator(
            "chime", [&timerDelegate, &audioManager]() { return std::make_unique<PosixChime>(timerDelegate, audioManager); });
    }
}

} // namespace app
} // namespace chip
