/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/NetworkCommissioningDelegate.h>
#include <platform/Linux/ThreadStackManagerImpl.h>
#include <platform/ThreadStackManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

void LinuxNetworkCommissioningDelegate::ConnectWiFiNetwork(ByteSpan ssid, ByteSpan credentials,
                                                           WiFiNetworkCommissioningDelegate::Callback * callback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NetworkProvisioning, "LinuxNetworkCommissioningDelegate: SSID: %s", ssid.data());

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    err = ConnectivityMgrImpl().ProvisionWiFiNetwork(reinterpret_cast<const char *>(ssid.data()),
                                                     reinterpret_cast<const char *>(credentials.data()));
#else
    err            = CHIP_ERROR_NOT_IMPLEMENTED;
#endif

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }
    callback->OnConnectWiFiResult(
        err == CHIP_NO_ERROR ? NetworkCommissioningStatus::kSuccess : NetworkCommissioningStatus::kUnknownError, CharSpan(), 0);
}

void LinuxNetworkCommissioningDelegate::ScanWiFiNetworks(ByteSpan ssid, WiFiNetworkCommissioningDelegate::Callback * callback)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    CHIP_ERROR err = DeviceLayer::ConnectivityMgrImpl().StartWiFiScan(ssid, callback);
#else
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
#endif
    if (err != CHIP_NO_ERROR)
    {
        callback->OnScanFinished(err, CharSpan(),
                                 Span<app::Clusters::NetworkCommissioning::Structs::WiFiInterfaceScanResult::Type>());
    }
}

void LinuxNetworkCommissioningDelegate::ConnectThreadNetwork(ByteSpan threadData,
                                                             ThreadNetworkCommissioningDelegate::Callback * callback)
{
    CHIP_ERROR err = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(threadData, callback);
    if (err != CHIP_NO_ERROR)
    {
        // If err is CHIP_NO_ERROR, then connect thread network result will be delivered asynchronously.
        callback->OnConnectThreadResult(NetworkCommissioningStatus::kUnknownError, CharSpan(), 0);
    }
}

void LinuxNetworkCommissioningDelegate::ScanThreadNetworks(ThreadNetworkCommissioningDelegate::Callback * callback)
{
    CHIP_ERROR err = DeviceLayer::ThreadStackMgrImpl().StartThreadScan(callback);
    if (err != CHIP_NO_ERROR)
    {
        callback->OnScanFinished(err, CharSpan(),
                                 Span<app::Clusters::NetworkCommissioning::Structs::ThreadInterfaceScanResult::Type>());
    }
}

} // namespace DeviceLayer
} // namespace chip
