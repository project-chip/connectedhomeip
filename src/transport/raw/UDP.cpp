/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements the CHIP Connection object that maintains a UDP connection.
 */
#include <transport/raw/UDP.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <transport/raw/MessageHeader.h>

#include <inttypes.h>

namespace chip {
namespace Transport {

UDP::~UDP()
{
    Close();
}

CHIP_ERROR UDP::Init(UdpListenParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mState != State::kNotReady)
    {
        Close();
    }

    err = params.GetInetLayer()->NewUDPEndPoint(&mUDPEndPoint);
    SuccessOrExit(err);

    ChipLogDetail(Inet, "UDP::Init bind&listen port=%d", params.GetListenPort());

    err = mUDPEndPoint->Bind(params.GetAddressType(), Inet::IPAddress::Any, params.GetListenPort(), params.GetInterfaceId());
    SuccessOrExit(err);

    err = mUDPEndPoint->Listen(OnUdpReceive, nullptr /*onReceiveError*/, this);
    SuccessOrExit(err);

    mUDPEndpointType = params.GetAddressType();

    mState = State::kInitialized;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Failed to initialize Udp transport: %s", ErrorStr(err));
        if (mUDPEndPoint)
        {
            mUDPEndPoint->Free();
            mUDPEndPoint = nullptr;
        }
    }

    return err;
}

void UDP::Close()
{
    if (mUDPEndPoint)
    {
        // Udp endpoint is only non null if udp endpoint is initialized and listening
        mUDPEndPoint->Close();
        mUDPEndPoint->Free();
        mUDPEndPoint = nullptr;
    }
    mState = State::kNotReady;
}

CHIP_ERROR UDP::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kUdp, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mUDPEndPoint != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Inet::IPPacketInfo addrInfo;
    addrInfo.Clear();

    addrInfo.DestAddress = address.GetIPAddress();
    addrInfo.DestPort    = address.GetPort();
    addrInfo.Interface   = address.GetInterface();

    return mUDPEndPoint->SendMsg(&addrInfo, std::move(msgBuf));
}

void UDP::OnUdpReceive(Inet::IPEndPointBasis * endPoint, System::PacketBufferHandle && buffer, const Inet::IPPacketInfo * pktInfo)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    UDP * udp               = reinterpret_cast<UDP *>(endPoint->AppState);
    PeerAddress peerAddress = PeerAddress::UDP(pktInfo->SrcAddress, pktInfo->SrcPort, pktInfo->Interface);

    udp->HandleMessageReceived(peerAddress, std::move(buffer));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to receive UDP message: %s", ErrorStr(err));
    }
}

} // namespace Transport
} // namespace chip
