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
#include "ota.h"
#include "pigweed/RpcService.h"
#include "pw_sys_io_qpg/init.h"
#include "task.h"

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
#include "pigweed/rpc_services/Button.h"
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
#include "pigweed/rpc_services/Device.h"
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
#include "pigweed/rpc_services/Lighting.h"
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
#include "pigweed/rpc_services/Locking.h"
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

namespace chip {
namespace rpc {

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
class QpgButton final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
        GetAppTask().ButtonEventHandler((uint8_t) request.idx, (bool) request.pushed);
        return pw::OkStatus();
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class QpgDevice final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response)
    {
        Clock::Timeout delay;

        if (request.delay_ms != 0)
        {
            delay = System::Clock::Milliseconds64(request.delay_ms);
        }
        else
        {
            delay = System::Clock::Seconds32(1);
            ChipLogProgress(NotSpecified, "Did not receive a reboot delay. Defaulting to 1s");
        }

        DeviceLayer::SystemLayer().StartTimer(delay, RebootImpl, nullptr);
        return pw::OkStatus();
    }
    pw::Status TriggerOta(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        TriggerOTAQuery();
        return pw::OkStatus();
    }

private:
    static void RebootImpl(System::Layer *, void *) { qvCHIP_ResetSystem(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

#define RPC_TASK_STACK_SIZE 4096
#define RPC_TASK_PRIORITY 1
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
QpgButton button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
QpgDevice device_service;
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
Lighting lighting_service;
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
Locking locking;
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
    server.RegisterService(attributes_service);
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
    server.RegisterService(button_service);
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
    server.RegisterService(device_service);
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
    server.RegisterService(lighting_service);
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
    server.RegisterService(locking);
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
}

} // namespace

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

bool Init(void)
{
    pw_sys_io_Init();

    PigweedLogger::init();

    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", MATTER_ARRAY_SIZE(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
    return (sRpcTaskHandle == nullptr);
}

} // namespace rpc
} // namespace chip
