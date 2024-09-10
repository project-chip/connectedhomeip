/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "RpcClientProcessor.h"

#include <string>
#include <thread>

#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_stream/socket_stream.h"

namespace chip {
namespace rpc {
namespace client {
namespace {

// Constants
constexpr size_t kMaxTransmissionUnit = 256;
const char * kDefaultRpcServerAddress = "127.0.0.1";

// RPC Stream and Channel Setup
pw::stream::SocketStream rpcSocketStream;
pw::hdlc::RpcChannelOutput hdlcChannelOutput(rpcSocketStream, pw::hdlc::kDefaultRpcAddress, "HDLC channel");
pw::rpc::Channel channels[] = { pw::rpc::Channel::Create<1>(&hdlcChannelOutput) };
pw::rpc::Client rpcClient(channels);

// RPC Stream and Channel Setup
uint16_t rpcServerPort        = 0;
const char * rpcServerAddress = kDefaultRpcServerAddress;

// Function to process incoming packets
void ProcessPackets()
{
    std::array<std::byte, kMaxTransmissionUnit> inputBuf;
    pw::hdlc::Decoder decoder(inputBuf);

    while (true)
    {
        std::array<std::byte, kMaxTransmissionUnit> data;
        auto ret = rpcSocketStream.Read(data);
        if (!ret.ok())
        {
            if (ret.status() == pw::Status::OutOfRange())
            {
                // Handle remote disconnect
                rpcSocketStream.Close();
                return;
            }
            continue;
        }

        for (std::byte byte : ret.value())
        {
            auto result = decoder.Process(byte);
            if (!result.ok())
            {
                // Wait for more bytes that form a complete packet
                continue;
            }
            pw::hdlc::Frame & frame = result.value();
            if (frame.address() != pw::hdlc::kDefaultRpcAddress)
            {
                // Wrong address; ignore the packet
                continue;
            }

            rpcClient.ProcessPacket(frame.data()).IgnoreError();
        }
    }
}

} // namespace

void SetRpcServerAddress(const char * address)
{
    rpcServerAddress = address;
}

void SetRpcServerPort(uint16_t port)
{
    rpcServerPort = port;
}

pw::rpc::Client & GetDefaultRpcClient()
{
    return rpcClient;
}

CHIP_ERROR StartPacketProcessing()
{
    if (rpcSocketStream.Connect(rpcServerAddress, rpcServerPort) != PW_STATUS_OK)
    {
        // Handle connection error
        return CHIP_ERROR_NOT_CONNECTED;
    }

    // Start a thread to process incoming packets
    std::thread packet_processor(ProcessPackets);
    packet_processor.detach();

    return CHIP_NO_ERROR;
}

} // namespace client
} // namespace rpc
} // namespace chip
