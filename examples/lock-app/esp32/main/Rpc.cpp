/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "AppTask.h"
#include "PigweedLoggerMutex.h"
#include "RpcService.h"
#include "button_service/button_service.rpc.pb.h"
#include "device_service/device_service.rpc.pb.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "locking_service/locking_service.rpc.pb.h"
#include "pw_log/log.h"
#include "pw_rpc/server.h"
#include "pw_sys_io/sys_io.h"
#include <support/logging/CHIPLogging.h>

const char * TAG = "RPC";

using chip::DeviceLayer::ConfigurationMgr;

static bool uartInitialised;

namespace chip {
namespace rpc {

class Locking final : public generated::Locking<Locking>
{

public:
    pw::Status Set(ServerContext &, const chip_rpc_LockingState & request, pw_protobuf_Empty & response)
    {
        BoltLockMgr().InitiateAction(AppEvent::kEventType_Lock,
                                     request.locked ? BoltLockManager::LOCK_ACTION : BoltLockManager::UNLOCK_ACTION);
        return pw::OkStatus();
    }

    pw::Status Get(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_LockingState & response)
    {
        response.locked = !BoltLockMgr().IsUnlocked();
        return pw::OkStatus();
    }
};

class Button final : public generated::Button<Button>
{
public:
    pw::Status Event(ServerContext &, const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
        GetAppTask().ButtonEventHandler(request.idx, request.pushed);
        return pw::OkStatus();
    }
};

class Device final : public generated::Device<Device>
{
public:
    pw::Status FactoryReset(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        ConfigurationMgr().InitiateFactoryReset();
        return pw::OkStatus();
    }
    pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
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

constexpr size_t kRpcStackSizeBytes = (4 * 1024);
constexpr uint8_t kRpcTaskPriority  = 5;

TaskHandle_t rpcTaskHandle;

Button button_service;
Locking locking_service;
Device device_service;

void RegisterServices(pw::rpc::Server & server)
{
    server.RegisterService(locking_service);
    server.RegisterService(button_service);
    server.RegisterService(device_service);
}

void RunRpcService(void *)
{
    Start(RegisterServices, &logger_mutex);
}

void Init()
{
    PigweedLogger::init();
    uartInitialised = true;

    ESP_LOGI(TAG, "----------- esp32-pigweed-service starting -----------");

    xTaskCreate(RunRpcService, "RPC", kRpcStackSizeBytes / sizeof(StackType_t), nullptr, kRpcTaskPriority, &rpcTaskHandle);
}

} // namespace rpc
} // namespace chip

#endif
