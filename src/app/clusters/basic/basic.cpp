/**
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
 *
 */

#include "basic.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>

#include <cstring>

using namespace chip;
using namespace chip::app::Clusters::Basic;
using namespace chip::DeviceLayer;

void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint)
{
    EmberAfStatus status;

    char vendorName[33];
    if (ConfigurationMgr().GetVendorName(vendorName, sizeof(vendorName)) == CHIP_NO_ERROR)
    {
        status = Attributes::VendorName::Set(endpoint, chip::CharSpan(vendorName, strlen(vendorName)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Vendor Name: 0x%02x", status));
    }

    uint16_t vendorId;
    if (ConfigurationMgr().GetVendorId(vendorId) == CHIP_NO_ERROR)
    {
        status = Attributes::VendorID::Set(endpoint, vendorId);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Vendor Id: 0x%02x", status));
    }

    char productName[33];
    if (ConfigurationMgr().GetProductName(productName, sizeof(productName)) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductName::Set(endpoint, chip::CharSpan(productName, strlen(productName)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product Name: 0x%02x", status));
    }

    uint16_t productId;
    if (ConfigurationMgr().GetProductId(productId) == CHIP_NO_ERROR)
    {
        status = Attributes::ProductID::Set(endpoint, productId);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Product Id: 0x%02x", status));
    }

    char productRevisionString[65];
    if (ConfigurationMgr().GetProductRevisionString(productRevisionString, sizeof(productRevisionString)) == CHIP_NO_ERROR)
    {
        status =
            Attributes::HardwareVersionString::Set(endpoint, chip::CharSpan(productRevisionString, strlen(productRevisionString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Error setting Hardware Version String: 0x%02x", status));
    }

    uint16_t productRevision;
    if (ConfigurationMgr().GetProductRevision(productRevision) == CHIP_NO_ERROR)
    {
        status = Attributes::HardwareVersion::Set(endpoint, productRevision);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Hardware Version: 0x%02x", status));
    }

    char firmwareRevisionString[65];
    if (ConfigurationMgr().GetFirmwareRevisionString(firmwareRevisionString, sizeof(firmwareRevisionString)) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersionString::Set(endpoint,
                                                        chip::CharSpan(firmwareRevisionString, strlen(firmwareRevisionString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Error setting Software Version String: 0x%02x", status));
    }

    uint16_t firmwareRevision;
    if (ConfigurationMgr().GetFirmwareRevision(firmwareRevision) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersion::Set(endpoint, firmwareRevision);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Software Version: 0x%02x", status));
    }
}

void MatterBasicPluginServerInitCallback() {}
