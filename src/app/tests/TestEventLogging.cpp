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
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <stack/Stack.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace {

class NoopTransportConfig : chip::TransportConfiguration
{
public:
    using transport = chip::TransportMgr<>;

    CHIP_ERROR Init(const chip::StackParameters & parameters, chip::Inet::InetLayer & inetLayer, chip::Ble::BleLayer * bleLayer)
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    chip::TransportMgrBase & Get() { return mTransportManager; }

private:
    transport mTransportManager;
};

static chip::Stack<NoopTransportConfig> gStack(chip::kTestDeviceNodeId);

static const chip::NodeId kTestDeviceNodeId     = 0x18B4300000000001ULL;
static const chip::ClusterId kLivenessClusterId = 0x00000022;
static const uint32_t kLivenessChangeEvent      = 1;
static const chip::EndpointId kTestEndpointId   = 2;
static const uint64_t kLivenessDeviceStatus     = chip::TLV::ContextTag(1);

static uint8_t gDebugEventBuffer[128];
static uint8_t gInfoEventBuffer[128];
static uint8_t gCritEventBuffer[128];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

void InitializeEventLogging()
{
    chip::app::LogStorageResources logStorageResources[] = {
        { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
        { &gCritEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
    };

    chip::app::EventManagement::CreateEventManagement(&gStack.GetExchangeManager(),
                                                      sizeof(logStorageResources) / sizeof(logStorageResources[0]),
                                                      gCircularEventBuffer, logStorageResources);
}

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
                            chip::EventNumber startingEventNumber, size_t expectedNumEvents)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVWriter writer;
    uint8_t backingStore[1024];
    size_t elementCount;
    writer.Init(backingStore, 1024);
    chip::app::ClusterInfo testClusterInfo;
    testClusterInfo.mEventId = 1;
    err                      = alogMgmt.FetchEventsSince(writer, &testClusterInfo, priority, startingEventNumber);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV);

    reader.Init(backingStore, writer.GetLengthWritten());

    err = chip::TLV::Utilities::Count(reader, elementCount, false);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, elementCount == expectedNumEvents);

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
    chip::app::EventSchema schema = { kTestDeviceNodeId, kTestEndpointId, kLivenessClusterId, kLivenessChangeEvent,
                                      chip::app::PriorityLevel::Info };
    chip::app::EventOptions options;
    TestEventGenerator testEventGenerator;

    options.mpEventSchema = &schema;

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 1, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 2, chip::app::PriorityLevel::Debug);
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3, chip::app::PriorityLevel::Debug);
    // Start to copy info event to next buffer since current debug buffer is full and info event is higher priority
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid4);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 4, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid5);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 5, chip::app::PriorityLevel::Info);

    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, options, eid6);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 6, chip::app::PriorityLevel::Info);

    PrintEventLog();

    NL_TEST_ASSERT(apSuite, (eid1 + 1) == eid2);
    NL_TEST_ASSERT(apSuite, (eid2 + 1) == eid3);
    NL_TEST_ASSERT(apSuite, (eid3 + 1) == eid4);
    NL_TEST_ASSERT(apSuite, (eid4 + 1) == eid5);
    NL_TEST_ASSERT(apSuite, (eid5 + 1) == eid6);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid1, 6);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid2, 5);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid3, 4);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid4, 3);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid5, 2);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Info, eid6, 1);
}

static void CheckLogEventWithDiscardLowEvent(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2, eid3, eid4, eid5, eid6;
    chip::app::EventSchema schema = { kTestDeviceNodeId, kTestEndpointId, kLivenessClusterId, kLivenessChangeEvent,
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
} // namespace

int TestEventLogging()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "EventLogging",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    NL_TEST_ASSERT(&theSuite, gStack.Init(chip::StackParameters()) == CHIP_NO_ERROR);
    InitializeEventLogging();
    nlTestRunner(&theSuite, nullptr);
    int result = nlTestRunnerStats(&theSuite);
    NL_TEST_ASSERT(&theSuite, gStack.Shutdown() == CHIP_NO_ERROR);

    return result;
}

CHIP_REGISTER_TEST_SUITE(TestEventLogging)
