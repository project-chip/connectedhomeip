/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include "basic-information.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/SpecificationDefinedRevisions.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstddef>
#include <cstring>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr size_t kExpectedFixedLocationLength = 2;
static_assert(kExpectedFixedLocationLength == DeviceLayer::ConfigurationManager::kMaxLocationLength,
              "Fixed location storage must be of size 2");

class BasicAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Basic cluster on all endpoints.
    BasicAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), BasicInformation::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadDataModelRevision(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadLocation(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteLocation(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadProductAppearance(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSpecificationVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxPathsPerInvoke(AttributeValueEncoder & aEncoder);
};

BasicAttrAccess gAttrAccess;

CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
}

CHIP_ERROR BasicAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != BasicInformation::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR status = CHIP_NO_ERROR;

    switch (aPath.mAttributeId)
    {
    case DataModelRevision::Id:
        status = ReadDataModelRevision(aEncoder);
        break;

    case Location::Id:
        status = ReadLocation(aEncoder);
        break;

    case VendorName::Id: {
        constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxVendorNameLength;
        char vendorName[kMaxLen + 1] = { 0 };
        status                       = GetDeviceInstanceInfoProvider()->GetVendorName(vendorName, sizeof(vendorName));
        status                       = EncodeStringOnSuccess(status, aEncoder, vendorName, kMaxLen);
        break;
    }

    case VendorID::Id: {
        uint16_t vendorId = 0;
        status            = GetDeviceInstanceInfoProvider()->GetVendorId(vendorId);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(vendorId);
        }
        break;
    }

    case ProductName::Id: {
        constexpr size_t kMaxLen      = DeviceLayer::ConfigurationManager::kMaxProductNameLength;
        char productName[kMaxLen + 1] = { 0 };
        status                        = GetDeviceInstanceInfoProvider()->GetProductName(productName, sizeof(productName));
        status                        = EncodeStringOnSuccess(status, aEncoder, productName, kMaxLen);
        break;
    }

    case ProductID::Id: {
        uint16_t productId = 0;
        status             = GetDeviceInstanceInfoProvider()->GetProductId(productId);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(productId);
        }
        break;
    }

    case HardwareVersion::Id: {
        uint16_t hardwareVersion = 0;
        status                   = GetDeviceInstanceInfoProvider()->GetHardwareVersion(hardwareVersion);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(hardwareVersion);
        }
        break;
    }

    case HardwareVersionString::Id: {
        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength;
        char hardwareVersionString[kMaxLen + 1] = { 0 };
        status = GetDeviceInstanceInfoProvider()->GetHardwareVersionString(hardwareVersionString, sizeof(hardwareVersionString));
        status = EncodeStringOnSuccess(status, aEncoder, hardwareVersionString, kMaxLen);
        break;
    }

    case SoftwareVersion::Id: {
        uint32_t softwareVersion = 0;
        status                   = ConfigurationMgr().GetSoftwareVersion(softwareVersion);
        if (status == CHIP_NO_ERROR)
        {
            status = aEncoder.Encode(softwareVersion);
        }
        break;
    }

    case SoftwareVersionString::Id: {
        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength;
        char softwareVersionString[kMaxLen + 1] = { 0 };
        status = ConfigurationMgr().GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString));
        status = EncodeStringOnSuccess(status, aEncoder, softwareVersionString, kMaxLen);
        break;
    }

    case ManufacturingDate::Id: {
        constexpr size_t kMaxLen                  = DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength;
        char manufacturingDateString[kMaxLen + 1] = { 0 };
        uint16_t manufacturingYear;
        uint8_t manufacturingMonth;
        uint8_t manufacturingDayOfMonth;
        status =
            GetDeviceInstanceInfoProvider()->GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);

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
            snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04u%02u%02u", manufacturingYear,
                     manufacturingMonth, manufacturingDayOfMonth);
            status = aEncoder.Encode(chip::CharSpan(manufacturingDateString, strnlen(manufacturingDateString, kMaxLen)));
        }
        break;
    }

    case PartNumber::Id: {
        constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxPartNumberLength;
        char partNumber[kMaxLen + 1] = { 0 };
        status                       = GetDeviceInstanceInfoProvider()->GetPartNumber(partNumber, sizeof(partNumber));

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
        status                       = GetDeviceInstanceInfoProvider()->GetProductURL(productUrl, sizeof(productUrl));

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
        status                         = GetDeviceInstanceInfoProvider()->GetProductLabel(productLabel, sizeof(productLabel));

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
        status = GetDeviceInstanceInfoProvider()->GetSerialNumber(serialNumberString, sizeof(serialNumberString));

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
        status                     = ConfigurationMgr().GetUniqueId(uniqueId, sizeof(uniqueId));

        // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            uniqueId[0] = '\0';
            status      = CHIP_NO_ERROR;
        }

        status = EncodeStringOnSuccess(status, aEncoder, uniqueId, kMaxLen);
        break;
    }

    case CapabilityMinima::Id: {
        BasicInformation::Structs::CapabilityMinimaStruct::Type capabilityMinima;

        // TODO: These values must be set from something based on the SDK impl, but there are no such constants today.
        constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

        capabilityMinima.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
        capabilityMinima.subscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric();

        status = aEncoder.Encode(capabilityMinima);
        break;
    }

    case ProductAppearance::Id: {
        status = ReadProductAppearance(aEncoder);
        break;
    }

    case SpecificationVersion::Id: {
        status = ReadSpecificationVersion(aEncoder);
        break;
    }

    case MaxPathsPerInvoke::Id: {
        status = ReadMaxPathsPerInvoke(aEncoder);
        break;
    }

    default:
        // We did not find a processing path, the caller will delegate elsewhere.
        break;
    }

    return status;
}

