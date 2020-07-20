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

#include "chip-zcl/chip-zcl.h"
extern "C" {
#include "gen/gen-cluster-id.h"
#include "gen/gen-types.h"
}

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

const uint8_t local_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                      0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                      0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

const uint8_t remote_public_key[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f,
                                      0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6,
                                      0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd,
                                      0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae,
                                      0xe8, 0x87, 0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

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

        HandleDataModelMessage(buffer);
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
        CHIP_ERROR err;

        LOG_INF("Received a new connection.");

        err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                                   sizeof(local_private_key));

        if (err != CHIP_NO_ERROR)
            LOG_INF("Failed to setup encryption");
    }

private:
    /**
     * Handle a message that should be processed via our data model processing
     * codepath.
     *
     * @param [in] buffer The buffer holding the message.  This function guarantees
     *                    that it will free the buffer before returning.
     */
    void HandleDataModelMessage(System::PacketBuffer * buffer)
    {
        ChipZclStatus_t zclStatus = chipZclProcessIncoming((ChipZclBuffer_t *) buffer);

        if (zclStatus == CHIP_ZCL_STATUS_SUCCESS)
            LOG_INF("Data model processing success!");
        else
            LOG_INF("Data model processing failure: %d", zclStatus);

        System::PacketBuffer::Free(buffer);
    }
};

static ServerCallback gCallbacks;

} // namespace

// The echo server assumes the platform's networking has been setup already
void StartServer(DemoSessionManager * sessions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = sessions->Init(EXAMPLE_SERVER_NODEID, &DeviceLayer::SystemLayer,
                         UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
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
    chipZclEndpointInit();
}
