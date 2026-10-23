/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// lwIP-backed transport for the Pigweed RPC server.
//
// This is a drop-in replacement for RpcService.cpp: it exposes the same
// chip::rpc::Start() entry point but carries HDLC framed RPC packets over a
// TCP socket (lwIP BSD sockets) instead of a UART / pw_sys_io stream. The
// pw::rpc::Server, service registration and HDLC framing are identical to the
// UART transport; only the byte source/sink differs.
//
// The design intentionally uses only blocking accept()/recv().

#include "RpcService.h"

#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/encoder.h"
#include "pw_rpc/channel.h"
#include "pw_span/span.h"
#include "pw_status/status.h"
#include "pw_stream/stream.h"

#include <lib/support/logging/TextOnlyLogging.h>

#include "lwip/sockets.h"

#include <array>
#include <cerrno>
#include <cstring>

#ifndef CHIP_PW_RPC_LWIP_SERVER_PORT
#define CHIP_PW_RPC_LWIP_SERVER_PORT 33000
#endif

// Close a connected client if it stays idle (no received bytes) for this long.
// This prevents a client that connects but sends no data from blocking the
// single-client accept loop and starving later clients.
//
// Requires lwIP to be built with LWIP_SO_RCVTIMEO == 1. On platforms where that
// option is disabled the timeout cannot be armed and the connection will block
// until the peer sends data or disconnects (logged as an error at connect time).
#ifndef CHIP_PW_RPC_LWIP_CLIENT_IDLE_TIMEOUT_MS
#define CHIP_PW_RPC_LWIP_CLIENT_IDLE_TIMEOUT_MS 30000
#endif

namespace chip {
namespace rpc {

namespace {

using std::byte;

constexpr size_t kMaxTransmissionUnit = 1500;

// File descriptor of the currently connected RPC client, or -1 when no client
// is connected. Written by the RPC task only.
int gClientFd = -1;

// May be nullptr. Serializes access to the output socket.
::chip::rpc::Mutex * gOutputMutex;

// A pw::stream::Writer that writes bytes to the connected TCP client socket.
class SocketWriter : public pw::stream::NonSeekableWriter
{
private:
    pw::Status DoWrite(pw::span<const std::byte> data) override
    {
        if (gClientFd < 0)
        {
            return pw::Status::Unavailable();
        }
        if (data.empty())
        {
            return pw::OkStatus();
        }

        const uint8_t * cursor = reinterpret_cast<const uint8_t *>(data.data());
        size_t remaining       = data.size();
        while (remaining > 0)
        {
            ssize_t sent = lwip_send(gClientFd, cursor, remaining, 0);
            if (sent <= 0)
            {
                return pw::Status::Unknown();
            }
            cursor += sent;
            remaining -= static_cast<size_t>(sent);
        }
        return pw::OkStatus();
    }
};

SocketWriter socketWriter;

// HDLC channel output over the TCP socket.
template <size_t buffer_size>
class ChipRpcSocketChannelOutput : public pw::rpc::ChannelOutput
{
public:
    constexpr ChipRpcSocketChannelOutput(pw::stream::Writer & writer, uint8_t address, const char * channel_name) :
        pw::rpc::ChannelOutput(channel_name), mWriter(writer), mAddress(address)
    {}

    pw::Status Send(pw::span<const std::byte> buffer) override
    {
        if (buffer.empty())
        {
            return pw::OkStatus();
        }
        gOutputMutex->Lock();
        pw::Status ret = pw::hdlc::WriteUIFrame(mAddress, buffer, mWriter);
        gOutputMutex->Unlock();
        return ret;
    }

private:
    pw::stream::Writer & mWriter;
    const uint8_t mAddress;
};

ChipRpcSocketChannelOutput<kMaxTransmissionUnit> hdlc_channel_output(socketWriter, pw::hdlc::kDefaultRpcAddress,
                                                                     "HDLC socket channel");

pw::rpc::Channel channels[] = { pw::rpc::Channel::Create<1>(&hdlc_channel_output) };

pw::rpc::Server server(channels);

// Create, bind and listen on the RPC server socket. Returns the listening
// socket fd, or -1 on failure.
int CreateListenSocket()
{
    int listen_fd = lwip_socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Error, "RPC: failed to create socket");
        return -1;
    }

    int reuse = 1;
    lwip_setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port   = lwip_htons(CHIP_PW_RPC_LWIP_SERVER_PORT);
    addr.sin6_addr   = in6addr_any;

