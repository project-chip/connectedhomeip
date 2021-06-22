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
#include "PigweedLoggerMutex.h"
#include "pigweed/RpcService.h"

#include "button_service/button_service.rpc.pb.h"
#include "device_service/device_service.rpc.pb.h"
#include "lighting_service/lighting_service.rpc.pb.h"
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

namespace {

void reboot_timer_handler(struct k_timer * dummy)
{
    NVIC_SystemReset();
}
K_TIMER_DEFINE(reboot_timer, reboot_timer_handler, NULL);

} // namespace

class Lighting final : public generated::Lighting<Lighting>
{
public:
    pw::Status Set(ServerContext &, const chip_rpc_LightingState & request, pw_protobuf_Empty & response)
    {
        LightingMgr().InitiateAction(request.on ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION,
                                     AppEvent::kEventType_Button, 0, NULL);
        return pw::OkStatus();
    }

    pw::Status Get(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_LightingState & response)
    {
        response.on = LightingMgr().IsTurnedOn();
        return pw::OkStatus();
    }
};

class Button final : public generated::Button<Button>
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.pushed << request.idx /* button_state */, 1 << request.idx /* has_changed */);
        return pw::OkStatus();
    }
};

class Device final : public generated::Device<Device>
{
public:
    pw::Status FactoryReset(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        // TODO: Clear data from KVS
        DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        // Reboot device after clearing data
        k_timer_start(&reboot_timer, K_SECONDS(1), K_FOREVER);
        return pw::OkStatus();
    }
    pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        k_timer_start(&reboot_timer, K_SECONDS(1), K_FOREVER);
        return pw::OkStatus();
    }
    pw::Status TriggerOta(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        // TODO: auto err = DeviceLayer::SoftwareUpdateMgr().CheckNow();
        return pw::Status::Unimplemented();
    }
    pw::Status GetDeviceInfo(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {
        response.vendor_id        = 1234;
        response.product_id       = 5678;
        response.software_version = 0;
        return pw::OkStatus();
    }
};

namespace {

constexpr size_t kRpcTaskSize = 4096;
constexpr int kRpcPriority    = 5;

K_THREAD_STACK_DEFINE(rpc_stack_area, kRpcTaskSize);
struct k_thread rpc_thread_data;

chip::rpc::Button button_service;
chip::rpc::Lighting lighting_service;
chip::rpc::Device device_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(lighting_service);
    server.RegisterService(button_service);
    server.RegisterService(device_service);
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
    Start(RegisterServices, &logger_mutex);
}

} // namespace rpc
} // namespace chip
