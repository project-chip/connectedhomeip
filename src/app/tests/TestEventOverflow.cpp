/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/ErrorStr.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPCounter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/Constants.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

static uint8_t gDebugEventBuffer[2048];
static uint8_t gInfoEventBuffer[2048];
static uint8_t gCritEventBuffer[2048];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

class TestContext : public chip::Test::AppContext
{
public:
    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        ReturnErrorOnFailure(chip::Test::AppContext::SetUp());

        CHIP_ERROR err = CHIP_NO_ERROR;
        VerifyOrExit((err = mEventCounter.Init(0)) == CHIP_NO_ERROR,
                     ChipLogError(AppServer, "Init EventCounter failed: %" CHIP_ERROR_FORMAT, err.Format()));
        chip::app::EventManagement::CreateEventManagement(&GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &mEventCounter);

    exit:
        return err;
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        chip::app::EventManagement::DestroyEventManagement();
        chip::Test::AppContext::TearDown();
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

class TestEventGenerator : public chip::app::EventLoggingDelegate
{
public:
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter)
    {
        chip::TLV::TLVType dataContainerType;
        ReturnErrorOnFailure(aWriter.StartContainer(chip::TLV::ContextTag(chip::to_underlying(chip::app::EventDataIB::Tag::kData)),
                                                    chip::TLV::kTLVType_Structure, dataContainerType));
        ReturnErrorOnFailure(aWriter.Put(chip::TLV::ContextTag(1), static_cast<uint32_t>(1)));
        ReturnErrorOnFailure(aWriter.Put(chip::TLV::ContextTag(2), static_cast<uint32_t>(2)));
        return aWriter.EndContainer(dataContainerType);
    }
};

static void CheckLogEventOverFlow(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    chip::EventNumber oldEid = 0;
    chip::EventNumber eid    = 0;
    chip::app::EventOptions options;
    TestEventGenerator testEventGenerator;

    chip::EndpointId testEndpointId = 1;
    chip::ClusterId testClusterId   = 0x00000006;
    chip::EventId testEvent         = 1;
    options.mPath                   = { testEndpointId, testClusterId, testEvent };
    options.mPriority               = chip::app::PriorityLevel::Debug;

    chip::app::EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    int alternate                        = 0;
    for (int i = 0; i < 500; i++)
    {
        switch (alternate)
        {
        case 0:
            options.mPriority = chip::app::PriorityLevel::Critical;
            break;
        case 1:
            options.mPriority = chip::app::PriorityLevel::Debug;
            break;
        case 2:
            options.mPriority = chip::app::PriorityLevel::Critical;
            break;
        case 3:
            options.mPriority = chip::app::PriorityLevel::Debug;
            break;
        case 4:
            options.mPriority = chip::app::PriorityLevel::Critical;
            break;
        case 5:
            options.mPriority = chip::app::PriorityLevel::Debug;
            break;
        case 6:
            options.mPriority = chip::app::PriorityLevel::Critical;
            break;
        case 7:
            options.mPriority = chip::app::PriorityLevel::Debug;
            break;
        case 8:
            options.mPriority = chip::app::PriorityLevel::Critical;
            break;
        case 9:
            options.mPriority = chip::app::PriorityLevel::Debug;
            break;
        }
        alternate = i % 10;

        err = logMgmt.LogEvent(&testEventGenerator, options, eid);
        NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
        if (eid > 0)
        {
            NL_TEST_ASSERT(apSuite, eid == oldEid + 1);
            oldEid = eid;
        }
    }
}

const nlTest sTests[] = { NL_TEST_DEF("CheckLogEventOverFlow", CheckLogEventOverFlow), NL_TEST_SENTINEL() };

// clang-format off
nlTestSuite sSuite =
{
    "TestEventOverflow",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

int TestEventOverflow()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestEventOverflow)
