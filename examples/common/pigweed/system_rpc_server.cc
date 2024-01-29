// Copyright 2020 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

// THIS FILE IS DEPRECATED, keep only until examples/pigweed-app depends on
// third_party/pigweed/repo/pw_hdlc/rpc_example/hdlc_rpc_server.cc

#include <cstddef>

#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_log/log.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_stream/sys_io_stream.h"

namespace pw::rpc::system_server {
namespace {

constexpr size_t kMaxTransmissionUnit = 256;

// Used to write HDLC data to pw::sys_io.
stream::SysIoWriter writer;
stream::SysIoReader reader;

// Set up the output channel for the pw_rpc server to use.
hdlc::RpcChannelOutput hdlc_channel_output(writer, pw::hdlc::kDefaultRpcAddress, "HDLC channel");
Channel channels[] = { pw::rpc::Channel::Create<1>(&hdlc_channel_output) };
rpc::Server server(channels);

} // namespace

void Init()
{
    // Send log messages to HDLC address 1. This prevents logs from interfering
    // with pw_rpc communications.
    pw::log_basic::SetOutput([](std::string_view log) { pw::hdlc::WriteUIFrame(1, pw::as_bytes(pw::span(log)), writer); });
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
        std::byte byte;
        Status ret_val = pw::sys_io::ReadByte(&byte);
        if (!ret_val.ok())
        {
            return ret_val;
        }
        if (auto result = decoder.Process(byte); result.ok())
        {
            hdlc::Frame & frame = result.value();
            if (frame.address() == hdlc::kDefaultRpcAddress)
            {
                server.ProcessPacket(frame.data());
            }
        }
    }
}

} // namespace pw::rpc::system_server
