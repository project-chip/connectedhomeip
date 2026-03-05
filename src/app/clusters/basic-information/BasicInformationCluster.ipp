/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/SpecificationDefinedRevisions.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>

namespace chip {
namespace app {
namespace Clusters {

namespace BasicInformationClusterImplDetails {
// Unique ID became mandatory in 4. If we have no unique id, claim revision 3
inline constexpr uint32_t kRevisionWithoutUniqueId = 3;

constexpr size_t kExpectedFixedLocationLength = 2; // DeviceLayer definition used to suffice, but we define it here for generic usage
// static_assert(kExpectedFixedLocationLength == DeviceLayer::ConfigurationManager::kMaxLocationLength,
//              "Fixed location storage must be of size 2"); // TODO: How to static assert against policy?

// Use a private helper namespace or static members for these constants
constexpr size_t kMaxStringLength = 256; // Max of all string lengths

inline CHIP_ERROR ClearNullTerminatedStringWhenUnimplemented(CHIP_ERROR status, char * strBuf)
{
    if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        *strBuf = '\0';
        return CHIP_NO_ERROR;
    }

    return status;
}

inline CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
}

} // namespace BasicInformationClusterImplDetails

template <typename Policy>
CHIP_ERROR BasicInformationClusterImpl<Policy>::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Register this cluster as the PlatformManager delegate to receive shutdown events.
    mPolicy.RegisterPlatformDelegate(this);

    AttributePersistence persistence(context.attributeStorage);

    (void) persistence.LoadString({ kRootEndpointId, BasicInformation::Id, BasicInformation::Attributes::NodeLabel::Id }, mNodeLabel);
    
    bool localConfigDisabled = false;
    (void) persistence.LoadNativeEndianValue<bool>({ kRootEndpointId, BasicInformation::Id, BasicInformation::Attributes::LocalConfigDisabled::Id },
                                                   localConfigDisabled, false);

    // Propagate the restored 'LocalConfigDisabled' state to the policy
    ReturnErrorOnFailure(mPolicy.SetLocalConfigDisabled(localConfigDisabled));

    return CHIP_NO_ERROR;
}

template <typename Policy>
void BasicInformationClusterImpl<Policy>::Shutdown(ClusterShutdownType shutdownType)
{
    mPolicy.UnregisterPlatformDelegate(this);
    DefaultServerCluster::Shutdown(shutdownType);
}

template <typename Policy>
void BasicInformationClusterImpl<Policy>::OnStartUp(uint32_t softwareVersion)
{
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnStartUp", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting StartUp event");

    BasicInformation::Events::StartUp::Type event{ softwareVersion };

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

template <typename Policy>
void BasicInformationClusterImpl<Policy>::OnShutDown()
{
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnShutDown", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting ShutDown event");

    BasicInformation::Events::ShutDown::Type event;

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
    eventsGenerator.ScheduleUrgentEventDeliverySync();
}

template <typename Policy>
CHIP_ERROR BasicInformationClusterImpl<Policy>::IncreaseConfigurationVersion()
{
    uint32_t globalConfig = 0;
    ReturnErrorOnFailure(mPolicy.GetConfigurationVersion(globalConfig));
    ReturnErrorOnFailure(mPolicy.StoreConfigurationVersion(globalConfig + 1));
    NotifyAttributeChanged(BasicInformation::Attributes::ConfigurationVersion::Id);
    return CHIP_NO_ERROR;
}

template <typename Policy>
template <typename EncodeFunction>
CHIP_ERROR BasicInformationClusterImpl<Policy>::ReadConfigurationString(EncodeFunction && getter, bool unimplementedAllowed,
                                                                        AttributeValueEncoder & encoder)
{
    char buffer[BasicInformationClusterImplDetails::kMaxStringLength + 1] = { 0 };
    CHIP_ERROR status = getter(buffer, sizeof(buffer));
    
    if (unimplementedAllowed)
    {
        status = BasicInformationClusterImplDetails::ClearNullTerminatedStringWhenUnimplemented(status, buffer);
    }
    return BasicInformationClusterImplDetails::EncodeStringOnSuccess(status, encoder, buffer, BasicInformationClusterImplDetails::kMaxStringLength);
}

