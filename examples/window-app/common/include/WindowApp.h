/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/clusters/window-covering-server/window-covering-server.h>
#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters::WindowCovering;

class WindowApp
{
public:
    struct Timer
    {
        typedef void (*Callback)(Timer & timer);

        Timer(const char * name, uint32_t timeoutInMs, Callback callback, void * context) :
            mName(name), mTimeoutInMs(timeoutInMs), mCallback(callback), mContext(context)
        {}
        virtual ~Timer()     = default;
        virtual void Start() = 0;
        virtual void Stop()  = 0;
        void Timeout();

        const char * mName    = nullptr;
        uint32_t mTimeoutInMs = 0;
        Callback mCallback    = nullptr;
        void * mContext       = nullptr;
        bool mIsActive        = false;
    };

    struct Button
    {
        enum class Id
        {
            Up   = 0,
            Down = 1
        };

        Button(Id id, const char * name) : mId(id), mName(name) {}
        virtual ~Button() = default;
        void Press();
        void Release();

        Id mId;
        const char * mName = nullptr;
    };

    enum class EventId
    {
        None = 0,
        Reset,
        ResetPressed,
        ResetWarning,
        ResetCanceled,
        // Button events
        UpPressed,
        UpReleased,
        DownPressed,
        DownReleased,
        // Cover events
        CoverChange,
        CoverTypeChange,
        TiltModeChange,

        // Cover Attribute update events
        AttributeChange,

        // Provisioning events
        ProvisionedStateChanged,
        ConnectivityStateChanged,
        BLEConnectionsChanged,
        WinkOff,
        WinkOn,
    };

    struct Event
    {
        Event(EventId id) : mId(id), mEndpoint(0) {}
        Event(EventId id, chip::EndpointId endpoint) : mId(id), mEndpoint(endpoint) {}
        Event(EventId id, chip::EndpointId endpoint, chip::AttributeId attributeId) :
            mId(id), mEndpoint(endpoint), mAttributeId(attributeId)
        {}

        EventId mId;
        chip::EndpointId mEndpoint;
        chip::AttributeId mAttributeId;
    };

    struct Cover
    {
        void Init(chip::EndpointId endpoint);
        void Finish();

        void LiftUpdate(bool newTarget);
        void LiftGoToTarget() { LiftUpdate(true); }
        void LiftContinueToTarget() { LiftUpdate(false); }
        void LiftStepToward(OperationalState direction);
        void LiftSchedulePositionSet(chip::Percent100ths position) { SchedulePositionSet(position, false); }
        void LiftScheduleOperationalStateSet(OperationalState opState) { ScheduleOperationalStateSet(opState, false); }

        void TiltUpdate(bool newTarget);
        void TiltGoToTarget() { TiltUpdate(true); }
        void TiltContinueToTarget() { TiltUpdate(false); }
        void TiltStepToward(OperationalState direction);
        void TiltSchedulePositionSet(chip::Percent100ths position) { SchedulePositionSet(position, true); }
        void TiltScheduleOperationalStateSet(OperationalState opState) { ScheduleOperationalStateSet(opState, true); }

        void StepToward(OperationalState direction, bool isTilt);

        Type CycleType();

        static void OnLiftTimeout(Timer & timer);
        static void OnTiltTimeout(Timer & timer);

        chip::EndpointId mEndpoint = 0;

        Timer * mLiftTimer            = nullptr;
        Timer * mTiltTimer            = nullptr;
        OperationalState mLiftOpState = OperationalState::Stall;
        OperationalState mTiltOpState = OperationalState::Stall;

        struct CoverWorkData
        {
            chip::EndpointId mEndpointId;
            bool isTilt;

            union
            {
                chip::Percent100ths percent100ths;
                OperationalState opState;
            };
        };

        void SchedulePositionSet(chip::Percent100ths position, bool isTilt);
        static void CallbackPositionSet(intptr_t arg);
        void ScheduleOperationalStateSet(OperationalState opState, bool isTilt);
        static void CallbackOperationalStateSet(intptr_t arg);
    };

    static WindowApp & Instance();

    virtual ~WindowApp() = default;
    virtual CHIP_ERROR Init();
    virtual CHIP_ERROR Start() = 0;
    virtual CHIP_ERROR Run();
    virtual void Finish();
    virtual void PostEvent(const Event & event)                                                = 0;
    virtual void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId) = 0;

protected:
    struct StateFlags
    {
#if CHIP_ENABLE_OPENTHREAD
        bool isThreadProvisioned = false;
        bool isThreadEnabled     = false;
#else
        bool isWiFiProvisioned = false;
        bool isWiFiEnabled     = false;
#endif
        bool haveBLEConnections = false;
        bool isWinking          = false;
    };

    Cover & GetCover();
    Cover * GetCover(chip::EndpointId endpoint);

    virtual Button * CreateButton(Button::Id id, const char * name) = 0;
    virtual void DestroyButton(Button * b);
    virtual Timer * CreateTimer(const char * name, uint32_t timeoutInMs, Timer::Callback callback, void * context) = 0;
    virtual void DestroyTimer(Timer * timer);

    virtual void ProcessEvents() = 0;
    virtual void DispatchEvent(const Event & event);
    virtual void OnMainLoop() = 0;
    static void OnLongPressTimeout(Timer & timer);

    Timer * mLongPressTimer = nullptr;
    Button * mButtonUp      = nullptr;
    Button * mButtonDown    = nullptr;
    StateFlags mState;
    bool mTiltMode       = false;
    bool mUpPressed      = false;
    bool mDownPressed    = false;
    bool mUpSuppressed   = false;
    bool mDownSuppressed = false;
    bool mResetWarning   = false;

private:
    void HandleLongPress();
    void DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute);

    Cover mCoverList[WINDOW_COVER_COUNT];
    uint8_t mCurrentCover = 0;
};
