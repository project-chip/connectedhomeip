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
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>

#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

using StartUpEventType              = chip::app::Clusters::BasicInformation::Events::StartUp::Type;
using AccessControlEntryChangedType = chip::app::Clusters::AccessControl::Events::AccessControlEntryChanged::Type;

constexpr uint32_t kFakeSoftwareVersion = 0x1234abcd;

/// Keeps the "last event" in-memory to allow tests to validate
/// that event writing and encoding worked.
class LogOnlyEvents : public EventsGenerator
{
public:
    CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventContentWriter, const EventOptions & options,
                             EventNumber & generatedEventNumber) override
    {
        TLV::TLVWriter writer;
        TLV::TLVType outerType;
        writer.Init(mLastEventEncodeBuffer);

        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(eventContentWriter->WriteEvent(writer));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        mLastEncodedSpan = ByteSpan(mLastEventEncodeBuffer, writer.GetLengthWritten());

        mLastOptions         = options;
        generatedEventNumber = ++mCurrentEventNumber;

        return CHIP_NO_ERROR;
    }

    EventNumber CurrentEventNumber() const { return mCurrentEventNumber; }
    const EventOptions & LastOptions() const { return mLastOptions; }
    ByteSpan LastWrittenEvent() const { return mLastEncodedSpan; }

    // This relies on the default encoding of events which uses
    // DataModel::Encode on a EventDataIB::Tag::kData
    template <typename T>
    CHIP_ERROR DecodeLastEvent(T & dest)
    {
        // attempt to decode the last encoded event
        TLV::TLVReader reader;
        TLV::TLVType outerType;

        reader.Init(LastWrittenEvent());

        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.EnterContainer(outerType));

        ReturnErrorOnFailure(reader.Next()); // MUST be positioned on the first element
        ReturnErrorOnFailure(DataModel::Decode(reader, dest));

        ReturnErrorOnFailure(reader.ExitContainer(outerType));

        return CHIP_NO_ERROR;
    }

private:
    EventNumber mCurrentEventNumber = 0;
    EventOptions mLastOptions;
    uint8_t mLastEventEncodeBuffer[128];
    ByteSpan mLastEncodedSpan;
};

} // namespace

TEST(TestInteractionModelEventEmitting, TestBasicType)
{
    LogOnlyEvents logOnlyEvents;
    EventsGenerator * events = &logOnlyEvents;

    StartUpEventType event{ kFakeSoftwareVersion };

    std::optional<EventNumber> n1 = events->GenerateEvent(event, 0 /* EndpointId */);

    ASSERT_EQ(n1, logOnlyEvents.CurrentEventNumber());
    ASSERT_EQ(logOnlyEvents.LastOptions().mPath,
              ConcreteEventPath(0 /* endpointId */, StartUpEventType::GetClusterId(), StartUpEventType::GetEventId()));

    chip::app::Clusters::BasicInformation::Events::StartUp::DecodableType decoded_event;
    CHIP_ERROR err = logOnlyEvents.DecodeLastEvent(decoded_event);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(decoded_event.softwareVersion, kFakeSoftwareVersion);

    std::optional<EventNumber> n2 = events->GenerateEvent(event, /* endpointId = */ 1);
    ASSERT_EQ(n2, logOnlyEvents.CurrentEventNumber());

    ASSERT_EQ(logOnlyEvents.LastOptions().mPath,
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

    ASSERT_EQ(n1, logOnlyEvents.CurrentEventNumber());
    ASSERT_EQ(logOnlyEvents.LastOptions().mPath,
              ConcreteEventPath(0 /* endpointId */, AccessControlEntryChangedType::GetClusterId(),
                                AccessControlEntryChangedType::GetEventId()));

    chip::app::Clusters::AccessControl::Events::AccessControlEntryChanged::DecodableType decoded_event;
    CHIP_ERROR err = logOnlyEvents.DecodeLastEvent(decoded_event);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(decoded_event.adminNodeID.ValueOr(0), kTestNodeId);
    ASSERT_EQ(decoded_event.adminPasscodeID.ValueOr(0), kTestPasscode);
    ASSERT_EQ(decoded_event.fabricIndex, kTestFabricIndex);
}
