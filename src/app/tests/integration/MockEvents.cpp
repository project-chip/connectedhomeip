/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This file declares mock event generator and example
 *
 */

#include "MockEvents.h"
#include "common.h"
#include <app/EventLoggingTypes.h>
#include <app/EventManagement.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>
#include <transport/SecureSessionMgr.h>

static uint64_t kLivenessDeviceStatus = chip::TLV::ContextTag(1);
static bool gMockEventStop            = false;
static bool gEventIsStopped           = false;

EventGenerator::EventGenerator(size_t aNumStates, size_t aInitialState) : mNumStates(aNumStates), mState(aInitialState) {}

size_t EventGenerator::GetNumStates()
{
    return mNumStates;
}

LivenessEventGenerator::LivenessEventGenerator(void) : EventGenerator(10, 0) {}

void LivenessEventGenerator::Generate(void)
{
    // Scenario: monitoring liveness for two devices -- self and remote.  Remote device goes offline and returns.
    switch (mState)
    {
    case 0:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 1:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 2:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 3:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_UNREACHABLE);
        break;

    case 4:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 5:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_REBOOTING);
        break;

    case 6:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 7:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 8:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    case 9:
        LogLiveness(chip::kTestDeviceNodeId, kTestEndpointId, LIVENESS_DEVICE_STATUS_ONLINE);
        break;

    default:
        mState = 0;
    }

    mState = (mState + 1) % mNumStates;
}

CHIP_ERROR LivenessEventGenerator::WriteEvent(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = aWriter.Put(kLivenessDeviceStatus, mStatus);
    return err;
}

chip::EventNumber LivenessEventGenerator::LogLiveness(chip::NodeId aNodeId, chip::EndpointId aEndpointId,
                                                      LivenessDeviceStatus aStatus)
{
    chip::app::EventManagement & logManager = chip::app::EventManagement::GetInstance();
    chip::EventNumber number                = 0;
    chip::app::EventSchema schema           = {
        aNodeId, aEndpointId, kTestClusterId, kLivenessChangeEvent, chip::app::PriorityLevel::Critical,
    };
    chip::app::EventOptions options;
    mStatus               = static_cast<int32_t>(aStatus);
    options.mpEventSchema = &schema;
    logManager.LogEvent(this, options, number);
    return number;
}

MockEventGenerator * MockEventGenerator::GetInstance(void)
{
    static MockEventGeneratorImpl gMockEventGenerator;
    return &gMockEventGenerator;
}

MockEventGeneratorImpl::MockEventGeneratorImpl(void) :
    mpExchangeMgr(nullptr), mTimeBetweenEvents(0), mEventWraparound(false), mpEventGenerator(nullptr), mEventsLeft(0)
{}

CHIP_ERROR MockEventGeneratorImpl::Init(chip::Messaging::ExchangeManager * apExchangeMgr, EventGenerator * apEventGenerator,
                                        uint32_t aDelayBetweenEvents, bool aWraparound)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    mpExchangeMgr      = apExchangeMgr;
    mpEventGenerator   = apEventGenerator;
    mTimeBetweenEvents = aDelayBetweenEvents;
    mEventWraparound   = aWraparound;

    if (mEventWraparound)
        mEventsLeft = INT32_MAX;
    else
        mEventsLeft = mpEventGenerator->GetNumStates();

    if (mTimeBetweenEvents != 0)
        mpExchangeMgr->GetSessionMgr()->SystemLayer()->StartTimer(mTimeBetweenEvents, HandleNextEvent, this);

    return err;
}

void MockEventGeneratorImpl::HandleNextEvent(chip::System::Layer * apSystemLayer, void * apAppState, CHIP_ERROR aErr)
{
    MockEventGeneratorImpl * generator = static_cast<MockEventGeneratorImpl *>(apAppState);
    if (gMockEventStop)
    {
        gEventIsStopped = true;
        apSystemLayer->CancelTimer(HandleNextEvent, generator);
    }
    else
    {
        generator->mpEventGenerator->Generate();
        generator->mEventsLeft--;
        if ((generator->mEventWraparound) || (generator->mEventsLeft > 0))
        {
            apSystemLayer->StartTimer(generator->mTimeBetweenEvents, HandleNextEvent, generator);
        }
    }
}

void MockEventGeneratorImpl::SetEventGeneratorStop()
{
    gMockEventStop = true;

    // If the timer is running, make it expire right away.
    // This helps quit the standalone app in an orderly way without
    // spurious leaked timers.
    if (mTimeBetweenEvents != 0)
        mpExchangeMgr->GetSessionMgr()->SystemLayer()->StartTimer(0, HandleNextEvent, this);
}

bool MockEventGeneratorImpl::IsEventGeneratorStopped()
{
    if (gEventIsStopped)
    {
        gMockEventStop  = false;
        gEventIsStopped = false;
        return true;
    }
    else
    {
        return false;
    }
}
