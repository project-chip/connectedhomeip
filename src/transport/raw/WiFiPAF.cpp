/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 *    @file
 *      This file implements the Matter Connection object that maintains a Wi-Fi PAF connection
 *
 */

#include <inttypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/WiFiPAF.h>

using namespace chip::System;

namespace chip {
namespace Transport {
CHIP_ERROR WiFiPAFBase::Init(const WiFiPAFListenParameters & param)
{
    ChipLogDetail(Inet, "WiFiPAFBase::Init - setting/overriding transport");
    mWiFiPAFLayer = DeviceLayer::ConnectivityMgr().GetWiFiPAF();
    SetWiFiPAFLayerTransportToSelf();
    mWiFiPAFLayer->SetWiFiPAFState(chip::WiFiPAF::State::kInitialized);

    if (!DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
    {
        ChipLogError(Inet, "Wi-Fi Management has not started, do it now.");
        static constexpr useconds_t kWiFiStartCheckTimeUsec = WIFI_START_CHECK_TIME_USEC;
        static constexpr uint8_t kWiFiStartCheckAttempts    = WIFI_START_CHECK_ATTEMPTS;
        DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        {
            for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
            {
                if (DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
                {
                    break;
                }
                usleep(kWiFiStartCheckTimeUsec);
            }
        }
        if (!DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
        {
            ChipLogError(Inet, "Wi-Fi Management taking too long to start - device configuration will be reset.");
        }
        ChipLogProgress(Inet, "Wi-Fi Management is started");
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kWiFiPAF, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mWiFiPAFLayer->GetWiFiPAFState() != chip::WiFiPAF::State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

    chip::WiFiPAF::WiFiPAFSession * pTxInfo = mWiFiPAFLayer->GetPAFInfo(address.GetRemoteId());
    if (pTxInfo == nullptr)
    {
        /*
            The session does not exist
        */
        ChipLogError(Inet, "WiFi-PAF: No valid session whose nodeId: %lu", address.GetRemoteId());
        return CHIP_ERROR_INCORRECT_STATE;
    }
    DeviceLayer::ConnectivityMgr().WiFiPAFSend(*pTxInfo, std::move(msgBuf));

    return CHIP_NO_ERROR;
}

bool WiFiPAFBase::CanSendToPeer(const Transport::PeerAddress & address)
{
    if (mWiFiPAFLayer != nullptr)
    {
        return (mWiFiPAFLayer->GetWiFiPAFState() != chip::WiFiPAF::State::kNotReady) &&
            (address.GetTransportType() == Type::kWiFiPAF);
    }
    return false;
}

void WiFiPAFBase::OnWiFiPAFMessageReceived(chip::WiFiPAF::WiFiPAFSession & RxInfo, System::PacketBufferHandle && buffer)
{
    auto pPafInfo = mWiFiPAFLayer->GetPAFInfo(RxInfo.id);
    if (pPafInfo == nullptr)
    {
        /*
            The session does not exist
        */
        ChipLogError(Inet, "WiFi-PAF: No valid session whose Id: %u", RxInfo.id);
        return;
    }

    if ((pPafInfo->id != RxInfo.id) || (pPafInfo->peer_id != RxInfo.peer_id) ||
        memcmp(pPafInfo->peer_addr, RxInfo.peer_addr, sizeof(uint8_t) * 6))
    {
        /*
            The packet is from the wrong sender
        */
        ChipLogError(Inet, "WiFi-PAF: packet from unexpected node:");
        ChipLogError(Inet, "session: [id: %u, peer_id: %u, [%02x:%02x:%02x:%02x:%02x:%02x]", pPafInfo->id, pPafInfo->peer_id,
                     pPafInfo->peer_addr[0], pPafInfo->peer_addr[1], pPafInfo->peer_addr[2], pPafInfo->peer_addr[3],
                     pPafInfo->peer_addr[4], pPafInfo->peer_addr[5]);
        ChipLogError(Inet, "pkt: [id: %u, peer_id: %u, [%02x:%02x:%02x:%02x:%02x:%02x]", RxInfo.id, RxInfo.peer_id,
                     RxInfo.peer_addr[0], RxInfo.peer_addr[1], RxInfo.peer_addr[2], RxInfo.peer_addr[3], RxInfo.peer_addr[4],
                     RxInfo.peer_addr[5]);
        return;
    }
    HandleMessageReceived(Transport::PeerAddress(Transport::Type::kWiFiPAF, pPafInfo->nodeId), std::move(buffer));
}

CHIP_ERROR WiFiPAFBase::SendAfterConnect(System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (size_t i = 0; i < mPendingPacketsSize; i++)
    {
        if (mPendingPackets[i].IsNull())
        {
            mPendingPackets[i] = std::move(msg);
            err                = CHIP_NO_ERROR;
            break;
        }
    }

    return err;
}

} // namespace Transport
} // namespace chip
