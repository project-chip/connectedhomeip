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
#include <transport/raw/Tuple.h>
#include <transport/raw/UDP.h>

#include "DataModelHandler.h"

static const char * TAG = "echo_server";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;

extern const NodeId kLocalNodeId = 12344321;

namespace {

/**
 * @brief implements something like "od -c", changes an arbitrary byte string
 *   into a single-line of ascii.  Destroys any byte-wise encoding that
 *   might be present, e.g. utf-8.
 *
 * @param bytes     potentially unprintable buffer
 * @param bytes_len length of bytes
 * @param out       where to put the printable string
 * @param out_len   length of out
 * @return size_t required size of output buffer, including null-termination
 */
static size_t odc(const uint8_t * bytes, size_t bytes_len, char * out, size_t out_len)
{
    size_t required = 1; // always need null termination
    memset(out, 0, out_len);
    // count and print
    for (; bytes_len > 0; bytes_len--, bytes++)
    {
        uint8_t byte = *bytes;

        if ((byte >= '\t' && byte <= '\r') || byte == '\\')
        {
            static const char * kCodes = "tnvfr";
            char code                  = (byte == '\\') ? '\\' : kCodes[byte - '\t'];
            required += 2;
            if (out_len > 2)
            {
                *out++ = '\\';
                *out++ = code;
                out_len -= 2;
            }
        }
        else if (byte >= ' ' && byte <= '~')
        {
            required += 1;
            if (out_len > 1)
            {
                *out++ = byte;
                out_len--;
            }
        }
        else
        {
            static const size_t kBinCodeLen = sizeof("\\xFF") - 1;
            static const char * kCodes      = "0123456789ABCDEF";

            required += kBinCodeLen;
            if (out_len > kBinCodeLen)
            {
                *out++ = '\\';
                *out++ = 'x';
                *out++ = kCodes[(byte & 0xf0) >> 4];
                *out++ = kCodes[byte & 0xf];
                out_len -= kBinCodeLen;
            }
        }
    }

    return required;
}

class EchoServerCallback : public SecureSessionMgrCallback
{
public:
    void OnMessageReceived(const PacketHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * buffer,
                           SecureSessionMgrBase * mgr) override
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

        // FIXME: Long-term we shouldn't be guessing what sort of message this is
        // based on the message bytes.  We're doing this for now to support both
        // data model messages and text echo messages, but in the long term we
        // should either do echo via a data model command or do echo on a separate
        // port from data model processing.
        if (ContentMayBeADataModelMessage(buffer))
        {
            HandleDataModelMessage(header, buffer, mgr);
            buffer = nullptr;
        }
        else
        {
            char logmsg[512];

            odc(buffer->Start(), data_len, logmsg, sizeof(logmsg));

            ESP_LOGI(TAG, "Client sent: %s", logmsg);

            // Attempt to echo back
            err    = mgr->SendMessage(header.GetSourceNodeId().Value(), buffer);
            buffer = nullptr;
            if (err != CHIP_NO_ERROR)
            {
                ESP_LOGE(TAG, "Unable to echo back to client: %s", ErrorStr(err));
            }
            else
            {
                ESP_LOGI(TAG, "Echo sent");
            }
        }

    exit:

        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != nullptr)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * mgr) override
    {
        ESP_LOGE(TAG, "ERROR: %s\n Got UDP error", ErrorStr(error));
    }

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override
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

EchoServerCallback gCallbacks;

SecureSessionMgr<Transport::UDP, // IPV6
                 Transport::UDP  // IPV4
                 >
    sessions;

} // namespace

namespace chip {
SecureSessionMgrBase & SessionManager()
{
    return sessions;
}
} // namespace chip

void PairingComplete(SecurePairingSession * pairing)
{
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);
    sessions.NewPairing(peer, pairing);
}

// The echo server assumes the platform's networking has been setup already
void startServer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = sessions.Init(kLocalNodeId, &DeviceLayer::SystemLayer,
                        UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6).SetInterfaceId(nullptr),
                        UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv4));
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
