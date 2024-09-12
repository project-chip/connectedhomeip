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
#pragma once

// This cluster is targeted by devices that are not resource constrained, for
// that reason we use std containers to simplify implementation of the cluster.
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EcosystemInformation {

// This intentionally mirrors Structs::EcosystemDeviceStruct::Type but has ownership
// of underlying types.
class EcosystemDeviceStruct
{
public:
    class Builder
    {
    public:
        Builder(){};

        Builder & SetDeviceName(std::string aDeviceName, uint64_t aDeviceNameLastEditEpochUs);
        Builder & SetBrigedEndpoint(EndpointId aBridgedEndpoint);
        Builder & SetOriginalEndpoint(EndpointId aOriginalEndpoint);
        Builder & AddDeviceType(Structs::DeviceTypeStruct::Type aDeviceType);
        Builder & AddUniqueLocationId(std::string aUniqueLocationId, uint64_t aUniqueLocationIdsLastEditEpochUs);
        Builder & SetFabricIndex(FabricIndex aFabricIndex);

        // Upon success this object will have moved all ownership of underlying
        // types to EcosystemDeviceStruct and should not be used afterwards.
        std::unique_ptr<EcosystemDeviceStruct> Build();

    private:
        std::string mDeviceName;
        uint64_t mDeviceNameLastEditEpochUs = 0;
        EndpointId mBridgedEndpoint         = kInvalidEndpointId;
        EndpointId mOriginalEndpoint        = kInvalidEndpointId;
        std::vector<Structs::DeviceTypeStruct::Type> mDeviceTypes;
        std::vector<std::string> mUniqueLocationIds;
        uint64_t mUniqueLocationIdsLastEditEpochUs = 0;
        FabricIndex mFabricIndex                   = kUndefinedFabricIndex;
        bool mIsAlreadyBuilt                       = false;
    };

    CHIP_ERROR Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder);

private:
    // Constructor is intentionally private. This is to ensure that it is only constructed with
    // values that conform to the spec.
    explicit EcosystemDeviceStruct(std::string && aDeviceName, uint64_t aDeviceNameLastEditEpochUs, EndpointId aBridgedEndpoint,
                                   EndpointId aOriginalEndpoint, std::vector<Structs::DeviceTypeStruct::Type> && aDeviceTypes,
                                   std::vector<std::string> && aUniqueLocationIds, uint64_t aUniqueLocationIdsLastEditEpochUs,
                                   FabricIndex aFabricIndex) :
        mDeviceName(std::move(aDeviceName)),
        mDeviceNameLastEditEpochUs(aDeviceNameLastEditEpochUs), mBridgedEndpoint(aBridgedEndpoint),
        mOriginalEndpoint(aOriginalEndpoint), mDeviceTypes(std::move(aDeviceTypes)),
        mUniqueLocationIds(std::move(aUniqueLocationIds)), mUniqueLocationIdsLastEditEpochUs(aUniqueLocationIdsLastEditEpochUs),
        mFabricIndex(aFabricIndex)

    {}

    const std::string mDeviceName;
    uint64_t mDeviceNameLastEditEpochUs;
    EndpointId mBridgedEndpoint;
    EndpointId mOriginalEndpoint;
    std::vector<Structs::DeviceTypeStruct::Type> mDeviceTypes;
    std::vector<std::string> mUniqueLocationIds;
    uint64_t mUniqueLocationIdsLastEditEpochUs;
    FabricIndex mFabricIndex;
};

struct LocationDescriptorStruct
{
    std::string mLocationName;
    std::optional<int16_t> mFloorNumber;
    std::optional<Globals::AreaTypeTag> mAreaType;
};

// This intentionally mirrors Structs::EcosystemLocationStruct::Type but has ownership
// of underlying types.
class EcosystemLocationStruct
{
public:
    class Builder
    {
    public:
        Builder(){};

        Builder & SetLocationName(std::string aLocationName);
        Builder & SetFloorNumber(std::optional<int16_t> aFloorNumber);
        Builder & SetAreaTypeTag(std::optional<Globals::AreaTypeTag> aAreaTypeTag);
        Builder & SetLocationDescriptorLastEdit(uint64_t aLocationDescriptorLastEditEpochUs);

        // Upon success this object will have moved all ownership of underlying
        // types to EcosystemDeviceStruct and should not be used afterwards.
        std::unique_ptr<EcosystemLocationStruct> Build();

