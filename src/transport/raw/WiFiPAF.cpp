/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
using namespace chip::WiFiPAF;

namespace chip {
namespace Transport {
CHIP_ERROR WiFiPAFBase::Init(const WiFiPAFListenParameters & param)
{
    ChipLogDetail(Inet, "WiFiPAFBase::Init - setting/overriding transport");
    mWiFiPAFLayer = DeviceLayer::ConnectivityMgr().GetWiFiPAF();
    SetWiFiPAFLayerTransportToSelf();
    mWiFiPAFLayer->SetWiFiPAFState(State::kInitialized);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::SendMessage(const Transport::PeerAddress & address, PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kWiFiPAF, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mWiFiPAFLayer->GetWiFiPAFState() != State::kNotReady, CHIP_ERROR_INCORRECT_STATE);

    WiFiPAFSession sessionInfo = { .nodeId = address.GetRemoteId() };
    WiFiPAFSession * pTxInfo   = mWiFiPAFLayer->GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccNodeId, sessionInfo);
    if (pTxInfo == nullptr)
    {
        /*
            The session does not exist
        */
        ChipLogError(Inet, "WiFi-PAF: No valid session whose nodeId: %lu", address.GetRemoteId());
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mWiFiPAFLayer->SendMessage(*pTxInfo, std::move(msgBuf));

    return CHIP_NO_ERROR;
}

bool WiFiPAFBase::CanSendToPeer(const Transport::PeerAddress & address)
{
    if (mWiFiPAFLayer != nullptr)
    {
        return (mWiFiPAFLayer->GetWiFiPAFState() != State::kNotReady) && (address.GetTransportType() == Type::kWiFiPAF);
    }
    return false;
}

CHIP_ERROR WiFiPAFBase::WiFiPAFMessageReceived(WiFiPAFSession & RxInfo, PacketBufferHandle && buffer)
{
    auto pPafInfo = mWiFiPAFLayer->GetPAFInfo(chip::WiFiPAF::PafInfoAccess::kAccSessionId, RxInfo);
    if (pPafInfo == nullptr)
    {
        /*
            The session does not exist
        */
        ChipLogError(Inet, "WiFi-PAF: No valid session whose Id: %u", RxInfo.id);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ((pPafInfo->id != RxInfo.id) || (pPafInfo->peer_id != RxInfo.peer_id) ||
        (memcmp(pPafInfo->peer_addr, RxInfo.peer_addr, sizeof(uint8_t) * 6) != 0))
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
        return CHIP_ERROR_INCORRECT_STATE;
    }
    HandleMessageReceived(Transport::PeerAddress(Transport::Type::kWiFiPAF, pPafInfo->nodeId), std::move(buffer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::WiFiPAFMessageSend(WiFiPAFSession & TxInfo, PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(mWiFiPAFLayer->GetWiFiPAFState() != State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    DeviceLayer::ConnectivityMgr().WiFiPAFSend(TxInfo, std::move(msgBuf));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFBase::WiFiPAFCloseSession(WiFiPAFSession & SessionInfo)
{
    VerifyOrReturnError(mWiFiPAFLayer->GetWiFiPAFState() != State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    DeviceLayer::ConnectivityMgr().WiFiPAFShutdown(SessionInfo.id, SessionInfo.role);
    mWiFiPAFLayer->SetWiFiPAFState(State::kInitialized);

    return CHIP_NO_ERROR;
}

bool WiFiPAFBase::WiFiPAFResourceAvailable(void)
{
    return DeviceLayer::ConnectivityMgr().WiFiPAFResourceAvailable();
}

CHIP_ERROR WiFiPAFBase::SendAfterConnect(PacketBufferHandle && msg)
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
