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
 *      This file implements a test for  CHIP Interaction Model Event logging
 *
 */

#include <app/ClusterInfo.h>
#include <app/EventLoggingDelegate.h>
#include <app/EventLoggingTypes.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

static const chip::NodeId kTestDeviceNodeId1      = 0x18B4300000000001ULL;
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

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(),
                                                          sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                          gCircularEventBuffer, logStorageResources);

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        chip::app::EventManagement::DestroyEventManagement();

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
    }
};

void SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

void PrintEventLog()
{
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    chip::app::EventManagement::GetInstance().GetEventReader(reader, chip::app::PriorityLevel::Debug, &bufWrapper);

    chip::TLV::Utilities::Count(reader, elementCount, false);
    printf("Found %zu elements\n", elementCount);
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
    printf("Num Events: %zu\n", elementCount);
}

static void CheckLogReadOut(nlTestSuite * apSuite, chip::app::EventManagement & alogMgmt, chip::app::PriorityLevel priority,
                            chip::EventNumber startingEventNumber, size_t expectedNumEvents, chip::app::ClusterInfo * clusterInfo)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVWriter writer;
    size_t eventCount = 0;
    uint8_t backingStore[1024];
    size_t totalNumElements;
    writer.Init(backingStore, 1024);
    err = alogMgmt.FetchEventsSince(writer, clusterInfo, priority, startingEventNumber, eventCount);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);

    reader.Init(backingStore, writer.GetLengthWritten());

    err = chip::TLV::Utilities::Count(reader, totalNumElements, false);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, totalNumElements == expectedNumEvents && totalNumElements == eventCount);
    reader.Init(backingStore, writer.GetLengthWritten());
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

class TestEventGenerator : public chip::app::EventLoggingDelegate
{
public:
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        err            = aWriter.Put(kLivenessDeviceStatus, mStatus);
        return err;
    }

    void SetStatus(int32_t aStatus) { mStatus = aStatus; }

private:
    int32_t mStatus;
};

static void CheckLogEventWithEvictToNextBuffer(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2, eid3, eid4, eid5, eid6;
    chip::app::EventSchema schema1 = { kTestDeviceNodeId1, kTestEndpointId1, kLivenessClusterId, kLivenessChangeEvent,
                                       chip::app::PriorityLevel::Info };
    chip::app::EventSchema schema2 = { kTestDeviceNodeId1, kTestEndpointId2, kLivenessClusterId, kLivenessChangeEvent,
                                       chip::app::PriorityLevel::Info };
    chip::app::EventOptions options1;
    chip::app::EventOptions options2;
    TestEventGenerator testEventGenerator;

    options1.mpEventSchema               = &schema1;
    options2.mpEventSchema               = &schema2;
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
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
    // Start to copy info event to next buffer since current debug buffer is full and info event is higher priority
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid4);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 4, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid5);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 5, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options2, eid6);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 6, chip::app::PriorityLevel::Info);

    PrintEventLog();

    NL_TEST_ASSERT(apSuite, (eid1 + 1) == eid2);
    NL_TEST_ASSERT(apSuite, (eid2 + 1) == eid3);
    NL_TEST_ASSERT(apSuite, (eid3 + 1) == eid4);
    NL_TEST_ASSERT(apSuite, (eid4 + 1) == eid5);
    NL_TEST_ASSERT(apSuite, (eid5 + 1) == eid6);

    chip::app::ClusterInfo testClusterInfo1;
    testClusterInfo1.mNodeId     = kTestDeviceNodeId1;
    testClusterInfo1.mEndpointId = kTestEndpointId1;
    testClusterInfo1.mClusterId  = kLivenessClusterId;
    chip::app::ClusterInfo testClusterInfo2;
    testClusterInfo2.mNodeId     = kTestDeviceNodeId1;
    testClusterInfo2.mEndpointId = kTestEndpointId2;
    testClusterInfo2.mClusterId  = kLivenessClusterId;
    testClusterInfo2.mEventId    = kLivenessChangeEvent;

    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid1, 3, &testClusterInfo1);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid2, 2, &testClusterInfo1);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid3, 1, &testClusterInfo1);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid4, 3, &testClusterInfo2);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid5, 2, &testClusterInfo2);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid6, 1, &testClusterInfo2);
}

static void CheckLogEventWithDiscardLowEvent(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2, eid3, eid4, eid5, eid6;
    chip::app::EventSchema schema = { kTestDeviceNodeId1, kTestEndpointId1, kLivenessClusterId, kLivenessChangeEvent,
                                      chip::app::PriorityLevel::Debug };
    chip::app::EventOptions options;
    TestEventGenerator testEventGenerator;

    options.mpEventSchema = &schema;

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
    // Start to drop off debug event since debug event can only be saved in debug buffer
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid4);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);

    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid5);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);

    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid6);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
}
/**
 *   Test Suite. It lists all the test functions.
 */

const nlTest sTests[] = { NL_TEST_DEF("CheckLogEventWithEvictToNextBuffer", CheckLogEventWithEvictToNextBuffer),
                          NL_TEST_DEF("CheckLogEventWithDiscardLowEvent", CheckLogEventWithDiscardLowEvent), NL_TEST_SENTINEL() };

// clang-format off
nlTestSuite sSuite =
{
    "EventLogging",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

int TestEventLogging()
{
    TestContext gContext;
    nlTestRunner(&sSuite, &gContext);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestEventLogging)
