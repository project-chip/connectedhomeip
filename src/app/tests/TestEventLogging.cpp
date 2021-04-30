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
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

namespace {

static const chip::NodeId kTestDeviceNodeId     = 0x18B4300000000001ULL;
static const chip::ClusterId kLivenessClusterId = 0x00000022;
static const uint32_t kLivenessChangeEvent      = 1;
static const chip::EndpointId kTestEndpointId   = 2;
static const uint64_t kLivenessDeviceStatus     = chip::TLV::ContextTag(1);
static const chip::Transport::AdminId gAdminId  = 0;
static chip::TransportMgr<chip::Transport::UDP> gTransportManager;
static chip::System::Layer gSystemLayer;

uint8_t gDebugEventBuffer[4096];
uint8_t gInfoEventBuffer[4096];
uint8_t gCritEventBuffer[4096];
chip::app::CircularEventBuffer gCircularEventBuffer[3];

chip::SecureSessionMgr gSessionManager;
chip::Messaging::ExchangeManager gExchangeManager;

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(gAdminId, kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    gSystemLayer.Init(nullptr);

    err = gSessionManager.Init(chip::kTestDeviceNodeId, &gSystemLayer, &gTransportManager, &admins);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = gExchangeManager.Init(&gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

void InitializeEventLogging()
{
    chip::app::LogStorageResources logStorageResources[] = {
        { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Debug },
        { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Info },
        { &gCritEventBuffer[0], sizeof(gCritEventBuffer), nullptr, 0, nullptr, chip::app::PriorityLevel::Critical },
    };

    chip::app::EventManagement::CreateEventManagement(
        &gExchangeManager, sizeof(logStorageResources) / sizeof(logStorageResources[0]), gCircularEventBuffer, logStorageResources);
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

    chip::TLV::Utilities::Count(reader, elementCount);
    printf("Found %zu elements\n", elementCount);
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

static void CheckLogState(nlTestSuite * apSuite, chip::app::EventManagement & aLogMgmt, size_t expectedNumEvents)
{
    CHIP_ERROR err;
    chip::TLV::TLVReader reader;
    size_t elementCount;
    chip::app::CircularEventBufferWrapper bufWrapper;
    err = aLogMgmt.GetEventReader(reader, chip::app::PriorityLevel::Debug, &bufWrapper);
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

    err = alogMgmt.FetchEventsSince(writer, priority, startingEventNumber);
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

static void CheckLogEventBasics(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::EventNumber eid1, eid2, eid3;
    chip::app::EventSchema schema = { kTestDeviceNodeId, kTestEndpointId, kLivenessClusterId, kLivenessChangeEvent,
                                      chip::app::PriorityLevel::Debug };
    chip::app::EventOptions options;
    TestEventGenerator testEventGenerator;

    InitializeEventLogging();
    options.mpEventSchema = &schema;
    options.mUrgent       = true;

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, &options, eid1);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 1);

    usleep(10000);
    testEventGenerator.SetStatus(1);
    err = logMgmt.LogEvent(&testEventGenerator, &options, eid2);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 2);

    usleep(10000);
    testEventGenerator.SetStatus(0);
    err = logMgmt.LogEvent(&testEventGenerator, &options, eid3);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    CheckLogState(apSuite, logMgmt, 3);

    PrintEventLog();

    NL_TEST_ASSERT(apSuite, (eid1 + 1) == eid2);
    NL_TEST_ASSERT(apSuite, (eid2 + 1) == eid3);

    // Verify that the readout supports the expected volume of events
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Debug, eid1, 3);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Debug, eid2, 2);
    CheckLogReadOut(apSuite, logMgmt, chip::app::PriorityLevel::Debug, eid3, 1);
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("CheckLogEventBasics", CheckLogEventBasics),
                NL_TEST_SENTINEL()
        };
// clang-format on
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

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestEventLogging)