    if (lwip_bind(listen_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Error, "RPC: bind failed");
        lwip_close(listen_fd);
        return -1;
    }

    if (lwip_listen(listen_fd, 1) < 0)
    {
        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Error, "RPC: listen failed");
        lwip_close(listen_fd);
        return -1;
    }

    return listen_fd;
}

// Disable the idle receive timeout on a connected client socket by reverting
// SO_RCVTIMEO to zero (blocking recv). Called once a real RPC session is
// established (first valid frame) so a long-lived server stream (e.g. the JFA
// GetStream call) is not torn down while it legitimately waits with no inbound
// frames. The pre-session idle guard still protects the single-client accept
// loop from a client that connects but never sends anything.
void ClearClientIdleTimeout(int fd)
{
    struct timeval no_timeout;
    no_timeout.tv_sec  = 0;
    no_timeout.tv_usec = 0;
    if (lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &no_timeout, sizeof(no_timeout)) < 0)
    {
        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Error, "RPC: failed to clear client idle timeout");
    }
}

} // namespace

void Start(void (*RegisterServices)(pw::rpc::Server &), ::chip::rpc::Mutex * output_mutex)
{
    PW_DASSERT(output_mutex != nullptr);
    PW_DASSERT(RegisterServices != nullptr);
    gOutputMutex = output_mutex;

    RegisterServices(server);

    int listen_fd = CreateListenSocket();
    if (listen_fd < 0)
    {
        return;
    }

    Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Detail, "Starting pw_rpc server (HDLC over TCP)");

    std::array<std::byte, kMaxTransmissionUnit> input_buffer;
    pw::hdlc::Decoder decoder(input_buffer);

    while (true)
    {
        struct sockaddr_in6 client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd        = lwip_accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_len);
        if (client_fd < 0)
        {
            continue;
        }

        gClientFd = client_fd;
        decoder.Clear();
        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Detail, "RPC: client connected");

        // True until the first valid RPC frame is seen on this connection. While
        // armed, an idle SO_RCVTIMEO drops a client that connects but never
        // speaks. It is cleared once a real session is established so a
        // long-lived server stream is not torn down (see ClearClientIdleTimeout).
        bool idleTimeoutArmed = true;

        // Arm an idle receive timeout so a client that connects but never sends
        // data does not block this single-client loop forever and starve other
        // clients. When the timeout elapses lwip_recv() returns -1 with errno
        // EAGAIN/EWOULDBLOCK and we close the connection to free the accept slot.
        struct timeval idle_timeout;
        idle_timeout.tv_sec  = CHIP_PW_RPC_LWIP_CLIENT_IDLE_TIMEOUT_MS / 1000;
        idle_timeout.tv_usec = (CHIP_PW_RPC_LWIP_CLIENT_IDLE_TIMEOUT_MS % 1000) * 1000;
        if (lwip_setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &idle_timeout, sizeof(idle_timeout)) < 0)
        {
            Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Error,
                         "RPC: failed to set client idle timeout (SO_RCVTIMEO unsupported?)");
        }

        // Read until the client disconnects or goes idle, then wait for a new
        // connection.
        std::array<std::byte, kMaxTransmissionUnit> read_buffer;
        while (true)
        {
            ssize_t received = lwip_recv(client_fd, read_buffer.data(), read_buffer.size(), 0);
            if (received == 0)
            {
                // Peer performed an orderly shutdown.
                break;
            }
            if (received < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // Idle timeout expired with no data; drop the connection so
                    // another client can be accepted.
                    Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Detail, "RPC: closing idle client");
                }
                break;
            }

            for (ssize_t i = 0; i < received; ++i)
            {
                if (auto result = decoder.Process(read_buffer[static_cast<size_t>(i)]); result.ok())
                {
                    pw::hdlc::Frame & frame = result.value();
                    if (frame.address() == pw::hdlc::kDefaultRpcAddress)
                    {
                        if (idleTimeoutArmed)
                        {
                            ClearClientIdleTimeout(client_fd);
                            idleTimeoutArmed = false;
                        }
                        server.ProcessPacket(frame.data()).IgnoreError();
                    }
                }
            }
        }

        Logging::Log(Logging::kLogModule_NotSpecified, Logging::kLogCategory_Detail, "RPC: client disconnected");
        gOutputMutex->Lock();
        gClientFd = -1;
        gOutputMutex->Unlock();
        lwip_close(client_fd);
    }
}

} // namespace rpc
} // namespace chip
