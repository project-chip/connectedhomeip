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

#include "CommissioningProxyDevice.h"

#include "CommissioningProxyBgScanCache.h"
#include <app/server/Server.h>
#if CONFIG_NETWORK_LAYER_BLE
#include "CommissioningProxyBleTransport.h"
#endif

#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include "CommissioningProxyPafTransport.h"
#include <app_options/AppOptions.h>
#include <cstring>
#include <platform/Linux/ConnectivityManagerImpl.h>
#endif

using namespace chip::app::Clusters::CommissioningProxy;

namespace chip {
namespace app {

CommissioningProxyDevice::CommissioningProxyDevice() :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kCommissioningByProxy, 1))
{}

CHIP_ERROR CommissioningProxyDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    BitMask<Feature> features(Feature::kBackgroundScan
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                              ,
                              Feature::kWiFiNetworkInterface
#endif
    );

    mCluster.Create(Clusters::CommissioningProxy::CommissioningProxyCluster::Config(endpoint, features, mDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    {
        BitMask<WiFiBandBitmap> bands;
        const AppOptions::AppConfig * cfg = AppOptions::TryGetConfig();
        const char * extCmds              = (cfg && !cfg->wifipafExtCmds.empty()) ? cfg->wifipafExtCmds.c_str() : nullptr;
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
                        bands.Set(WiFiBandBitmap::k2g4);
                    else if (freq >= 5035 && freq <= 5980)
                        bands.Set(WiFiBandBitmap::k5g);
                    while (*p != '\0' && *p != ',' && *p != ' ')
                        ++p;
                    if (*p == ',')
                        ++p;
                }
            }
        }
        // Spec WiFiBand attribute has Fallback: 2G4 — if no valid frequency was parsed,
        // default to 2.4 GHz rather than leaving the bitmap empty.
        if (!bands.HasAny())
            bands.Set(WiFiBandBitmap::k2g4);
        mDelegate.SetSupportedWiFiBands(bands);
    }

    if (Server::GetInstance().GetFabricTable().FabricCount() > 0)
    {
        DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
    }
    else
    {
        (void) DeviceLayer::PlatformMgr().AddEventHandler(OnDeviceEvent, 0);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    return CHIP_NO_ERROR;
}

void CommissioningProxyDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mCluster.IsConstructed())
    {
        // Drop every cached reference to the cluster before it is destroyed so a
        // self-re-arming background-scan sweep/timer can never dereference it.
        Clusters::CommissioningProxy::BgScanCache::Unregister(&mCluster.Cluster());
#if CONFIG_NETWORK_LAYER_BLE
        Clusters::CommissioningProxy::Ble::Shutdown();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        Clusters::CommissioningProxy::Paf::Shutdown();
#endif
        LogErrorOnFailure(provider.RemoveCluster(&mCluster.Cluster()));
        mCluster.Destroy();
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
void CommissioningProxyDevice::OnDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ChipLogProgress(AppServer, "CommissioningProxy: commissioning complete, disconnecting nanreceive handler");
        DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

} // namespace app
} // namespace chip
