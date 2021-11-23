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

#include "app/server/Server.h"
#include "credentials/FabricTable.h"
#include "device_service/device_service.rpc.pb.h"
#include "platform/ConfigurationManager.h"
#include "platform/PlatformManager.h"
#include <platform/DiagnosticDataProvider.h>

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

    virtual pw::Status GetDeviceState(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceState & response)
    {
        uint64_t time_since_boot_sec;
        DeviceLayer::GetDiagnosticDataProvider().GetUpTime(time_since_boot_sec);
        response.time_since_boot_millis = time_since_boot_sec * 1000;
        size_t count                    = 0;
        for (const FabricInfo & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            if (count < ArraySize(response.fabric_info) && fabricInfo.IsInitialized())
            {
                response.fabric_info[count].fabric_id = fabricInfo.GetFabricId();
                response.fabric_info[count].node_id   = fabricInfo.GetPeerId().GetNodeId();
                count++;
            }
        }
        response.fabric_info_count = count;
        return pw::OkStatus();
    }

    virtual pw::Status GetDeviceInfo(ServerContext &, const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {
        uint16_t vendor_id;
        if (DeviceLayer::ConfigurationMgr().GetVendorId(vendor_id) == CHIP_NO_ERROR)
        {
            response.vendor_id = static_cast<uint32_t>(vendor_id);
        }

        uint16_t product_id;
        if (DeviceLayer::ConfigurationMgr().GetProductId(product_id) == CHIP_NO_ERROR)
        {
            response.product_id = static_cast<uint32_t>(product_id);
        }

        uint16_t software_version;
        if (DeviceLayer::ConfigurationMgr().GetFirmwareRevision(software_version) == CHIP_NO_ERROR)
        {
            response.software_version = software_version;
        }

        uint32_t code;
        if (DeviceLayer::ConfigurationMgr().GetSetupPinCode(code) == CHIP_NO_ERROR)
        {
            response.pairing_info.code = code;
            response.has_pairing_info  = true;
        }

        uint16_t discriminator;
        if (DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(discriminator) == CHIP_NO_ERROR)
        {
            response.pairing_info.discriminator = static_cast<uint32_t>(discriminator);
            response.has_pairing_info           = true;
        }

        DeviceLayer::ConfigurationMgr().GetSerialNumber(response.serial_number, sizeof(response.serial_number));

        return pw::OkStatus();
    }

    virtual pw::Status SetPairingInfo(ServerContext &, const chip_rpc_PairingInfo & request, pw_protobuf_Empty & response)
    {
        if (DeviceLayer::ConfigurationMgr().StoreSetupPinCode(request.code) != CHIP_NO_ERROR ||
            DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(request.discriminator) != CHIP_NO_ERROR)
        {
            return pw::Status::Unknown();
        }
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
