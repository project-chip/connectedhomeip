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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model-provider/EventsGenerator.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>

#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

using StartUpEventType              = chip::app::Clusters::BasicInformation::Events::StartUp::Type;
using AccessControlEntryChangedType = chip::app::Clusters::AccessControl::Events::AccessControlEntryChanged::Type;
using chip::Testing::LogOnlyEvents;

constexpr uint32_t kFakeSoftwareVersion = 0x1234abcd;

} // namespace

TEST(TestInteractionModelEventEmitting, TestBasicType)
{
    LogOnlyEvents logOnlyEvents;
    EventsGenerator * events = &logOnlyEvents;

    StartUpEventType event{ kFakeSoftwareVersion };

    std::optional<EventNumber> n1 = events->GenerateEvent(event, 0 /* EndpointId */);

    auto eventInfo = logOnlyEvents.GetNextEvent();
    ASSERT_TRUE(eventInfo.has_value());
    ASSERT_EQ(n1, eventInfo->eventNumber);   // NOLINT(bugprone-unchecked-optional-access)
    ASSERT_EQ(eventInfo->eventOptions.mPath, // NOLINT(bugprone-unchecked-optional-access)
              ConcreteEventPath(0 /* endpointId */, StartUpEventType::GetClusterId(), StartUpEventType::GetEventId()));

    chip::app::Clusters::BasicInformation::Events::StartUp::DecodableType decoded_event;
    CHIP_ERROR err = eventInfo->GetEventData(decoded_event); // NOLINT(bugprone-unchecked-optional-access)

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(decoded_event.softwareVersion, kFakeSoftwareVersion);

    std::optional<EventNumber> n2 = events->GenerateEvent(event, /* endpointId = */ 1);
    eventInfo                     = logOnlyEvents.GetNextEvent();
    ASSERT_TRUE(eventInfo.has_value());
    ASSERT_EQ(n2, eventInfo->eventNumber); // NOLINT(bugprone-unchecked-optional-access)

    ASSERT_EQ(eventInfo->eventOptions.mPath, // NOLINT(bugprone-unchecked-optional-access)
              ConcreteEventPath(1 /* endpointId */, StartUpEventType::GetClusterId(), StartUpEventType::GetEventId()));
}

TEST(TestInteractionModelEventEmitting, TestFabricScoped)
{
    constexpr NodeId kTestNodeId           = 0x12ab;
    constexpr uint16_t kTestPasscode       = 12345;
    constexpr FabricIndex kTestFabricIndex = kMinValidFabricIndex + 10;
    static_assert(kTestFabricIndex != kUndefinedFabricIndex);

    LogOnlyEvents logOnlyEvents;
    EventsGenerator * events = &logOnlyEvents;

    AccessControlEntryChangedType event;
    event.adminNodeID     = chip::app::DataModel::MakeNullable(kTestNodeId);
    event.adminPasscodeID = chip::app::DataModel::MakeNullable(kTestPasscode);

    std::optional<EventNumber> n1 = events->GenerateEvent(event, 0 /* EndpointId */);
    // encoding without a fabric ID MUST fail for fabric events
    ASSERT_FALSE(n1.has_value());

    event.fabricIndex = kTestFabricIndex;
    n1                = events->GenerateEvent(event, /* endpointId = */ 0);

    auto eventInfo = logOnlyEvents.GetNextEvent();
    ASSERT_EQ(n1, eventInfo->eventNumber);   // NOLINT(bugprone-unchecked-optional-access)
    ASSERT_EQ(eventInfo->eventOptions.mPath, // NOLINT(bugprone-unchecked-optional-access)
              ConcreteEventPath(0 /* endpointId */, AccessControlEntryChangedType::GetClusterId(),
                                AccessControlEntryChangedType::GetEventId()));

    chip::app::Clusters::AccessControl::Events::AccessControlEntryChanged::DecodableType decoded_event;
    CHIP_ERROR err = eventInfo->GetEventData(decoded_event); // NOLINT(bugprone-unchecked-optional-access)

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(decoded_event.adminNodeID.ValueOr(0), kTestNodeId);
    ASSERT_EQ(decoded_event.adminPasscodeID.ValueOr(0), kTestPasscode);
    ASSERT_EQ(decoded_event.fabricIndex, kTestFabricIndex);
}
