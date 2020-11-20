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

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include <algorithm>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/Tuple.h>
#include <transport/raw/UDP.h>

#include "DataModelHandler.h"

static const char * TAG = "echo_server";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;

extern const NodeId kLocalNodeId = 12344321;

namespace {

class ResponseServerCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader,
                           const Transport::PeerConnectionState * state, System::PacketBuffer * buffer,
                           SecureSessionMgr * mgr) override
    {
        CHIP_ERROR err;
        const size_t data_len = buffer->DataLength();

        // as soon as a client connects, assume it is connected
        VerifyOrExit(mgr != nullptr && buffer != nullptr, ESP_LOGE(TAG, "Received data but couldn't process it..."));
        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, ESP_LOGE(TAG, "Unknown source for received message"));

        {
            char src_addr[Transport::PeerAddress::kMaxToStringSize];

            state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

            ESP_LOGI(TAG, "Packet received from %s: %zu bytes", src_addr, static_cast<size_t>(data_len));
        }

        HandleDataModelMessage(header, buffer, mgr);
        buffer = nullptr;

    exit:

        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != nullptr)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * mgr) override
    {
        ESP_LOGE(TAG, "ERROR: %s\n Got UDP error", ErrorStr(error));
    }

    void OnNewConnection(const Transport::PeerConnectionState * state, SecureSessionMgr * mgr) override
    {
        ESP_LOGI(TAG, "Received a new connection.");
    }

private:
    /**
     * A data model message has nonzero length and always has a first byte whose
     * value is one of: 0x00, 0x01, 0x02, 0x03.  See chipZclEncodeZclHeader for the
     * construction of the message and in particular the first byte.
     *
     * Echo messages should generally not have a first byte with those values, so we
     * can use that to try to distinguish between the two.
     */
    bool ContentMayBeADataModelMessage(System::PacketBuffer * buffer)
    {
        const size_t data_len      = buffer->DataLength();
        const uint8_t * data       = buffer->Start();
        bool maybeDataModelMessage = true;

        // Has to have nonzero length.
        VerifyOrExit(data_len > 0, maybeDataModelMessage = false);

        // Has to have a valid first byte value.
        VerifyOrExit(data[0] < 0x04, maybeDataModelMessage = false);

    exit:
        return maybeDataModelMessage;
    }
};

ResponseServerCallback gCallbacks;
TransportMgr<Transport::UDP, // IPV6
             Transport::UDP  // IPV4
             >
    gTransports;
SecureSessionMgr sessions;

} // namespace

namespace chip {
SecureSessionMgr & SessionManager()
{
    return sessions;
}
} // namespace chip

// The echo server assumes the platform's networking has been setup already
void startServer(NodeId localNodeId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6).SetInterfaceId(nullptr),
                           UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv4));
    SuccessOrExit(err);
    err = sessions.Init(localNodeId, &DeviceLayer::SystemLayer, &gTransports);
    SuccessOrExit(err);

    sessions.SetDelegate(&gCallbacks);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        ESP_LOGI(TAG, "Echo Server Listening...");
    }
}

void PairingComplete(NodeId assignedNodeId, NodeId peerNodeId, SecurePairingSession * pairing)
{
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);
    startServer(assignedNodeId);
    sessions.NewPairing(peer, peerNodeId, pairing);
}
