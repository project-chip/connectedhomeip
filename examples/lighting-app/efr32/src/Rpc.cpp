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

#include "AppTask.h"
#include "FreeRTOS.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
#include "pigweed/rpc_services/Button.h"
#include "pigweed/rpc_services/Device.h"
#include "pigweed/rpc_services/Lighting.h"
#include "pw_sys_io_efr32/init.h"
#include "task.h"

namespace chip {
namespace rpc {

class Efr32Button final : public Button
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        GetAppTask().ButtonEventHandler(SL_SIMPLE_BUTTON_INSTANCE(request.idx) /* PB 0 or PB 1 */, request.pushed);
        return pw::OkStatus();
    }
};

class Efr32Device final : public Device
{
public:
    pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response) override
    {
        NVIC_SystemReset();
        // WILL NOT RETURN
        return pw::OkStatus();
    }
};

namespace {

#define RPC_TASK_STACK_SIZE 4096
#define RPC_TASK_PRIORITY 1
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

chip::rpc::Efr32Button button_service;
chip::rpc::Lighting lighting_service;
chip::rpc::Efr32Device device_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(lighting_service);
    server.RegisterService(button_service);
    server.RegisterService(device_service);
}

} // namespace

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

void Init()
{
    pw_sys_io_Init();

    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", ArraySize(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
}

} // namespace rpc
} // namespace chip
