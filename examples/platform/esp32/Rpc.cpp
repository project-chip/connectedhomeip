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

#include "sdkconfig.h"

#if CONFIG_ENABLE_PW_RPC
#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "pw_log/log.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include "support/CodeUtils.h"

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
#include "ScreenManager.h"
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

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
#define PW_TRACE_BUFFER_SIZE_BYTES 1024
#include "pw_trace/trace.h"
#include "pw_trace_tokenized/trace_rpc_service_nanopb.h"

// Define trace time for pw_trace
PW_TRACE_TIME_TYPE pw_trace_GetTraceTime()
{
    return (PW_TRACE_TIME_TYPE) chip::System::SystemClock().GetMonotonicMicroseconds64().count();
}
// Microsecond time source
size_t pw_trace_GetTraceTimeTicksPerSecond()
{
    return 1000000;
}

#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

namespace chip {
namespace rpc {

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
class Esp32Button final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
#if CONFIG_DEVICE_TYPE_M5STACK
        if (request.pushed)
        {
            ScreenManager::ButtonPressed(1 + request.idx);
        }
        return pw::OkStatus();
#else  // CONFIG_DEVICE_TYPE_M5STACK
        return pw::Status::Unimplemented();
#endif // CONFIG_DEVICE_TYPE_M5STACK
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class Esp32Device final : public Device
{
public:
    pw::Status Reboot(const pw_protobuf_Empty & request, pw_protobuf_Empty & response) override
    {
        mRebootTimer = xTimerCreateStatic("Reboot", kRebootTimerPeriodTicks, false, nullptr, RebootHandler, &mRebootTimerBuffer);
        xTimerStart(mRebootTimer, 0);
        return pw::OkStatus();
    }

private:
    static constexpr TickType_t kRebootTimerPeriodTicks = 1000;
    TimerHandle_t mRebootTimer;
    StaticTimer_t mRebootTimerBuffer;

    static void RebootHandler(TimerHandle_t) { esp_restart(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

#define RPC_TASK_STACK_SIZE (8 * 1024)
#define RPC_TASK_PRIORITY 5
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
Esp32Button button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
Esp32Device device_service;
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
Lighting lighting_service;
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
Locking locking;
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
pw::trace::TraceService trace_service;
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

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

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
    server.RegisterService(trace_service);
    PW_TRACE_SET_ENABLED(true);
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
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
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", ArraySize(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
}

} // namespace rpc
} // namespace chip

#endif // CONFIG_ENABLE_PW_RPC
