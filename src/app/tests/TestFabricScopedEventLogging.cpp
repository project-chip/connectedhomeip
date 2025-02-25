/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

class TestFabricScopedEventLogging : public chip::Test::AppContext
{
public:
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
};

void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

void PrintEventLog(chip::app::PriorityLevel aPriorityLevel)
{
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    chip::app::EventManagement::GetInstance().GetEventReader(reader, aPriorityLevel, &bufWrapper);

    chip::TLV::Utilities::Count(reader, elementCount, false);
    printf("Found %u elements \n", static_cast<unsigned int>(elementCount));
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

static void CheckLogState(chip::app::EventManagement & aLogMgmt, size_t expectedNumEvents, chip::app::PriorityLevel aPriority)
{
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    EXPECT_EQ(aLogMgmt.GetEventReader(reader, aPriority, &bufWrapper), CHIP_NO_ERROR);

    EXPECT_EQ(chip::TLV::Utilities::Count(reader, elementCount, false), CHIP_NO_ERROR);

    EXPECT_EQ(elementCount, expectedNumEvents);
    printf("elementCount vs expectedNumEvents : %u vs %u \n", static_cast<unsigned int>(elementCount),
           static_cast<unsigned int>(expectedNumEvents));
}

static void CheckLogReadOut(chip::app::EventManagement & alogMgmt, chip::EventNumber startingEventNumber, size_t expectedNumEvents,
                            chip::SingleLinkedListNode<chip::app::EventPathParams> * clusterInfo,
                            const chip::Access::SubjectDescriptor & aSubjectDescriptor)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVWriter writer;
    size_t eventCount = 0;
    uint8_t backingStore[1024];
    size_t totalNumElements;
    writer.Init(backingStore, 1024);
    err = alogMgmt.FetchEventsSince(writer, clusterInfo, startingEventNumber, eventCount, aSubjectDescriptor);
    EXPECT_TRUE(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);

    reader.Init(backingStore, writer.GetLengthWritten());

    EXPECT_EQ(chip::TLV::Utilities::Count(reader, totalNumElements, false), CHIP_NO_ERROR);

    printf("totalNumElements vs expectedNumEvents vs eventCount : %u vs %u vs %u \n", static_cast<unsigned int>(totalNumElements),
           static_cast<unsigned int>(expectedNumEvents), static_cast<unsigned int>(eventCount));
    EXPECT_EQ(totalNumElements, expectedNumEvents);
    EXPECT_EQ(totalNumElements, eventCount);

    reader.Init(backingStore, writer.GetLengthWritten());
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

TEST_F(TestFabricScopedEventLogging, TestCheckLogEventWithEvictToNextBuffer)
{
    chip::EventNumber eid1, eid2, eid3, eid4;
    chip::app::EventOptions options1;
    chip::app::EventOptions options2;
    TestEventGenerator testEventGenerator;

    options1.mPath                       = { kTestEndpointId1, kLivenessClusterId, kLivenessChangeEvent };
    options1.mPriority                   = chip::app::PriorityLevel::Info;
    options1.mFabricIndex                = 1;
    options2.mPath                       = { kTestEndpointId2, kLivenessClusterId, kLivenessChangeEvent };
    options2.mPriority                   = chip::app::PriorityLevel::Info;
    options2.mFabricIndex                = 2;
    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid1), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 1, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid2), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 2, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(0);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options1, eid3), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 3, chip::app::PriorityLevel::Info);
    // Start to copy info event to next buffer since current debug buffer is full and info event is higher priority
    testEventGenerator.SetStatus(1);
    EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options2, eid4), CHIP_NO_ERROR);
    CheckLogState(logMgmt, 4, chip::app::PriorityLevel::Info);

    PrintEventLog(chip::app::PriorityLevel::Debug);
    PrintEventLog(chip::app::PriorityLevel::Info);

    EXPECT_EQ((eid1 + 1), eid2);
    EXPECT_EQ((eid2 + 1), eid3);
    EXPECT_EQ((eid3 + 1), eid4);

    chip::SingleLinkedListNode<chip::app::EventPathParams> paths[2];

    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    paths[1].mValue.mEndpointId = kTestEndpointId2;
    paths[1].mValue.mClusterId  = kLivenessClusterId;
    paths[1].mValue.mEventId    = kLivenessChangeEvent;

    chip::Access::SubjectDescriptor descriptor;
    descriptor.fabricIndex = 1;

    CheckLogReadOut(logMgmt, 0, 3, &paths[0], descriptor);
    CheckLogReadOut(logMgmt, 1, 2, &paths[0], descriptor);
    CheckLogReadOut(logMgmt, 2, 1, &paths[0], descriptor);
    CheckLogReadOut(logMgmt, 3, 0, &paths[1], descriptor);
    descriptor.fabricIndex = 2;
    CheckLogReadOut(logMgmt, 3, 1, &paths[1], descriptor);

    paths[0].mpNext        = &paths[1];
    descriptor.fabricIndex = 1;

    CheckLogReadOut(logMgmt, 0, 3, paths, descriptor);
    descriptor.fabricIndex = 2;
    CheckLogReadOut(logMgmt, 0, 1, paths, descriptor);

    // Fabric event + wildcard test, only have one fabric-scoped event with fabric 2
    chip::SingleLinkedListNode<chip::app::EventPathParams> pathsWithWildcard[2];
    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    CheckLogReadOut(logMgmt, 0, 1, &pathsWithWildcard[1], descriptor);

    paths[0].mpNext = &paths[1];
    CheckLogReadOut(logMgmt, 0, 1, pathsWithWildcard, descriptor);

    // Invalidate obsolete fabric-scope event

    // Invalidate 3 event with fabric 1
    descriptor.fabricIndex = 1;
    logMgmt.FabricRemoved(descriptor.fabricIndex);
    CheckLogReadOut(logMgmt, 0, 0, &pathsWithWildcard[1], descriptor);

    // Invalidate 1 event with fabric 2
    descriptor.fabricIndex = 2;
    logMgmt.FabricRemoved(descriptor.fabricIndex);
    CheckLogReadOut(logMgmt, 0, 0, &pathsWithWildcard[1], descriptor);
}

} // namespace
