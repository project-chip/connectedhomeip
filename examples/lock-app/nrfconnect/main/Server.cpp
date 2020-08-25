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

#include "attribute-storage.h"
#include "gen/znet-bookkeeping.h"
#include "util.h"
#include <app/chip-zcl-zpro-codec.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

#include <logging/log.h>

#include <string.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;

LOG_MODULE_DECLARE(app);

// Transport Callbacks
namespace {

#ifndef EXAMPLE_SERVER_NODEID
// "nRF5"
#define EXAMPLE_SERVER_NODEID 0x3546526e
#endif // EXAMPLE_SERVER_NODEID

class ServerCallback : public SecureSessionMgrCallback
{
public:
    void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * buffer,
                           SecureSessionMgrBase * mgr) override
    {
        const size_t data_len = buffer->DataLength();
        char src_addr[PeerAddress::kMaxToStringSize];
        const char * error = nullptr;

        // as soon as a client connects, assume it is connected
        VerifyOrExit(buffer != NULL, error = "Received data but couldn't process it...");
        VerifyOrExit(header.GetSourceNodeId().HasValue(), error = "Unknown source for received message");
        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, error = "Unknown source for received message");

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        LOG_INF("Packet received from %s: %zu bytes", log_strdup(src_addr), static_cast<size_t>(data_len));

        HandleDataModelMessage(header, buffer, mgr);
        buffer = nullptr;

    exit:
        if (error != nullptr)
            LOG_INF("%s", error);

        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != nullptr)
            System::PacketBuffer::Free(buffer);
    }

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override
    {
        LOG_INF("Received a new connection.");
    }

private:
    /**
     * Handle a message that should be processed via our data model processing
     * codepath.
     *
     * @param [in] buffer The buffer holding the message.  This function guarantees
     *                    that it will free the buffer before returning.
     */
    void HandleDataModelMessage(const MessageHeader & header, System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
    {
        EmberApsFrame frame;
        bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
        if (ok)
        {
            LOG_INF("APS frame processing success!");
        }
        else
        {
            LOG_INF("Aps frame processing failure!");
            System::PacketBuffer::Free(buffer);
            return;
        }

        ChipResponseDestination responseDest(header.GetSourceNodeId().Value(), mgr);
        uint8_t * message;
        uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
        ok                  = emberAfProcessMessage(&frame,
                                   0, // type
                                   message, messageLen,
                                   &responseDest, // source identifier
                                   NULL);

        System::PacketBuffer::Free(buffer);

        if (ok)
            LOG_INF("Data model processing success!");
        else
            LOG_INF("Data model processing failure!");
    }
};

static ServerCallback gCallbacks;
static SecurePairingUsingTestSecret gTestPairing;

} // namespace

// The echo server assumes the platform's networking has been setup already
void StartServer(DemoSessionManager * sessions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    err = sessions->Init(EXAMPLE_SERVER_NODEID, &DeviceLayer::SystemLayer,
                         UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    err = sessions->NewPairing(Optional<NodeId>::Value(kUndefinedNodeId), peer, 0, 0, &gTestPairing);
    SuccessOrExit(err);

    sessions->SetDelegate(&gCallbacks);

exit:
    if (err != CHIP_NO_ERROR)
        LOG_ERR("ERROR setting up transport: %s", ErrorStr(err));
    else
        LOG_INF("Lock Server Listening...");
}

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emAfInit();
}
