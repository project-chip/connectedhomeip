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
#include <platform/CommissionableDataProvider.h>

#include "app/server/OnboardingCodesUtil.h"
#include "app/server/Server.h"
#include "credentials/FabricTable.h"
#include "device_service/device_service.rpc.pb.h"
#include "platform/ConfigurationManager.h"
#include "platform/DiagnosticDataProvider.h"
#include "platform/PlatformManager.h"

namespace chip {
namespace rpc {

class Device : public pw_rpc::nanopb::Device::Service<Device>
{
public:
    virtual ~Device() = default;

    virtual pw::Status FactoryReset(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
        return pw::OkStatus();
    }

    virtual pw::Status Reboot(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }

    virtual pw::Status TriggerOta(const pw_protobuf_Empty & request, pw_protobuf_Empty & response)
    {
        // TODO: auto err = DeviceLayer::SoftwareUpdateMgr().CheckNow();
        return pw::Status::Unimplemented();
    }

    virtual pw::Status SetPairingState(const chip_rpc_PairingState & request, pw_protobuf_Empty & response)
    {
        if (request.pairing_enabled)
        {
            DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);
            DeviceLayer::ConnectivityMgr().SetBLEAdvertisingMode(DeviceLayer::ConnectivityMgr().kFastAdvertising);
        }
        else
        {
            DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        }
        return pw::OkStatus();
    }

    virtual pw::Status GetPairingState(const pw_protobuf_Empty & request, chip_rpc_PairingState & response)
    {
        response.pairing_enabled = DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled();
        return pw::OkStatus();
    }

    virtual pw::Status GetDeviceState(const pw_protobuf_Empty & request, chip_rpc_DeviceState & response)
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

    virtual pw::Status GetDeviceInfo(const pw_protobuf_Empty & request, chip_rpc_DeviceInfo & response)
    {

        uint16_t vendor_id;
        if (DeviceLayer::ConfigurationMgr().GetVendorId(vendor_id) == CHIP_NO_ERROR)
        {
            response.vendor_id = static_cast<uint32_t>(vendor_id);
        }
        else
        {
            response.vendor_id = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        }

        uint16_t product_id;
        if (DeviceLayer::ConfigurationMgr().GetProductId(product_id) == CHIP_NO_ERROR)
        {
            response.product_id = static_cast<uint32_t>(product_id);
        }
        else
        {
            response.product_id = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        }

        uint32_t software_version;
        if (DeviceLayer::ConfigurationMgr().GetSoftwareVersion(software_version) == CHIP_NO_ERROR)
        {
            response.software_version = software_version;
        }
        else
        {
            response.software_version = CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION;
        }

        uint32_t code;
        if (DeviceLayer::GetCommissionableDataProvider()->GetSetupPasscode(code) == CHIP_NO_ERROR)
        {
            response.pairing_info.code = code;
            response.has_pairing_info  = true;
        }

        uint16_t discriminator;
        if (DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator) == CHIP_NO_ERROR)
        {
            response.pairing_info.discriminator = static_cast<uint32_t>(discriminator);
            response.has_pairing_info           = true;
        }

        if (DeviceLayer::ConfigurationMgr().GetSerialNumber(response.serial_number, sizeof(response.serial_number)) ==
            CHIP_NO_ERROR)
        {
            snprintf(response.serial_number, sizeof(response.serial_number), CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER);
        }

        std::string qrCodeText;
        if (GetQRCode(qrCodeText, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)) == CHIP_NO_ERROR)
        {
            snprintf(response.pairing_info.qr_code, sizeof(response.pairing_info.qr_code), "%s", qrCodeText.c_str());
            GetQRCodeUrl(response.pairing_info.qr_code_url, sizeof(response.pairing_info.qr_code_url), qrCodeText);
            response.has_pairing_info = true;
        }

        return pw::OkStatus();
    }

    virtual pw::Status SetPairingInfo(const chip_rpc_PairingInfo & request, pw_protobuf_Empty & response)
    {
        if (DeviceLayer::GetCommissionableDataProvider()->SetSetupPasscode(request.code) != CHIP_NO_ERROR ||
            DeviceLayer::GetCommissionableDataProvider()->SetSetupDiscriminator(request.discriminator) != CHIP_NO_ERROR)
        {
            return pw::Status::Unknown();
        }
        return pw::OkStatus();
    }
};

} // namespace rpc
} // namespace chip
