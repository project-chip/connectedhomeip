/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/boolean-state-server/boolean-state-cluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/BooleanState/Attributes.h>
#include <clusters/BooleanState/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanState;
using namespace chip::app::Clusters::BooleanState::Attributes;

namespace {

struct TestBooleanStateCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

constexpr size_t kBooleanStateFixedClusterCount = 2;

class BooleanStateClusterTest
{
    BooleanStateCluster booleanState;

public:
    template <typename... Args>
    BooleanStateClusterTest(Args &&... args) : booleanState(std::forward<Args>(args)...)
    {}

    template <typename F>
    void Check(F check)
    {
        check(booleanState);
    }
};

class ReadAttribute
{
    BooleanStateCluster & mCluster;
    DataModel::ActionReturnStatus mStatus;

public:
    ReadAttribute(BooleanStateCluster & cluster) : mCluster(cluster), mStatus(CHIP_NO_ERROR) {}

    DataModel::ActionReturnStatus GetStatus() const { return mStatus; }

    void operator()(DataModel::ReadAttributeRequest & request)
    {
        Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
        ASSERT_NE(buffer.Alloc(1024).Get(), nullptr);

        AttributeReportIBs::Builder attributeReportIBsBuilder;
        TLV::TLVWriter reportWriter;
        reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
        ASSERT_EQ(attributeReportIBsBuilder.Init(&reportWriter), CHIP_NO_ERROR);

        AttributeValueEncoder encoder(attributeReportIBsBuilder, Access::SubjectDescriptor{}, request.path, 0 /* dataVersion */);

        mStatus = mCluster.ReadAttribute(request, encoder);
    }
};

} // namespace

TEST_F(TestBooleanStateCluster, AttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
            ASSERT_EQ(booleanState.Attributes(ConcreteClusterPath(endpoint, BooleanState::Id), attributes), CHIP_NO_ERROR);

            ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
            AttributeListBuilder listBuilder(expected);
            ASSERT_EQ(listBuilder.Append(Span(BooleanState::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
            ASSERT_TRUE(Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
        });
    }
}

TEST_F(TestBooleanStateCluster, ReadAttributeTest)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(booleanState.Startup(context.Get()), CHIP_NO_ERROR);

            DataModel::ReadAttributeRequest request;
            request.path.mEndpointId  = endpoint;
            request.path.mClusterId   = BooleanState::Id;
            request.path.mAttributeId = Globals::Attributes::ClusterRevision::Id;

            ReadAttribute readAttribute(booleanState);
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = FeatureMap::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = StateValue::Id;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsSuccess());

            request.path.mAttributeId = 0xFFFF;
            readAttribute(request);
            EXPECT_TRUE(readAttribute.GetStatus().IsError());
        });
    }
}

TEST_F(TestBooleanStateCluster, StateValue)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([](BooleanStateCluster & booleanState) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(booleanState.Startup(context.Get()), CHIP_NO_ERROR);

            bool stateValue  = false;
            auto eventNumber = booleanState.SetStateValue(stateValue);
            auto stateVal    = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);
            EXPECT_FALSE(eventNumber.has_value());

            stateValue  = true;
            eventNumber = booleanState.SetStateValue(stateValue);
            stateVal    = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);
            EXPECT_TRUE(eventNumber.has_value());

            stateValue  = true;
            eventNumber = booleanState.SetStateValue(stateValue);
            stateVal    = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);
            EXPECT_FALSE(eventNumber.has_value());

            stateValue  = false;
            eventNumber = booleanState.SetStateValue(stateValue);
            stateVal    = booleanState.GetStateValue();
            EXPECT_EQ(stateVal, stateValue);
            EXPECT_TRUE(eventNumber.has_value());
        });
    }
}

TEST_F(TestBooleanStateCluster, EventGeneratedOnStateChange)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(booleanState.Startup(context.Get()), CHIP_NO_ERROR);

            // Ensure initial value is false, then change to true and expect an event
            EXPECT_EQ(booleanState.GetStateValue(), false);
            auto eventNumber = booleanState.SetStateValue(true);
            EXPECT_TRUE(eventNumber.has_value());

            // Validate the last emitted event metadata and payload
            auto & logOnlyEvents = context.EventsGenerator();
            EXPECT_EQ(eventNumber.value(), logOnlyEvents.CurrentEventNumber());

            using EventType = chip::app::Clusters::BooleanState::Events::StateChange::Type;
            EXPECT_EQ(logOnlyEvents.LastOptions().mPath,
                      ConcreteEventPath(endpoint, EventType::GetClusterId(), EventType::GetEventId()));

            // Decode the last event and verify its contents
            chip::app::Clusters::BooleanState::Events::StateChange::DecodableType decodedEvent;
            ASSERT_EQ(logOnlyEvents.DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);
            EXPECT_TRUE(decodedEvent.stateValue);
        });
    }
}

TEST_F(TestBooleanStateCluster, NoEventWhenValueUnchanged)
{
    for (EndpointId endpoint = 0; endpoint < kBooleanStateFixedClusterCount; ++endpoint)
    {
        BooleanStateClusterTest(endpoint).Check([&](BooleanStateCluster & booleanState) {
            chip::Test::TestServerClusterContext context;
            EXPECT_EQ(booleanState.Startup(context.Get()), CHIP_NO_ERROR);

            // Ensure initial value is false, then try to set false again and confirm no event occurs
            EXPECT_EQ(booleanState.GetStateValue(), false);

            // Get initial event count before attempting to set the same value
            auto & logOnlyEvents     = context.EventsGenerator();
            EventNumber initialCount = logOnlyEvents.CurrentEventNumber();

            // Re-set to the same value (false) and confirm no new event is generated
            auto firstEvent = booleanState.SetStateValue(false);
            EXPECT_FALSE(firstEvent.has_value());
            EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), initialCount);

            // Change from false -> true and confirm an event occurs
            auto secondEvent = booleanState.SetStateValue(true);
            EXPECT_TRUE(secondEvent.has_value());
            EXPECT_GT(logOnlyEvents.CurrentEventNumber(), initialCount);

            // Re-set to the same value (true) and confirm no new event is generated
            EventNumber before = logOnlyEvents.CurrentEventNumber();

            auto thirdEvent = booleanState.SetStateValue(true);
            EXPECT_FALSE(thirdEvent.has_value());
            EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), before);
        });
    }
}
