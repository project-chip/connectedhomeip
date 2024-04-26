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

#include "pw_rpc/server.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_rpc_system_server/socket.h"

#include <thread>
#include <platform/PlatformManager.h>

#include <map>
#include "Rpc.h"

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
#include "pigweed/rpc_services/Attributes.h"
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
#include "pigweed/rpc_services/BooleanState.h"
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
#include "pigweed/rpc_services/Descriptor.h"
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
#include "pigweed/rpc_services/Device.h"
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
#include "pigweed/rpc_services/Actions.h"
namespace chip {
namespace rpc {

ActionsSubscriber * gActionsSubscriber = nullptr;

void RegisterActionsSubscriber(ActionsSubscriber * subscriber)
{
    gActionsSubscriber = subscriber;
}

void RpcActionsDispatch(EndpointId endpointId, ClusterId clusterId, std::queue<Actions::Action> rpcActions) 
{
    ChipLogProgress(NotSpecified, "Receiving the Rpc Actions to be dispatched, endpointId=0x%x, clusterId=0x%x, rpcActions count=%lu", endpointId, clusterId, rpcActions.size());
//    std::queue<ActionTask> * queue = new std::queue<ActionTask>();
printf("\033[41m %s , %d, rpcActions.type=%u, rpcAction.delayMs = %d \033[0m \n", __func__, __LINE__, to_underlying(rpcActions.front().type), rpcActions.front().delayMs);

    for (; !rpcActions.empty();) {
        Actions::Action action = rpcActions.front();

        // Since application cannot diretly include Actions.h, so the event is relayed by Rpc.cpp to the subscriber
        ActionTask task(endpointId, clusterId, static_cast<chip::rpc::ActionType>(to_underlying(action.type)), action.delayMs, action.actionId, action.args);

        if (nullptr != gActionsSubscriber) {
            gActionsSubscriber->publishAction(task);
        }
        // TBD: insert to Device Queue

        rpcActions.pop();
    }

}	

} // rpc
} // chip
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

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
namespace {

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
Attributes attributes_service;
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
BooleanState boolean_state_service;
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
Descriptor descriptor_service;
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
Device device_service;
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
Actions actions_service;
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
Lighting lighting_service;
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
pw::trace::TraceService trace_service(pw::trace::GetTokenizedTracer());
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE

void RegisterServices(pw::rpc::Server & server)
{
#if defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE
    server.RegisterService(actions_service);
    actions_service.RegisterActionsSubscriber(RpcActionsDispatch);
#endif // defined(PW_RPC_ACTIONS_SERVICE) && PW_RPC_ACTIONS_SERVICE

#if defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE
    server.RegisterService(attributes_service);
#endif // defined(PW_RPC_ATTRIBUTE_SERVICE) && PW_RPC_ATTRIBUTE_SERVICE

#if defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE
    server.RegisterService(boolean_state_service);
#endif // defined(PW_RPC_BOOLEAN_STATE_SERVICE) && PW_RPC_BOOLEAN_STATE_SERVICE

#if defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE
    server.RegisterService(descriptor_service);
#endif // defined(PW_RPC_DESCRIPTOR_SERVICE) && PW_RPC_DESCRIPTOR_SERVICE

#if defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE
    server.RegisterService(device_service);
#endif // defined(PW_RPC_DEVICE_SERVICE) && PW_RPC_DEVICE_SERVICE

#if defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE
    server.RegisterService(lighting_service);
#endif // defined(PW_RPC_LIGHTING_SERVICE) && PW_RPC_LIGHTING_SERVICE

#if defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
    server.RegisterService(trace_service);
    PW_TRACE_SET_ENABLED(true);
#endif // defined(PW_RPC_TRACING_SERVICE) && PW_RPC_TRACING_SERVICE
}

} // namespace

void RunRpcService()
{
    pw::rpc::system_server::Init();
    RegisterServices(pw::rpc::system_server::Server());
    pw::rpc::system_server::Start();
}

int Init(uint16_t rpcServerPort)
{
    int err = 0;
    pw::rpc::system_server::set_socket_port(rpcServerPort);
    std::thread rpc_service(RunRpcService);
    rpc_service.detach();
    return err;
}

} // namespace rpc
} // namespace chip
