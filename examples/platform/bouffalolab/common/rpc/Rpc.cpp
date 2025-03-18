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
#include "pw_sys_io/init.h"
#include "task.h"
extern "C" {
#include "bl_sys.h"
}
#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
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

#if defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE
#include "pigweed/rpc_services/OtCli.h"
#endif // defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE

#if defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE
#include "pigweed/rpc_services/Thread.h"
#endif // defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE

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
class BouffaloButton final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        GetAppTask().ButtonEventHandler(request.idx, request.pushed);
        return pw::OkStatus();
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class BouffaloDevice final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response) override
    {
        if (mRebootTimer)
        {
            return pw::Status::Unavailable();
        }

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
        mRebootTimer = xTimerCreateStatic("Reboot", pdMS_TO_TICKS(delayMs), false, nullptr, RebootHandler, &mRebootTimerBuffer);
        xTimerStart(mRebootTimer, 0);
        return pw::OkStatus();
    }

    pw::Status FactoryReset(const pw_protobuf_Empty & request, pw_protobuf_Empty & response) override
    {
        if (mRebootTimer)
        {
            return pw::Status::Unavailable();
        }

        // Notice: reboot delay not configurable here
        mRebootTimer = xTimerCreateStatic("FactoryReset", pdMS_TO_TICKS(kRebootTimerPeriodMs), false, nullptr, FactoryResetHandler,
                                          &mRebootTimerBuffer);
        xTimerStart(mRebootTimer, 0);
        return pw::OkStatus();
    }

private:
    static constexpr uint32_t kRebootTimerPeriodMs = 1000;
    TimerHandle_t mRebootTimer                     = 0;
    StaticTimer_t mRebootTimerBuffer;

    static void RebootHandler(TimerHandle_t) { bl_sys_reset_por(); }

    static void FactoryResetHandler(TimerHandle_t) { DeviceLayer::ConfigurationMgr().InitiateFactoryReset(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

#define RPC_TASK_STACK_SIZE 1536
#define RPC_TASK_PRIORITY 1
static TaskHandle_t sRpcTaskHandle;
StaticTask_t sRpcTaskBuffer;
StackType_t sRpcTaskStack[RPC_TASK_STACK_SIZE];

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
BouffaloButton button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
Descriptor descriptor_service;
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
BouffaloDevice device_service;
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
Lighting lighting_service;
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE
Locking locking;
#endif // defined(PW_RPC_LOCKING_SERVICE) && PW_RPC_LOCKING_SERVICE

#if defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE
OtCli ot_cli_service;
#endif // defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE

#if defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE
Thread thread;
#endif // defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
pw::trace::TraceService trace_service(pw::trace::GetTokenizedTracer());
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

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

#if defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE
    server.RegisterService(ot_cli_service);
#endif // defined(PW_RPC_OTCLI_SERVICE) && PW_RPC_OTCLI_SERVICE

#if defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE
    server.RegisterService(thread);
#endif // defined(PW_RPC_THREAD_SERVICE) && PW_RPC_THREAD_SERVICE

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
    // Start App task.
    sRpcTaskHandle = xTaskCreateStatic(RunRpcService, "RPC_TASK", MATTER_ARRAY_SIZE(sRpcTaskStack), nullptr, RPC_TASK_PRIORITY,
                                       sRpcTaskStack, &sRpcTaskBuffer);
}

} // namespace rpc
} // namespace chip
