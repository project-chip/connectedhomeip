/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "bridged-device-basic-information-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/PlatformManager.h>

#include <cstddef>
#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BridgedDeviceBasic;
using namespace chip::app::Clusters::BridgedDeviceBasic::Attributes;
using namespace chip::DeviceLayer;

namespace {

class BridgeDeviceBasicInformationAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Bridged Device Basic Information cluster on all endpoints.
    BridgedDeviceBasicInformationAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), BridgeDeviceBasic::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

BridgedDeviceBasicInformationAttrAccess gAttrAccess;

CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan(buf, strnlen(buf, maxBufSize)));
}

CHIP_ERROR BridgedDeviceBasicInformationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != BridgedDeviceBasic::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR status = CHIP_NO_ERROR;

    switch (aPath.mAttributeId)
    {
    case VendorName::Id: {
        constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxVendorNameLength;
        char vendorName[kMaxLen + 1] = { 0 };
        status                       =
            DeviceLayer::PlatformMgr().GetBridgedVendorName(aPath.mEndpointId, vendorName, sizeof(vendorName));
        status                       = EncodeStringOnSuccess(status, aEncoder, vendorName, kMaxLen);
        break;
    }

    case VendorID::Id: {
        uint16_t vendorId = 0;
        status            = DeviceLayer::PlatformMgr().GetBridgedVendorId(aPath.mEndpointId, vendorId);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(vendorId);
        }
        break;
    }

    case ProductName::Id: {
        constexpr size_t kMaxLen      = DeviceLayer::ConfigurationManager::kMaxProductNameLength;
        char productName[kMaxLen + 1] = { 0 };
        status                        =
            DeviceLayer::PlatformMgr().GetBridgedProductName(aPath.mEndpointId, productName, sizeof(productName));
        status                        = EncodeStringOnSuccess(status, aEncoder, productName, kMaxLen);
        break;
    }

    case HardwareVersion::Id: {
        uint16_t hardwareVersion = 0;
        status                   = DeviceLayer::PlatformMgr().GetBridgedHardwareVersion(aPath.mEndpointId, hardwareVersion);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(hardwareVersion);
        }
        break;
    }

    case HardwareVersionString::Id: {
        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength;
        char hardwareVersionString[kMaxLen + 1] = { 0 };
        status = DeviceLayer::PlatformMgr().GetBridgedHardwareVersionString(
            aPath.mEndpointId, hardwareVersionString, sizeof(hardwareVersionString));
        status = EncodeStringOnSuccess(status, aEncoder, hardwareVersionString, kMaxLen);
        break;
    }

    case SoftwareVersion::Id: {
        uint32_t softwareVersion = 0;
        status                   = DeviceLayer::PlatformMgr().GetBridgedSoftwareVersion(aPath.mEndpointId, softwareVersion);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(softwareVersion);
        }
        break;
    }

    case SoftwareVersionString::Id: {
        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength;
        char softwareVersionString[kMaxLen + 1] = { 0 };
        status = DeviceLayer::PlatformMgr().GetBridgedSoftwareVersionString(
            aPath.mEndpointId, softwareVersionString, sizeof(softwareVersionString));
        status = EncodeStringOnSuccess(status, aEncoder, softwareVersionString, kMaxLen);
        break;
    }

    case ManufacturingDate::Id: {
        constexpr size_t kMaxLen                  = DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength;
        char manufacturingDateString[kMaxLen + 1] = { 0 };
        uint16_t manufacturingYear;
        uint8_t manufacturingMonth;
        uint8_t manufacturingDayOfMonth;
        status = DeviceLayer::PlatformMgr().GetBridgedManufacturingDate(
            aPath.mEndpointId, manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            manufacturingYear       = 2020;
            manufacturingMonth      = 1;
            manufacturingDayOfMonth = 1;
            status                  = CHIP_NO_ERROR;
        }

        if (status == CHIP_NO_ERROR)
        {
            // Format is YYYYMMDD
            snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04" PRIu16 "%02" PRIu8 "%02" PRIu8,
                     manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);
            status = aEncoder.Encode(CharSpan(manufacturingDateString, strnlen(manufacturingDateString, kMaxLen)));
        }
        break;
    }

    case PartNumber::Id: {
        constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxPartNumberLength;
        char partNumber[kMaxLen + 1] = { 0 };
        status                       =
            DeviceLayer::PlatformMgr().GetBridgedPartNumber(aPath.mEndpointId, partNumber, sizeof(partNumber));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            partNumber[0] = '\0';
            status        = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, partNumber, kMaxLen);
        break;
    }

    case ProductURL::Id: {
        constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxProductURLLength;
        char productUrl[kMaxLen + 1] = { 0 };
        status                       =
            DeviceLayer::PlatformMgr().GetBridgedProductURL(aPath.mEndpointId, productUrl, sizeof(productUrl));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            productUrl[0] = '\0';
            status        = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, productUrl, kMaxLen);
        break;
    }

    case ProductLabel::Id: {
        constexpr size_t kMaxLen       = DeviceLayer::ConfigurationManager::kMaxProductLabelLength;
        char productLabel[kMaxLen + 1] = { 0 };
        status                         =
            DeviceLayer::PlatformMgr().GetBridgedProductLabel(aPath.mEndpointId, productLabel, sizeof(productLabel));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            productLabel[0] = '\0';
            status          = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, productLabel, kMaxLen);
        break;
    }

    case SerialNumber::Id: {
        constexpr size_t kMaxLen             = DeviceLayer::ConfigurationManager::kMaxSerialNumberLength;
        char serialNumberString[kMaxLen + 1] = { 0 };
        status                               =
            DeviceLayer::PlatformMgr().GetBridgedSerialNumber(aPath.mEndpointId, serialNumberString, sizeof(serialNumberString));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            serialNumberString[0] = '\0';
            status                = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, serialNumberString, kMaxLen);
        break;
    }

    case UniqueID::Id: {
        constexpr size_t kMaxLen   = DeviceLayer::ConfigurationManager::kMaxUniqueIDLength;
        char uniqueId[kMaxLen + 1] = { 0 };
        status                     = DeviceLayer::PlatformMgr().GetBridgedUniqueId(aPath.mEndpointId, uniqueId, sizeof(uniqueId));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            uniqueId[0] = '\0';
            status      = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, uniqueId, kMaxLen);
        break;
    }

    default:
        // We did not find a processing path, the caller will delegate elsewhere.
        break;
    }

    return status;
}

} // anonymous namespace

void emberAfBridgedDeviceBasicClusterServerInitCallback(EndpointId endpoint)
{
    EmberAfStatus status;

    char nodeLabel[DeviceLayer::ConfigurationManager::kMaxNodeLabelLength + 1];
    if (DeviceLayer::PlatformMgr().GetBridgedNodeLabel(aPath.mEndpointId, nodeLabel, sizeof(nodeLabel)) == CHIP_NO_ERROR)
    {
        status = Attributes::NodeLabel::Set(endpoint, CharSpan::fromCharString(nodeLabel));
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Bridged Node Label: 0x%02x", status));
    }

    bool reachable;
    if (DeviceLayer::PlatformMgr().GetBridgedReachable(aPath.mEndpointId, reachable) == CHIP_NO_ERROR)
    {
        status = Attributes::Reachable::Set(endpoint, reachable);
        VerifyOrdo(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(Zcl, "Error setting Bridged Reachable: 0x%02x", status));
    }
}

void MatterBridgedDeviceBasicPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
