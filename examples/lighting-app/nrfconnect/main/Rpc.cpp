/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "pigweed/RpcService.h"

#include "main/pigweed_lighting.rpc.pb.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_stream/sys_io_stream.h"
#include "pw_sys_io/sys_io.h"
#include "pw_sys_io_nrfconnect/init.h"

#include <array>
#include <kernel.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(app);

namespace chip {
namespace rpc {

class LightingService final : public generated::LightingService<LightingService>
{
public:
    pw::Status ButtonEvent(ServerContext & ctx, const chip_rpc_Button & request, chip_rpc_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.action << request.idx /* button_state */, 1 << request.idx /* has_changed */);
        return pw::OkStatus();
    }
};

namespace {

using std::byte;

class PigweedLoggerMutex : public ::chip::rpc::Mutex
{
public:
    PigweedLoggerMutex() {}
    void Lock() override
    {
        k_sem * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            k_sem_take(sem, K_FOREVER);
        }
    }
    void Unlock() override
    {
        k_sem * sem = PigweedLogger::GetSemaphore();
        if (sem)
        {
            k_sem_give(sem);
        }
    }
};

PigweedLoggerMutex uart_mutex;

constexpr size_t kRpcTaskSize = 4096;
constexpr int kRpcPriority    = 5;

K_THREAD_STACK_DEFINE(rpc_stack_area, kRpcTaskSize);
struct k_thread rpc_thread_data;

chip::rpc::LightingService lighting_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(lighting_service);
}

} // namespace

k_tid_t Init()
{
    pw_sys_io_Init();
    k_tid_t tid = k_thread_create(&rpc_thread_data, rpc_stack_area, K_THREAD_STACK_SIZEOF(rpc_stack_area), RunRpcService, NULL,
                                  NULL, NULL, kRpcPriority, 0, K_NO_WAIT);
    return tid;
}

void RunRpcService(void *, void *, void *)
{
    Start(RegisterServices, &uart_mutex);
}

} // namespace rpc
} // namespace chip
