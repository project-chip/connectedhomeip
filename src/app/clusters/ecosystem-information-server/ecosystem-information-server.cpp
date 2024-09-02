/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "ecosystem-information-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EcosystemInformation {
namespace {

#define ZCL_ECOSYSTEM_INFORMATION_CLUSTER_REVISION (1u)
#define ZCL_ECOSYSTEM_INFORMATION_FEATURE_MAP (0u)

constexpr size_t kDeviceNameMaxSize             = 64;
constexpr size_t kUniqueLocationIdMaxSize       = 64;
constexpr size_t kUniqueLocationIdsListMaxSize  = 64;
constexpr size_t kLocationDescriptorNameMaxSize = 128;

class AttrAccess : public AttributeAccessInterface
{
public:
    // Register for the EcosystemInformationCluster on all endpoints.
    AttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::EcosystemInformation::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

CHIP_ERROR AttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::EcosystemInformation::Id);
    return EcosystemInformationServer::Instance().ReadAttribute(aPath, aEncoder);
}

// WARNING: caller is expected to use the returned LocationDescriptorStruct::Type immediately. Caller must
// be certain that the provided aLocationDescriptor has not been destroyed, prior to using the return
// struct to encode.
// TODO(#33223) To improve safety we could make GetEncodableLocationDescriptorStruct a private
// memeber method where we explicitly delete member method for the parameter that matches
// (LocationDescriptorStruct && aLocationDescriptor).
Globals::Structs::LocationDescriptorStruct::Type
GetEncodableLocationDescriptorStruct(const LocationDescriptorStruct & aLocationDescriptor)
{
    Globals::Structs::LocationDescriptorStruct::Type locationDescriptor;
    // This would imply data is either not properly validated before being
    // stored here or corruption has occurred.
    VerifyOrDie(!aLocationDescriptor.mLocationName.empty());
    locationDescriptor.locationName = CharSpan(aLocationDescriptor.mLocationName.c_str(), aLocationDescriptor.mLocationName.size());

    if (aLocationDescriptor.mFloorNumber.has_value())
    {
        locationDescriptor.floorNumber.SetNonNull(aLocationDescriptor.mFloorNumber.value());
    }
    else
    {
        locationDescriptor.floorNumber.SetNull();
    }

    if (aLocationDescriptor.mAreaType.has_value())
    {
        locationDescriptor.areaType.SetNonNull(aLocationDescriptor.mAreaType.value());
    }
    else
    {
        locationDescriptor.areaType.SetNull();
    }
    return locationDescriptor;
}

} // namespace

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetDeviceName(std::string aDeviceName,
                                                                               uint64_t aDeviceNameLastEditEpochUs)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mDeviceName                = std::move(aDeviceName);
    mDeviceNameLastEditEpochUs = aDeviceNameLastEditEpochUs;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetBrigedEndpoint(EndpointId aBridgedEndpoint)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mBridgedEndpoint = aBridgedEndpoint;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetOriginalEndpoint(EndpointId aOriginalEndpoint)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mOriginalEndpoint = aOriginalEndpoint;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::AddDeviceType(Structs::DeviceTypeStruct::Type aDeviceType)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mDeviceTypes.push_back(std::move(aDeviceType));
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::AddUniqueLocationId(std::string aUniqueLocationId,
                                                                                     uint64_t aUniqueLocationIdsLastEditEpochUs)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mUniqueLocationIds.push_back(std::move(aUniqueLocationId));
    mUniqueLocationIdsLastEditEpochUs = aUniqueLocationIdsLastEditEpochUs;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetFabricIndex(FabricIndex aFabricIndex)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mFabricIndex = aFabricIndex;
    return *this;
}

