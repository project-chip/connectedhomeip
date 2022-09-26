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

/**
 *    @file
 *      This file implements a test for CHIP Interaction Model Fabric-scoped Event logging
 *
 */

#include <access/SubjectDescriptor.h>
#include <app/EventLoggingDelegate.h>
#include <app/EventLoggingTypes.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/ObjectList.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CHIPCounter.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/Constants.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

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

class TestContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);

        if (ctx->mEventCounter.Init(0) != CHIP_NO_ERROR)
        {
            return FAILURE;
        }

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(),
                                                          sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                          gCircularEventBuffer, logStorageResources, &ctx->mEventCounter);

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        chip::app::EventManagement::DestroyEventManagement();

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
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

static void CheckLogState(nlTestSuite * apSuite, chip::app::EventManagement & aLogMgmt, size_t expectedNumEvents,
                          chip::app::PriorityLevel aPriority)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    err = aLogMgmt.GetEventReader(reader, aPriority, &bufWrapper);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::TLV::Utilities::Count(reader, elementCount, false);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(apSuite, elementCount == expectedNumEvents);
    printf("elementCount vs expectedNumEvents : %u vs %u \n", static_cast<unsigned int>(elementCount),
           static_cast<unsigned int>(expectedNumEvents));
}

static void CheckLogReadOut(nlTestSuite * apSuite, chip::app::EventManagement & alogMgmt, chip::EventNumber startingEventNumber,
                            size_t expectedNumEvents, chip::app::ObjectList<chip::app::EventPathParams> * clusterInfo,
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
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);

    reader.Init(backingStore, writer.GetLengthWritten());

    err = chip::TLV::Utilities::Count(reader, totalNumElements, false);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    printf("totalNumElements vs expectedNumEvents vs eventCount : %u vs %u vs %u \n", static_cast<unsigned int>(totalNumElements),
           static_cast<unsigned int>(expectedNumEvents), static_cast<unsigned int>(eventCount));
    NL_TEST_ASSERT(apSuite, totalNumElements == expectedNumEvents && totalNumElements == eventCount);
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

static void CheckLogEventWithEvictToNextBuffer(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 1, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 2, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options1, eid3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Info);
    // Start to copy info event to next buffer since current debug buffer is full and info event is higher priority
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid4);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 4, chip::app::PriorityLevel::Info);

    PrintEventLog(chip::app::PriorityLevel::Debug);
    PrintEventLog(chip::app::PriorityLevel::Info);

    NL_TEST_ASSERT(apSuite, (eid1 + 1) == eid2);
    NL_TEST_ASSERT(apSuite, (eid2 + 1) == eid3);
    NL_TEST_ASSERT(apSuite, (eid3 + 1) == eid4);

    chip::app::ObjectList<chip::app::EventPathParams> paths[2];

    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    paths[1].mValue.mEndpointId = kTestEndpointId2;
    paths[1].mValue.mClusterId  = kLivenessClusterId;
    paths[1].mValue.mEventId    = kLivenessChangeEvent;

    chip::Access::SubjectDescriptor descriptor;
    descriptor.fabricIndex = 1;

    CheckLogReadOut(apSuite, logMgmt, 0, 3, &paths[0], descriptor);
    CheckLogReadOut(apSuite, logMgmt, 1, 2, &paths[0], descriptor);
    CheckLogReadOut(apSuite, logMgmt, 2, 1, &paths[0], descriptor);
    CheckLogReadOut(apSuite, logMgmt, 3, 0, &paths[1], descriptor);
    descriptor.fabricIndex = 2;
    CheckLogReadOut(apSuite, logMgmt, 3, 1, &paths[1], descriptor);

    paths[0].mpNext        = &paths[1];
    descriptor.fabricIndex = 1;

    CheckLogReadOut(apSuite, logMgmt, 0, 3, paths, descriptor);
    descriptor.fabricIndex = 2;
    CheckLogReadOut(apSuite, logMgmt, 0, 1, paths, descriptor);

    // Fabric event + wildcard test, only have one fabric-scoped event with fabric 2
    chip::app::ObjectList<chip::app::EventPathParams> pathsWithWildcard[2];
    paths[0].mValue.mEndpointId = kTestEndpointId1;
    paths[0].mValue.mClusterId  = kLivenessClusterId;

    CheckLogReadOut(apSuite, logMgmt, 0, 1, &pathsWithWildcard[1], descriptor);

    paths[0].mpNext = &paths[1];
    CheckLogReadOut(apSuite, logMgmt, 0, 1, pathsWithWildcard, descriptor);

    // Invalidate obsolete fabric-scope event

    // Invalidate 3 event with fabric 1
    descriptor.fabricIndex = 1;
    logMgmt.FabricRemoved(descriptor.fabricIndex);
    CheckLogReadOut(apSuite, logMgmt, 0, 0, &pathsWithWildcard[1], descriptor);

    // Invalidate 1 event with fabric 2
    descriptor.fabricIndex = 2;
    logMgmt.FabricRemoved(descriptor.fabricIndex);
    CheckLogReadOut(apSuite, logMgmt, 0, 0, &pathsWithWildcard[1], descriptor);
}
/**
 *   Test Suite. It lists all the test functions.
 */

const nlTest sTests[] = { NL_TEST_DEF("CheckLogEventWithEvictToNextBuffer", CheckLogEventWithEvictToNextBuffer),
                          NL_TEST_SENTINEL() };

// clang-format off
nlTestSuite sSuite =
{
    "TestFabricScopedEventLogging",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestFabricScopedEventLogging()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFabricScopedEventLogging)
