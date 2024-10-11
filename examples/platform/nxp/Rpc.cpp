/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AppRpc.h"
#include "FreeRTOS.h"
#include "PigweedLogger.h"
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
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

#ifndef RPC_TASK_STACK_SIZE
#define RPC_TASK_STACK_SIZE 2048
#endif

#ifndef RPC_TASK_PRIORITY
#define RPC_TASK_PRIORITY 1
#endif

TaskHandle_t RpcTaskHandle;

namespace chip {
namespace rpc {

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
class NxpButton final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        chip::NXP::App::Rpc::ButtonHandler(request);
        return pw::OkStatus();
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class NxpDevice final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response) override
    {
        mRebootTimer = xTimerCreate("Reboot", kRebootTimerPeriodTicks, false, nullptr, RebootHandler);
        xTimerStart(mRebootTimer, pdMS_TO_TICKS(0));
        return pw::OkStatus();
    }

private:
    static constexpr TickType_t kRebootTimerPeriodTicks = 300;
    TimerHandle_t mRebootTimer;

    static void RebootHandler(TimerHandle_t) { chip::NXP::App::Rpc::Reboot(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
NxpButton button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
NxpDevice device_service;
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

void Init()
{
    PigweedLogger::Init();

    xTaskCreate(RunRpcService, "RPC_TASK", RPC_TASK_STACK_SIZE, nullptr, RPC_TASK_PRIORITY, &RpcTaskHandle);
}

} // namespace rpc
} // namespace chip
