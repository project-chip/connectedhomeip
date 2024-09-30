/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
#pragma once

#include <app/EventLoggingDelegate.h>
#include <app/InteractionModelEngine.h>
#include <lib/core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>

class EventGenerator : public chip::app::EventLoggingDelegate
{
public:
    virtual void Generate(void) = 0;
    virtual size_t GetNumStates(void);
    virtual ~EventGenerator() = default;

protected:
    EventGenerator(size_t aNumStates, size_t aInitialState);
    size_t mNumStates;
    size_t mState;
};

class MockEventGenerator
{
public:
    static MockEventGenerator * GetInstance(void);
    virtual CHIP_ERROR Init(chip::Messaging::ExchangeManager * apExchangeMgr, EventGenerator * apEventGenerator,
                            uint32_t aDelayBetweenEvents, bool aWraparound) = 0;
    virtual void SetEventGeneratorStop()                                    = 0;
    virtual bool IsEventGeneratorStopped()                                  = 0;
    virtual ~MockEventGenerator()                                           = default;
};

class MockEventGeneratorImpl : public MockEventGenerator
{
public:
    MockEventGeneratorImpl(void);
    CHIP_ERROR Init(chip::Messaging::ExchangeManager * apExchangeMgr, EventGenerator * apEventGenerator,
                    uint32_t aDelayBetweenEvents, bool aWraparound);
    void SetEventGeneratorStop();
    bool IsEventGeneratorStopped();

private:
    static void HandleNextEvent(chip::System::Layer * apSystemLayer, void * apAppState);
    chip::Messaging::ExchangeManager * mpExchangeMgr;
    uint32_t mTimeBetweenEvents; //< delay, in miliseconds, between events.
    bool mEventWraparound;       //< does the event generator run indefinitely, or does it stop after iterating through its states
    EventGenerator * mpEventGenerator; //< the event generator to use
    size_t mEventsLeft;
};

enum LivenessDeviceStatus
{
    LIVENESS_DEVICE_STATUS_UNSPECIFIED    = 0, /// Device status is unspecified
    LIVENESS_DEVICE_STATUS_ONLINE         = 1, /// Device is sending messages
    LIVENESS_DEVICE_STATUS_UNREACHABLE    = 2, /// Device is not reachable over network
    LIVENESS_DEVICE_STATUS_UNINITIALIZED  = 3, /// Device has not been initialized
    LIVENESS_DEVICE_STATUS_REBOOTING      = 4, /// Device being rebooted
    LIVENESS_DEVICE_STATUS_UPGRADING      = 5, /// Device offline while upgrading
    LIVENESS_DEVICE_STATUS_SCHEDULED_DOWN = 6  /// Device on a scheduled downtime
};

class LivenessEventGenerator : public EventGenerator
{
public:
    LivenessEventGenerator(void);
    void Generate(void);
    chip::EventNumber LogLiveness(chip::NodeId aNodeId, chip::EndpointId aEndpointId, LivenessDeviceStatus aStatus,
                                  chip::EventId aEventId, chip::app::PriorityLevel aPriorityLevel);
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter);

private:
    int32_t mStatus;
};
