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

#include <app/server/Server.h>
#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
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

    // Supported Wi-Fi bands are a device capability supplied to the cluster at
    // construction. Only meaningful with the Wi-Fi PAF transport.
    BitMask<WiFiBandBitmap> bands;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    {
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
        // If no valid frequency was parsed from the --wifipaf freq_list, default the
        // advertised WiFiBand to 2.4 GHz rather than leaving the bitmap empty, so a
        // Wi-Fi-PAF-capable proxy does not report supporting no bands at all.
        if (!bands.HasAny())
        {
            bands.Set(WiFiBandBitmap::k2g4);
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    // MaxSessions = 1 (one device at a time across transports); MaxCachedResults = 10.
    mCluster.Create(endpoint,
                    Clusters::CommissioningProxy::CommissioningProxyCluster::Config(features, /*maxSessions=*/1,
                                                                                    /*maxCachedResults=*/10, bands));
#if CONFIG_NETWORK_LAYER_BLE
    mCluster.Cluster().RegisterTransport(mBleTransport);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    mCluster.Cluster().RegisterTransport(mPafTransport);
#endif

    ReturnErrorOnFailure(provider.AddCluster(mCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    DeviceLayer::PlatformMgr().RemoveEventHandler(OnDeviceEvent, 0);
#endif

    if (mCluster.IsConstructed())
    {
        // The cluster destructor shuts down its subsystems (scan cache, session
        // manager, aggregator) and every registered transport, cancelling their
        // timers so nothing outlives the cluster.
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
