/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>

#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/bouffalolab/BL702/NetworkCommissioningDriver.h>
#include <platform/bouffalolab/BL702/WiFiInterface.h>
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <eth_bd.h>
#include <platform/bouffalolab/BL702/EthernetInterface.h>
#endif

using namespace ::chip;

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
void ConnectivityManagerImpl::OnWiFiStationDisconnected()
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEventOrDie(&event);

    NetworkCommissioning::BLWiFiDriver::GetInstance().SetLastDisconnectReason(NULL);
    uint16_t reason = NetworkCommissioning::BLWiFiDriver::GetInstance().GetLastDisconnectReason();
    uint8_t associationFailureCause =
        chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kUnknown);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();
    if (delegate)
    {
        delegate->OnDisconnectionDetected(reason);
        delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum::kNotConnected));
    }
}

extern "C" void wifiInterface_eventConnected(struct netif * interface)
{
    ChipLogProgress(DeviceLayer, "wifiInterface_eventConnected");
    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting_Succeeded);
}

extern "C" void wifiInterface_eventDisconnected(struct netif * interface)
{
    ChipLogProgress(DeviceLayer, "wifiInterface_eventDisconnected");
    if (ConnectivityManager::kWiFiStationState_Connecting == ConnectivityMgrImpl().GetWiFiStationState())
    {
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting_Failed);
    }
    else
    {
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_NotConnected);
    }
}

extern "C" void wifiInterface_eventLinkStatusDone(struct netif * interface, netbus_fs_link_status_ind_cmd_msg_t * pkg_data)
{
    ChipLogProgress(DeviceLayer, "wifiInterface_eventLinkStatusDone");

    struct bflbwifi_ap_record * record = &pkg_data->record;
    if (record->link_status == BF1B_WIFI_LINK_STATUS_UP)
    {
        ChipLogProgress(DeviceLayer, "link status up!");
    }
    else if (record->link_status == BF1B_WIFI_LINK_STATUS_DOWN)
    {
        ChipLogProgress(DeviceLayer, "link status down!");
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Disconnecting);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "link status unknown!");
    }
}

extern "C" void wifiInterface_eventGotIP(struct netif * interface)
{
    ChipLogProgress(DeviceLayer, "wifiInterface_eventGotIP");
    ConnectivityMgrImpl().OnConnectivityChanged(interface);
    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connected);
}

extern "C" void wifiInterface_eventScanDone(struct netif * interface, netbus_fs_scan_ind_cmd_msg_t * pmsg)
{
    ChipLogProgress(DeviceLayer, "wifiInterface_eventScanDone");
    NetworkCommissioning::BLWiFiDriver::GetInstance().OnScanWiFiNetworkDone(pmsg);
}
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
extern "C" void ethernetInterface_eventGotIP(struct netif * interface)
{
    ChipLogProgress(DeviceLayer, "ethernetInterface_eventGotIP");
    ConnectivityMgrImpl().OnConnectivityChanged(interface);
}
#endif

} // namespace DeviceLayer
} // namespace chip
