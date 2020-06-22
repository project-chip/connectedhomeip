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

#include "FreeRTOS.h"
#include "nrf_log.h"
#include "task.h"
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

#include "DataModelHandler.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;

// Transport Callbacks
namespace {

#ifndef EXAMPLE_SERVER_NODEID
// "lock"
#define EXAMPLE_SERVER_NODEID 0x6c6f636b
#endif // EXAMPLE_SERVER_NODEID

const uint8_t local_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                      0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                      0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

const uint8_t remote_public_key[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f,
                                      0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6,
                                      0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd,
                                      0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae,
                                      0xe8, 0x87, 0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

void newConnectionHandler(PeerConnectionState * state, SecureSessionMgr * transport)
{
    CHIP_ERROR err;

    ESP_LOGI(TAG, "Received a new connection.");

    err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                               sizeof(local_private_key));
    VerifyOrExit(err == CHIP_NO_ERROR, ESP_LOGE(TAG, "Failed to setup encryption"));

exit:
    return;
}

static void OnRecv(const MessageHeader & header, const IPPacketInfo & packet_info, System::PacketBuffer * buffer,
                   SecureSessionMgr * transport)
{
    const size_t data_len = buffer->DataLength();
    char src_addr[128];

    // as soon as a client connects, assume it is connected
    VerifyOrExit(transport != NULL && buffer != NULL, NRF_LOG_INFO("Received data but couldn't process it..."));
    VerifyOrExit(header.GetSourceNodeId().HasValue(), NRF_LOG_INFO("Unknown source for received message"));

    VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, ESP_LOGE(TAG, "Unknown source for received message"));

    state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

    NRF_LOG_INFO("Packet received from %s (%zu bytes)", src_addr, static_cast<size_t>(data_len));

    HandleDataModelMessage(buffer);
    buffer = NULL;

exit:
    // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
    if (buffer != NULL)
    {
        System::PacketBuffer::Free(buffer);
    }
}

} // namespace

// The echo server assumes the platform's networking has been setup already
void SetupTransport(IPAddressType type, SecureSessionMgr * transport)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = transport->Init(EXAMPLE_SERVER_NODEID, &DeviceLayer::InetLayer, UdpListenParameters().SetAddressType(type));
    SuccessOrExit(err);

    transport->SetMessageReceiveHandler(OnRecv, transport);
    transport->SetNewConnectionHandler(newConnectionHandler, transport);

exit:
    if (err != CHIP_NO_ERROR)
    {
        NRF_LOG_ERROR("ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        NRF_LOG_INFO("Lock Server Listening...");
    }
}

// The echo server assumes the platform's networking has been setup already
void StartServer(SecureSessionMgr * transport_ipv6)
{
    SetupTransport(kIPAddressType_IPv6, transport_ipv6);
}
