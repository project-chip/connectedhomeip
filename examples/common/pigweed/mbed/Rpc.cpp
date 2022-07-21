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
 */

#include "Rpc.h"
#include "PigweedLogger.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"

#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_stream/sys_io_stream.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_mbed/init.h"

#include <support/logging/CHIPLogging.h>

#ifdef CHIP_PW_RPC_ECHO_PROTO
#include "pw_rpc/echo_service_nanopb.h"
#endif

#ifdef CHIP_PW_RPC_COMMON_PROTO
#include "AppTask.h"
#include "rpc_services/Button.h"
#include "rpc_services/Device.h"
#endif

#ifdef CHIP_PW_RPC_LIGHTING_PROTO
#include "rpc_services/Lighting.h"
#endif

#ifdef CHIP_PW_RPC_LOCKING_PROTO
#include "rpc_services/Locking.h"
#endif

using namespace ::chip::DeviceLayer;
using namespace ::rtos;

namespace chip {
namespace rpc {

#ifdef CHIP_PW_RPC_COMMON_PROTO
class MbedButton final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.idx, request.pushed);
        return pw::OkStatus();
    }
};
#endif

namespace {

#define RPC_THREAD_NAME "RPC"
#define RPC_STACK_SIZE (4 * 1024)

rtos::Thread rpcThread{ osPriorityNormal, RPC_STACK_SIZE, /* memory provided */ nullptr, RPC_THREAD_NAME };

#ifdef CHIP_PW_RPC_ECHO_PROTO
pw::rpc::EchoService echo_service;
#endif

#ifdef CHIP_PW_RPC_COMMON_PROTO
chip::rpc::MbedButton button_service;
chip::rpc::Device device_service;
#endif

#ifdef CHIP_PW_RPC_LIGHTING_PROTO
chip::rpc::Lighting lighting_service;
#endif

#ifdef CHIP_PW_RPC_LOCKING_PROTO
chip::rpc::Locking locking_service;
#endif

void RegisterServices(pw::rpc::Server & server)
{
#ifdef CHIP_PW_RPC_ECHO_PROTO
    server.RegisterService(echo_service);
#endif

#ifdef CHIP_PW_RPC_COMMON_PROTO
    server.RegisterService(button_service);
    server.RegisterService(device_service);
#endif

#ifdef CHIP_PW_RPC_LIGHTING_PROTO
    server.RegisterService(lighting_service);
#endif

#ifdef CHIP_PW_RPC_LOCKING_PROTO
    server.RegisterService(locking_service);
#endif
}

} // namespace

void RunRpcService()
{
    Start(RegisterServices, &logger_mutex);
}

Thread * Init()
{
    pw_sys_io_Init();

    ChipLogProgress(NotSpecified, "RPC service starting...\r\n");

    long error = rpcThread.start(RunRpcService);
    if (error != osOK)
    {
        ChipLogError(NotSpecified, "Run RPC service failed[%ld]", error);
        return NULL;
    }

    return &rpcThread;
}

} // namespace rpc
} // namespace chip
