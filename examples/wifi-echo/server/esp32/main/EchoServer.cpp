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
#include "tcpip_adapter.h"

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
#include <transport/Tuple.h>
#include <transport/UDP.h>

#include "DataModelHandler.h"
#include "LEDWidget.h"

static const char * TAG = "echo_server";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;

constexpr NodeId kLocalNodeId = 12344321;
extern LEDWidget statusLED; // In wifi-echo.cpp

namespace {

const unsigned char local_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                            0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                            0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

const unsigned char remote_public_key[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f,
                                            0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6,
                                            0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd,
                                            0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae,
                                            0xe8, 0x87, 0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

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
    void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * buffer,
                           SecureSessionMgrBase * mgr) override
    {
        CHIP_ERROR err;
        const size_t data_len = buffer->DataLength();

        // as soon as a client connects, assume it is connected
        VerifyOrExit(mgr != NULL && buffer != NULL, ESP_LOGE(TAG, "Received data but couldn't process it..."));
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
            buffer = NULL;
        }
        else
        {
            char logmsg[512];

            odc(buffer->Start(), data_len, logmsg, sizeof(logmsg));

            ESP_LOGI(TAG, "Client sent: %s", logmsg);

            // Attempt to echo back
            err    = mgr->SendMessage(header.GetSourceNodeId().Value(), buffer);
            buffer = NULL;
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
        if (buffer != NULL)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * mgr) override
    {
        ESP_LOGE(TAG, "ERROR: %s\n Got UDP error", ErrorStr(error));
        statusLED.BlinkOnError();
    }

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override
    {
        CHIP_ERROR err;

        ESP_LOGI(TAG, "Received a new connection.");

        err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                                   sizeof(local_private_key));
        VerifyOrExit(err == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to setup encryption"));

    exit:
        return;
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

// The echo server assumes the platform's networking has been setup already
void startServer()
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    struct netif * ipV6NetIf = NULL;
    tcpip_adapter_get_netif(TCPIP_ADAPTER_IF_AP, (void **) &ipV6NetIf);

    err = sessions.Init(kLocalNodeId, &DeviceLayer::SystemLayer,
                        UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6).SetInterfaceId(ipV6NetIf),
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
