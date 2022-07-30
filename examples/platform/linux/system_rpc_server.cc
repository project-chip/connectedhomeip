/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *
 *    This file is a fork from: third_party/pigweed/repo/targets/host/system_rpc_server.cc
 *    But changes the disconnect behaviour from closing and returning, to awaiting a
 *    reconnection.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "pw_assert/check.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_log/log.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_stream/socket_stream.h"

namespace pw::rpc::system_server {
namespace {

constexpr size_t kMaxTransmissionUnit = 512;
uint16_t socket_port                  = 33000;

stream::SocketStream socket_stream;

hdlc::RpcChannelOutput hdlc_channel_output(socket_stream, hdlc::kDefaultRpcAddress, "HDLC channel");
Channel channels[] = { rpc::Channel::Create<1>(&hdlc_channel_output) };
rpc::Server server(channels);

} // namespace

void set_socket_port(uint16_t new_socket_port)
{
    socket_port = new_socket_port;
}

int GetServerSocketFd()
{
    return socket_stream.connection_fd();
}

void Init()
{
    log_basic::SetOutput([](std::string_view log) {
        std::fprintf(stderr, "%.*s\n", static_cast<int>(log.size()), log.data());
        hdlc::WriteUIFrame(1, as_bytes(span(log)), socket_stream).IgnoreError(); // TODO(pwbug/387): Handle Status properly
    });

    PW_LOG_INFO("Starting pw_rpc server on port %d", socket_port);
    PW_CHECK_OK(socket_stream.Serve(socket_port));
}

rpc::Server & Server()
{
    return server;
}

Status Start()
{
    // Declare a buffer for decoding incoming HDLC frames.
    std::array<std::byte, kMaxTransmissionUnit> input_buffer;
    hdlc::Decoder decoder(input_buffer);

    while (true)
    {
        std::array<std::byte, kMaxTransmissionUnit> data;
        auto ret_val = socket_stream.Read(data);
        if (!ret_val.ok())
        {
            if (ret_val.status() == Status::OutOfRange())
            {
                // An out of range status indicates the remote end has disconnected.
                // Start to serve the connection again, which will allow another
                // remote to connect.
                PW_CHECK_OK(socket_stream.Serve(socket_port));
            }
            continue;
        }

        for (std::byte byte : ret_val.value())
        {
            auto result = decoder.Process(byte);
            if (!result.ok())
            {
                // Non-OK means there isn't a complete packet yet, or there was some
                // other issue. Wait for more bytes that form a complete packet.
                continue;
            }
            hdlc::Frame & frame = result.value();
            if (frame.address() != hdlc::kDefaultRpcAddress)
            {
                // Wrong address; ignore the packet for now. In the future, this branch
                // could expand to add packet routing or metrics.
                continue;
            }

            server.ProcessPacket(frame.data(), hdlc_channel_output).IgnoreError();
        }
    }
}

} // namespace pw::rpc::system_server
