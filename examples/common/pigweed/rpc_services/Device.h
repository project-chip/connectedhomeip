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

#pragma once

#include <platform/CHIPDeviceConfig.h>

#include "device_service/device_service.rpc.pb.h"
#include "platform/ConfigurationManager.h"

namespace chip {
namespace rpc {

class Device : public generated::Device<Device>
{
public:
    virtual ~Device() = default;

    virtual pw::Status FactoryReset(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        return pw::OkStatus();
    }

    virtual pw::Status Reboot(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    virtual pw::Status TriggerOta(ServerContext & ctx, const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        // TODO: auto err = DeviceLayer::SoftwareUpdateMgr().CheckNow();
        return pw::Status::Unimplemented();
    }

    virtual pw::Status GetDeviceInfo(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {
        response.vendor_id        = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        response.product_id       = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        response.software_version = CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION;
        snprintf(response.serial_number, sizeof(response.serial_number), CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
