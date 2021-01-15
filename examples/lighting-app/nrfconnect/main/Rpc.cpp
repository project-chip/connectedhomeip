/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Rpc.h"
#include "AppTask.h"

#include "main/pigweed_lighting.rpc.pb.h"
#include "pw_hdlc_lite/rpc_channel.h"
#include "pw_hdlc_lite/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_stream/sys_io_stream.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_nrfconnect/init.h"

#include <logging/log.h>

#include <array>

LOG_MODULE_DECLARE(app);

namespace chip {
namespace rpc {

class LightingService final : public generated::LightingService<LightingService>
{
public:
    pw::Status ButtonEvent(ServerContext & ctx, const chip_rpc_Button & request, chip_rpc_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.action << request.idx /* button_state */, 1 << request.idx /* has_changed */);
        return pw::Status::OK;
    }
};

namespace {

using std::byte;

constexpr size_t kRpcTaskSize         = 4096;
constexpr int kRpcPriority            = 5;
constexpr size_t kMaxTransmissionUnit = 1500;

K_THREAD_STACK_DEFINE(rpc_stack_area, kRpcTaskSize);
struct k_thread rpc_thread_data;

// Used to write HDLC data to pw::sys_io.
pw::stream::SysIoWriter writer;

// Set up the output channel for the pw_rpc server to use.
pw::hdlc_lite::RpcChannelOutputBuffer<kMaxTransmissionUnit> hdlc_channel_output(writer, pw::hdlc_lite::kDefaultRpcAddress,
                                                                                "HDLC channel");

pw::rpc::Channel channels[] = { pw::rpc::Channel::Create<1>(&hdlc_channel_output) };

// pw_rpc server with the HDLC channel.
pw::rpc::Server server(channels);

chip::rpc::LightingService lighting_service;

void RegisterServices()
{
    server.RegisterService(lighting_service);
}

void Start()
{
    // Set up the server and start processing data.
    RegisterServices();

    // Buffer for decoding incoming HDLC frames.
    std::array<std::byte, kMaxTransmissionUnit> input_buffer;

    LOG_INF("Starting pw_rpc server");
    pw::hdlc_lite::ReadAndProcessPackets(server, hdlc_channel_output, input_buffer);
}

} // namespace

k_tid_t Init()
{
    pw_sys_io_Init();
    k_tid_t tid = k_thread_create(&rpc_thread_data, rpc_stack_area, K_THREAD_STACK_SIZEOF(rpc_stack_area), RunRpcService, NULL,
                                  NULL, NULL, kRpcPriority, 0, K_NO_WAIT);
    return tid;
}

void RunRpcService(void *, void *, void *)
{
    Start();
}

} // namespace rpc
} // namespace chip
