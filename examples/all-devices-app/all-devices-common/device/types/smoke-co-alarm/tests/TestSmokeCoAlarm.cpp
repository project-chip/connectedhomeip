/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/smoke-co-alarm/impl/LoggingOnlySmokeCoAlarm.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/AttributeIds.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/PowerSource/AttributeIds.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

#include <algorithm>
#include <memory>
#include <vector>

using namespace chip;
using namespace chip::app;

namespace {

class SmokeCoAlarmTestFixture : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    static SmokeCoAlarm::Config DefaultSmokeCoConfig()
    {
        SmokeCoAlarm::Config config;
        config.alarmConfig.WithSmokeAlarm({}).WithCOAlarm();
        return config;
    }

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    Testing::ScopedAttributeChangeListenerRegistration mChangeListenerRegistration{ mProvider, mContext };
    TimerDelegateMock mTimerDelegate;
};

class FailingEndpointIdAllocator : public EndpointIdAllocator
{
public:
    explicit FailingEndpointIdAllocator(size_t failureIndex) : mFailureIndex(failureIndex) {}

    EndpointId Allocate() override
    {
        if (mAllocationIndex++ == mFailureIndex)
        {
            return kInvalidEndpointId;
        }
        return mNextEndpoint++;
    }

private:
    const size_t mFailureIndex;
    size_t mAllocationIndex  = 0;
    EndpointId mNextEndpoint = 1;
};

class ProviderAttributeReader
{
public:
    explicit ProviderAttributeReader(CodeDrivenDataModelProvider & provider) : mProvider(provider) {}

    template <typename T>
    CHIP_ERROR Read(EndpointId endpoint, ClusterId cluster, AttributeId attribute, T & value)
    {
        mOperations.push_back(std::make_unique<Testing::ReadOperation>(endpoint, cluster, attribute));
        Testing::ReadOperation & operation             = *mOperations.back();
        std::unique_ptr<AttributeValueEncoder> encoder = operation.StartEncoding();
        VerifyOrReturnError(encoder != nullptr, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(mProvider.ReadAttribute(operation.GetRequest(), *encoder).GetUnderlyingError());
        ReturnErrorOnFailure(operation.FinishEncoding());

        std::vector<Testing::DecodedAttributeData> attributeData;
        ReturnErrorOnFailure(operation.GetEncodedIBs().Decode(attributeData));
        VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);
        return DataModel::Decode(attributeData[0].dataReader, value);
    }

private:
    CodeDrivenDataModelProvider & mProvider;
    std::vector<std::unique_ptr<Testing::ReadOperation>> mOperations;
};

std::vector<DataModel::EndpointEntry> GetEndpoints(CodeDrivenDataModelProvider & provider)
{
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
    EXPECT_EQ(provider.Endpoints(builder), CHIP_NO_ERROR);
    const auto buffer = builder.TakeBuffer();
    return std::vector<DataModel::EndpointEntry>(buffer.begin(), buffer.end());
}

void ExpectSingleDeviceType(CodeDrivenDataModelProvider & provider, EndpointId endpoint,
                            const DataModel::DeviceTypeEntry & expected)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    ASSERT_EQ(provider.DeviceTypes(endpoint, builder), CHIP_NO_ERROR);
    const auto deviceTypes = builder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    EXPECT_EQ(deviceTypes[0].deviceTypeId, expected.deviceTypeId);
    EXPECT_EQ(deviceTypes[0].deviceTypeRevision, expected.deviceTypeRevision);
}

TEST_F(SmokeCoAlarmTestFixture, RegistersPeerPowerSourceEndpointWithMatchingEndpointList)
{
    ConsecutiveEndpointIdAllocator allocator;
    LoggingOnlySmokeCoAlarm device(mTimerDelegate, DefaultSmokeCoConfig());

    ASSERT_EQ(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    const EndpointId alarmEndpoint       = device.GetEndpointId();
    const EndpointId powerSourceEndpoint = device.GetPowerSource().GetEndpointId();
    EXPECT_NE(alarmEndpoint, kInvalidEndpointId);
    EXPECT_NE(powerSourceEndpoint, kInvalidEndpointId);
    EXPECT_NE(alarmEndpoint, powerSourceEndpoint);

    const auto endpoints = GetEndpoints(mProvider);
    ASSERT_EQ(endpoints.size(), 2u);

    const auto alarmEntry =
        std::find_if(endpoints.begin(), endpoints.end(), [alarmEndpoint](const auto & entry) { return entry.id == alarmEndpoint; });
    ASSERT_NE(alarmEntry, endpoints.end());
    EXPECT_EQ(alarmEntry->parentId, kInvalidEndpointId);

    const auto powerSourceEntry = std::find_if(
        endpoints.begin(), endpoints.end(), [powerSourceEndpoint](const auto & entry) { return entry.id == powerSourceEndpoint; });
    ASSERT_NE(powerSourceEntry, endpoints.end());
    // PowerSource is a separate sibling endpoint, NOT a child part of SmokeCoAlarm.
    EXPECT_EQ(powerSourceEntry->parentId, kInvalidEndpointId);

    ProviderAttributeReader reader(mProvider);

    // Verify SmokeCoAlarm's Descriptor::PartsList is empty.
    Clusters::Descriptor::Attributes::PartsList::TypeInfo::DecodableType partsList;
    ASSERT_EQ(reader.Read(alarmEndpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id, partsList),
              CHIP_NO_ERROR);
    auto partsIter = partsList.begin();
    EXPECT_FALSE(partsIter.Next());
    EXPECT_EQ(partsIter.GetStatus(), CHIP_NO_ERROR);

    // Verify PowerSource::EndpointList references the SmokeCoAlarm endpoint.
    Clusters::PowerSource::Attributes::EndpointList::TypeInfo::DecodableType endpointList;
    ASSERT_EQ(reader.Read(powerSourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::EndpointList::Id,
                          endpointList),
              CHIP_NO_ERROR);
    std::vector<EndpointId> actualEndpoints;
    auto epIter = endpointList.begin();
    while (epIter.Next())
    {
        actualEndpoints.push_back(epIter.GetValue());
    }
    EXPECT_EQ(epIter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(actualEndpoints, std::vector<EndpointId>{ alarmEndpoint });

    ExpectSingleDeviceType(mProvider, alarmEndpoint, Device::Type::kSmokeCoAlarm);
    ExpectSingleDeviceType(mProvider, powerSourceEndpoint, Device::Type::kPowerSource);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetPowerSource().GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SmokeCoAlarmTestFixture, PropagatesBridgedParentToSiblingPowerSourceEndpoint)
{
    constexpr EndpointId kBridgedNodeEndpoint = 10;
    ConsecutiveEndpointIdAllocator allocator;
    LoggingOnlySmokeCoAlarm device(mTimerDelegate, DefaultSmokeCoConfig());

    ASSERT_EQ(device.Register(allocator, mProvider, EndpointComposition::WithParent(kBridgedNodeEndpoint)), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    const auto endpoints = GetEndpoints(mProvider);
    ASSERT_EQ(endpoints.size(), 2u);
    for (const auto & entry : endpoints)
    {
        EXPECT_EQ(entry.parentId, kBridgedNodeEndpoint);
    }

    device.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

TEST_F(SmokeCoAlarmTestFixture, RollsBackWhenPowerSourceEndpointAllocationFails)
{
    FailingEndpointIdAllocator allocator(/* failureIndex = */ 1);
    LoggingOnlySmokeCoAlarm device(mTimerDelegate, DefaultSmokeCoConfig());

    EXPECT_NE(device.Register(allocator, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetPowerSource().GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
}

} // namespace
