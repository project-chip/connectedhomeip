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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/SpecificationDefinedRevisions.h>
#include <app/util/attribute-storage.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/Events.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
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

CHIP_ERROR ClearNullTerminatedStringWhenUnimplemented(CHIP_ERROR status, char * strBuf)
{
    if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        *strBuf = '\0';
        return CHIP_NO_ERROR;
    }

    return status;
}

class BasicAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Basic cluster on all endpoints.
    BasicAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), BasicInformation::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR WriteLocation(AttributeValueDecoder & aDecoder);
};

BasicAttrAccess gAttrAccess;

CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
}

inline CHIP_ERROR ReadVendorName(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxVendorNameLength;
    char vendorName[kMaxLen + 1] = { 0 };
    return EncodeStringOnSuccess(deviceInfoProvider->GetVendorName(vendorName, sizeof(vendorName)), aEncoder, vendorName, kMaxLen);
}

inline CHIP_ERROR ReadVendorID(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t vendorId = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetVendorId(vendorId));
    return aEncoder.Encode(vendorId);
}

inline CHIP_ERROR ReadProductName(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen      = DeviceLayer::ConfigurationManager::kMaxProductNameLength;
    char productName[kMaxLen + 1] = { 0 };
    return EncodeStringOnSuccess(deviceInfoProvider->GetProductName(productName, sizeof(productName)), aEncoder, productName,
                                 kMaxLen);
}

inline CHIP_ERROR ReadProductID(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t productId = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetProductId(productId));
    return aEncoder.Encode(productId);
}

inline CHIP_ERROR ReadHardwareVersion(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t hardwareVersion = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetHardwareVersion(hardwareVersion));
    return aEncoder.Encode(hardwareVersion);
}

inline CHIP_ERROR ReadHardwareVersionString(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength;
    char hardwareVersionString[kMaxLen + 1] = { 0 };
    return EncodeStringOnSuccess(deviceInfoProvider->GetHardwareVersionString(hardwareVersionString, sizeof(hardwareVersionString)),
                                 aEncoder, hardwareVersionString, kMaxLen);
}

inline CHIP_ERROR ReadSoftwareVersion(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    uint32_t softwareVersion = 0;
    ReturnErrorOnFailure(configManager.GetSoftwareVersion(softwareVersion));
    return aEncoder.Encode(softwareVersion);
}

inline CHIP_ERROR ReadSoftwareVersionString(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength;
    char softwareVersionString[kMaxLen + 1] = { 0 };
    return EncodeStringOnSuccess(deviceInfoProvider->GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString)),
                                 aEncoder, softwareVersionString, kMaxLen);
}

inline CHIP_ERROR ReadManufacturingDate(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen                  = DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength;
    char manufacturingDateString[kMaxLen + 1] = { 0 };
    uint16_t manufacturingYear;
    uint8_t manufacturingMonth;
    uint8_t manufacturingDayOfMonth;
    CHIP_ERROR status = deviceInfoProvider->GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);

    // TODO: Remove defaulting once proper runtime defaulting of unimplemented factory data is done
    if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        manufacturingYear       = 2020;
        manufacturingMonth      = 1;
        manufacturingDayOfMonth = 1;
        status                  = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(status);

    // Format is YYYYMMDD
    snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04u%02u%02u", manufacturingYear, manufacturingMonth,
             manufacturingDayOfMonth);
    return aEncoder.Encode(chip::CharSpan(manufacturingDateString, strnlen(manufacturingDateString, kMaxLen)));
}

inline CHIP_ERROR ReadPartNumber(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxPartNumberLength;
    char partNumber[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = deviceInfoProvider->GetPartNumber(partNumber, sizeof(partNumber));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, partNumber);
    return EncodeStringOnSuccess(status, aEncoder, partNumber, kMaxLen);
}

inline CHIP_ERROR ReadProductURL(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen     = DeviceLayer::ConfigurationManager::kMaxProductURLLength;
    char productUrl[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = deviceInfoProvider->GetProductURL(productUrl, sizeof(productUrl));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, productUrl);
    return EncodeStringOnSuccess(status, aEncoder, productUrl, kMaxLen);
}

inline CHIP_ERROR ReadProductLabel(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen       = DeviceLayer::ConfigurationManager::kMaxProductLabelLength;
    char productLabel[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = deviceInfoProvider->GetProductLabel(productLabel, sizeof(productLabel));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, productLabel);
    return EncodeStringOnSuccess(status, aEncoder, productLabel, kMaxLen);
}

inline CHIP_ERROR ReadSerialNumber(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen             = DeviceLayer::ConfigurationManager::kMaxSerialNumberLength;
    char serialNumberString[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = deviceInfoProvider->GetSerialNumber(serialNumberString, sizeof(serialNumberString));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, serialNumberString);
    return EncodeStringOnSuccess(status, aEncoder, serialNumberString, kMaxLen);
}

inline CHIP_ERROR ReadUniqueID(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen   = DeviceLayer::ConfigurationManager::kMaxUniqueIDLength;
    char uniqueId[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = configManager.GetUniqueId(uniqueId, sizeof(uniqueId));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, uniqueId);
    return EncodeStringOnSuccess(status, aEncoder, uniqueId, kMaxLen);
}

