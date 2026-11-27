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
#include <clusters/Descriptor/Enums.h>
#include <clusters/PowerSource/AttributeIds.h>
#include <clusters/PowerSource/Attributes.h>
#include <clusters/PowerSource/ClusterId.h>
#include <clusters/shared/Enums.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
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
    static void SetUpTestSuite() { ASSERT_SUCCESS(Platform::MemoryInit()); }
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
    EXPECT_SUCCESS(provider.Endpoints(builder));
    const auto buffer = builder.TakeBuffer();
    return std::vector<DataModel::EndpointEntry>(buffer.begin(), buffer.end());
}

void ExpectSingleDeviceType(CodeDrivenDataModelProvider & provider, EndpointId endpoint,
                            const DataModel::DeviceTypeEntry & expected)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    ASSERT_SUCCESS(provider.DeviceTypes(endpoint, builder));
    const auto deviceTypes = builder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    EXPECT_EQ(deviceTypes[0].deviceTypeId, expected.deviceTypeId);
    EXPECT_EQ(deviceTypes[0].deviceTypeRevision, expected.deviceTypeRevision);
}

void ExpectPowerSourceEndpointList(ProviderAttributeReader & reader, EndpointId powerSourceEndpoint,
                                   std::vector<EndpointId> expectedEndpoints)
{
    Clusters::PowerSource::Attributes::EndpointList::TypeInfo::DecodableType endpointList;
    ASSERT_SUCCESS(reader.Read(powerSourceEndpoint, Clusters::PowerSource::Id, Clusters::PowerSource::Attributes::EndpointList::Id,
                               endpointList));
    std::vector<EndpointId> actualEndpoints;
    auto epIter = endpointList.begin();
    while (epIter.Next())
    {
        actualEndpoints.push_back(epIter.GetValue());
    }
    EXPECT_SUCCESS(epIter.GetStatus());
    std::sort(actualEndpoints.begin(), actualEndpoints.end());
    std::sort(expectedEndpoints.begin(), expectedEndpoints.end());
    EXPECT_EQ(actualEndpoints, expectedEndpoints);
}

TEST_F(SmokeCoAlarmTestFixture, RegistersPeerPowerSourceEndpointWithMatchingEndpointList)
{
    ConsecutiveEndpointIdAllocator allocator;
    LoggingOnlySmokeCoAlarm device(mTimerDelegate, DefaultSmokeCoConfig());

    ASSERT_SUCCESS(device.Register(allocator, mProvider));
    ASSERT_SUCCESS(mProvider.Startup(mContext.ImContext()));

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
    ASSERT_SUCCESS(
        reader.Read(alarmEndpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id, partsList));
    auto partsIter = partsList.begin();
    EXPECT_FALSE(partsIter.Next());
    EXPECT_SUCCESS(partsIter.GetStatus());

    // By default (no powerSourceTagList provided), TagList feature is not enabled on the standalone Power Source endpoint.
    uint32_t descriptorFeatureMap = 0;
    ASSERT_SUCCESS(reader.Read(powerSourceEndpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::FeatureMap::Id,
                               descriptorFeatureMap));
    EXPECT_EQ(descriptorFeatureMap & to_underlying(Clusters::Descriptor::Feature::kTagList), 0u);

    // Verify PowerSource::EndpointList includes both the PowerSource endpoint itself and the SmokeCoAlarm endpoint.
    ExpectPowerSourceEndpointList(reader, powerSourceEndpoint, { powerSourceEndpoint, alarmEndpoint });

    ExpectSingleDeviceType(mProvider, alarmEndpoint, Device::Type::kSmokeCoAlarm);
    ExpectSingleDeviceType(mProvider, powerSourceEndpoint, Device::Type::kPowerSource);

    device.Unregister(mProvider);
    EXPECT_EQ(device.GetEndpointId(), kInvalidEndpointId);
    EXPECT_EQ(device.GetPowerSource().GetEndpointId(), kInvalidEndpointId);
    EXPECT_TRUE(GetEndpoints(mProvider).empty());
    EXPECT_SUCCESS(mProvider.Shutdown());
}

TEST_F(SmokeCoAlarmTestFixture, PropagatesBridgedParentAndForwardsPowerSourceTagList)
{
    constexpr EndpointId kBridgedNodeEndpoint          = 10;
    static const EndpointComposition::SemanticTag kTag = {
        .mfgCode     = DataModel::NullNullable,
        .namespaceID = to_underlying(Clusters::Globals::SemanticTagNamespaceId::kPosition),
        .tag         = static_cast<uint8_t>(Clusters::Globals::PositionTag::kMiddle),
    };
    ConsecutiveEndpointIdAllocator allocator;
    LoggingOnlySmokeCoAlarm device(mTimerDelegate, DefaultSmokeCoConfig(), Span(&kTag, 1));

    ASSERT_SUCCESS(device.Register(allocator, mProvider, EndpointComposition::WithParent(kBridgedNodeEndpoint)));
    ASSERT_SUCCESS(mProvider.Startup(mContext.ImContext()));

    const EndpointId alarmEndpoint       = device.GetEndpointId();
    const EndpointId powerSourceEndpoint = device.GetPowerSource().GetEndpointId();

    const auto endpoints = GetEndpoints(mProvider);
    ASSERT_EQ(endpoints.size(), 2u);
    for (const auto & entry : endpoints)
    {
        EXPECT_EQ(entry.parentId, kBridgedNodeEndpoint);
    }

    ProviderAttributeReader reader(mProvider);
    ExpectPowerSourceEndpointList(reader, powerSourceEndpoint, { powerSourceEndpoint, alarmEndpoint });

    Clusters::Descriptor::Attributes::TagList::TypeInfo::DecodableType tagList;
    ASSERT_SUCCESS(
        reader.Read(powerSourceEndpoint, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::TagList::Id, tagList));
    auto tagIter = tagList.begin();
    ASSERT_TRUE(tagIter.Next());
    EXPECT_EQ(tagIter.GetValue().namespaceID, to_underlying(Clusters::Globals::SemanticTagNamespaceId::kPosition));
    EXPECT_EQ(tagIter.GetValue().tag, static_cast<uint8_t>(Clusters::Globals::PositionTag::kMiddle));
    EXPECT_FALSE(tagIter.Next());
    EXPECT_SUCCESS(tagIter.GetStatus());

    device.Unregister(mProvider);
    EXPECT_SUCCESS(mProvider.Shutdown());
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
