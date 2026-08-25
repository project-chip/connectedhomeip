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
#include <device/types/commissioning-proxy/CommissioningProxyDevice.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <CommissioningProxyBleAdapter.h>
// The ble-transport / paf-transport dependencies in BUILD.gn are conditional on
// chip_config_network_layer_ble and chip_device_config_enable_wifipaf, which gn check
// cannot evaluate, hence the nogncheck on the transport includes below.
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBleTransport.h> // nogncheck
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <CommissioningProxyPafAdapter.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyPafTransport.h> // nogncheck
#include <app_options/AppOptions.h>

#include <cstdlib>
#include <cstring>
#endif

namespace chip {
namespace app {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
namespace {

/// Derive the Wi-Fi bands the proxy advertises from the "--wifipaf freq_list=" the app
/// was started with.  This lives here rather than in the device because it reads the
/// app's command line; the device takes the resulting bands injected.
BitMask<Clusters::CommissioningProxy::WiFiBandBitmap> ProxyWiFiBands()
{
    using Clusters::CommissioningProxy::WiFiBandBitmap;

    BitMask<WiFiBandBitmap> bands;

    const AppOptions::AppConfig * cfg = AppOptions::TryGetConfig();
    const char * extCmds              = (cfg != nullptr && !cfg->wifipafExtCmds.empty()) ? cfg->wifipafExtCmds.c_str() : nullptr;
    if (extCmds != nullptr)
    {
        const char * p = std::strstr(extCmds, "freq_list=");
        if (p != nullptr)
        {
            p += std::strlen("freq_list=");
            while (*p != '\0' && *p != ' ')
            {
                uint32_t freq = static_cast<uint32_t>(std::strtoul(p, nullptr, 10));
                if (freq >= 2412 && freq <= 2484)
                {
                    bands.Set(WiFiBandBitmap::k2g4);
                }
                else if (freq >= 5035 && freq <= 5980)
                {
                    bands.Set(WiFiBandBitmap::k5g);
                }
                while (*p != '\0' && *p != ',' && *p != ' ')
                {
                    ++p;
                }
                if (*p == ',')
                {
                    ++p;
                }
            }
        }
    }

    // With no valid frequency in the freq_list, advertise 2.4 GHz rather than an empty
    // bitmap, so a Wi-Fi-PAF-capable proxy does not report supporting no bands at all.
    if (!bands.HasAny())
    {
        bands.Set(WiFiBandBitmap::k2g4);
    }

    return bands;
}

} // namespace
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                    PersistentStorageDelegate * storageDelegate, PosixAudioManager & audioManager)
{
    if constexpr (ALL_DEVICES_ENABLE_COMMISSIONING_PROXY)
    {
        const CommissioningProxyDevice::Context proxyContext{ fabricTable, timerDelegate };

        // The adapters and their transport drivers outlive every device the factory
        // creates, because a registered transport holds a pointer back to the device's
        // cluster. Only one commissioning proxy device is ever created, so one instance
        // of each serves it.
        //
        // Adding a technology here is one more block like these plus one more
        // AddTransport() call below; no new device type is involved.
        BitMask<Clusters::CommissioningProxy::Feature> proxyFeatures;
        BitMask<Clusters::CommissioningProxy::WiFiBandBitmap> proxyBands;

#if CONFIG_NETWORK_LAYER_BLE
        static CommissioningProxyBleAdapter sBleProxyAdapter;
        static Clusters::CommissioningProxy::CommissioningProxyBleTransport sBleProxyTransport(sBleProxyAdapter, timerDelegate);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        static CommissioningProxyPafAdapter sPafProxyAdapter;
        static Clusters::CommissioningProxy::CommissioningProxyPafTransport sPafProxyTransport(sPafProxyAdapter, timerDelegate,
                                                                                               &fabricTable);
#endif

#if CONFIG_NETWORK_LAYER_BLE || CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        // Every transport driver implements ProxyBackgroundScanStart/Stop, so the
        // feature follows from having any transport at all.
        proxyFeatures.Set(Clusters::CommissioningProxy::Feature::kBackgroundScan);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        // Wi-Fi PAF makes this a Wi-Fi device, which is what makes WiFiBand present.
        proxyFeatures.Set(Clusters::CommissioningProxy::Feature::kWiFiNetworkInterface);
        proxyBands = ProxyWiFiBands();
#endif

        const Clusters::CommissioningProxy::CommissioningProxyCluster::Config proxyConfig(proxyFeatures, proxyBands);

        DeviceFactory::GetInstance().RegisterCreator("commissioning-proxy", [proxyContext, proxyConfig]() {
            auto device = std::make_unique<CommissioningProxyDevice>(proxyContext, proxyConfig);
#if CONFIG_NETWORK_LAYER_BLE
            device->AddTransport(sBleProxyTransport);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
            device->AddTransport(sPafProxyTransport);
#endif
            return device;
        });
    }

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