template <typename Policy>
DataModel::ActionReturnStatus BasicInformationClusterImpl<Policy>::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                 AttributeValueEncoder & encoder)
{
    using namespace BasicInformation::Attributes;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        if (!mEnabledOptionalAttributes.IsSet(UniqueID::Id))
        {
            return encoder.Encode(BasicInformationClusterImplDetails::kRevisionWithoutUniqueId);
        }
        return encoder.Encode(BasicInformation::kRevision);
    case NodeLabel::Id:
        return encoder.Encode(mNodeLabel.Content());
    case LocalConfigDisabled::Id: {
        bool localConfigDisabled = false;
        ReturnErrorOnFailure(mPolicy.GetLocalConfigDisabled(localConfigDisabled));
        return encoder.Encode(localConfigDisabled);
    }
    case DataModelRevision::Id:
        return encoder.Encode(Revision::kDataModelRevision);
    case Location::Id: {
        char location[BasicInformationClusterImplDetails::kExpectedFixedLocationLength + 1] = { 0 };
        size_t codeLen = 0;
        CHIP_ERROR err = mPolicy.GetCountryCode(location, sizeof(location), codeLen);
        if ((err != CHIP_NO_ERROR) || (codeLen != BasicInformationClusterImplDetails::kExpectedFixedLocationLength))
        {
            return encoder.Encode("XX"_span);
        }
        return encoder.Encode(CharSpan{ location, codeLen });
    }
    case VendorName::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetVendorName(buf, size); },
                                       false, encoder);
    case VendorID::Id: {
        uint16_t vendorId = 0;
        ReturnErrorOnFailure(mPolicy.GetVendorId(vendorId));
        return encoder.Encode(vendorId);
    }
    case ProductName::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetProductName(buf, size); },
                                       false, encoder);
    case ProductID::Id: {
        uint16_t productId = 0;
        ReturnErrorOnFailure(mPolicy.GetProductId(productId));
        return encoder.Encode(productId);
    }
    case HardwareVersion::Id: {
        uint16_t hardwareVersion = 0;
        ReturnErrorOnFailure(mPolicy.GetHardwareVersion(hardwareVersion));
        return encoder.Encode(hardwareVersion);
    }
    case HardwareVersionString::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetHardwareVersionString(buf, size); },
                                       false, encoder);
    case SoftwareVersion::Id: {
        uint32_t softwareVersion = 0;
        ReturnErrorOnFailure(mPolicy.GetSoftwareVersion(softwareVersion));
        return encoder.Encode(softwareVersion);
    }
    case SoftwareVersionString::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetSoftwareVersionString(buf, size); },
                                       false /* unimplementedAllowed */, encoder); // Wait, original was false for SW Version String
    case ManufacturingDate::Id: {
        constexpr size_t kMaxDateLength = 8; // YYYYMMDD
        char manufacturingDateString[17] = { 0 }; // kMaxManufacturingDateLength around 16
        uint16_t manufacturingYear;
        uint8_t manufacturingMonth;
        uint8_t manufacturingDayOfMonth;
        size_t totalManufacturingDateLen = 0;
        MutableCharSpan vendorSuffixSpan(manufacturingDateString + kMaxDateLength, sizeof(manufacturingDateString) - kMaxDateLength);
        CHIP_ERROR status = mPolicy.GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);

        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            manufacturingYear       = 2020;
            manufacturingMonth      = 1;
            manufacturingDayOfMonth = 1;
            vendorSuffixSpan.reduce_size(0);
            status = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(status);

        snprintf(manufacturingDateString, sizeof(manufacturingDateString), "%04u%02u%02u", manufacturingYear, manufacturingMonth,
                 manufacturingDayOfMonth);

        totalManufacturingDateLen = kMaxDateLength;
        status = mPolicy.GetManufacturingDateSuffix(vendorSuffixSpan);
        if (status == CHIP_NO_ERROR)
        {
            totalManufacturingDateLen += vendorSuffixSpan.size();
        }

        return encoder.Encode(CharSpan(manufacturingDateString, totalManufacturingDateLen));
    }
    case PartNumber::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetPartNumber(buf, size); },
                                       true, encoder);
    case ProductURL::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetProductURL(buf, size); },
                                       true, encoder);
    case ProductLabel::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetProductLabel(buf, size); },
                                       true, encoder);
    case SerialNumber::Id:
        return ReadConfigurationString([this](char* buf, size_t size) { return mPolicy.GetSerialNumber(buf, size); },
                                       true, encoder);
    case UniqueID::Id: {
        char uniqueId[33] = { 0 };
        CHIP_ERROR status = mPolicy.GetUniqueId(uniqueId, sizeof(uniqueId));
        status = BasicInformationClusterImplDetails::ClearNullTerminatedStringWhenUnimplemented(status, uniqueId);
        return BasicInformationClusterImplDetails::EncodeStringOnSuccess(status, encoder, uniqueId, 32);
    }
    case CapabilityMinima::Id: {
        BasicInformation::Structs::CapabilityMinimaStruct::Type capabilityMinima;

        // TODO: These values must be set from something based on the SDK impl
        constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

        auto capabilityMinimasFromPolicy = mPolicy.GetSupportedCapabilityMinimaValues();

        capabilityMinima.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
        capabilityMinima.subscriptionsPerFabric = mPolicy.GetSubscriptionsPerFabric();
        capabilityMinima.simultaneousInvocationsSupported =
            chip::MakeOptional<uint16_t>(capabilityMinimasFromPolicy.simultaneousInvocationsSupported);
        capabilityMinima.simultaneousWritesSupported =
            chip::MakeOptional<uint16_t>(capabilityMinimasFromPolicy.simultaneousWritesSupported);
        capabilityMinima.readPathsSupported = chip::MakeOptional<uint16_t>(capabilityMinimasFromPolicy.readPathsSupported);
        capabilityMinima.subscribePathsSupported =
            chip::MakeOptional<uint16_t>(capabilityMinimasFromPolicy.subscribePathsSupported);

        return encoder.Encode(capabilityMinima);
    }
    case ProductAppearance::Id: {
        BasicInformation::ProductFinishEnum finish;
        ReturnErrorOnFailure(mPolicy.GetProductFinish(&finish));

        BasicInformation::ColorEnum color;
        CHIP_ERROR colorStatus = mPolicy.GetProductPrimaryColor(&color);
        if (colorStatus != CHIP_NO_ERROR && colorStatus != CHIP_ERROR_NOT_IMPLEMENTED)
        {
            return colorStatus;
        }

        BasicInformation::Structs::ProductAppearanceStruct::Type productAppearance;
        productAppearance.finish = finish;
        if (colorStatus == CHIP_NO_ERROR)
        {
            productAppearance.primaryColor.SetNonNull(color);
        }
        else
        {
            productAppearance.primaryColor.SetNull();
        }

        return encoder.Encode(productAppearance);
    }
    case SpecificationVersion::Id:
        return encoder.Encode(Revision::kSpecificationVersion);
    case MaxPathsPerInvoke::Id:
        return encoder.Encode<uint16_t>(CHIP_CONFIG_MAX_PATHS_PER_INVOKE);
    case ConfigurationVersion::Id: {
        uint32_t configurationVersion = 0;
        ReturnErrorOnFailure(mPolicy.GetConfigurationVersion(configurationVersion));
        return encoder.Encode(configurationVersion);
    }
    case Reachable::Id:
        return encoder.Encode<bool>(true);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

