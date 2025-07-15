/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>
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

using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace {

// fixed size for location strings
static constexpr size_t kFixedLocationLength = 2;

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

/// a storage buffer that is provided by callers to store string data.
/// It is sized to support reading any of the underlying data items in the
/// logic class.
struct StringReadBuffer
{
    static constexpr size_t kMaxStringLength = std::max<size_t>({
        kFixedLocationLength,                                               //
        DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength, //
        DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength,     //
        DeviceLayer::ConfigurationManager::kMaxPartNumberLength,            //
        DeviceLayer::ConfigurationManager::kMaxProductLabelLength,          //
        DeviceLayer::ConfigurationManager::kMaxProductNameLength,           //
        DeviceLayer::ConfigurationManager::kMaxProductURLLength,            //
        DeviceLayer::ConfigurationManager::kMaxSerialNumberLength,          //
        DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength, //
        DeviceLayer::ConfigurationManager::kMaxUniqueIDLength,              //
        DeviceLayer::ConfigurationManager::kMaxVendorNameLength,            //
    });
    // allow for full string length + a null terminator.
    char buffer[kMaxStringLength + 1];
};

static_assert(sizeof(bool) == 1, "I/O assumption for backwards compatibility");

void LogIfReadError(AttributeId attributeId, CHIP_ERROR err)
{
    VerifyOrReturn(err != CHIP_NO_ERROR);
    VerifyOrReturn(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    ChipLogError(Zcl, "BasicInformation: failed to load attribute " ChipLogFormatMEI ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueMEI(attributeId), err.Format());
}

CHIP_ERROR ClearNullTerminatedStringWhenUnimplemented(CHIP_ERROR status, char * strBuf)
{
    if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        *strBuf = '\0';
        return CHIP_NO_ERROR;
    }

    return status;
}

DataModel::ActionReturnStatus ReadLocation(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= kFixedLocationLength + 1);

    size_t codeLen = 0;
    CHIP_ERROR err = ConfigurationMgr().GetCountryCode(buffer.buffer, sizeof(buffer.buffer), codeLen);
    if ((err != CHIP_NO_ERROR) || (codeLen != kFixedLocationLength))
    {
        static_assert(kFixedLocationLength == 2); // we write a string of size 2
        strcpy(buffer.buffer, "XX");
        codeLen = kFixedLocationLength;
    }
    return encoder.Encode(CharSpan(buffer.buffer, codeLen));
}

using DeviceInfoStringGetFn = CHIP_ERROR (DeviceInstanceInfoProvider::*)(char * buf, size_t buffSize);

DataModel::ActionReturnStatus ReadVendorName(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= ConfigurationManager::kMaxVendorNameLength + 1);
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetVendorName(buffer.buffer, sizeof(buffer.buffer)));
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadProductName(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= ConfigurationManager::kMaxProductNameLength + 1,
                  "StringReadBuffer for product name is too small");
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetProductName(buffer.buffer, sizeof(buffer.buffer)));
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadHardwareVersionString(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= ConfigurationManager::kMaxHardwareVersionStringLength + 1,
                  "StringReadBuffer for hardware version string is too small");
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetHardwareVersionString(buffer.buffer, sizeof(buffer.buffer)));
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadSoftwareVersionString(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= ConfigurationManager::kMaxSoftwareVersionStringLength + 1,
                  "StringReadBuffer for software version string is too small");
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetSoftwareVersionString(buffer.buffer, sizeof(buffer.buffer)));
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadManufacturingDate(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxManufacturingDateLength + 1,
                  "StringReadBuffer for manufacturing date is too small");
    uint16_t manufacturingYear;
    uint8_t manufacturingMonth;
    uint8_t manufacturingDayOfMonth;
    CHIP_ERROR status =
        GetDeviceInstanceInfoProvider()->GetManufacturingDate(manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);

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
    snprintf(buffer.buffer, sizeof(buffer.buffer), "%04u%02u%02u", manufacturingYear, manufacturingMonth, manufacturingDayOfMonth);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadPartNumber(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxPartNumberLength + 1,
                  "StringReadBuffer for part number is too small");
    CHIP_ERROR status = GetDeviceInstanceInfoProvider()->GetPartNumber(buffer.buffer, sizeof(buffer.buffer));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, buffer.buffer);
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadProductURL(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxProductURLLength + 1,
                  "StringReadBuffer for product URL is too small");
    CHIP_ERROR status = GetDeviceInstanceInfoProvider()->GetProductURL(buffer.buffer, sizeof(buffer.buffer));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, buffer.buffer);
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadProductLabel(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxProductLabelLength + 1,
                  "StringReadBuffer for product label is too small");
    CHIP_ERROR status = GetDeviceInstanceInfoProvider()->GetProductLabel(buffer.buffer, sizeof(buffer.buffer));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, buffer.buffer);
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadSerialNumber(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1,
                  "StringReadBuffer for serial number is too small");
    CHIP_ERROR status = GetDeviceInstanceInfoProvider()->GetSerialNumber(buffer.buffer, sizeof(buffer.buffer));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, buffer.buffer);
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadUniqueID(AttributeValueEncoder & encoder, StringReadBuffer & buffer)
{
    static_assert(sizeof(buffer.buffer) >= DeviceLayer::ConfigurationManager::kMaxUniqueIDLength + 1,
                  "StringReadBuffer for unique ID is too small");
    CHIP_ERROR status = ConfigurationMgr().GetUniqueId(buffer.buffer, sizeof(buffer.buffer));
    status            = ClearNullTerminatedStringWhenUnimplemented(status, buffer.buffer);
    ReturnErrorOnFailure(status);
    return encoder.Encode(CharSpan::fromCharString(buffer.buffer));
}

