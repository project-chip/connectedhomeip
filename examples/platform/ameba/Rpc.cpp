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

#if CONFIG_ENABLE_PW_RPC
#include "FreeRTOS.h"
#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "pw_sys_io_ameba/init.h"
#include "sys_api.h"
#include "task.h"
// #include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/event_groups.h"
// #include "freertos/semphr.h"
// #include "freertos/task.h"
#include "pw_log/log.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include "support/CodeUtils.h"

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
// #include "ScreenManager.h"
#include "pigweed/rpc_services/Button.h"
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
#include "pigweed/rpc_services/Descriptor.h"
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

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
class AmebaButton final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        return pw::Status::Unimplemented();
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class AmebaDevice final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response) override
    {
        TickType_t delayMs = kRebootTimerPeriodMs;
        if (request.delay_ms != 0)
        {
            delayMs = request.delay_ms;
        }
        else
        {
            ChipLogProgress(NotSpecified, "Did not receive a reboot delay. Defaulting to %d ms",
                            static_cast<int>(kRebootTimerPeriodMs));
        }
        mRebootTimer =
            xTimerCreateStatic("Reboot", pdMS_TO_TICKS(kRebootTimerPeriodMs), false, nullptr, RebootHandler, &mRebootTimerBuffer);
        xTimerStart(mRebootTimer, 0);
        return pw::OkStatus();
    }

private:
    static constexpr uint32_t kRebootTimerPeriodMs = 1000;
    TimerHandle_t mRebootTimer;
    StaticTimer_t mRebootTimerBuffer;

    static void RebootHandler(TimerHandle_t) { sys_reset(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

#define RPC_TASK_STACK_SIZE (8 * 1024)
#define RPC_TASK_PRIORITY 1
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
AmebaButton button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
Descriptor descriptor_service;
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
AmebaDevice device_service;
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

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
    server.RegisterService(descriptor_service);
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

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
    PigweedLogger::init();

    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", MATTER_ARRAY_SIZE(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
}

} // namespace rpc
} // namespace chip

#endif // CONFIG_ENABLE_PW_RPCC
