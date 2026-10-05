/*
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <utility>

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/SpecificationDefinedRevisions.h>
#include <app/clusters/basic-information/BasicInformationOptionalAttributes.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/String.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/ClusterId.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Events.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceError.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

namespace chip::app::Clusters::BasicInformation {

/// This class provides a code-driven implementation for the Basic Information cluster,
/// centralizing its logic and state.
///
/// It uses a Policy-based design to decouple from DeviceLayer and generic platform logic.
///
/// The template parameter `Policy` must provide a `LifetimeDelegate` type that defines the
/// following methods:
///
/// - `void OnStartUp(uint32_t softwareVersion)`
/// - `void OnShutDown()`
template <typename Policy>
class PolicyBased : public DefaultServerCluster, public Policy::LifetimeDelegate
{
public:
    template <typename... Args>
    PolicyBased(Args &&... args) : DefaultServerCluster({ kRootEndpointId, Id }), mPolicy(std::forward<Args>(args)...)
    {}

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Policy::DelegateBase implementation (PlatformManagerDelegate for DeviceLayer)
    /**
     * @brief Initialize the cluster
     *
     * This method attempts to register the cluster as the DeviceLayer::PlatformManagerDelegate
     * to receive system shutdown events (OnShutDown).
     * * NOTE: Registration is conditional. It will ONLY register this cluster as the delegate
     * if the PlatformManager does not currently have a delegate set. If the application
     * has already registered a delegate, this cluster will respect that configuration
     * and will NOT overwrite it.
     */
    void OnStartUp(uint32_t softwareVersion) override;

    BasicInformationOptionalAttributesSet & GetOptionalAttributes() { return mPolicy.GetOptionalAttributes(); }

    void OnShutDown() override;

    // ConfigurationVersionDelegate, however NOT overridable to save
    // some flash in case this feature is never used. This means applications that may
    // change configurations at runtime pay a bit more flash, however those are probably more
    // dynamic (i.e. larger) systems like bridges or more complex systems.
    CHIP_ERROR IncreaseConfigurationVersion();

private:
    // Implementation of just the write, without doing a attribute change notification.
    //
    // Will return ActionReturnStatus::FixedStatus::kWriteSuccessNoop if the attribute write
    // is not changing the underlying data.
    DataModel::ActionReturnStatus WriteImpl(const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder);

    // Reads a single device info string using the provided getter and encodes it on success.
    // The getter (Policy) is responsible for handling optional / unimplemented attributes,
    // including any necessary error sanitization (e.g. clearing internal buffers).
    template <typename EncodeFunction>
    CHIP_ERROR ReadConfigurationString(EncodeFunction && getter, AttributeValueEncoder & encoder);

    Storage::String<Attributes::NodeLabel::TypeInfo::MaxLength()> mNodeLabel;
    Policy mPolicy;
};

namespace details {
// Unique ID became mandatory in 4. If we have no unique id, claim revision 3
inline constexpr uint32_t kRevisionWithoutUniqueId = 3;

// This is generally DeviceLayer::ConfigurationManager::kMaxLocationLength
// However we try to not tie these constants to device layer.
//
// Technically we should static_assert(kExpectedFixedLocationLength == DeviceLayer::ConfigurationManager::kMaxLocationLength);
constexpr size_t kExpectedFixedLocationLength = 2;

// The largest buffer size needed to read string attributes. This allows re-use of a common
// buffer when reading any string attribute (saves code size).
constexpr size_t kMaxStringLength = std::max({
    Attributes::VendorName::TypeInfo::MaxLength(),
    Attributes::ProductName::TypeInfo::MaxLength(),
    Attributes::HardwareVersionString::TypeInfo::MaxLength(),
    Attributes::SoftwareVersionString::TypeInfo::MaxLength(),
    Attributes::PartNumber::TypeInfo::MaxLength(),
    Attributes::ProductURL::TypeInfo::MaxLength(),
    Attributes::ProductLabel::TypeInfo::MaxLength(),
    Attributes::SerialNumber::TypeInfo::MaxLength(),
});

inline CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan(buf, strnlen(buf, maxBufSize)));
}

} // namespace details

