/*
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

#include "lib/support/CHIPMem.h"

#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace {

using namespace Clusters;
using namespace Clusters::EcosystemInformation;

const EndpointId kValidEndpointId                      = 1;
const Structs::DeviceTypeStruct::Type kValidDeviceType = { .deviceType = 0, .revision = 1 };
constexpr Access::SubjectDescriptor kSubjectDescriptor = Testing::kAdminSubjectDescriptor;
const FabricIndex kValidFabricIndex                    = kSubjectDescriptor.fabricIndex;

struct RequiredEcosystemDeviceParams
{
    EndpointId originalEndpointId              = kValidEndpointId;
    Structs::DeviceTypeStruct::Type deviceType = kValidDeviceType;
    FabricIndex fabicIndex                     = kValidFabricIndex;
};

const RequiredEcosystemDeviceParams kDefaultRequiredDeviceParams;

const EndpointId kAnotherValidEndpointId = 2;
static_assert(kValidEndpointId != kAnotherValidEndpointId);
const char * kValidLocationName                 = "AValidLocationName";
const ClusterId kEcosystemInfoClusterId         = EcosystemInformation::Id;
const AttributeId kDeviceDirectoryAttributeId   = EcosystemInformation::Attributes::DeviceDirectory::Id;
const AttributeId kLocationDirectoryAttributeId = EcosystemInformation::Attributes::LocationDirectory::Id;

class MockMatterContext : public MatterContext
{
public:
    virtual void MarkDirty(EndpointId endpointId, AttributeId attributeId) override
    {
        ConcreteAttributePath path(endpointId, kEcosystemInfoClusterId, attributeId);
        mDirtyMarkedList.push_back(path);
    }

    std::vector<ConcreteAttributePath> & GetDirtyList() { return mDirtyMarkedList; }

private:
    std::vector<ConcreteAttributePath> mDirtyMarkedList;
};

} // namespace

class TestEcosystemInformationCluster : public ::testing::Test
{
public:
    TestEcosystemInformationCluster() : mClusterServer(TestOnlyParameter(), mMockMatterContext) {}

    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    Clusters::EcosystemInformation::EcosystemInformationServer & EcoInfoCluster() { return mClusterServer; }

    std::unique_ptr<EcosystemDeviceStruct>
    CreateSimplestValidDeviceStruct(const RequiredEcosystemDeviceParams & requiredParams = kDefaultRequiredDeviceParams)
    {
        std::unique_ptr<EcosystemDeviceStruct> deviceInfo = EcosystemDeviceStruct::Builder()
                                                                .SetOriginalEndpoint(requiredParams.originalEndpointId)
                                                                .AddDeviceType(requiredParams.deviceType)
                                                                .SetFabricIndex(requiredParams.fabicIndex)
                                                                .Build();
        VerifyOrDie(deviceInfo);
        return deviceInfo;
    }

    std::unique_ptr<EcosystemLocationStruct> CreateValidLocationStruct(const char * requiredLocationName = kValidLocationName)
    {
        std::string locationName(requiredLocationName);
        std::unique_ptr<EcosystemLocationStruct> locationInfo =
            EcosystemLocationStruct::Builder().SetLocationName(locationName).Build();
        VerifyOrDie(locationInfo);
        return locationInfo;
    }

    MockMatterContext & GetMockMatterContext() { return mMockMatterContext; }

private:
    MockMatterContext mMockMatterContext;
    Clusters::EcosystemInformation::EcosystemInformationServer mClusterServer;
};

TEST_F(TestEcosystemInformationCluster, UnsupportedClusterWhenReadingDeviceDirectoryOnNewClusterServer)
{
    ConcreteAttributePath path(kValidEndpointId, kEcosystemInfoClusterId, kDeviceDirectoryAttributeId);

    Testing::ReadOperation testRequest(path);
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();

    ASSERT_EQ(EcoInfoCluster().ReadAttribute(path, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));
}

TEST_F(TestEcosystemInformationCluster, UnsupportedClusterWhenReadingLocationDirectoryOnNewClusterServer)
{
    ConcreteAttributePath path(kValidEndpointId, kEcosystemInfoClusterId, kLocationDirectoryAttributeId);

    Testing::ReadOperation testRequest(path);
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();

    ASSERT_EQ(EcoInfoCluster().ReadAttribute(path, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));
}

TEST_F(TestEcosystemInformationCluster, EmptyReadAfterAddEcosystemInformationClusterToEndpoint)
{
    ConcreteAttributePath deviceDirectoryPath(kValidEndpointId, kEcosystemInfoClusterId, kDeviceDirectoryAttributeId);
    ConcreteAttributePath locationDirectoryPath(kValidEndpointId, kEcosystemInfoClusterId, kLocationDirectoryAttributeId);

    ASSERT_EQ(EcoInfoCluster().AddEcosystemInformationClusterToEndpoint(kValidEndpointId), CHIP_NO_ERROR);

    Testing::ReadOperation testDeviceDirectoryRequest(deviceDirectoryPath);
    std::unique_ptr<AttributeValueEncoder> deviceDirectoryEncoder = testDeviceDirectoryRequest.StartEncoding();
    ASSERT_EQ(EcoInfoCluster().ReadAttribute(deviceDirectoryPath, *deviceDirectoryEncoder), CHIP_NO_ERROR);
    ASSERT_EQ(testDeviceDirectoryRequest.FinishEncoding(), CHIP_NO_ERROR);
    std::vector<Testing::DecodedAttributeData> deviceDirectoryAttributeData;
    ASSERT_EQ(testDeviceDirectoryRequest.GetEncodedIBs().Decode(deviceDirectoryAttributeData), CHIP_NO_ERROR);
    ASSERT_EQ(deviceDirectoryAttributeData.size(), 1u);
    Testing::DecodedAttributeData & deviceDirectoryEncodedData = deviceDirectoryAttributeData[0];
    ASSERT_EQ(deviceDirectoryEncodedData.attributePath, testDeviceDirectoryRequest.GetRequest().path);
    EcosystemInformation::Attributes::DeviceDirectory::TypeInfo::DecodableType decodableDeviceDirectory;
    ASSERT_EQ(decodableDeviceDirectory.Decode(deviceDirectoryEncodedData.dataReader), CHIP_NO_ERROR);
    size_t deviceDirectorySize = 0;
    ASSERT_EQ(decodableDeviceDirectory.ComputeSize(&deviceDirectorySize), CHIP_NO_ERROR);
    ASSERT_EQ(deviceDirectorySize, 0u);

    Testing::ReadOperation testLocationDirectoryRequest(locationDirectoryPath);
    std::unique_ptr<AttributeValueEncoder> locationDirectoryEncoder = testLocationDirectoryRequest.StartEncoding();
    ASSERT_EQ(EcoInfoCluster().ReadAttribute(locationDirectoryPath, *locationDirectoryEncoder), CHIP_NO_ERROR);
    ASSERT_EQ(testLocationDirectoryRequest.FinishEncoding(), CHIP_NO_ERROR);
    std::vector<Testing::DecodedAttributeData> locationDirectoryAttributeData;
    ASSERT_EQ(testLocationDirectoryRequest.GetEncodedIBs().Decode(locationDirectoryAttributeData), CHIP_NO_ERROR);
    ASSERT_EQ(locationDirectoryAttributeData.size(), 1u);
    Testing::DecodedAttributeData & locationDirectoryEncodedData = locationDirectoryAttributeData[0];
    ASSERT_EQ(locationDirectoryEncodedData.attributePath, testLocationDirectoryRequest.GetRequest().path);
    EcosystemInformation::Attributes::LocationDirectory::TypeInfo::DecodableType decodableLocationDirectory;
    ASSERT_EQ(decodableLocationDirectory.Decode(locationDirectoryEncodedData.dataReader), CHIP_NO_ERROR);
    size_t locationDirectorySize = 0;
    ASSERT_EQ(decodableLocationDirectory.ComputeSize(&locationDirectorySize), CHIP_NO_ERROR);
    ASSERT_EQ(locationDirectorySize, 0u);
}

TEST_F(TestEcosystemInformationCluster, BuildingEcosystemDeviceStruct)
{
    EcosystemDeviceStruct::Builder deviceInfoBuilder;
    std::unique_ptr<EcosystemDeviceStruct> deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    deviceInfoBuilder.SetOriginalEndpoint(1);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    auto deviceType     = Structs::DeviceTypeStruct::Type();
    deviceType.revision = 1;
    deviceInfoBuilder.AddDeviceType(deviceType);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    deviceInfoBuilder.SetFabricIndex(1);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_TRUE(deviceInfo);

    // Building a second device info with previously successfully built deviceInfoBuilder
    // is expected to fail.
    std::unique_ptr<EcosystemDeviceStruct> secondDeviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(secondDeviceInfo);
}

TEST_F(TestEcosystemInformationCluster, BuildingInvalidEcosystemDeviceStruct)
{
    auto deviceType                       = Structs::DeviceTypeStruct::Type();
    deviceType.revision                   = 1;
    const FabricIndex kFabricIndexTooLow  = 0;
    const FabricIndex kFabricIndexTooHigh = kMaxValidFabricIndex + 1;

    EcosystemDeviceStruct::Builder deviceInfoBuilder;
    deviceInfoBuilder.SetOriginalEndpoint(1);
    deviceInfoBuilder.AddDeviceType(deviceType);
    deviceInfoBuilder.SetFabricIndex(kFabricIndexTooLow);
    std::unique_ptr<EcosystemDeviceStruct> deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    deviceInfoBuilder.SetFabricIndex(kFabricIndexTooHigh);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    deviceInfoBuilder.SetFabricIndex(1);
    // At this point deviceInfoBuilder would be able to be built successfully.

    std::string nameThatsTooLong(65, 'x');
    uint64_t nameEpochValueUs = 0; // This values doesn't matter.
    deviceInfoBuilder.SetDeviceName(std::move(nameThatsTooLong), nameEpochValueUs);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_FALSE(deviceInfo);

    // Ending unit test by building something that should work just to make sure
    // Builder isn't silently failing on building for some other reason.
    std::string nameThatsMaxLength(64, 'x');
    deviceInfoBuilder.SetDeviceName(std::move(nameThatsMaxLength), nameEpochValueUs);
    deviceInfo = deviceInfoBuilder.Build();
    ASSERT_TRUE(deviceInfo);
}

TEST_F(TestEcosystemInformationCluster, AddDeviceInfoInvalidArguments)
{
    ASSERT_EQ(EcoInfoCluster().AddDeviceInfo(kValidEndpointId, nullptr), CHIP_ERROR_INVALID_ARGUMENT);

    std::unique_ptr<EcosystemDeviceStruct> deviceInfo = CreateSimplestValidDeviceStruct();
    ASSERT_TRUE(deviceInfo);
    ASSERT_EQ(EcoInfoCluster().AddDeviceInfo(kRootEndpointId, std::move(deviceInfo)), CHIP_ERROR_INVALID_ARGUMENT);

    deviceInfo = CreateSimplestValidDeviceStruct();
    ASSERT_TRUE(deviceInfo);
    ASSERT_EQ(EcoInfoCluster().AddDeviceInfo(kInvalidEndpointId, std::move(deviceInfo)), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestEcosystemInformationCluster, AddDeviceInfo)
{
    std::unique_ptr<EcosystemDeviceStruct> deviceInfo = CreateSimplestValidDeviceStruct();
    // originalEndpoint and path endpoint do not need to be the same, for that reason we use a different value for
    // path endpoint
    static_assert(kAnotherValidEndpointId != kValidEndpointId);
    ASSERT_EQ(EcoInfoCluster().AddDeviceInfo(kAnotherValidEndpointId, std::move(deviceInfo)), CHIP_NO_ERROR);
    ConcreteAttributePath deviceDirectoryPath(kAnotherValidEndpointId, kEcosystemInfoClusterId, kDeviceDirectoryAttributeId);
    Testing::ReadOperation testDeviceDirectoryRequest(deviceDirectoryPath);
    testDeviceDirectoryRequest.SetSubjectDescriptor(kSubjectDescriptor);
    std::unique_ptr<AttributeValueEncoder> deviceDirectoryEncoder = testDeviceDirectoryRequest.StartEncoding();

    ASSERT_EQ(EcoInfoCluster().ReadAttribute(deviceDirectoryPath, *deviceDirectoryEncoder), CHIP_NO_ERROR);
    ASSERT_EQ(testDeviceDirectoryRequest.FinishEncoding(), CHIP_NO_ERROR);

    std::vector<Testing::DecodedAttributeData> attributeData;
    ASSERT_EQ(testDeviceDirectoryRequest.GetEncodedIBs().Decode(attributeData), CHIP_NO_ERROR);
    ASSERT_EQ(attributeData.size(), 1u);
    Testing::DecodedAttributeData & encodedData = attributeData[0];
    ASSERT_EQ(encodedData.attributePath, testDeviceDirectoryRequest.GetRequest().path);
    EcosystemInformation::Attributes::DeviceDirectory::TypeInfo::DecodableType decodableDeviceDirectory;
    ASSERT_EQ(decodableDeviceDirectory.Decode(encodedData.dataReader), CHIP_NO_ERROR);
    size_t size = 0;
    ASSERT_EQ(decodableDeviceDirectory.ComputeSize(&size), CHIP_NO_ERROR);
    ASSERT_EQ(size, 1u);
    auto iterator = decodableDeviceDirectory.begin();
    ASSERT_TRUE(iterator.Next());
    auto deviceDirectoryEntry = iterator.GetValue();
    ASSERT_FALSE(deviceDirectoryEntry.deviceName.HasValue());
    ASSERT_FALSE(deviceDirectoryEntry.deviceNameLastEdit.HasValue());
    ASSERT_EQ(deviceDirectoryEntry.bridgedEndpoint, kInvalidEndpointId);
    ASSERT_EQ(deviceDirectoryEntry.originalEndpoint, kValidEndpointId);
    size_t deviceTypeListSize = 0;
    ASSERT_EQ(deviceDirectoryEntry.deviceTypes.ComputeSize(&deviceTypeListSize), CHIP_NO_ERROR);
    ASSERT_EQ(deviceTypeListSize, 1u);
    auto deviceTypeIterator = deviceDirectoryEntry.deviceTypes.begin();
    ASSERT_TRUE(deviceTypeIterator.Next());
    auto deviceTypeEntry = deviceTypeIterator.GetValue();
    ASSERT_EQ(deviceTypeEntry.deviceType, 0u);
    ASSERT_EQ(deviceTypeEntry.revision, 1);
    ASSERT_FALSE(deviceTypeIterator.Next());
    size_t uniqueLocationIdListSize = 0;
    ASSERT_EQ(deviceDirectoryEntry.uniqueLocationIDs.ComputeSize(&uniqueLocationIdListSize), CHIP_NO_ERROR);
    ASSERT_EQ(uniqueLocationIdListSize, 0u);
    ASSERT_EQ(deviceDirectoryEntry.uniqueLocationIDsLastEdit, 0u);
    ASSERT_EQ(deviceDirectoryEntry.fabricIndex, kSubjectDescriptor.fabricIndex);
    ASSERT_FALSE(iterator.Next());
}

TEST_F(TestEcosystemInformationCluster, AddDeviceInfoResultInMarkDirty)
{
    std::unique_ptr<EcosystemDeviceStruct> deviceInfo = CreateSimplestValidDeviceStruct();
    ASSERT_EQ(EcoInfoCluster().AddDeviceInfo(kValidEndpointId, std::move(deviceInfo)), CHIP_NO_ERROR);

    auto markedDirtyList = GetMockMatterContext().GetDirtyList();
    ASSERT_EQ(markedDirtyList.size(), 1u);
    ConcreteAttributePath path = markedDirtyList[0];
    ASSERT_EQ(path.mEndpointId, kValidEndpointId);
    ASSERT_EQ(path.mClusterId, kEcosystemInfoClusterId);
    ASSERT_EQ(path.mAttributeId, kDeviceDirectoryAttributeId);
}

TEST_F(TestEcosystemInformationCluster, BuildingEcosystemLocationStruct)
{
    EcosystemLocationStruct::Builder locationInfoBuilder;

    std::string validLocationName = "validName";
    locationInfoBuilder.SetLocationName(validLocationName);

    std::unique_ptr<EcosystemLocationStruct> locationInfo = locationInfoBuilder.Build();
    ASSERT_TRUE(locationInfo);

    // Building a second device info with previously successfully built deviceInfoBuilder
    // is expected to fail.
    locationInfo = locationInfoBuilder.Build();
    ASSERT_FALSE(locationInfo);
}

TEST_F(TestEcosystemInformationCluster, BuildingInvalidEcosystemLocationStruct)
{
    EcosystemLocationStruct::Builder locationInfoBuilder;

    std::string nameThatsTooLong(129, 'x');
    locationInfoBuilder.SetLocationName(nameThatsTooLong);

    std::unique_ptr<EcosystemLocationStruct> locationInfo = locationInfoBuilder.Build();
    ASSERT_FALSE(locationInfo);

    // Ending unit test by building something that should work just to make sure
    // Builder isn't silently failing on building for some other reason.
    std::string nameThatsMaxLength(128, 'x');
    locationInfoBuilder.SetLocationName(nameThatsMaxLength);
    locationInfo = locationInfoBuilder.Build();
    ASSERT_TRUE(locationInfo);
}

TEST_F(TestEcosystemInformationCluster, AddLocationInfoInvalidArguments)
{
    const FabricIndex kFabricIndexTooLow  = 0;
    const FabricIndex kFabricIndexTooHigh = kMaxValidFabricIndex + 1;
    const std::string kEmptyLocationIdStr;
    const std::string kValidLocationIdStr = "SomeLocationString";
    const std::string kInvalidLocationIdTooLongStr(65, 'x');

    std::unique_ptr<EcosystemLocationStruct> locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kInvalidEndpointId, kValidLocationIdStr, kValidFabricIndex, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kRootEndpointId, kValidLocationIdStr, kValidFabricIndex, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kEmptyLocationIdStr, kValidFabricIndex, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kInvalidLocationIdTooLongStr, kValidFabricIndex,
                                               std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, kFabricIndexTooLow, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, kFabricIndexTooHigh, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);

    // Sanity check that we can successfully add something after all the previously failed attempts
    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, kValidFabricIndex, std::move(locationInfo)),
              CHIP_NO_ERROR);

    // Adding a second identical entry is expected to fail
    locationInfo = CreateValidLocationStruct();
    ASSERT_TRUE(locationInfo);
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, kValidFabricIndex, std::move(locationInfo)),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestEcosystemInformationCluster, AddLocationInfo)
{
    std::unique_ptr<EcosystemLocationStruct> locationInfo = CreateValidLocationStruct();
    const char * kValidLocationIdStr                      = "SomeLocationIdString";
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, Testing::kAdminSubjectDescriptor.fabricIndex,
                                               std::move(locationInfo)),
              CHIP_NO_ERROR);

    ConcreteAttributePath locationDirectoryPath(kValidEndpointId, kEcosystemInfoClusterId, kLocationDirectoryAttributeId);
    Testing::ReadOperation testLocationDirectoryRequest(locationDirectoryPath);
    testLocationDirectoryRequest.SetSubjectDescriptor(Testing::kAdminSubjectDescriptor);
    std::unique_ptr<AttributeValueEncoder> locationDirectoryEncoder = testLocationDirectoryRequest.StartEncoding();
    ASSERT_EQ(EcoInfoCluster().ReadAttribute(locationDirectoryPath, *locationDirectoryEncoder), CHIP_NO_ERROR);
    ASSERT_EQ(testLocationDirectoryRequest.FinishEncoding(), CHIP_NO_ERROR);

    std::vector<Testing::DecodedAttributeData> locationDirectoryAttributeData;
    ASSERT_EQ(testLocationDirectoryRequest.GetEncodedIBs().Decode(locationDirectoryAttributeData), CHIP_NO_ERROR);
    ASSERT_EQ(locationDirectoryAttributeData.size(), 1u);
    Testing::DecodedAttributeData & locationDirectoryEncodedData = locationDirectoryAttributeData[0];
    ASSERT_EQ(locationDirectoryEncodedData.attributePath, testLocationDirectoryRequest.GetRequest().path);
    EcosystemInformation::Attributes::LocationDirectory::TypeInfo::DecodableType decodableLocationDirectory;
    ASSERT_EQ(decodableLocationDirectory.Decode(locationDirectoryEncodedData.dataReader), CHIP_NO_ERROR);
    size_t locationDirectorySize = 0;
    ASSERT_EQ(decodableLocationDirectory.ComputeSize(&locationDirectorySize), CHIP_NO_ERROR);
    ASSERT_EQ(locationDirectorySize, 1u);
    auto iterator = decodableLocationDirectory.begin();
    ASSERT_TRUE(iterator.Next());
    auto locationDirectoryEntry = iterator.GetValue();
    ASSERT_TRUE(locationDirectoryEntry.uniqueLocationID.data_equal(CharSpan::fromCharString(kValidLocationIdStr)));
    ASSERT_TRUE(locationDirectoryEntry.locationDescriptor.locationName.data_equal(CharSpan::fromCharString(kValidLocationName)));
    ASSERT_TRUE(locationDirectoryEntry.locationDescriptor.floorNumber.IsNull());
    ASSERT_TRUE(locationDirectoryEntry.locationDescriptor.areaType.IsNull());
    ASSERT_EQ(locationDirectoryEntry.locationDescriptorLastEdit, 0u);
    ASSERT_EQ(locationDirectoryEntry.fabricIndex, Testing::kAdminSubjectDescriptor.fabricIndex);
    ASSERT_FALSE(iterator.Next());
}

TEST_F(TestEcosystemInformationCluster, AddLocationInfoResultInMarkDirty)
{
    std::unique_ptr<EcosystemLocationStruct> locationInfo = CreateValidLocationStruct();
    const char * kValidLocationIdStr                      = "SomeLocationIdString";
    ASSERT_EQ(EcoInfoCluster().AddLocationInfo(kValidEndpointId, kValidLocationIdStr, Testing::kAdminSubjectDescriptor.fabricIndex,
                                               std::move(locationInfo)),
              CHIP_NO_ERROR);

    auto markedDirtyList = GetMockMatterContext().GetDirtyList();
    ASSERT_EQ(markedDirtyList.size(), 1u);
    ConcreteAttributePath path = markedDirtyList[0];
    ASSERT_EQ(path.mEndpointId, kValidEndpointId);
    ASSERT_EQ(path.mClusterId, kEcosystemInfoClusterId);
    ASSERT_EQ(path.mAttributeId, kLocationDirectoryAttributeId);
}

} // namespace app
} // namespace chip
