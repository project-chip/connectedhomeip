/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppMain.h>

#include "CPAppCommandDelegate.h"
#include "NamedPipeCommands.h"
#include "commissioning-proxy-delegate-impl.h"

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
#include <platform/Linux/ConnectivityManagerImpl.h>
#include <platform/PlatformManager.h>
#endif

#include <cstring>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

NamedPipeCommands sChipNamedPipeCommands;
CPAppCommandDelegate sCPAppCommandDelegate;
CommissioningProxy::MyCPDelegate gMyCPDelegate;

BitMask<CommissioningProxy::Feature> gFeatures(CommissioningProxy::Feature::kBackgroundScan
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                                               ,
                                               CommissioningProxy::Feature::kWiFiNetworkInterface
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
);

RegisteredServerCluster<CommissioningProxy::CommissioningProxyCluster>
    gCPCluster(CommissioningProxy::CommissioningProxyCluster::Config(CommissioningProxyEndpoint, gFeatures, gMyCPDelegate));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
void OnChipDeviceEvent(const DeviceLayer::ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        // CommissioningWindowManager cancels the NAN publish automatically.
        // Disconnect the nanreceive signal handler so that a subsequent
        // _WiFiPAFSubscribe (triggered by ProxyConnectRequest) registers
        // exactly one handler and packets are not delivered twice.
        ChipLogProgress(AppServer, "CommissioningProxy: commissioning complete, disconnecting nanreceive handler");
        DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

} // namespace

void ApplicationInit()
{
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);
    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sCPAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

    // Register the Commissioning Proxy Code Driven mechanism
    VerifyOrDie(CodegenDataModelProvider::Instance().Registry().Register(gCPCluster.Registration()) == CHIP_NO_ERROR);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    // Derive the supported WiFi bands from the configured freq_list and report them via the WiFiBand attribute.
    {
        BitMask<CommissioningProxy::WiFiBandBitmap> bands;
        const char * extCmds = LinuxDeviceOptions::GetInstance().mWiFiPAFExtCmds;
        if (extCmds != nullptr)
        {
            const char * p = std::strstr(extCmds, "freq_list=");
            if (p != nullptr)
            {
                p += std::strlen("freq_list=");
                while (*p != '\0' && *p != ' ')
                {
                    uint32_t freq = static_cast<uint32_t>(std::strtoul(p, nullptr, 10));
                    if ((freq >= 2412 && freq <= 2484))
                    {
                        bands.Set(CommissioningProxy::WiFiBandBitmap::k2g4);
                    }
                    else if ((freq >= 5035 && freq <= 5980))
                    {
                        bands.Set(CommissioningProxy::WiFiBandBitmap::k5g);
                    }
                    // Advance past this number and any comma
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
        gMyCPDelegate.SetSupportedWiFiBands(bands);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY
    // If the proxy is already on a fabric (i.e. it was previously commissioned
    // via PAF and restarted), cancel the WiFi-PAF publish immediately.
    // If it still needs to be commissioned, register an event handler to cancel
    // the publish once commissioning completes.
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0)
    {
        // Already commissioned: disconnect the nanreceive handler so a subsequent
        // _WiFiPAFSubscribe registers exactly one handler.  CommissioningWindowManager
        // handles the actual publish cancel.
        DeviceLayer::ConnectivityMgrImpl().WiFiPAFDisconnectPublishReceiveHandler();
    }
    else
    {
        TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().AddEventHandler(OnChipDeviceEvent, 0);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF && CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

    ChipLogProgress(AppServer, "%s(): Main function is Proxy Commissioner on endpoint %u", __func__, CommissioningProxyEndpoint);
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }
    ChipLinuxAppMainLoop();
    return 0;
}