template <typename Policy>
CHIP_ERROR PolicyBased<Policy>::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    mPolicy.RegisterPlatformDelegate(this);

    AttributePersistence persistence(context.attributeStorage);

    (void) persistence.LoadString({ kRootEndpointId, Id, Attributes::NodeLabel::Id }, mNodeLabel);

    bool localConfigDisabled = false;
    (void) persistence.LoadNativeEndianValue<bool>({ kRootEndpointId, Id, Attributes::LocalConfigDisabled::Id },
                                                   localConfigDisabled, false);

    ReturnErrorOnFailure(mPolicy.SetLocalConfigDisabled(localConfigDisabled));

    return CHIP_NO_ERROR;
}

template <typename Policy>
void PolicyBased<Policy>::Shutdown(ClusterShutdownType shutdownType)
{
    mPolicy.UnregisterPlatformDelegate(this);
    DefaultServerCluster::Shutdown(shutdownType);
}

template <typename Policy>
void PolicyBased<Policy>::OnStartUp(uint32_t softwareVersion)
{
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnStartUp", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting StartUp event");

    Events::StartUp::Type event{ softwareVersion };

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

template <typename Policy>
void PolicyBased<Policy>::OnShutDown()
{
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnShutDown", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting ShutDown event");

    Events::ShutDown::Type event;

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
    eventsGenerator.ScheduleUrgentEventDeliverySync();
}

template <typename Policy>
CHIP_ERROR PolicyBased<Policy>::IncreaseConfigurationVersion()
{
    uint32_t globalConfig = 0;
    ReturnErrorOnFailure(mPolicy.GetConfigurationVersion(globalConfig));
    ReturnErrorOnFailure(mPolicy.StoreConfigurationVersion(globalConfig + 1));
    NotifyAttributeChanged(Attributes::ConfigurationVersion::Id);
    return CHIP_NO_ERROR;
}

template <typename Policy>
template <typename EncodeFunction>
CHIP_ERROR PolicyBased<Policy>::ReadConfigurationString(EncodeFunction && getter, AttributeValueEncoder & encoder)
{
    char buffer[details::kMaxStringLength + 1];
    CHIP_ERROR status = getter(buffer, sizeof(buffer));
    return details::EncodeStringOnSuccess(status, encoder, buffer, details::kMaxStringLength);
}

