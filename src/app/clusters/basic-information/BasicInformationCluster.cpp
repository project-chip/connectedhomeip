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
#include <app/clusters/basic-information/BasicInformationLogic.h>
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
    UniqueID::kMetadataEntry,
    CapabilityMinima::kMetadataEntry,
    SpecificationVersion::kMetadataEntry,
    MaxPathsPerInvoke::kMetadataEntry,
    ConfigurationVersion::kMetadataEntry,

};

/// a storage buffer that is provideded by callers to store string data.
/// It is sized to support reading any of the underlying data items in the
/// logic class.
struct StringReadBuffer
{
    static constexpr size_t kMaxStringLength = std::max<size_t>({
        BasicInformationLogic::kFixedLocationLength,                        //
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
    static_assert(sizeof(buffer.buffer) >= BasicInformationLogic::kFixedLocationLength + 1);

    size_t codeLen = 0;
    CHIP_ERROR err = ConfigurationMgr().GetCountryCode(buffer.buffer, sizeof(buffer.buffer), codeLen);
    if ((err != CHIP_NO_ERROR) || (codeLen != BasicInformationLogic::kFixedLocationLength))
    {
        static_assert(BasicInformationLogic::kFixedLocationLength == 2); // we write a string of size 2
        strcpy(buffer.buffer, "XX");
        codeLen = BasicInformationLogic::kFixedLocationLength;
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

DataModel::ActionReturnStatus BasicInformationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                     AttributeValueEncoder & encoder)
{
    using namespace BasicInformation::Attributes;

    StringReadBuffer readBuffer;

    auto & logic = BasicInformationLogic::Instance();

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
        return encoder.Encode(logic.GetNodeLabel());
    case LocalConfigDisabled::Id:
        return encoder.Encode(logic.GetLocalConfigDisabled());
    case Reachable::Id:
        return encoder.Encode(logic.GetReachable());
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
        return NotifyAttributeChangedIfSuccess(Location::Id, BasicInformationLogic::Instance().SetLocation(location));
    }
    case NodeLabel::Id: {
        CharSpan label;
        ReturnErrorOnFailure(decoder.Decode(label));
        return NotifyAttributeChangedIfSuccess(NodeLabel::Id, BasicInformationLogic::Instance().SetNodeLabel(label));
    }
    case LocalConfigDisabled::Id: {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        BasicInformationLogic::Instance().SetLocalConfigDisabled(value);
        NotifyAttributeChanged(request.path.mAttributeId);
        return CHIP_NO_ERROR;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR BasicInformationCluster::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryAttributes));

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(8));

#define OPTIONAL_ATTR_SET(name)                                                                                                    \
    if (mEnabledOptionalAttributes.Has(OptionalBasicInformationAttributes::k##name))                                               \
    {                                                                                                                              \
        ReturnErrorOnFailure(builder.Append(name::kMetadataEntry));                                                                \
    }                                                                                                                              \
    (void) 0

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
    if (PlatformMgr().GetDelegate() == nullptr)
    {
        PlatformMgr().SetDelegate(this);
    }
    return DefaultServerCluster::Startup(context);
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