template <typename Policy>
DataModel::ActionReturnStatus BasicInformationClusterImpl<Policy>::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                  AttributeValueDecoder & decoder)
{
    // WriteImpl is private, so we can access it
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

template <typename Policy>
DataModel::ActionReturnStatus BasicInformationClusterImpl<Policy>::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                             AttributeValueDecoder & decoder)
{
    using namespace BasicInformation::Attributes;

    AttributePersistence persistence(mContext->attributeStorage);

    switch (request.path.mAttributeId)
    {
    case Location::Id: {
        CharSpan location;
        ReturnErrorOnFailure(decoder.Decode(location));
        VerifyOrReturnError(location.size() == BasicInformationClusterImplDetails::kExpectedFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
        return mPolicy.StoreCountryCode(location.data(), location.size());
    }
    case NodeLabel::Id: {
        CharSpan label;
        ReturnErrorOnFailure(decoder.Decode(label));
        VerifyOrReturnError(mNodeLabel.SetContent(label), Protocols::InteractionModel::Status::ConstraintError);
        return persistence.StoreString(request.path, mNodeLabel);
    }
    case LocalConfigDisabled::Id: {
        bool localConfigDisabled = false;
        ReturnErrorOnFailure(mPolicy.GetLocalConfigDisabled(localConfigDisabled));
        auto decodeStatus = persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, localConfigDisabled);
        ReturnErrorOnFailure(mPolicy.SetLocalConfigDisabled(localConfigDisabled));
        return decodeStatus;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

template <typename Policy>
CHIP_ERROR BasicInformationClusterImpl<Policy>::Attributes(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace BasicInformation::Attributes;

    // TODO: MetadataEntry for these are static constants in the generated code.
    // They are available as BasicInformation::Attributes::X::kMetadataEntry

    DataModel::AttributeEntry optionalAttributes[] = {
        ManufacturingDate::kMetadataEntry,   //
        PartNumber::kMetadataEntry,          //
        ProductURL::kMetadataEntry,          //
        ProductLabel::kMetadataEntry,        //
        SerialNumber::kMetadataEntry,        //
        LocalConfigDisabled::kMetadataEntry, //
        Reachable::kMetadataEntry,           //
        ProductAppearance::kMetadataEntry,   //

        // Optional because of forced multi-revision support for backwards compatibility
        // emulation: we emulate revision 3 when uniqueid is not enabled.
        UniqueID::kMetadataEntry, //
    };

    // kMandatoryAttributes equivalent
    DataModel::AttributeEntry mandatoryAttributes[] = {
        DataModelRevision::kMetadataEntry,
        VendorName::kMetadataEntry,
        VendorID::kMetadataEntry,
        ProductName::kMetadataEntry,
        ProductID::kMetadataEntry,
        NodeLabel::kMetadataEntry,
        Location::kMetadataEntry,
        HardwareVersion::kMetadataEntry,
        HardwareVersionString::kMetadataEntry,
        SoftwareVersion::kMetadataEntry,
        SoftwareVersionString::kMetadataEntry,
        CapabilityMinima::kMetadataEntry,
        SpecificationVersion::kMetadataEntry,
        MaxPathsPerInvoke::kMetadataEntry,
        ConfigurationVersion::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(mandatoryAttributes), Span(optionalAttributes), mEnabledOptionalAttributes);
}

} // namespace Clusters
} // namespace app
} // namespace chip
