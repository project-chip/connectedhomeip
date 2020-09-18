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

#include "Server.h"

#include <string.h>

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

#include <DataModelHandler.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

#ifndef EXAMPLE_SERVER_NODEID
#define EXAMPLE_SERVER_NODEID 12344321
#endif // EXAMPLE_SERVER_NODEID

// Transport Callbacks

DemoSessionManager sessions;

namespace chip {

SecureSessionMgrBase & SessionManager()
{
    return sessions;
}

} // namespace chip

namespace {

void HandleBLEConnectionClosed(chip::Ble::BLEEndPoint * endPoint, BLE_ERROR err)
{
    if (err != BLE_NO_ERROR)
    {
        ChipLogError(AppServer, "BLE Error: %d", err);
    }
    ChipLogProgress(AppServer, "BLE Connection closed");
}

void HandleBLEMessageReceived(chip::Ble::BLEEndPoint * endPoint, chip::System::PacketBuffer * buffer)
{
#if CHIP_ENABLE_OPENTHREAD
    uint16_t bufferLen = buffer->DataLength();
    uint8_t * data     = buffer->Start();
    chip::DeviceLayer::Internal::DeviceNetworkInfo networkInfo;
    ChipLogProgress(AppServer, "Receive BLE message size=%u", bufferLen);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadNetworkName),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadNetworkName, data, sizeof(networkInfo.ThreadNetworkName));
    data += sizeof(networkInfo.ThreadNetworkName);
    bufferLen -= sizeof(networkInfo.ThreadNetworkName);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadExtendedPANId),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadExtendedPANId, data, sizeof(networkInfo.ThreadExtendedPANId));
    data += sizeof(networkInfo.ThreadExtendedPANId);
    bufferLen -= sizeof(networkInfo.ThreadExtendedPANId);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadMeshPrefix),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadMeshPrefix, data, sizeof(networkInfo.ThreadMeshPrefix));
    data += sizeof(networkInfo.ThreadMeshPrefix);
    bufferLen -= sizeof(networkInfo.ThreadMeshPrefix);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadNetworkKey),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadNetworkKey, data, sizeof(networkInfo.ThreadNetworkKey));
    data += sizeof(networkInfo.ThreadNetworkKey);
    bufferLen -= sizeof(networkInfo.ThreadNetworkKey);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadPSKc), ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadPSKc, data, sizeof(networkInfo.ThreadPSKc));
    data += sizeof(networkInfo.ThreadPSKc);
    bufferLen -= sizeof(networkInfo.ThreadPSKc);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadPANId), ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.ThreadPANId = data[0] | (data[1] << 8);
    data += sizeof(networkInfo.ThreadPANId);
    bufferLen -= sizeof(networkInfo.ThreadPANId);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadChannel), ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.ThreadChannel = data[0];
    data += sizeof(networkInfo.ThreadChannel);
    bufferLen -= sizeof(networkInfo.ThreadChannel);

    VerifyOrExit(bufferLen >= 3, ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.FieldPresent.ThreadExtendedPANId = *data;
    data++;
    networkInfo.FieldPresent.ThreadMeshPrefix = *data;
    data++;
    networkInfo.FieldPresent.ThreadPSKc = *data;
    data++;
    networkInfo.NetworkId              = 0;
    networkInfo.FieldPresent.NetworkId = true;

    ThreadStackMgr().SetThreadEnabled(false);
    ThreadStackMgr().SetThreadProvision(networkInfo);
    ThreadStackMgr().SetThreadEnabled(true);

#endif
exit:
    endPoint->Close();
    chip::System::PacketBuffer::Free(buffer);
}

void HandleBLEConnectionOpened(chip::Ble::BLEEndPoint * endPoint)
{
    ChipLogProgress(AppServer, "BLE Connection opened");

    endPoint->OnMessageReceived  = HandleBLEMessageReceived;
    endPoint->OnConnectionClosed = HandleBLEConnectionClosed;
}

class ServerCallback : public SecureSessionMgrCallback
{
public:
    virtual void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                   System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
    {
        const size_t data_len = buffer->DataLength();
        char src_addr[PeerAddress::kMaxToStringSize];

        // as soon as a client connects, assume it is connected
        VerifyOrExit(buffer != NULL, ChipLogProgress(AppServer, "Received data but couldn't process it..."));
        VerifyOrExit(header.GetSourceNodeId().HasValue(), ChipLogProgress(AppServer, "Unknown source for received message"));

        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, ChipLogProgress(AppServer, "Unknown source for received message"));

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        ChipLogProgress(AppServer, "Packet received from %s: %zu bytes", src_addr, static_cast<size_t>(data_len));

        HandleDataModelMessage(header, buffer, mgr);
        buffer = NULL;

    exit:
        // HandleDataModelMessage calls Free on the buffer without an AddRef, if HandleDataModelMessage was not called, free the
        // buffer.
        if (buffer != NULL)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    virtual void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr)
    {
        ChipLogProgress(AppServer, "Received a new connection.");
    }
};

ServerCallback gCallbacks;
SecurePairingUsingTestSecret gTestPairing;

} // namespace

// The function will initialize datamodel handler and then start the server
// The server assumes the platform's networking has been setup already
void InitServer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    InitDataModelHandler();

    err = sessions.Init(EXAMPLE_SERVER_NODEID, &DeviceLayer::SystemLayer,
                        UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    // TODO: ESP32 echo server supports actual pairing, needs to investigate how to share this with ESP32
    err = sessions.NewPairing(peer, &gTestPairing);
    SuccessOrExit(err);

    sessions.SetDelegate(&gCallbacks);

    chip::DeviceLayer::ConnectivityMgr().AddCHIPoBLEConnectionHandler(HandleBLEConnectionOpened);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        ChipLogProgress(AppServer, "Server Listening...");
    }
}
