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

#include "Rpc.h"

#include "LightingManager.h"
#include <platform/CHIPDeviceError.h>

/* ignore GCC Wconversion warnings for pigweed */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "button_service/button_service.rpc.pb.h"
#include "device_service/device_service.rpc.pb.h"
#include "lighting_service/lighting_service.rpc.pb.h"
#include "pw_hdlc/rpc_channel.h"
#include "pw_hdlc/rpc_packets.h"
#include "pw_rpc/server.h"
#include "pw_rpc_system_server/rpc_server.h"
#include "pw_stream/sys_io_stream.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <thread>

namespace chip {
namespace rpc {

class Lighting final : public generated::Lighting<Lighting>
{
public:
    pw::Status Set(ServerContext &, const chip_rpc_LightingState & request, pw_protobuf_Empty & response)
    {
        LightingMgr().InitiateAction(request.on ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
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
        return pw::Status::Unavailable();
    }
};

class Device final : public generated::Device<Device>
{
public:
    pw::Status FactoryReset(ServerContext &, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    pw::Status Reboot(ServerContext &, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    pw::Status TriggerOta(ServerContext &, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    pw::Status GetDeviceInfo(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {
        // Temporary fake values used for testing.
        response.vendor_id        = 1234;
        response.product_id       = 5678;
        response.software_version = 1;
        return pw::OkStatus();
    }
};

namespace {

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

void RunRpcService()
{
    pw::rpc::system_server::Init();
    RegisterServices(pw::rpc::system_server::Server());
    pw::rpc::system_server::Start();
}

int Init()
{
    int err = 0;
    std::thread rpc_service(RunRpcService);
    rpc_service.detach();
    return err;
}

} // namespace rpc
} // namespace chip