std::unique_ptr<EcosystemDeviceStruct> EcosystemDeviceStruct::Builder::Build()
{
    VerifyOrReturnValue(!mIsAlreadyBuilt, nullptr, ChipLogError(Zcl, "Build() already called"));
    VerifyOrReturnValue(mDeviceName.size() <= kDeviceNameMaxSize, nullptr, ChipLogError(Zcl, "Device name too large"));
    VerifyOrReturnValue(mOriginalEndpoint != kInvalidEndpointId, nullptr, ChipLogError(Zcl, "Invalid original endpoint"));
    VerifyOrReturnValue(!mDeviceTypes.empty(), nullptr, ChipLogError(Zcl, "No device types added"));
    VerifyOrReturnValue(mUniqueLocationIds.size() <= kUniqueLocationIdsListMaxSize, nullptr,
                        ChipLogError(Zcl, "Too many location ids"));
    VerifyOrReturnValue(mFabricIndex >= kMinValidFabricIndex, nullptr, ChipLogError(Zcl, "Fabric index is invalid"));
    VerifyOrReturnValue(mFabricIndex <= kMaxValidFabricIndex, nullptr, ChipLogError(Zcl, "Fabric index is invalid"));

    for (auto & locationId : mUniqueLocationIds)
    {
        VerifyOrReturnValue(locationId.size() <= kUniqueLocationIdMaxSize, nullptr, ChipLogError(Zcl, "Location id too long"));
    }

    // std::make_unique does not have access to private constructor we workaround with using new
    std::unique_ptr<EcosystemDeviceStruct> ret{ new EcosystemDeviceStruct(
        std::move(mDeviceName), mDeviceNameLastEditEpochUs, mBridgedEndpoint, mOriginalEndpoint, std::move(mDeviceTypes),
        std::move(mUniqueLocationIds), mUniqueLocationIdsLastEditEpochUs, mFabricIndex) };
    mIsAlreadyBuilt = true;
    return ret;
}

CHIP_ERROR EcosystemDeviceStruct::Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder)
{
    Structs::EcosystemDeviceStruct::Type deviceStruct;
    if (!mDeviceName.empty())
    {
        deviceStruct.deviceName.SetValue(CharSpan(mDeviceName.c_str(), mDeviceName.size()));
        // When there is a device name we also include mDeviceNameLastEditEpochUs
        deviceStruct.deviceNameLastEdit.SetValue(mDeviceNameLastEditEpochUs);
    }
    deviceStruct.bridgedEndpoint  = mBridgedEndpoint;
    deviceStruct.originalEndpoint = mOriginalEndpoint;
    deviceStruct.deviceTypes = DataModel::List<const Structs::DeviceTypeStruct::Type>(mDeviceTypes.data(), mDeviceTypes.size());

    std::vector<CharSpan> locationIds;
    locationIds.reserve(mUniqueLocationIds.size());
    for (auto & id : mUniqueLocationIds)
    {
        locationIds.push_back(CharSpan(id.c_str(), id.size()));
    }
    deviceStruct.uniqueLocationIDs = DataModel::List<CharSpan>(locationIds.data(), locationIds.size());

    deviceStruct.uniqueLocationIDsLastEdit = mUniqueLocationIdsLastEditEpochUs;
    deviceStruct.SetFabricIndex(mFabricIndex);
    return aEncoder.Encode(deviceStruct);
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetLocationName(std::string aLocationName)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mLocationDescriptor.mLocationName = std::move(aLocationName);
    return *this;
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetFloorNumber(std::optional<int16_t> aFloorNumber)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mLocationDescriptor.mFloorNumber = aFloorNumber;
    return *this;
}

EcosystemLocationStruct::Builder &
EcosystemLocationStruct::Builder::SetAreaTypeTag(std::optional<Globals::AreaTypeTag> aAreaTypeTag)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mLocationDescriptor.mAreaType = aAreaTypeTag;
    return *this;
}

EcosystemLocationStruct::Builder &
EcosystemLocationStruct::Builder::SetLocationDescriptorLastEdit(uint64_t aLocationDescriptorLastEditEpochUs)
{
    VerifyOrDie(!mIsAlreadyBuilt);
    mLocationDescriptorLastEditEpochUs = aLocationDescriptorLastEditEpochUs;
    return *this;
}

std::unique_ptr<EcosystemLocationStruct> EcosystemLocationStruct::Builder::Build()
{
    VerifyOrReturnValue(!mIsAlreadyBuilt, nullptr, ChipLogError(Zcl, "Build() already called"));
    VerifyOrReturnValue(!mLocationDescriptor.mLocationName.empty(), nullptr, ChipLogError(Zcl, "Must Provided Location Name"));
    VerifyOrReturnValue(mLocationDescriptor.mLocationName.size() <= kLocationDescriptorNameMaxSize, nullptr,
                        ChipLogError(Zcl, "Must Location Name must be less than 64 bytes"));

    // std::make_unique does not have access to private constructor we workaround with using new
    std::unique_ptr<EcosystemLocationStruct> ret{ new EcosystemLocationStruct(std::move(mLocationDescriptor),
                                                                              mLocationDescriptorLastEditEpochUs) };
    mIsAlreadyBuilt = true;
    return ret;
}

CHIP_ERROR EcosystemLocationStruct::Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder,
                                           const std::string & aUniqueLocationId, const FabricIndex & aFabricIndex)
{
    Structs::EcosystemLocationStruct::Type locationStruct;
    locationStruct.uniqueLocationID           = CharSpan(aUniqueLocationId.c_str(), aUniqueLocationId.size());
    locationStruct.locationDescriptor         = GetEncodableLocationDescriptorStruct(mLocationDescriptor);
    locationStruct.locationDescriptorLastEdit = mLocationDescriptorLastEditEpochUs;
    locationStruct.SetFabricIndex(aFabricIndex);
    return aEncoder.Encode(locationStruct);
}

