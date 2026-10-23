/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

// FreeRTOS entry point for the Pigweed RPC server carried over lwIP (HDLC over
// TCP). This is a self-contained alternative to Rpc.cpp: it does not use the
// UART based PigweedLogger or pw_sys_io backend. Instead it provides its own
// FreeRTOS mutex to serialize socket writes and spawns the RPC task that runs
// the lwIP transport (RpcServiceLwip.cpp).

#include "AppRpc.h"
#include "FreeRTOS.h"
#include "pigweed/RpcService.h"
#include "semphr.h"
#include "task.h"

#include <lib/core/CHIPError.h>

#if defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE
#include "JFAManager.h"
#include "pigweed/rpc_services/JointFabric.h"
#endif // defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE

#ifndef RPC_TASK_STACK_SIZE
#define RPC_TASK_STACK_SIZE 4096
#endif

#ifndef RPC_TASK_PRIORITY
#define RPC_TASK_PRIORITY 1
#endif

namespace chip {
namespace rpc {

namespace {

TaskHandle_t sRpcTaskHandle;

// Serializes access to the RPC output socket. Implements chip::rpc::Mutex so it
// can be handed to the lwIP transport's Start().
class FreeRTOSRpcMutex : public chip::rpc::Mutex
{
public:
    FreeRTOSRpcMutex() { mMutex = xSemaphoreCreateMutex(); }
    void Lock() override { xSemaphoreTake(mMutex, portMAX_DELAY); }
    void Unlock() override { xSemaphoreGive(mMutex); }

private:
    SemaphoreHandle_t mMutex;
};

FreeRTOSRpcMutex output_mutex;

#if defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE
joint_fabric_service::JointFabric joint_fabric_service_instance;
#endif // defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE
    server.RegisterService(joint_fabric_service_instance);
    chip::JFAMgr().SetJFARpc(joint_fabric_service_instance);
#endif // defined(PW_RPC_JF_ADMIN_SERVICE) && PW_RPC_JF_ADMIN_SERVICE
}

void RunRpcService(void *)
{
    Start(RegisterServices, &output_mutex);
}

} // namespace

CHIP_ERROR Init()
{
    if (xTaskCreate(RunRpcService, "RPC_TASK", RPC_TASK_STACK_SIZE, nullptr, RPC_TASK_PRIORITY, &sRpcTaskHandle) != pdPASS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

} // namespace rpc
} // namespace chip
