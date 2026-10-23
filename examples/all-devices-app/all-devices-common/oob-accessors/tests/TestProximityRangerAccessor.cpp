/*
 *
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

#include <app/data-model/Encode.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ProximityRanging/Attributes.h>
#include <clusters/ProximityRanging/Enums.h>
#include <clusters/ProximityRanging/Structs.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/proximity-ranger/OOBAccessors.h>
#include <device/types/proximity-ranger/impl/LoggingProximityRanger.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <oob-accessors/InMemoryOOBAccessorRegistry.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <pw_unit_test/framework.h>

#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ProximityRanging;
using chip::Testing::ClusterTester;

namespace {

constexpr EndpointId kEndpoint = 1;

constexpr uint8_t kTagEndpointId  = 1;
constexpr uint8_t kTagConstraints = 2;

Structs::RangingConstraintStruct::Type SetupConstraint(RangingTechEnum technology, RangingRoleEnum role)
{
    Structs::RangingConstraintStruct::Type entry;
    entry.technology = technology;
    entry.role       = role;
    return entry;
}

class TestProximityRangerAccessor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        EXPECT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.Register(kEndpoint, mProvider), CHIP_NO_ERROR);
        EXPECT_EQ(mDevice.ProximityRangingCluster().Startup(mClusterContext.Get()), CHIP_NO_ERROR);
        EXPECT_EQ(mRegistry.Register(std::make_unique<ProximityRangerOOBAccessor>(mDevice, kEndpoint)), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mDevice.ProximityRangingCluster().Shutdown(ClusterShutdownType::kClusterShutdown);
        mDevice.Unregister(mProvider);
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }

    // Encode { 1: endpoint, 2: [constraints] } and dispatch through the registry.
    CHIP_ERROR SendSetRangingConstraints(EndpointId endpoint, Span<const Structs::RangingConstraintStruct::Type> constraints)
    {
        uint8_t buffer[128];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagEndpointId), endpoint));
        TLV::TLVType array;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(kTagConstraints), TLV::kTLVType_Array, array));
        for (const auto & entry : constraints)
        {
            ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), entry));
        }
        ReturnErrorOnFailure(writer.EndContainer(array));
        ReturnErrorOnFailure(writer.EndContainer(outer));
        ReturnErrorOnFailure(writer.Finalize());
        return mRegistry.HandleAction("SetRangingConstraints"_span, ByteSpan(buffer, writer.GetLengthWritten()));
    }

    // Reads the constraints attribute back and returns the decoded entries.
    std::vector<Structs::RangingConstraintStruct::DecodableType> ReadPublishedConstraints()
    {
        std::vector<Structs::RangingConstraintStruct::DecodableType> out;
        ClusterTester tester(mDevice.ProximityRangingCluster());
        Attributes::RangingConstraints::TypeInfo::DecodableType list;
        EXPECT_EQ(tester.ReadAttribute(Attributes::RangingConstraints::Id, list), CHIP_NO_ERROR);
        auto iter = list.begin();
        while (iter.Next())
        {
            out.push_back(iter.GetValue());
        }
        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        return out;
    }

    DefaultTimerDelegate mTimerDelegate;
    Testing::TestServerClusterContext mClusterContext;
    CodeDrivenDataModelProvider mProvider{ mClusterContext.StorageDelegate(), mClusterContext.AttributePersistenceProvider() };
    LoggingProximityRanger mDevice{ mTimerDelegate, mClusterContext.StorageDelegate() };
    InMemoryOOBAccessorRegistry mRegistry;
};

// Decoding and positive adatper routing: constraint entries are decoded from TLV, assigned to their
// technology adapter and read back in adapter order
TEST_F(TestProximityRangerAccessor, DecodesAndRoutesConstraints)
{
    EXPECT_TRUE(ReadPublishedConstraints().empty());

    auto wifi = SetupConstraint(RangingTechEnum::kWiFiRoundTripTimeRanging, RangingRoleEnum::kWiFiSubscriberRole);
    wifi.minRangingInterval.SetValue(10);
    auto ble = SetupConstraint(RangingTechEnum::kBLEBeaconRSSIRanging, RangingRoleEnum::kBLEScanningRole);
    ble.enabled.SetValue(false);
    // Sent in reverse adapter order so the read back validates routing per adapter.
    const Structs::RangingConstraintStruct::Type entries[] = { wifi, ble };

    EXPECT_EQ(SendSetRangingConstraints(kEndpoint, Span<const Structs::RangingConstraintStruct::Type>(entries)), CHIP_NO_ERROR);

    // read back published constraints expecting adapter order
    auto published = ReadPublishedConstraints();
    ASSERT_EQ(published.size(), 2u);
    EXPECT_EQ(published[0].technology, RangingTechEnum::kBLEBeaconRSSIRanging);
    ASSERT_TRUE(published[0].enabled.HasValue());
    EXPECT_FALSE(published[0].enabled.Value());
    EXPECT_EQ(published[1].technology, RangingTechEnum::kWiFiRoundTripTimeRanging);
    ASSERT_TRUE(published[1].minRangingInterval.HasValue());
    EXPECT_EQ(published[1].minRangingInterval.Value(), 10u);

    // Empty constraints list clears every adapter.
    EXPECT_EQ(SendSetRangingConstraints(kEndpoint, Span<const Structs::RangingConstraintStruct::Type>()), CHIP_NO_ERROR);
    EXPECT_TRUE(ReadPublishedConstraints().empty());
}

// Endpoint filtering: a payload for wrong endpoint is rejected
TEST_F(TestProximityRangerAccessor, IgnoresOtherEndpoints)
{
    const Structs::RangingConstraintStruct::Type entries[] = {
        SetupConstraint(RangingTechEnum::kBLEBeaconRSSIRanging, RangingRoleEnum::kBLEBeaconRole),
    };
    EXPECT_EQ(SendSetRangingConstraints(kEndpoint + 1, Span<const Structs::RangingConstraintStruct::Type>(entries)),
              CHIP_ERROR_NOT_FOUND);
    EXPECT_TRUE(ReadPublishedConstraints().empty());
}

// Adapter negative routing: setting up constraints for a technology with adapter outside enum range is rejected
TEST_F(TestProximityRangerAccessor, RejectsTechnologyWithoutAdapter)
{
    const Structs::RangingConstraintStruct::Type entries[] = {
        SetupConstraint(RangingTechEnum::kBLEBeaconRSSIRanging, RangingRoleEnum::kBLEBeaconRole),
        SetupConstraint(static_cast<RangingTechEnum>(0x7F), RangingRoleEnum::kWiFiPublisherRole),
    };
    EXPECT_EQ(SendSetRangingConstraints(kEndpoint, Span<const Structs::RangingConstraintStruct::Type>(entries)),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_TRUE(ReadPublishedConstraints().empty());
}

} // namespace
