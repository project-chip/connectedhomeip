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
#include <lib/support/logging/Constants.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

static uint8_t gDebugEventBuffer[2048];
static uint8_t gInfoEventBuffer[2048];
static uint8_t gCritEventBuffer[2048];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

class TestEventOverflow : public chip::Test::AppContext
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
        VerifyOrReturn(!HasFailure());

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

TEST_F(TestEventOverflow, TestCheckLogEventOverFlow)
{
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

        EXPECT_EQ(logMgmt.LogEvent(&testEventGenerator, options, eid), CHIP_NO_ERROR);
        if (eid > 0)
        {
            EXPECT_EQ(eid, oldEid + 1);
            oldEid = eid;
        }
    }
}

} // namespace
