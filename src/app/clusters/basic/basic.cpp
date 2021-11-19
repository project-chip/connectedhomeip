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
#include <platform/ConfigurationManager.h>

#include <cstring>

using namespace chip;
using namespace chip::app::Clusters::Basic;
using namespace chip::DeviceLayer;

void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint)
{
    EmberAfStatus status;

    char vendorName[DeviceLayer::ConfigurationManager::kMaxVendorNameLength + 1];
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

    char productName[DeviceLayer::ConfigurationManager::kMaxProductNameLength + 1];
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

    char hardwareVersionString[DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength + 1];
    if (ConfigurationMgr().GetHardwareVersionString(hardwareVersionString, sizeof(hardwareVersionString)) == CHIP_NO_ERROR)
    {
        status = Attributes::HardwareVersionString::Set(endpoint, CharSpan(hardwareVersionString, strlen(hardwareVersionString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Error setting Hardware Version String: 0x%02x", status));
    }

    uint16_t hardwareVersion;
    if (ConfigurationMgr().GetHardwareVersion(hardwareVersion) == CHIP_NO_ERROR)
    {
        status = Attributes::HardwareVersion::Set(endpoint, hardwareVersion);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Hardware Version: 0x%02x", status));
    }

    char softwareVersionString[DeviceLayer::ConfigurationManager::kMaxSoftwareVersionLength + 1];
    if (ConfigurationMgr().GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString)) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersionString::Set(endpoint, CharSpan(softwareVersionString, strlen(softwareVersionString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Error setting Software Version String: 0x%02x", status));
    }

    uint16_t softwareVersion;
    if (ConfigurationMgr().GetSoftwareVersion(softwareVersion) == CHIP_NO_ERROR)
    {
        status = Attributes::SoftwareVersion::Set(endpoint, softwareVersion);
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Software Version: 0x%02x", status));
    }

    char serialNumberString[DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1];
    if (ConfigurationMgr().GetSerialNumber(serialNumberString, sizeof(serialNumberString)) == CHIP_NO_ERROR)
    {
        status = Attributes::SerialNumber::Set(endpoint, CharSpan(serialNumberString, strlen(serialNumberString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Serial Number String: 0x%02x", status));
    }

    char manufacturingDateString[DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength + 1];
    uint16_t manufacturingYear;
    uint8_t manufacturingMonth;
    uint8_t manufacturingDayOfMonth;
    if (ConfigurationMgr().GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth) == CHIP_NO_ERROR)
    {
        snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04" PRIu16 "-%02" PRIu16 "-%02" PRIu16,
                 manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);
        status = Attributes::ManufacturingDate::Set(endpoint, CharSpan(manufacturingDateString, strlen(manufacturingDateString)));
        VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status,
                       ChipLogError(Zcl, "Error setting Manufacturing Date String: 0x%02x", status));
    }
}

void MatterBasicPluginServerInitCallback() {}