EcosystemInformationServer EcosystemInformationServer::mInstance;

EcosystemInformationServer & EcosystemInformationServer::Instance()
{
    return mInstance;
}

CHIP_ERROR EcosystemInformationServer::AddEcosystemInformationClusterToEndpoint(EndpointId aEndpoint)
{
    VerifyOrReturnError((aEndpoint != kRootEndpointId && aEndpoint != kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);
    auto it = mDevicesMap.find(aEndpoint);
    // We expect that the device has not been previously added.
    VerifyOrReturnError((it == mDevicesMap.end()), CHIP_ERROR_INCORRECT_STATE);
    // This create an empty DeviceInfo in mDevicesMap.
    mDevicesMap[aEndpoint] = DeviceInfo();
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::AddDeviceInfo(EndpointId aEndpoint, std::unique_ptr<EcosystemDeviceStruct> aDevice)
{
    VerifyOrReturnError(aDevice, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((aEndpoint != kRootEndpointId && aEndpoint != kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);

    auto & deviceInfo = mDevicesMap[aEndpoint];
    deviceInfo.mDeviceDirectory.push_back(std::move(aDevice));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::AddLocationInfo(EndpointId aEndpoint, const std::string & aLocationId,
                                                       FabricIndex aFabricIndex, std::unique_ptr<EcosystemLocationStruct> aLocation)
{
    VerifyOrReturnError(aLocation, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((aEndpoint != kRootEndpointId && aEndpoint != kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!aLocationId.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aFabricIndex >= kMinValidFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aFabricIndex <= kMaxValidFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);

    auto & deviceInfo        = mDevicesMap[aEndpoint];
    EcosystemLocationKey key = { .mUniqueLocationId = aLocationId, .mFabricIndex = aFabricIndex };
    VerifyOrReturnError((deviceInfo.mLocationDirectory.find(key) == deviceInfo.mLocationDirectory.end()),
                        CHIP_ERROR_INVALID_ARGUMENT);
    deviceInfo.mLocationDirectory[key] = std::move(aLocation);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::ReadAttribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::DeviceDirectory::Id:
        return EncodeDeviceDirectoryAttribute(aPath.mEndpointId, aEncoder);
    case Attributes::LocationDirectory::Id:
        return EncodeLocationStructAttribute(aPath.mEndpointId, aEncoder);
    case Attributes::ClusterRevision::Id: {
        uint16_t rev = ZCL_ECOSYSTEM_INFORMATION_CLUSTER_REVISION;
        return aEncoder.Encode(rev);
    }
    case Attributes::FeatureMap::Id: {
        uint32_t featureMap = ZCL_ECOSYSTEM_INFORMATION_FEATURE_MAP;
        return aEncoder.Encode(featureMap);
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::EncodeDeviceDirectoryAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder)
{

    auto it = mDevicesMap.find(aEndpoint);
    if (it == mDevicesMap.end())
    {
        // We are always going to be given a valid endpoint. If the endpoint
        // doesn't exist in our map that indicate that the cluster was not
        // added on this endpoint, hence UnsupportedCluster.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    }

    auto & deviceInfo = it->second;
    if (deviceInfo.mDeviceDirectory.empty())
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & device : deviceInfo.mDeviceDirectory)
        {
            ReturnErrorOnFailure(device->Encode(encoder));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR EcosystemInformationServer::EncodeLocationStructAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder)
{
    auto it = mDevicesMap.find(aEndpoint);
    if (it == mDevicesMap.end())
    {
        // We are always going to be given a valid endpoint. If the endpoint
        // doesn't exist in our map that indicate that the cluster was not
        // added on this endpoint, hence UnsupportedCluster.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    }

    auto & deviceInfo = it->second;
    if (deviceInfo.mLocationDirectory.empty())
    {
        return aEncoder.EncodeEmptyList();
    }

    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & [key, device] : deviceInfo.mLocationDirectory)
        {
            ReturnErrorOnFailure(device->Encode(encoder, key.mUniqueLocationId, key.mFabricIndex));
        }
        return CHIP_NO_ERROR;
    });
    return CHIP_NO_ERROR;
}

} // namespace EcosystemInformation
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

chip::app::Clusters::EcosystemInformation::AttrAccess gAttrAccess;

void MatterEcosystemInformationPluginServerInitCallback()
{
    chip::app::AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