DataModel::ActionReturnStatus ReadProductAppearance(AttributeValueEncoder & encoder)
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

} // namespace

BasicInformationCluster & BasicInformationCluster::Instance()
{
    static BasicInformationCluster instance;
    return instance;
}

DataModel::ActionReturnStatus BasicInformationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    using namespace BasicInformation::Attributes;

    StringReadBuffer readBuffer;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id: {
        uint32_t revision = BasicInformation::kRevision;
        return encoder.Encode(revision);
    }
    case DataModelRevision::Id:
        return encoder.Encode<uint32_t>(Revision::kDataModelRevision);
    case Location::Id:
        return ReadLocation(encoder, readBuffer);
    case VendorName::Id:
        return ReadVendorName(encoder, readBuffer);
    case VendorID::Id: {
        uint16_t vendorId;
        ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
        return encoder.Encode<uint32_t>(vendorId);
    }
    case ProductName::Id:
        return ReadProductName(encoder, readBuffer);
    case ProductID::Id: {
        uint16_t productId;
        ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetProductId(productId));
        return encoder.Encode<uint32_t>(productId);
    }
    case HardwareVersion::Id: {
        uint16_t hardwareVersion;
        ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetHardwareVersion(hardwareVersion));
        return encoder.Encode<uint32_t>(hardwareVersion);
    }
    case HardwareVersionString::Id:
        return ReadHardwareVersionString(encoder, readBuffer);
    case SoftwareVersion::Id: {
        uint32_t softwareVersion;
        ReturnErrorOnFailure(ConfigurationMgr().GetSoftwareVersion(softwareVersion));
        return encoder.Encode(softwareVersion);
    }
    case SoftwareVersionString::Id:
        return ReadSoftwareVersionString(encoder, readBuffer);
    case ManufacturingDate::Id:
        return ReadManufacturingDate(encoder, readBuffer);
    case PartNumber::Id:
        return ReadPartNumber(encoder, readBuffer);
    case ProductURL::Id:
        return ReadProductURL(encoder, readBuffer);
    case ProductLabel::Id:
        return ReadProductLabel(encoder, readBuffer);
    case SerialNumber::Id:
        return ReadSerialNumber(encoder, readBuffer);
    case UniqueID::Id:
        return ReadUniqueID(encoder, readBuffer);
    case CapabilityMinima::Id: {
        BasicInformation::Structs::CapabilityMinimaStruct::Type data;

        constexpr uint16_t kMinCaseSessionsPerFabricMandatedBySpec = 3;

        data.caseSessionsPerFabric  = kMinCaseSessionsPerFabricMandatedBySpec;
        data.subscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric();

        return encoder.Encode(data);
    }
    case ProductAppearance::Id:
        return ReadProductAppearance(encoder);
    case SpecificationVersion::Id: {
        const uint32_t version = Revision::kSpecificationVersion;
        return encoder.Encode(version);
    }
    case MaxPathsPerInvoke::Id:
        return encoder.Encode<uint32_t>(CHIP_CONFIG_MAX_PATHS_PER_INVOKE);
    case ConfigurationVersion::Id: {
        uint32_t configVersion;
        ReturnErrorOnFailure(ConfigurationMgr().GetConfigurationVersion(configVersion));
        return encoder.Encode(configVersion);
    }
    case NodeLabel::Id:
        return encoder.Encode(Storage::ShortPascalString::ContentOf(mNodeLabelBuffer));
    case LocalConfigDisabled::Id:
        return encoder.Encode(mLocalConfigDisabled);
    case Reachable::Id:
        // NOTE: we are always reachable (this is the same node)
        return encoder.Encode<bool>(true);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus BasicInformationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                      AttributeValueDecoder & decoder)
{
    using namespace BasicInformation::Attributes;

    switch (request.path.mAttributeId)
    {
    case Location::Id: {
        CharSpan location;
        ReturnErrorOnFailure(decoder.Decode(location));
        VerifyOrReturnError(location.size() == kFixedLocationLength, Protocols::InteractionModel::Status::ConstraintError);
        return NotifyAttributeChangedIfSuccess(Location::Id,
                                               DeviceLayer::ConfigurationMgr().StoreCountryCode(location.data(), location.size()));
    }
    case NodeLabel::Id: {
        CharSpan label;
        ReturnErrorOnFailure(decoder.Decode(label));

        Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
        VerifyOrReturnError(labelBuffer.SetValue(label), Protocols::InteractionModel::Status::ConstraintError);

        return NotifyAttributeChangedIfSuccess(
            NodeLabel::Id,
            mContext->attributeStorage->WriteValue({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id },
                                                   labelBuffer.RawValidData()));
    }
    case LocalConfigDisabled::Id: {
        ReturnErrorOnFailure(decoder.Decode(mLocalConfigDisabled));
        return NotifyAttributeChangedIfSuccess(
            LocalConfigDisabled::Id,
            mContext->attributeStorage->WriteValue(
                { kRootEndpointId, BasicInformation::Id, Attributes::LocalConfigDisabled::Id },
                { reinterpret_cast<const uint8_t *>(&mLocalConfigDisabled), sizeof(mLocalConfigDisabled) }));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR BasicInformationCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryAttributes));

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(9));

    if (!mEnabledOptionalAttributes.Has(OptionalBasicInformationAttributes::kDisableMandatoryUniqueIDOnPurpose))
    {
        ReturnErrorOnFailure(builder.Append(UniqueID::kMetadataEntry));
    }