    private:
        LocationDescriptorStruct mLocationDescriptor;
        uint64_t mLocationDescriptorLastEditEpochUs = 0;
        bool mIsAlreadyBuilt                        = false;
    };

    CHIP_ERROR Encode(const AttributeValueEncoder::ListEncodeHelper & aEncoder, const std::string & aUniqueLocationId,
                      const FabricIndex & aFabricIndex);

private:
    // Constructor is intentionally private. This is to ensure that it is only constructed with
    // values that conform to the spec.
    explicit EcosystemLocationStruct(LocationDescriptorStruct && aLocationDescriptor, uint64_t aLocationDescriptorLastEditEpochUs) :
        mLocationDescriptor(aLocationDescriptor), mLocationDescriptorLastEditEpochUs(aLocationDescriptorLastEditEpochUs)
    {}
    // EcosystemLocationStruct is used as a value in a key-value map.
    // Because UniqueLocationId and FabricIndex are mandatory when an entry exist,
    // and needs to be unique, we use it as a key to the key-value pair and is why it is
    // not explicitly in this struct.
    LocationDescriptorStruct mLocationDescriptor;
    uint64_t mLocationDescriptorLastEditEpochUs;
};

class EcosystemInformationServer
{
public:
    static EcosystemInformationServer & Instance();

    /**
     * @brief Add EcosystemInformation Cluster to endpoint so we respond appropriately on endpoint
     *
     * EcosystemInformation cluster is only ever on dynamic bridge endpoint. If cluster is added
     * to a new endpoint, but does not contain any ecosystem information presently,
     * this is called to let ECOINFO cluster code know it is supposed to provide blank attribute
     * information on this endpoint.
     *
     * This approach was intentionally taken instead of relying on emberAfDeviceTypeListFromEndpoint
     * to keep this cluster more unit testable. This does add burden to application but is worth
     * the trade-off.
     *
     * @param[in] aEndpoint Which endpoint is the device being added to the device directory.
     * @return #CHIP_NO_ERROR on success.
     * @return Other CHIP_ERROR associated with issue.
     */
    CHIP_ERROR AddEcosystemInformationClusterToEndpoint(EndpointId aEndpoint);

    /**
     * @brief Adds device as entry to DeviceDirectory list Attribute.
     *
     * @param[in] aEndpoint Which endpoint is the device being added to the device directory.
     * @param[in] aDevice Device information.
     * @return #CHIP_NO_ERROR on success.
     * @return Other CHIP_ERROR associated with issue.
     */
    CHIP_ERROR AddDeviceInfo(EndpointId aEndpoint, std::unique_ptr<EcosystemDeviceStruct> aDevice);
    /**
     * @brief Adds location as entry to LocationDirectory list Attribute.
     *
     * @param[in] aEndpoint Which endpoint is the location being added to the location directory.
     * @param[in] aLocationId LocationID associated with location.
     * @param[in] aLocation Location information.
     * @return #CHIP_NO_ERROR on success.
     * @return Other CHIP_ERROR associated with issue.
     */
    CHIP_ERROR AddLocationInfo(EndpointId aEndpoint, const std::string & aLocationId, FabricIndex aFabricIndex,
                               std::unique_ptr<EcosystemLocationStruct> aLocation);
    // TODO(#33223) Add removal and update counterparts to AddDeviceInfo and AddLocationInfo.

    CHIP_ERROR ReadAttribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

private:
    struct EcosystemLocationKey
    {
        bool operator<(const EcosystemLocationKey & other) const
        {
            return mUniqueLocationId < other.mUniqueLocationId ||
                (mUniqueLocationId == other.mUniqueLocationId && mFabricIndex < other.mFabricIndex);
        }
        std::string mUniqueLocationId;
        FabricIndex mFabricIndex;
    };

    struct DeviceInfo
    {
        std::vector<std::unique_ptr<EcosystemDeviceStruct>> mDeviceDirectory;
        std::map<EcosystemLocationKey, std::unique_ptr<EcosystemLocationStruct>> mLocationDirectory;
    };

    CHIP_ERROR EncodeDeviceDirectoryAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR EncodeLocationStructAttribute(EndpointId aEndpoint, AttributeValueEncoder & aEncoder);

    std::map<EndpointId, DeviceInfo> mDevicesMap;

    static EcosystemInformationServer mInstance;
};

} // namespace EcosystemInformation
} // namespace Clusters
} // namespace app
} // namespace chip