CHIP_ERROR BasicAttrAccess::ReadDataModelRevision(AttributeValueEncoder & aEncoder)
{
    uint16_t revision = Revision::kDataModelRevision;
    return aEncoder.Encode(revision);
}

CHIP_ERROR BasicAttrAccess::ReadLocation(AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen   = DeviceLayer::ConfigurationManager::kMaxLocationLength;
    char location[kMaxLen + 1] = { 0 };
    size_t codeLen             = 0;

    CHIP_ERROR err = ConfigurationMgr().GetCountryCode(location, sizeof(location), codeLen);
    if ((err != CHIP_NO_ERROR) || (codeLen == 0))
    {
        Platform::CopyString(location, "XX");
        codeLen = strnlen(location, kMaxLen);
        err     = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return aEncoder.Encode(chip::CharSpan(location, codeLen));
}

CHIP_ERROR BasicAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == BasicInformation::Id);

    switch (aPath.mAttributeId)
    {
    case Location::Id: {
        CHIP_ERROR err = WriteLocation(aDecoder);

        return err;
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BasicAttrAccess::WriteLocation(AttributeValueDecoder & aDecoder)
{
    chip::CharSpan location;

    ReturnErrorOnFailure(aDecoder.Decode(location));

    bool isValidLength = location.size() == DeviceLayer::ConfigurationManager::kMaxLocationLength;
    if (!isValidLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%.*s'", static_cast<int>(location.size()), location.data());
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
}

CHIP_ERROR BasicAttrAccess::ReadProductAppearance(AttributeValueEncoder & aEncoder)
{
    auto * provider = GetDeviceInstanceInfoProvider();
    ProductFinishEnum finish;
    ReturnErrorOnFailure(provider->GetProductFinish(&finish));

    ColorEnum color;
    CHIP_ERROR colorStatus = provider->GetProductPrimaryColor(&color);
    if (colorStatus != CHIP_NO_ERROR && colorStatus != CHIP_ERROR_NOT_IMPLEMENTED)
    {
        return colorStatus;
    }

    Structs::ProductAppearanceStruct::Type productAppearance;
    productAppearance.finish = finish;
    if (colorStatus == CHIP_NO_ERROR)
    {
        productAppearance.primaryColor.SetNonNull(color);
    }
    else
    {
        productAppearance.primaryColor.SetNull();
    }

    return aEncoder.Encode(productAppearance);
}

CHIP_ERROR BasicAttrAccess::ReadSpecificationVersion(AttributeValueEncoder & aEncoder)
{
    uint32_t specification_version = Revision::kSpecificationVersion;
    return aEncoder.Encode(specification_version);
}

CHIP_ERROR BasicAttrAccess::ReadMaxPathsPerInvoke(AttributeValueEncoder & aEncoder)
{
    uint16_t max_path_per_invoke = CHIP_CONFIG_MAX_PATHS_PER_INVOKE;
    return aEncoder.Encode(max_path_per_invoke);
}

class PlatformMgrDelegate : public DeviceLayer::PlatformManagerDelegate
{
    void OnStartUp(uint32_t softwareVersion) override
    {
        MATTER_TRACE_INSTANT("OnStartUp", "BasicInfo");
        // The StartUp event SHALL be emitted by a Node after completing a boot or reboot process
        ChipLogDetail(Zcl, "Emitting StartUp event");

        for (auto endpoint : EnabledEndpointsWithServerCluster(BasicInformation::Id))
        {
            // If Basic cluster is implemented on this endpoint
            Events::StartUp::Type event{ softwareVersion };
            EventNumber eventNumber;

            CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
            if (CHIP_NO_ERROR != err)
            {
                ChipLogError(Zcl, "Failed to emit StartUp event: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }

    void OnShutDown() override
    {
        MATTER_TRACE_INSTANT("OnShutDown", "BasicInfo");
        // The ShutDown event SHOULD be emitted on a best-effort basis by a Node prior to any orderly shutdown sequence.
        ChipLogDetail(Zcl, "Emitting ShutDown event");

        for (auto endpoint : EnabledEndpointsWithServerCluster(BasicInformation::Id))
        {
            // If Basic cluster is implemented on this endpoint
            Events::ShutDown::Type event;
            EventNumber eventNumber;

            CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
            if (CHIP_NO_ERROR != err)
            {
                ChipLogError(Zcl, "Failed to emit ShutDown event: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }

        // Flush the events to increase chances that they get sent before the shutdown
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleUrgentEventDeliverySync();
    }
};

PlatformMgrDelegate gPlatformMgrDelegate;

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {
bool IsLocalConfigDisabled()
{
    bool disabled = false;
    Status status = LocalConfigDisabled::Get(0, &disabled);
    return status == Status::Success && disabled;
}
} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterBasicInformationPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
    PlatformMgr().SetDelegate(&gPlatformMgrDelegate);
}
