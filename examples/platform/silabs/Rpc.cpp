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
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"
#include "pw_sys_io_efr32/init.h"
#include <cmsis_os2.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>
#include <sl_cmsis_os2_common.h>

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

using namespace chip::DeviceLayer::Silabs;

namespace chip {
namespace rpc {

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
class Efr32Button final : public Button
{
public:
    pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response) override
    {
        AppTask::GetAppTask().ButtonEventHandler(request.idx /* PB 0 or PB 1 */, request.pushed);
        return pw::OkStatus();
    }
};
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
class Efr32Device final : public Device
{
public:
    pw::Status Reboot(const chip_rpc_RebootRequest & request, pw_protobuf_Empty & response) override
    {
        uint32_t delayMs = kRebootTimerPeriodMs;
        if (request.delay_ms != 0)
        {
            delayMs = request.delay_ms;
        }
        else
        {
            ChipLogProgress(NotSpecified, "Did not receive a reboot delay. Defaulting to %d ms",
                            static_cast<int>(kRebootTimerPeriodMs));
        }

        mRebootTimer        = osTimerNew(RebootHandler, osTimerOnce, nullptr, &mRebootTimerAttr);
        uint32_t delayTicks = ((uint64_t) osKernelGetTickFreq() * delayMs) / 1000;
        osTimerStart(mRebootTimer, delayTicks);
        return pw::OkStatus();
    }

private:
    static constexpr uint32_t kRebootTimerPeriodMs = 1000;
    osTimerId_t mRebootTimer;
    osTimer_t mRebootTimerBuffer;
    osTimerAttr_t mRebootTimerAttr = { .name = "Reboot", .cb_mem = &mRebootTimerBuffer, .cb_size = osTimerCbSize };

    static void RebootHandler(void * timerCbArg) { GetPlatform().SoftwareReset(); }
};
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

namespace {

static osThreadId_t sRpcTaskHandle;
osThread_t sRpcTaskControlBlock;
constexpr uint32_t kRpcTaskSize = 4096;
uint8_t sRpcTaskStack[kRpcTaskSize];
constexpr osThreadAttr_t kRpcTaskAttr = { .name       = "RPC",
                                          .attr_bits  = osThreadDetached,
                                          .cb_mem     = &sRpcTaskControlBlock,
                                          .cb_size    = osThreadCbSize,
                                          .stack_mem  = sRpcTaskStack,
                                          .stack_size = kRpcTaskSize,
                                          .priority   = osPriorityLow };

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE
Efr32Button button_service;
#endif // defined(PW_RPC_BUTTON_SERVICE) && PW_RPC_BUTTON_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
Descriptor descriptor_service;
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
Efr32Device device_service;
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
    pw_sys_io_Init();

    // Start App task.
    sRpcTaskHandle = osThreadNew(RunRpcService, nullptr, &kRpcTaskAttr);
}
} // namespace rpc
} // namespace chip
