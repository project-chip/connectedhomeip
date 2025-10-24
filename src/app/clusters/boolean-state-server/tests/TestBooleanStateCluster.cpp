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
#include <app/clusters/testing/ClusterTester.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/BooleanState/Attributes.h>
#include <clusters/BooleanState/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BooleanState;
using namespace chip::app::Clusters::BooleanState::Attributes;
using namespace chip::Test;

namespace {

struct TestBooleanStateCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(booleanState.Startup(context), CHIP_NO_ERROR); }

    void TearDown() override { booleanState.Shutdown(); }

    TestBooleanStateCluster() : context(testContext.Create()), booleanState(kRootEndpointId) {}

    chip::Test::TestServerClusterContext testContext;
    ServerClusterContext context;
    BooleanStateCluster booleanState;
};

} // namespace

TEST_F(TestBooleanStateCluster, AttributeTest)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(booleanState.Attributes(ConcreteClusterPath(kRootEndpointId, BooleanState::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    ASSERT_EQ(listBuilder.Append(Span(BooleanState::Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestBooleanStateCluster, ReadAttributeTest)
{
    ClusterTester tester(booleanState);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    bool stateValue{};
    ASSERT_EQ(tester.ReadAttribute(StateValue::Id, stateValue), CHIP_NO_ERROR);
}

TEST_F(TestBooleanStateCluster, StateValue)
{
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
}

TEST_F(TestBooleanStateCluster, EventGeneratedOnStateChange)
{
    // Ensure initial value is false, then change to true and expect an event
    EXPECT_EQ(booleanState.GetStateValue(), false);
    auto eventNumber = booleanState.SetStateValue(true);

    auto & logOnlyEvents = testContext.EventsGenerator();
    using EventType      = chip::app::Clusters::BooleanState::Events::StateChange::Type;

    // Lambda to verify the last emitted event metadata and payload
    auto verifyLastEvent = [&](bool expectedStateValue) {
        ASSERT_TRUE(eventNumber.has_value());
        EXPECT_EQ(eventNumber.value(), logOnlyEvents.CurrentEventNumber());
        EXPECT_EQ(logOnlyEvents.LastOptions().mPath,
                  ConcreteEventPath(kRootEndpointId, EventType::GetClusterId(), EventType::GetEventId()));
        chip::app::Clusters::BooleanState::Events::StateChange::DecodableType decodedEvent;
        ASSERT_EQ(logOnlyEvents.DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);
        EXPECT_EQ(decodedEvent.stateValue, expectedStateValue);
    };

    // Verify event with expected true value
    verifyLastEvent(true);

    // Now, change from true to false and expect an event
    eventNumber = booleanState.SetStateValue(false);

    // Verify event with expected false value
    verifyLastEvent(false);
}

TEST_F(TestBooleanStateCluster, NoEventWhenValueUnchanged)
{
    // Ensure initial value is false, then try to set false again and confirm no event occurs
    EXPECT_EQ(booleanState.GetStateValue(), false);

    // Get initial event count before attempting to set the same value
    auto & logOnlyEvents     = testContext.EventsGenerator();
    EventNumber initialCount = logOnlyEvents.CurrentEventNumber();

    // Re-set to the same value (false) and confirm no new event is generated
    auto firstEvent = booleanState.SetStateValue(false);
    EXPECT_FALSE(firstEvent.has_value());
    EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), initialCount);

    // Change from false -> true and confirm an event occurs
    auto secondEvent = booleanState.SetStateValue(true);
    EXPECT_TRUE(secondEvent.has_value());
    EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), initialCount + 1);

    // Re-set to the same value (true) and confirm no new event is generated
    EventNumber eventCountAfterChange = logOnlyEvents.CurrentEventNumber();

    auto thirdEvent = booleanState.SetStateValue(true);
    EXPECT_FALSE(thirdEvent.has_value());
    EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), eventCountAfterChange);
}
