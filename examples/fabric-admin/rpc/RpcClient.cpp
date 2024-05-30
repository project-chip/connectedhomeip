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

#include "RpcClient.h"

#include <string>
#include <thread>
#include <unistd.h>

#include "fabric_bridge_service/fabric_bridge_service.rpc.pb.h"
#include "pw_assert/check.h"
#include "pw_hdlc/decoder.h"
#include "pw_hdlc/default_addresses.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_rpc/client.h"
#include "pw_stream/socket_stream.h"

namespace {

constexpr size_t kMaxTransmissionUnit = 256;
constexpr uint32_t kRpcTimeoutMs      = 1000;
const char * rpcServerAddress         = "127.0.0.1";

pw::stream::SocketStream rpcSocketStream;

// Set up the output channel for the pw_rpc client to use.
pw::hdlc::RpcChannelOutput hdlc_channel_output(rpcSocketStream, pw::hdlc::kDefaultRpcAddress, "HDLC channel");

// An array of RPC channels (channels) is created, each associated with an HDLC channel output.
// This sets up the communication channels for RPC calls.
pw::rpc::Channel channels[] = { pw::rpc::Channel::Create<1>(&hdlc_channel_output) };

// Initialize the RPC client with the channels.
pw::rpc::Client client(channels);

// Generated clients are namespaced with their proto library.
using FabricBridgeClient = chip::rpc::pw_rpc::nanopb::FabricBridge::Client;

// RPC channel ID on which to make client calls. RPC calls cannot be made on
// channel 0 (Channel::kUnassignedChannelId).
constexpr uint32_t kDefaultChannelId = 1;

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

            client.ProcessPacket(frame.data()).IgnoreError();
        }
    }
}

template <typename CallType>
CHIP_ERROR WaitForResponse(CallType & call)
{
    if (!call.active())
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Wait for the response or timeout
    uint32_t elapsedTimeMs     = 0;
    const uint32_t sleepTimeMs = 100;

    while (call.active() && elapsedTimeMs < kRpcTimeoutMs)
    {
        usleep(sleepTimeMs * 1000);
        elapsedTimeMs += sleepTimeMs;
    }

    if (elapsedTimeMs >= kRpcTimeoutMs)
    {
        fprintf(stderr, "RPC Response timed out!");
        return CHIP_ERROR_TIMEOUT;
    }

    return CHIP_NO_ERROR;
}

void AddDeviceResponse(const pw_protobuf_Empty & response, pw::Status status)
{
    if (status.ok())
    {
        printf("RPC call succeeded\n");
    }
    else
    {
        printf("RPC call failed with status: %d\n", status.code());
    }
}

} // namespace

CHIP_ERROR InitRpcClient(uint16_t rpcServerPort)
{
    if (rpcSocketStream.Connect(rpcServerAddress, rpcServerPort) != PW_STATUS_OK)
    {
        return CHIP_ERROR_NOT_CONNECTED;
    }

    // Start a thread to process incoming packets
    std::thread packet_processor(ProcessPackets);
    packet_processor.detach();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AddSynchronizedDevice(chip::NodeId nodeId)
{
    ChipLogProgress(NotSpecified, "AddSynchronizedDevice");

    FabricBridgeClient fabric_bridge_client(client, kDefaultChannelId);
    chip_rpc_SynchronizedDevice device;
    device.node_id = nodeId;

    // The RPC will remain active as long as `call` is alive.
    auto call = fabric_bridge_client.AddSynchronizedDevice(device, AddDeviceResponse);
    return WaitForResponse(call);
}