inline CHIP_ERROR ReadCapabilityMinima(AttributeValueEncoder & aEncoder)
{
    BasicInformation::Structs::CapabilityMinimaStruct::Type capabilityMinima;

    // TODO: These values must be set from something based on the SDK impl, but there are no such constants today.
    constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

    capabilityMinima.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
    capabilityMinima.subscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric();

    return aEncoder.Encode(capabilityMinima);
}

inline CHIP_ERROR ReadConfigurationVersion(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    uint32_t configurationVersion = 0;
    ReturnErrorOnFailure(configManager.GetConfigurationVersion(configurationVersion));
    return aEncoder.Encode(configurationVersion);
}

inline CHIP_ERROR ReadLocation(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    char location[kExpectedFixedLocationLength + 1] = { 0 };
    size_t codeLen                                  = 0;
    CharSpan countryCodeSpan;

    CHIP_ERROR err = configManager.GetCountryCode(location, sizeof(location), codeLen);
    if ((err != CHIP_NO_ERROR) || (codeLen != kExpectedFixedLocationLength))
    {
        countryCodeSpan = "XX"_span;
        err             = CHIP_NO_ERROR;
    }
    else
    {
        countryCodeSpan = CharSpan{ location, codeLen };
    }
    return aEncoder.Encode(countryCodeSpan);
}

inline CHIP_ERROR ReadMaxPathsPerInvoke(AttributeValueEncoder & aEncoder)
{
    uint16_t max_path_per_invoke = CHIP_CONFIG_MAX_PATHS_PER_INVOKE;
    return aEncoder.Encode(max_path_per_invoke);
}

inline CHIP_ERROR ReadSpecificationVersion(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(Revision::kSpecificationVersion);
}

inline CHIP_ERROR ReadProductAppearance(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    ProductFinishEnum finish;
    ReturnErrorOnFailure(deviceInfoProvider->GetProductFinish(&finish));

    ColorEnum color;
    CHIP_ERROR colorStatus = deviceInfoProvider->GetProductPrimaryColor(&color);
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

CHIP_ERROR BasicAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    // NOTE: this is NEVER nullptr, using pointer as we have seen converting to reference
    //       costs some flash (even though code would be more readable that way...)
    auto * deviceInfoProvider = GetDeviceInstanceInfoProvider();
    auto & configManager      = ConfigurationMgr();

    switch (aPath.mAttributeId)
    {
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    case DataModelRevision::Id:
        return aEncoder.Encode(Revision::kDataModelRevision);
    case Location::Id:
        return ReadLocation(configManager, aEncoder);
    case VendorName::Id:
        return ReadVendorName(deviceInfoProvider, aEncoder);
    case VendorID::Id:
        return ReadVendorID(deviceInfoProvider, aEncoder);
    case ProductName::Id:
        return ReadProductName(deviceInfoProvider, aEncoder);
    case ProductID::Id:
        return ReadProductID(deviceInfoProvider, aEncoder);
    case HardwareVersion::Id:
        return ReadHardwareVersion(deviceInfoProvider, aEncoder);
    case HardwareVersionString::Id:
        return ReadHardwareVersionString(deviceInfoProvider, aEncoder);
    case SoftwareVersion::Id:
        return ReadSoftwareVersion(configManager, aEncoder);
    case SoftwareVersionString::Id:
        return ReadSoftwareVersionString(deviceInfoProvider, aEncoder);
    case ManufacturingDate::Id:
        return ReadManufacturingDate(deviceInfoProvider, aEncoder);
    case PartNumber::Id:
        return ReadPartNumber(deviceInfoProvider, aEncoder);
    case ProductURL::Id:
        return ReadProductURL(deviceInfoProvider, aEncoder);
    case ProductLabel::Id:
        return ReadProductLabel(deviceInfoProvider, aEncoder);
    case SerialNumber::Id:
        return ReadSerialNumber(deviceInfoProvider, aEncoder);
    case UniqueID::Id:
        return ReadUniqueID(configManager, aEncoder);
    case CapabilityMinima::Id:
        return ReadCapabilityMinima(aEncoder);
    case ProductAppearance::Id:
        return ReadProductAppearance(deviceInfoProvider, aEncoder);
    case SpecificationVersion::Id:
        return ReadSpecificationVersion(aEncoder);
    case MaxPathsPerInvoke::Id:
        return ReadMaxPathsPerInvoke(aEncoder);
    case ConfigurationVersion::Id:
        return ReadConfigurationVersion(configManager, aEncoder);
    default:
        // We did not find a processing path, the caller will delegate elsewhere.
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BasicAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Location::Id: {
        return WriteLocation(aDecoder);
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

    bool isValidLength = location.size() == kExpectedFixedLocationLength;
    if (!isValidLength)
    {
        ChipLogError(Zcl, "Invalid country code: '%s'", NullTerminated(location).c_str());
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
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
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
    PlatformMgr().SetDelegate(&gPlatformMgrDelegate);
}

void MatterBasicInformationPluginServerShutdownCallback()
{
    PlatformMgr().SetDelegate(nullptr);
    AttributeAccessInterfaceRegistry::Instance().Unregister(&gAttrAccess);
}
