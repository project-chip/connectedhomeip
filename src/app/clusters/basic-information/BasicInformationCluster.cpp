/*
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
#include <app/clusters/basic-information/BasicInformationCluster.h>

#include <app/InteractionModelEngine.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Events.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;
using namespace chip::DeviceLayer;

namespace {

// Unique ID became mandatory in 4. If we have no unique id, claim revision 3
inline constexpr uint32_t kRevisionWithoutUniqueId = 3;

// This is NOT the same as the auto-generated attributes:
// see comment below about UniqueID (which we make behave as optional)
//
// TLDR: DO NOT USE kMandatoryMetadata (because unique ID is special for backwards compat builds)
constexpr DataModel::AttributeEntry kMandatoryAttributes[] = {
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
    // NOTE: UniqueID used to NOT be mandatory in previous spec version, so we add
    // this as a separate condition
    // UniqueID::kMetadataEntry,

};

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

CHIP_ERROR EncodeStringOnSuccess(CHIP_ERROR status, AttributeValueEncoder & encoder, const char * buf, size_t maxBufSize)
{
    ReturnErrorOnFailure(status);
    return encoder.Encode(chip::CharSpan(buf, strnlen(buf, maxBufSize)));
}

constexpr size_t kMaxStringLength = std::max({
    DeviceLayer::ConfigurationManager::kMaxVendorNameLength,
    DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength,
    DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength,
    DeviceLayer::ConfigurationManager::kMaxPartNumberLength,
    DeviceLayer::ConfigurationManager::kMaxProductURLLength,
    DeviceLayer::ConfigurationManager::kMaxProductLabelLength,
    DeviceLayer::ConfigurationManager::kMaxSerialNumberLength,
    DeviceLayer::ConfigurationManager::kMaxUniqueIDLength,
});

/// Reads a single device info string. Separate function to optimize for flash cost
/// as querying strings seems to be a frequent operation.
CHIP_ERROR ReadConfigurationString(DeviceInstanceInfoProvider * deviceInfoProvider,
                                   CHIP_ERROR (DeviceInstanceInfoProvider::*getter)(char *, size_t), bool unimplementedAllowed,
                                   AttributeValueEncoder & encoder)
{
    char buffer[kMaxStringLength + 1] = { 0 };
    CHIP_ERROR status                 = ((deviceInfoProvider)->*(getter))(buffer, sizeof(buffer));
    if (unimplementedAllowed)
    {
        status = ClearNullTerminatedStringWhenUnimplemented(status, buffer);
    }
    return EncodeStringOnSuccess(status, encoder, buffer, kMaxStringLength);
}

inline CHIP_ERROR ReadVendorID(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t vendorId = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetVendorId(vendorId));
    return aEncoder.Encode(vendorId);
}

inline CHIP_ERROR ReadProductID(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t productId = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetProductId(productId));
    return aEncoder.Encode(productId);
}

inline CHIP_ERROR ReadLocalConfigDisabled(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    bool localConfigDisabled = false;
    ReturnErrorOnFailure(deviceInfoProvider->GetLocalConfigDisabled(localConfigDisabled));
    return aEncoder.Encode(localConfigDisabled);
}

inline CHIP_ERROR ReadHardwareVersion(DeviceInstanceInfoProvider * deviceInfoProvider, AttributeValueEncoder & aEncoder)
{
    uint16_t hardwareVersion = 0;
    ReturnErrorOnFailure(deviceInfoProvider->GetHardwareVersion(hardwareVersion));
    return aEncoder.Encode(hardwareVersion);
}

inline CHIP_ERROR ReadSoftwareVersion(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    uint32_t softwareVersion = 0;
    ReturnErrorOnFailure(configManager.GetSoftwareVersion(softwareVersion));
    return aEncoder.Encode(softwareVersion);
}

inline CHIP_ERROR ReadSoftwareVersionString(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength;
    char softwareVersionString[kMaxLen + 1] = { 0 };
    ReturnErrorOnFailure(configManager.GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString)));
    return aEncoder.Encode(CharSpan(softwareVersionString, strnlen(softwareVersionString, kMaxLen)));
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
    return aEncoder.Encode(CharSpan(manufacturingDateString, strnlen(manufacturingDateString, kMaxLen)));
}

inline CHIP_ERROR ReadUniqueID(DeviceLayer::ConfigurationManager & configManager, AttributeValueEncoder & aEncoder)
{
    constexpr size_t kMaxLen   = DeviceLayer::ConfigurationManager::kMaxUniqueIDLength;
    char uniqueId[kMaxLen + 1] = { 0 };

    CHIP_ERROR status = configManager.GetUniqueId(uniqueId, sizeof(uniqueId));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, uniqueId);
    return EncodeStringOnSuccess(status, aEncoder, uniqueId, kMaxLen);
}

inline CHIP_ERROR ReadCapabilityMinima(AttributeValueEncoder & aEncoder, DeviceInstanceInfoProvider * deviceInfoProvider)
{
    BasicInformation::Structs::CapabilityMinimaStruct::Type capabilityMinima;

    // TODO: These values must be set from something based on the SDK impl, but there are no such constants today.
    constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

    DeviceInstanceInfoProvider::DeviceInfoCapabilityMinimas capabilityMinimasFromDeviceInfo =
        deviceInfoProvider->GetSupportedCapabilityMinimaValues();

    capabilityMinima.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
    capabilityMinima.subscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric();
    capabilityMinima.simultaneousInvocationsSupported =
        chip::MakeOptional<uint16_t>(capabilityMinimasFromDeviceInfo.simultaneousInvocationsSupported);
    capabilityMinima.simultaneousWritesSupported =
        chip::MakeOptional<uint16_t>(capabilityMinimasFromDeviceInfo.simultaneousWritesSupported);
    capabilityMinima.readPathsSupported = chip::MakeOptional<uint16_t>(capabilityMinimasFromDeviceInfo.readPathsSupported);
    capabilityMinima.subscribePathsSupported =
        chip::MakeOptional<uint16_t>(capabilityMinimasFromDeviceInfo.subscribePathsSupported);

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

} // namespace

namespace chip::app::Clusters {

DataModel::ActionReturnStatus BasicInformationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    using namespace BasicInformation::Attributes;

    DeviceInstanceInfoProvider * deviceInfoProvider;
    ReturnErrorOnFailure(GetDeviceInstanceInfoProviderImpl(&deviceInfoProvider));
    auto & configManager = ConfigurationMgr();

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        // Explicit specialization: TLVWriter.Put has specialization for various types
        // but fails for `0u` with `unsigned int &` being ambigous.
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        if (!mEnabledOptionalAttributes.IsSet(UniqueID::Id))
        {
            return encoder.Encode(kRevisionWithoutUniqueId);
        }
        return encoder.Encode(BasicInformation::kRevision);
    case NodeLabel::Id:
        return encoder.Encode(mNodeLabel.Content());
    case LocalConfigDisabled::Id:
        return ReadLocalConfigDisabled(deviceInfoProvider, encoder);
    case DataModelRevision::Id:
        return encoder.Encode(Revision::kDataModelRevision);
    case Location::Id:
        return ReadLocation(configManager, encoder);
    case VendorName::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetVendorName,
                                       false /* unimplementedAllowed */, encoder);
    case VendorID::Id:
        return ReadVendorID(deviceInfoProvider, encoder);
    case ProductName::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetProductName,
                                       false /* unimplementedAllowed */, encoder);
    case ProductID::Id:
        return ReadProductID(deviceInfoProvider, encoder);
    case HardwareVersion::Id:
        return ReadHardwareVersion(deviceInfoProvider, encoder);
    case HardwareVersionString::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetHardwareVersionString,
                                       false /* unimplementedAllowed */, encoder);
    case SoftwareVersion::Id:
        return ReadSoftwareVersion(configManager, encoder);
    case SoftwareVersionString::Id:
        return ReadSoftwareVersionString(configManager, encoder);
    case ManufacturingDate::Id:
        return ReadManufacturingDate(deviceInfoProvider, encoder);
    case PartNumber::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetPartNumber,
                                       true /* unimplementedAllowed */, encoder);
    case ProductURL::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetProductURL,
                                       true /* unimplementedAllowed */, encoder);
    case ProductLabel::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetProductLabel,
                                       true /* unimplementedAllowed */, encoder);
    case SerialNumber::Id:
        return ReadConfigurationString(deviceInfoProvider, &DeviceInstanceInfoProvider::GetSerialNumber,
                                       true /* unimplementedAllowed */, encoder);
    case UniqueID::Id:
        return ReadUniqueID(configManager, encoder);
    case CapabilityMinima::Id:
        return ReadCapabilityMinima(encoder, deviceInfoProvider);
    case ProductAppearance::Id:
        return ReadProductAppearance(deviceInfoProvider, encoder);
    case SpecificationVersion::Id:
        return encoder.Encode(Revision::kSpecificationVersion);
    case MaxPathsPerInvoke::Id:
        return encoder.Encode<uint16_t>(CHIP_CONFIG_MAX_PATHS_PER_INVOKE);
    case ConfigurationVersion::Id:
        return ReadConfigurationVersion(configManager, encoder);
    case Reachable::Id:
        // On some platforms `true` is defined as a unsigned int and that gets
        // a ambigous TLVWriter::Put error. Hence the specialization.
        return encoder.Encode<bool>(true);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus BasicInformationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus BasicInformationCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                 AttributeValueDecoder & decoder)
{
    using namespace BasicInformation::Attributes;

    AttributePersistence persistence(mContext->attributeStorage);

    switch (request.path.mAttributeId)
    {
    case Location::Id: {
        CharSpan location;
        ReturnErrorOnFailure(decoder.Decode(location));
        VerifyOrReturnError(location.size() == kExpectedFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
        return DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size());
    }
    case NodeLabel::Id: {
        CharSpan label;
        ReturnErrorOnFailure(decoder.Decode(label));
        VerifyOrReturnError(mNodeLabel.SetContent(label), Protocols::InteractionModel::Status::ConstraintError);
        return persistence.StoreString(request.path, mNodeLabel);
    }
    case LocalConfigDisabled::Id: {
        auto deviceInfoProvider  = GetDeviceInstanceInfoProvider();
        bool localConfigDisabled = false;
        ReturnErrorOnFailure(deviceInfoProvider->GetLocalConfigDisabled(localConfigDisabled));
        auto decodeStatus = persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, localConfigDisabled);
        ReturnErrorOnFailure(deviceInfoProvider->SetLocalConfigDisabled(localConfigDisabled));
        return decodeStatus;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR BasicInformationCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{

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

    AttributeListBuilder listBuilder(builder);

    // NOTE: do NOT use kMandatoryMetadata (see comment on kMandatoryAttributes: UniqueID is special)
    return listBuilder.Append(Span(kMandatoryAttributes), Span(optionalAttributes), mEnabledOptionalAttributes);
}

CHIP_ERROR BasicInformationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    if (PlatformMgr().GetDelegate() == nullptr)
    {
        PlatformMgr().SetDelegate(this);
    }

    AttributePersistence persistence(context.attributeStorage);

    (void) persistence.LoadString({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, mNodeLabel);
    // Specialization because some platforms `#define` true/false as 1/0 and we get;
    // error: no matching function for call to
    //   'chip::app::AttributePersistence::LoadNativeEndianValue(<brace-enclosed initializer list>, bool&, int)'

    bool localConfigDisabled = false;
    (void) persistence.LoadNativeEndianValue<bool>({ kRootEndpointId, BasicInformation::Id, Attributes::LocalConfigDisabled::Id },
                                                   localConfigDisabled, false);
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->SetLocalConfigDisabled(localConfigDisabled));

    return CHIP_NO_ERROR;
}

void BasicInformationCluster::Shutdown()
{
    if (PlatformMgr().GetDelegate() == this)
    {
        PlatformMgr().SetDelegate(nullptr);
    }
    DefaultServerCluster::Shutdown();
}

void BasicInformationCluster::OnStartUp(uint32_t softwareVersion)
{
    // The StartUp event SHALL be emitted by a Node after completing a boot or reboot process
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnStartUp", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting StartUp event");

    BasicInformation::Events::StartUp::Type event{ softwareVersion };

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

void BasicInformationCluster::OnShutDown()
{
    // The ShutDown event SHOULD be emitted on a best-effort basis by a Node prior to any orderly shutdown sequence.
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnShutDown", "BasicInfo");
    ChipLogDetail(Zcl, "Emitting ShutDown event");

    BasicInformation::Events::ShutDown::Type event;

    DataModel::EventsGenerator & eventsGenerator = mContext->interactionContext.eventsGenerator;
    eventsGenerator.GenerateEvent(event, kRootEndpointId);
    eventsGenerator.ScheduleUrgentEventDeliverySync();
}

CHIP_ERROR
BasicInformationCluster::GetDeviceInstanceInfoProviderImpl(DeviceLayer::DeviceInstanceInfoProvider ** outDeviceInfoProvider)
{
    if (mDeviceInfoProvider == nullptr)
    {
        // NOTE: this should NEVER be nullptr
        *outDeviceInfoProvider = GetDeviceInstanceInfoProvider();
        VerifyOrReturnError(*outDeviceInfoProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        *outDeviceInfoProvider = mDeviceInfoProvider;
    }
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