template <typename Policy>
DataModel::ActionReturnStatus PolicyBased<Policy>::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    using namespace Attributes;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        if (!mPolicy.GetOptionalAttributes().IsSet(UniqueID::Id))
        {
            return encoder.Encode(details::kRevisionWithoutUniqueId);
        }
        return encoder.Encode(kRevision);
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
        char location[details::kExpectedFixedLocationLength + 1] = { 0 };
        size_t codeLen                                           = 0;
        CHIP_ERROR err                                           = mPolicy.GetCountryCode(location, sizeof(location), codeLen);
        if ((err != CHIP_NO_ERROR) || (codeLen != details::kExpectedFixedLocationLength))
        {
            return encoder.Encode("XX"_span);
        }
        return encoder.Encode(CharSpan{ location, codeLen });
    }
    case VendorName::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetVendorName(buf, size); }, encoder);
    case VendorID::Id: {
        uint16_t vendorId = 0;
        ReturnErrorOnFailure(mPolicy.GetVendorId(vendorId));
        return encoder.Encode(vendorId);
    }
    case ProductName::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetProductName(buf, size); }, encoder);
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
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetHardwareVersionString(buf, size); },
                                       encoder);
    case SoftwareVersion::Id: {
        uint32_t softwareVersion = 0;
        ReturnErrorOnFailure(mPolicy.GetSoftwareVersion(softwareVersion));
        return encoder.Encode(softwareVersion);
    }
    case SoftwareVersionString::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetSoftwareVersionString(buf, size); },
                                       encoder);
    case ManufacturingDate::Id: {
        constexpr size_t kMaxDateLength                   = 8;                                        // YYYYMMDD
        constexpr size_t kMaxTotalLength                  = ManufacturingDate::TypeInfo::MaxLength(); // 16
        constexpr size_t kMaxSuffixLength                 = kMaxTotalLength - kMaxDateLength;         // 8
        char manufacturingDateString[kMaxTotalLength + 1] = { 0 };                                    // +1 for snprintf NUL
        uint16_t manufacturingYear;
        uint8_t manufacturingMonth;
        uint8_t manufacturingDayOfMonth;
        size_t totalManufacturingDateLen = 0;
        MutableCharSpan vendorSuffixSpan(manufacturingDateString + kMaxDateLength, kMaxSuffixLength);
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
        status                    = mPolicy.GetManufacturingDateSuffix(vendorSuffixSpan);
        if (status == CHIP_NO_ERROR)
        {
            totalManufacturingDateLen += vendorSuffixSpan.size();
        }

        return encoder.Encode(CharSpan(manufacturingDateString, totalManufacturingDateLen));
    }
    case PartNumber::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetPartNumber(buf, size); }, encoder);
    case ProductURL::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetProductURL(buf, size); }, encoder);
    case ProductLabel::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetProductLabel(buf, size); }, encoder);
    case SerialNumber::Id:
        return ReadConfigurationString([this](char * buf, size_t size) { return mPolicy.GetSerialNumber(buf, size); }, encoder);
    case UniqueID::Id: {
        constexpr size_t kMaxLength   = Attributes::UniqueID::TypeInfo::MaxLength();
        char uniqueId[kMaxLength + 1] = { 0 };
        CHIP_ERROR status             = mPolicy.GetUniqueId(uniqueId, sizeof(uniqueId));
        return details::EncodeStringOnSuccess(status, encoder, uniqueId, kMaxLength);
    }
    case CapabilityMinima::Id: {
        Structs::CapabilityMinimaStruct::Type capabilityMinima;

        // TODO: These values must be set from something based on the SDK impl
        constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

        auto capabilityMinimasFromPolicy = mPolicy.GetSupportedCapabilityMinimaValues();

        capabilityMinima.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
        capabilityMinima.subscriptionsPerFabric = mPolicy.GetSubscriptionsPerFabric();
        capabilityMinima.simultaneousInvocationsSupported =
            MakeOptional<uint16_t>(capabilityMinimasFromPolicy.simultaneousInvocationsSupported);
        capabilityMinima.simultaneousWritesSupported =
            MakeOptional<uint16_t>(capabilityMinimasFromPolicy.simultaneousWritesSupported);
        capabilityMinima.readPathsSupported      = MakeOptional<uint16_t>(capabilityMinimasFromPolicy.readPathsSupported);
        capabilityMinima.subscribePathsSupported = MakeOptional<uint16_t>(capabilityMinimasFromPolicy.subscribePathsSupported);

        return encoder.Encode(capabilityMinima);
    }
    case ProductAppearance::Id: {
        ProductFinishEnum finish;
        ReturnErrorOnFailure(mPolicy.GetProductFinish(&finish));

        ColorEnum color;
        CHIP_ERROR colorStatus = mPolicy.GetProductPrimaryColor(&color);
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
DataModel::ActionReturnStatus PolicyBased<Policy>::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

template <typename Policy>
DataModel::ActionReturnStatus PolicyBased<Policy>::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                             AttributeValueDecoder & decoder)
{
    using namespace Attributes;

    AttributePersistence persistence(mContext->attributeStorage);

    switch (request.path.mAttributeId)
    {
    case Location::Id: {
        CharSpan location;
        ReturnErrorOnFailure(decoder.Decode(location));
        VerifyOrReturnError(location.size() == details::kExpectedFixedLocationLength,
                            Protocols::InteractionModel::Status::ConstraintError);
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
CHIP_ERROR PolicyBased<Policy>::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace Attributes;

    // TODO: MetadataEntry for these are static constants in the generated code.
    // They are available as Attributes::X::kMetadataEntry

    static constexpr DataModel::AttributeEntry optionalAttributes[] = {
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
    static constexpr DataModel::AttributeEntry mandatoryAttributes[] = {
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

    return listBuilder.Append(Span(mandatoryAttributes), Span(optionalAttributes), mPolicy.GetOptionalAttributes());
}

} // namespace chip::app::Clusters::BasicInformation
