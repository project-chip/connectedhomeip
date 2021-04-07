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
#include "AppTask.h"
#include "PigweedLogger.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"

#include "button_service/button_service.rpc.pb.h"
#include "lighting_service/lighting_service.rpc.pb.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_stream/sys_io_stream.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_efr32/init.h"

namespace chip {
namespace rpc {

class Lighting : public generated::Lighting<Lighting>
{
public:
    pw::Status Set(ServerContext &, const chip_rpc_LightingState & request, chip_rpc_Empty & response)
    {
        LightMgr().InitiateAction(AppEvent::kEventType_Light,
                                  request.on ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
        return pw::OkStatus();
    }

    pw::Status Get(ServerContext &, const chip_rpc_Empty & request, chip_rpc_LightingState & response)
    {
        response.on = LightMgr().IsLightOn();
        return pw::OkStatus();
    }
};

class Button : public generated::Button<Button>
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, chip_rpc_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.idx /* PB 0 or PB 1 */, request.pushed);
        return pw::OkStatus();
    }
};

namespace {
using std::byte;

#define RPC_TASK_STACK_SIZE 4096
#define RPC_TASK_PRIORITY 1
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

chip::rpc::Button button_service;
chip::rpc::Lighting lighting_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(lighting_service);
    server.RegisterService(button_service);
}

} // namespace

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

int Init()
{
    int err = CHIP_ERROR_MAX;
    pw_sys_io_Init();

    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", RPC_TASK_STACK_SIZE / sizeof(StackType_t), nullptr,
                                       RPC_TASK_PRIORITY, sRpcTaskStack, &sRpcTaskBuffer);
    return err;
}

} // namespace rpc
} // namespace chip
