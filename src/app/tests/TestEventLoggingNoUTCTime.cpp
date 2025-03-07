/*
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <access/SubjectDescriptor.h>
#include <app/EventLoggingDelegate.h>
#include <app/EventLoggingTypes.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/LinkedList.h>
#include <lib/support/logging/Constants.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

static const chip::ClusterId kLivenessClusterId   = 0x00000022;
static const uint32_t kLivenessChangeEvent        = 1;
static const chip::EndpointId kTestEndpointId1    = 2;
static const chip::EndpointId kTestEndpointId2    = 3;
static const chip::TLV::Tag kLivenessDeviceStatus = chip::TLV::ContextTag(1);

static uint8_t gDebugEventBuffer[128];
static uint8_t gInfoEventBuffer[128];
static uint8_t gCritEventBuffer[128];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

class MockClock : public chip::System::Clock::ClockBase
{
public:
    MockClock(chip::System::Clock::ClockBase & aRealClock) : mRealClock(aRealClock)
    {
        chip::System::Clock::Internal::SetSystemClockForTesting(this);
    }

    ~MockClock() { chip::System::Clock::Internal::SetSystemClockForTesting(&mRealClock); }

    chip::System::Clock::Microseconds64 GetMonotonicMicroseconds64() override { return mRealClock.GetMonotonicMicroseconds64(); }
    chip::System::Clock::Milliseconds64 GetMonotonicMilliseconds64() override { return mRealClock.GetMonotonicMilliseconds64(); }

    CHIP_ERROR GetClock_RealTime(chip::System::Clock::Microseconds64 & aCurTime) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetClock_RealTimeMS(chip::System::Clock::Milliseconds64 & aCurTime) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetClock_RealTime(chip::System::Clock::Microseconds64 aNewCurTime) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

private:
    chip::System::Clock::ClockBase & mRealClock;
};

class TestEventLoggingNoUTCTime : public chip::Test::AppContext
{
public:
    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        AppContext::SetUpTestSuite();
        sClock.Emplace(chip::System::SystemClock());
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        sClock.ClearValue();
        AppContext::TearDownTestSuite();
    }

    // Performs setup for each individual test in the test suite
    void SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        AppContext::SetUp();
        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), MATTER_ARRAY_SIZE(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        AppContext::TearDown();
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
    static chip::Optional<MockClock> sClock;
};

chip::Optional<MockClock> TestEventLoggingNoUTCTime::sClock;

void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

void PrintEventLog()
{
    chip::TLV::TLVReader reader;
    chip::app::CircularEventBufferWrapper bufWrapper;
    chip::app::EventManagement::GetInstance().GetEventReader(reader, chip::app::PriorityLevel::Critical, &bufWrapper);
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

static void CheckLogState(chip::app::EventManagement & aLogMgmt, size_t expectedMinNumEvents, size_t expectedMaxNumEvents,
                          chip::app::PriorityLevel aPriority)
{

    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    EXPECT_EQ(aLogMgmt.GetEventReader(reader, aPriority, &bufWrapper), CHIP_NO_ERROR);

    EXPECT_EQ(chip::TLV::Utilities::Count(reader, elementCount, false), CHIP_NO_ERROR);

    EXPECT_GE(elementCount, expectedMinNumEvents);
    EXPECT_LE(elementCount, expectedMaxNumEvents);
    printf("elementCount vs [expectedMinNumEvents, expectedMaxNumEvents] : %u vs [%u, %u] \n",
           static_cast<unsigned int>(elementCount), static_cast<unsigned int>(expectedMinNumEvents),
           static_cast<unsigned int>(expectedMaxNumEvents));
}

static void CheckLogState(chip::app::EventManagement & aLogMgmt, size_t expectedNumEvents, chip::app::PriorityLevel aPriority)
{
    CheckLogState(aLogMgmt, expectedNumEvents, expectedNumEvents, aPriority);
}

static void CheckLogReadOut(chip::app::EventManagement & alogMgmt, chip::EventNumber startingEventNumber, size_t expectedNumEvents,
                            chip::SingleLinkedListNode<chip::app::EventPathParams> * clusterInfo)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVWriter writer;
    size_t eventCount = 0;

    chip::Platform::ScopedMemoryBuffer<uint8_t> backingStore;
    VerifyOrDie(backingStore.Alloc(1024));

    size_t totalNumElements;
    writer.Init(backingStore.Get(), 1024);
    err = alogMgmt.FetchEventsSince(writer, clusterInfo, startingEventNumber, eventCount, chip::Access::SubjectDescriptor{});
    EXPECT_TRUE(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);

    reader.Init(backingStore.Get(), writer.GetLengthWritten());

    EXPECT_EQ(chip::TLV::Utilities::Count(reader, totalNumElements, false), CHIP_NO_ERROR);

    printf("totalNumElements vs expectedNumEvents vs eventCount : %u vs %u vs %u \n", static_cast<unsigned int>(totalNumElements),
           static_cast<unsigned int>(expectedNumEvents), static_cast<unsigned int>(eventCount));
    EXPECT_EQ(totalNumElements, expectedNumEvents);
    EXPECT_EQ(totalNumElements, eventCount);
    reader.Init(backingStore.Get(), writer.GetLengthWritten());
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

class TestEventGenerator : public chip::app::EventLoggingDelegate
{
public:
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter)
    {
        chip::TLV::TLVType dataContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::EventDataIB::Tag::kData)),
                                                    chip::TLV::kTLVType_Structure, dataContainerType));
        ReturnErrorOnFailure(aWriter.Put(kLivenessDeviceStatus, mStatus));
        return aWriter.EndContainer(dataContainerType);
    }

    void SetStatus(int32_t aStatus) { mStatus = aStatus; }

private:
    int32_t mStatus;
};

TEST_F(TestEventLoggingNoUTCTime, TestCheckLogEventWithEvictToNextBuffer)
{
    chip::EventNumber eid1, eid2, eid3, eid4, eid5, eid6;
    chip::app::EventOptions options1;
    chip::app::EventOptions options2;
    TestEventGenerator testEventGenerator;

    options1.mPath                       = { kTestEndpointId1, kLivenessClusterId, kLivenessChangeEvent };
    options1.mPriority                   = chip::app::PriorityLevel::Info;
    options2.mPath                       = { kTestEndpointId2, kLivenessClusterId, kLivenessChangeEvent };
    options2.mPriority                   = chip::app::PriorityLevel::Info;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid1), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Info);
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid2), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 2, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 2, chip::app::PriorityLevel::Info);
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid3), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 3, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 3, chip::app::PriorityLevel::Info);
    // Start to copy info event to next buffer since current debug buffer is full and info event is higher priority
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid4), CHIP_NO_ERROR);
    // At this point depending on timing and hence event sizes we might have
    // either 3 or 4 events in the Debug buffer.
    CheckLogState(logMgmt, 3, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid5), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 3, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 5, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid6), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 3, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 6, chip::app::PriorityLevel::Info);

    PrintEventLog();

    EXPECT_EQ((eid1 + 1), eid2);
    EXPECT_EQ((eid2 + 1), eid3);
    EXPECT_EQ((eid3 + 1), eid4);
    EXPECT_EQ((eid4 + 1), eid5);
    EXPECT_EQ((eid5 + 1), eid6);

    chip::SingleLinkedListNode<chip::app::EventPathParams> paths[2];

    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    paths[1].mValue.mEndpointId = kTestEndpointId2;
    paths[1].mValue.mClusterId  = kLivenessClusterId;
    paths[1].mValue.mEventId    = kLivenessChangeEvent;

    // interested paths are path list, expect to retrieve all events for each particular interested path
    CheckLogReadOut(logMgmt, 0, 3, &paths[0]);
    CheckLogReadOut(logMgmt, 1, 2, &paths[0]);
    CheckLogReadOut(logMgmt, 2, 1, &paths[0]);
    CheckLogReadOut(logMgmt, 3, 3, &paths[1]);
    CheckLogReadOut(logMgmt, 4, 2, &paths[1]);
    CheckLogReadOut(logMgmt, 5, 1, &paths[1]);

    paths[0].mpNext = &paths[1];
    // interested paths are path list, expect to retrieve all events for those interested paths
    CheckLogReadOut(logMgmt, 0, 6, paths);

    chip::SingleLinkedListNode<chip::app::EventPathParams> pathsWithWildcard[2];
    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    // second path is wildcard path at default, expect to retrieve all events
    CheckLogReadOut(logMgmt, 0, 6, &pathsWithWildcard[1]);

    paths[0].mpNext = &paths[1];
    // first path is not wildcard, second path is wildcard path at default, expect to retrieve all events
    CheckLogReadOut(logMgmt, 0, 6, pathsWithWildcard);
}

TEST_F(TestEventLoggingNoUTCTime, TestCheckLogEventWithDiscardLowEvent)
{
    chip::EventNumber eid1, eid2, eid3, eid4, eid5, eid6;
    chip::app::EventOptions options;
    options.mPath     = { kTestEndpointId1, kLivenessClusterId, kLivenessChangeEvent };
    options.mPriority = chip::app::PriorityLevel::Debug;
    TestEventGenerator testEventGenerator;

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    CheckLogState(logMgmt, 0, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 0, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid1), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Info);
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid2), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 2, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 2, chip::app::PriorityLevel::Info);
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid3), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 3, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 3, chip::app::PriorityLevel::Info);
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid4), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Info);

    // Start to drop off debug event since debug event can only be saved in debug buffer

    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid5), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid6), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Debug);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Info);
}

} // namespace