#define OPTIONAL_ATTR_SET(name)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mEnabledOptionalAttributes.Has(OptionalBasicInformationAttributes::k##name))                                           \
        {                                                                                                                          \
            ReturnErrorOnFailure(builder.Append(name::kMetadataEntry));                                                            \
        }                                                                                                                          \
    } while (false)

    OPTIONAL_ATTR_SET(ManufacturingDate);
    OPTIONAL_ATTR_SET(PartNumber);
    OPTIONAL_ATTR_SET(ProductURL);
    OPTIONAL_ATTR_SET(ProductLabel);
    OPTIONAL_ATTR_SET(SerialNumber);
    OPTIONAL_ATTR_SET(LocalConfigDisabled);
    OPTIONAL_ATTR_SET(Reachable);
    OPTIONAL_ATTR_SET(ProductAppearance);

    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR BasicInformationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    if (PlatformMgr().GetDelegate() == nullptr)
    {
        PlatformMgr().SetDelegate(this);
    }
    {
        Storage::ShortPascalString labelBuffer(mNodeLabelBuffer);
        MutableByteSpan labelSpan = labelBuffer.RawBuffer();

        LogIfReadError(
            Attributes::NodeLabel::Id,
            context.attributeStorage->ReadValue({ kRootEndpointId, BasicInformation::Id, Attributes::NodeLabel::Id }, labelSpan));

        if (!Storage::ShortPascalString::IsValid({ mNodeLabelBuffer, labelSpan.size() }))
        {
            // invalid value
            labelBuffer.SetValue(""_span);
        }
    }

    {
        MutableByteSpan localConfigBytes(reinterpret_cast<uint8_t *>(&mLocalConfigDisabled), sizeof(mLocalConfigDisabled));
        LogIfReadError(Attributes::LocalConfigDisabled::Id,
                       context.attributeStorage->ReadValue(
                           { kRootEndpointId, BasicInformation::Id, Attributes::LocalConfigDisabled::Id }, localConfigBytes));

        if (localConfigBytes.size() == 0)
        {
            // invalid value
            mLocalConfigDisabled = false;
        }
    }
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
    BasicInformation::Events::StartUp::Type event{ softwareVersion };
    mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

void BasicInformationCluster::OnShutDown()
{
    // The ShutDown event SHOULD be emitted on a best-effort basis by a Node prior to any orderly shutdown sequence.
    VerifyOrReturn(mContext != nullptr);

    MATTER_TRACE_INSTANT("OnShutDown", "BasicInfo");
    BasicInformation::Events::ShutDown::Type event;
    mContext->interactionContext->eventsGenerator->GenerateEvent(event, kRootEndpointId);
}

} // namespace Clusters
} // namespace app
} // namespace chip
