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

class AttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    AttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::EcosystemInformation::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
};

CHIP_ERROR AttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::EcosystemInformation::Id);
    switch (aPath.mAttributeId)
    {
    case Attributes::RemovedOn::Id:
        return EcosystemInformationServer::Instance().EncodeRemovedOnAttribute(aPath.mEndpointId, aEncoder);
    case Attributes::DeviceDirectory ::Id:
        return EcosystemInformationServer::Instance().EncodeDeviceDirectoryAttribute(aPath.mEndpointId, aEncoder);
    case Attributes::LocationDirectory ::Id:
        return EcosystemInformationServer::Instance().EncodeLocationStructAttribute(aPath.mEndpointId, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

Structs::HomeLocationStruct::Type GetEncodableHomeLocationStruct(const HomeLocationStruct & aHomeLocation)
{
    Structs::HomeLocationStruct::Type homeLocationStruct;
    // This would imply data is either not properly validated before being
    // stored here or corruption has occurred.
    // If VerifyOrDie is too harse we can return optional
    VerifyOrDie(!aHomeLocation.mLocationName.empty());
    homeLocationStruct.locationName = CharSpan(aHomeLocation.mLocationName.c_str(), aHomeLocation.mLocationName.size());

    if (aHomeLocation.mFloorNumber.has_value())
    {
        homeLocationStruct.floorNumber.SetNonNull(aHomeLocation.mFloorNumber.value());
    }
    else
    {
        homeLocationStruct.floorNumber.SetNull();
    }

    if (aHomeLocation.mAreaType.has_value())
    {
        homeLocationStruct.areaType.SetNonNull(aHomeLocation.mAreaType.value());
    }
    else
    {
        homeLocationStruct.areaType.SetNull();
    }
    return homeLocationStruct;
}

} // namespace

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetDeviceName(std::string aDeviceName,
                                                                               uint64_t aDeviceNameLastEditEpochUs)
{
    VerifyOrDie(!mIsBuilt);
    mDeviceName                = std::move(aDeviceName);
    mDeviceNameLastEditEpochUs = aDeviceNameLastEditEpochUs;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetBrigedEndpoint(EndpointId aBridgedEndpoint)
{
    VerifyOrDie(!mIsBuilt);
    mBridgedEndpoint = aBridgedEndpoint;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::SetOriginalEndpoint(EndpointId aOriginalEndpoint)
{
    VerifyOrDie(!mIsBuilt);
    mOriginalEndpoint = aOriginalEndpoint;
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::AddDeviceType(Structs::DeviceTypeStruct::Type aDeviceType)
{
    VerifyOrDie(!mIsBuilt);
    mDeviceTypes.push_back(std::move(aDeviceType));
    return *this;
}

EcosystemDeviceStruct::Builder & EcosystemDeviceStruct::Builder::AddUniqueLocationId(std::string aUniqueLocationId,
                                                                                     uint64_t aUniqueLocationIdsLastEditEpochUs)
{
    VerifyOrDie(!mIsBuilt);
    mUniqueLocationIds.push_back(std::move(aUniqueLocationId));
    mUniqueLocationIdsLastEditEpochUs = aUniqueLocationIdsLastEditEpochUs;
    return *this;
}

std::unique_ptr<EcosystemDeviceStruct> EcosystemDeviceStruct::Builder::Build()
{
    if (mIsBuilt || mDeviceName.empty() || mOriginalEndpoint == kInvalidEndpointId || mDeviceTypes.empty() ||
        mUniqueLocationIds.size() >= 64)
    {
        return nullptr;
    }
    for (auto & locationId : mUniqueLocationIds)
    {
        if (locationId.size() >= 64)
        {
            return nullptr;
        }
    }
    // std::make_unique does not have access to private constructor we workaround with using new
    std::unique_ptr<EcosystemDeviceStruct> ret{ new EcosystemDeviceStruct(
        std::move(mDeviceName), mDeviceNameLastEditEpochUs, mBridgedEndpoint, mOriginalEndpoint, std::move(mDeviceTypes),
        std::move(mUniqueLocationIds), mUniqueLocationIdsLastEditEpochUs) };
    mIsBuilt = true;
    return ret;
}

CHIP_ERROR EcosystemDeviceStruct::Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder, const FabricIndex & aFabricIndex)
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

    // TODO(#33223) this is a hack, use mFabricIndex when it exists.
    // Additionally check at top should prevent doing all this work if we know
    // it won't be encoded due to fabric scoping.
    deviceStruct.SetFabricIndex(aFabricIndex);
    return aEncoder.Encode(deviceStruct);
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetLocationName(std::string aLocationName)
{
    VerifyOrDie(!mIsBuilt);
    mHomeLocation.mLocationName = std::move(aLocationName);
    return *this;
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetFloorNumber(std::optional<int16_t> aFloorNumber)
{
    VerifyOrDie(!mIsBuilt);
    mHomeLocation.mFloorNumber = aFloorNumber;
    return *this;
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetAreaTypeTag(std::optional<AreaTypeTag> aAreaTypeTag)
{
    VerifyOrDie(!mIsBuilt);
    mHomeLocation.mAreaType = aAreaTypeTag;
    return *this;
}

EcosystemLocationStruct::Builder & EcosystemLocationStruct::Builder::SetHomeLocationLastEdit(uint64_t aHomeLocationLastEditEpochUs)
{
    VerifyOrDie(!mIsBuilt);
    mHomeLocationLastEditEpochUs = aHomeLocationLastEditEpochUs;
    return *this;
}

std::unique_ptr<EcosystemLocationStruct> EcosystemLocationStruct::Builder::Build()
{
    if (mIsBuilt || mHomeLocation.mLocationName.empty() || mHomeLocation.mLocationName.size() >= 128)
    {
        return nullptr;
    }

    // std::make_unique does not have access to private constructor we workaround with using new
    std::unique_ptr<EcosystemLocationStruct> ret{ new EcosystemLocationStruct(std::move(mHomeLocation),
                                                                              mHomeLocationLastEditEpochUs) };
    mIsBuilt = true;
    return ret;
}

CHIP_ERROR EcosystemLocationStruct::Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder,
                                           const std::string & aUniqueLocationId, const FabricIndex & aFabricIndex)
{
    Structs::EcosystemLocationStruct::Type locationStruct;
    VerifyOrDie(!aUniqueLocationId.empty());
    locationStruct.uniqueLocationID     = CharSpan(aUniqueLocationId.c_str(), aUniqueLocationId.size());
    locationStruct.homeLocation         = GetEncodableHomeLocationStruct(mHomeLocation);
    locationStruct.homeLocationLastEdit = mHomeLocationLastEditEpochUs;

    // TODO(#33223) this is a hack, use mFabricIndex when it exists.
    // Additionally check at top should prevent doing all this work if we know
    // it won't be encoded due to fabric scoping.
    locationStruct.SetFabricIndex(aFabricIndex);
    return aEncoder.Encode(locationStruct);
}

EcosystemInformationServer EcosystemInformationServer::mInstance;

EcosystemInformationServer & EcosystemInformationServer::Instance()
{
    return mInstance;
}

CHIP_ERROR EcosystemInformationServer::AddDeviceInfo(EndpointId aEndpoint, uint64_t aUniqueId,
                                                     std::unique_ptr<EcosystemDeviceStruct> aDevice)
{
    VerifyOrReturnError(aDevice, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((aEndpoint != kRootEndpointId && aEndpoint != kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);

    auto & deviceInfo = mDevicesMap[aEndpoint];
    VerifyOrReturnError((deviceInfo.mDeviceDirectory.find(aUniqueId) == deviceInfo.mDeviceDirectory.end()),
                        CHIP_ERROR_INVALID_ARGUMENT);
    deviceInfo.mDeviceDirectory[aUniqueId] = std::move(aDevice);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::AddLocationInfo(EndpointId aEndpoint, const std::string & aLocationId,
                                                       std::unique_ptr<EcosystemLocationStruct> aLocation)
{
    VerifyOrReturnError(aLocation, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((aEndpoint != kRootEndpointId && aEndpoint != kInvalidEndpointId), CHIP_ERROR_INVALID_ARGUMENT);

    auto & deviceInfo = mDevicesMap[aEndpoint];
    VerifyOrReturnError((deviceInfo.mLocationDirectory.find(aLocationId) == deviceInfo.mLocationDirectory.end()),
                        CHIP_ERROR_INVALID_ARGUMENT);
    deviceInfo.mLocationDirectory[aLocationId] = std::move(aLocation);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::RemoveDevice(EndpointId aEndpoint, uint64_t aEpochUs)
{
    VerifyOrReturnError((mDevicesMap.find(aEndpoint) != mDevicesMap.end()), CHIP_ERROR_INVALID_ARGUMENT);
    auto & deviceInfo = mDevicesMap[aEndpoint];
    deviceInfo.mRemovedOn.SetValue(aEpochUs);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::EncodeRemovedOnAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder)
{
    if (mDevicesMap.find(aEndpoint) == mDevicesMap.end())
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    }

    auto & deviceInfo = mDevicesMap[aEndpoint];
    if (!deviceInfo.mRemovedOn.HasValue())
    {
        aEncoder.EncodeNull();
        return CHIP_NO_ERROR;
    }

    aEncoder.Encode(deviceInfo.mRemovedOn.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcosystemInformationServer::EncodeDeviceDirectoryAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder)
{
    if (mDevicesMap.find(aEndpoint) == mDevicesMap.end())
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    }

    auto & deviceInfo = mDevicesMap[aEndpoint];
    if (deviceInfo.mDeviceDirectory.empty() || deviceInfo.mRemovedOn.HasValue())
    {
        return aEncoder.EncodeEmptyList();
    }

    FabricIndex fabricIndex = aEncoder.AccessingFabricIndex();
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & [_, device] : deviceInfo.mDeviceDirectory)
        {
            ReturnErrorOnFailure(device->Encode(encoder, fabricIndex));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR EcosystemInformationServer::EncodeLocationStructAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder)
{
    if (mDevicesMap.find(aEndpoint) == mDevicesMap.end())
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCluster);
    }

    auto & deviceInfo = mDevicesMap[aEndpoint];
    if (deviceInfo.mLocationDirectory.empty() || deviceInfo.mRemovedOn.HasValue())
    {
        return aEncoder.EncodeEmptyList();
    }

    FabricIndex fabricIndex = aEncoder.AccessingFabricIndex();
    return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
        for (auto & [id, device] : deviceInfo.mLocationDirectory)
        {
            ReturnErrorOnFailure(device->Encode(encoder, id, fabricIndex));
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
    registerAttributeAccessOverride(&gAttrAccess);
}
