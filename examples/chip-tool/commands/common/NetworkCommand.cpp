/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "NetworkCommand.h"

#include "Configuration.h"

using namespace ::chip;
using namespace ::chip::DeviceController;
using namespace ::chip::System;

static void onConnect(ChipDeviceController * dc, Transport::PeerConnectionState * state, void * appReqState)
{
    ChipLogDetail(chipTool, "OnConnect");

    NetworkCommand * command = reinterpret_cast<NetworkCommand *>(dc->AppState);
    command->OnConnect(dc);
}

static void onError(ChipDeviceController * dc, void * appReqState, CHIP_ERROR err, const Inet::IPPacketInfo * pi)
{
    ChipLogError(chipTool, "OnError: %s", ErrorStr(err));

    NetworkCommand * command = reinterpret_cast<NetworkCommand *>(dc->AppState);
    command->OnError(dc, err);
}

static void onMessage(ChipDeviceController * dc, void * appReqState, PacketBuffer * buffer)
{
    ChipLogDetail(chipTool, "OnMessage: Received %zu bytes", buffer->DataLength());

    NetworkCommand * command = reinterpret_cast<NetworkCommand *>(dc->AppState);
    command->OnMessage(dc, buffer);

    PacketBuffer::Free(buffer);
}

CHIP_ERROR NetworkCommand::Run(ChipDeviceController * dc, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    dc->AppState = reinterpret_cast<void *>(this);

    switch (mNetworkType)
    {
    case NetworkType::BLE:
        err = ConnectBLE(dc, remoteId);
        break;

    case NetworkType::UDP:
        err = ConnectUDP(dc, remoteId);
        break;

    case NetworkType::ALL:
        ChipLogError(chipTool, "Not implemented yet.");
        err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        break;
    }

    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Failed to connect to the device"));

exit:
    return err;
}

CHIP_ERROR NetworkCommand::ConnectBLE(ChipDeviceController * dc, NodeId remoteId)
{
    snprintf(mName, sizeof(mName), "BLE:%u", mDiscriminator);

    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator);
    return dc->ConnectDevice(remoteId, params, NULL, onConnect, onMessage, onError);
}

CHIP_ERROR NetworkCommand::ConnectUDP(ChipDeviceController * dc, NodeId remoteId)
{
    char hostIpStr[40];
    mRemoteAddr.address.ToString(hostIpStr, sizeof(hostIpStr));
    mRemotePort = GetRemotePort();
    snprintf(mName, sizeof(mName), "%s:%d", hostIpStr, mRemotePort);

    return dc->ConnectDeviceWithoutSecurePairing(remoteId, mRemoteAddr.address, nullptr, onConnect, onMessage, onError, mRemotePort,
                                                 mRemoteAddr.interfaceId);
}
