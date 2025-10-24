/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    Copyright (c) 2025 NXP
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

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <algorithm>
#include <cstdlib>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <credentials/CHIPCert.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include <platform/Linux/WiFiPAFDriver.h>
#include <wifipaf/WiFiPAFLayer.h>

using namespace ::chip::DeviceLayer;
using namespace ::chip::WiFiPAF;

namespace chip {
namespace DeviceLayer {

PAFPublishSSI WiFiPAFDriver::BuildSSI()
{
    PAFPublishSSI PafPublish_ssi;

    PafPublish_ssi.DevOpCode = 0;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(PafPublish_ssi.ProductId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.ProductId = 0;
    }
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(PafPublish_ssi.VendorId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.VendorId = 0;
    }

    return PafPublish_ssi;
}

CHIP_ERROR WiFiPAFDriver::_Publish(uint32_t publish_id)
{
    WiFiPAFSession sessionInfo  = { .role = WiFiPafRole::kWiFiPafRole_Publisher, .id = publish_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    ReturnErrorOnFailure(WiFiPafLayer.AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFDriver::_Subscribe(uint32_t subscribe_id, uint16_t discriminator)
{
    WiFiPAFSession sessionInfo  = { .discriminator = discriminator };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    if (pPafInfo != nullptr)
    {
        pPafInfo->id   = subscribe_id;
        pPafInfo->role = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber;
    }
    return CHIP_NO_ERROR;
}

void WiFiPAFDriver::_OnDiscoveryResult(uint32_t subscribe_id, uint32_t peer_publish_id, uint32_t srv_proto_type,
                                       std::array<uint8_t, 6> peer_addr, const PAFPublishSSI * pPublishSSI)
{
    DeviceLayer::ConnectivityMgr().GetWiFiPAF()->SetWiFiPAFState(WiFiPAF::State::kConnected);

    /*
        Error Checking
    */
    WiFiPAFSession sessionInfo  = { .discriminator = pPublishSSI->DevInfo };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    if (pPafInfo == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, no valid session with discriminator: %u", pPublishSSI->DevInfo);
        return;
    }
    if ((pPafInfo->id == subscribe_id) && (pPafInfo->peer_id != UINT32_MAX))
    {
        // Reentrance, depends on wpa_supplicant behaviors
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, reentrance, subscribe_id: %u ", subscribe_id);
        return;
    }
    if (srv_proto_type != nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, Incorrect Protocol Type: %u, exp: %u", srv_proto_type,
                     nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER);
        return;
    }

    /*
        Set the PAF session information
    */
    ChipLogProgress(DeviceLayer, "WiFi-PAF: DiscoveryResult, set PafInfo, whose nodeId: %lu", pPafInfo->nodeId);
    ChipLogProgress(DeviceLayer, "\t (subscribe_id, peer_publish_id): (%u, %u)", subscribe_id, peer_publish_id);
    ChipLogProgress(DeviceLayer, "\t peer_addr: [%02x:%02x:%02x:%02x:%02x:%02x]", peer_addr[0], peer_addr[1], peer_addr[2],
                    peer_addr[3], peer_addr[4], peer_addr[5]);
    ChipLogProgress(DeviceLayer, "\t DevInfo: %x", pPublishSSI->DevInfo);

    pPafInfo->role    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber;
    pPafInfo->id      = subscribe_id;
    pPafInfo->peer_id = peer_publish_id;
    memcpy(pPafInfo->peer_addr, peer_addr.data(), sizeof(uint8_t) * 6);
    /*
        Indicate the connection event
    */
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFConnected };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, pPafInfo, sizeof(chip::WiFiPAF::WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void WiFiPAFDriver::_OnReplied(uint32_t peer_subscribe_id, uint32_t publish_id, uint32_t srv_proto_type,
                               std::array<uint8_t, 6> peer_addr, const PAFPublishSSI * pPublishSSI)
{
    /*
        Error Checking
    */
    uint16_t SetupDiscriminator;
    DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(SetupDiscriminator);
    if ((pPublishSSI->DevInfo != SetupDiscriminator) || (srv_proto_type != nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER))
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: OnReplied, mismatched discriminator, got %u, ours: %u", pPublishSSI->DevInfo,
                        SetupDiscriminator);
        return;
    }
    WiFiPAFSession sessionInfo  = { .id = publish_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccSessionId, sessionInfo);
    if (pPafInfo == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: OnReplied, no valid session with publish_id: %d", publish_id);
        return;
    }
    if ((pPafInfo->role == WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher) && (pPafInfo->peer_id == peer_subscribe_id) &&
        !memcmp(pPafInfo->peer_addr, peer_addr.data(), sizeof(uint8_t) * 6))
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: OnReplied, reentrance, publish_id: %u ", publish_id);
        return;
    }
    /*
        Set the PAF session information
    */
    ChipLogProgress(DeviceLayer, "WiFi-PAF: OnReplied, set PafInfo, whose nodeId: %lu", pPafInfo->nodeId);
    ChipLogProgress(DeviceLayer, "\t (publish_id, peer_subscribe_id): (%u, %u)", publish_id, peer_subscribe_id);
    ChipLogProgress(DeviceLayer, "\t peer_addr: [%02x:%02x:%02x:%02x:%02x:%02x]", peer_addr[0], peer_addr[1], peer_addr[2],
                    peer_addr[3], peer_addr[4], peer_addr[5]);
    ChipLogProgress(DeviceLayer, "\t DevInfo: %x", pPublishSSI->DevInfo);

    pPafInfo->role    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher;
    pPafInfo->id      = publish_id;
    pPafInfo->peer_id = peer_subscribe_id;
    memcpy(pPafInfo->peer_addr, peer_addr.data(), sizeof(uint8_t) * 6);
    WiFiPafLayer.HandleTransportConnectionInitiated(*pPafInfo);
}

void WiFiPAFDriver::_OnNanReceive(const WiFiPAF::WiFiPAFSession & rxInfo, System::PacketBufferHandle rxData)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: wpa_supplicant: nan-rx: [len: %lu]", rxData->DataLength());

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event{ .Type                 = DeviceEventType::kCHIPoWiFiPAFReceived,
                           .CHIPoWiFiPAFReceived = { .Data = std::move(rxData).UnsafeRelease() } };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &rxInfo, sizeof(WiFiPAF::WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void WiFiPAFDriver::_OnNanPublishTerminated(uint32_t publish_id, const char * reason)
{
    ChipLogProgress(Controller, "WiFi-PAF: Publish terminated (%u, %s)", publish_id, reason);

    WiFiPAFSession sessionInfo  = { .id = publish_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
}

void WiFiPAFDriver::_OnNanSubscribeTerminated(uint32_t subscribe_id, const char * reason)
{
    ChipLogProgress(Controller, "WiFi-PAF: Subscription terminated (%u, %s)", subscribe_id, reason);
    WiFiPAFSession sessionInfo  = { .id = subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
    /*
        Indicate the connection event
    */
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFCancelConnect };
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR WiFiPAFDriver::_Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    switch (role)
    {
    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher:
        return CancelPublish(id);
    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber:
        return CancelSubscribe(id);
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR WiFiPAFDriver::_Send(const WiFiPAF::WiFiPAFSession & TxInfo, bool result)
{
    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFWriteDone };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &TxInfo, sizeof(chip::WiFiPAF::WiFiPAFSession));
    event.CHIPoWiFiPAFReceived.result = result;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
}
} // namespace DeviceLayer
} // namespace chip
