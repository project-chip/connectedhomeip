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
#include "PigweedLogger.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"

#include "echo_service/echo_service.rpc.pb.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_stream/sys_io_stream.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_mbed/init.h"

#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip::DeviceLayer;

namespace chip {
namespace rpc {

class Echo final : public generated::Echo<Echo>
{
public:
    pw::Status Send(ServerContext &, const chip_rpc_EchoMessage & request, chip_rpc_EchoMessage & response)
    {
        std::strncpy(response.msg, request.msg, sizeof(response.msg));
        return pw::OkStatus();
    }
};

namespace {

#define RPC_THREAD_NAME "RPC"
#define RPC_STACK_SIZE (4 * 1024)

rtos::Thread rpcThread{ osPriorityNormal, RPC_STACK_SIZE, /* memory provided */ nullptr, RPC_THREAD_NAME };

chip::rpc::Echo echo_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(echo_service);
}

} // namespace

void RunRpcService()
{
    Start(RegisterServices, &logger_mutex);
}

} // namespace rpc
} // namespace chip
