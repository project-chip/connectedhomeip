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

#include <app/common/gen/enums.h>
#include <core/CHIPError.h>

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

    enum class Event
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
        CoverTypeChange,
        TiltModeChange,
        LiftUp,
        LiftDown,
        LiftChanged,
        TiltUp,
        TiltDown,
        TiltChanged,
        StopMotion,
        // Provisioning events
        ProvisionedStateChanged,
        ConnectivityStateChanged,
        BLEConnectionsChanged,
    };

    static WindowApp & Instance();

    virtual ~WindowApp() = default;
    virtual CHIP_ERROR Init();
    virtual CHIP_ERROR Start() = 0;
    virtual CHIP_ERROR Run();
    virtual void Finish();
    virtual void PostEvent(const Event & event) = 0;

protected:
    struct StateFlags
    {
        bool upPressed               = false;
        bool downPressed             = false;
        bool longPress               = false;
        bool tiltMode                = false;
        bool resetWarning            = false;
        bool isThreadProvisioned     = false;
        bool isThreadEnabled         = false;
        bool haveBLEConnections      = false;
        bool haveServiceConnectivity = false;
    };

    virtual Button * CreateButton(Button::Id id, const char * name) = 0;
    virtual void DestroyButton(Button * b);
    virtual Timer * CreateTimer(const char * name, uint32_t timeoutInMs, Timer::Callback callback, void * context) = 0;
    virtual void DestroyTimer(Timer * timer);

    virtual void ProcessEvents() = 0;
    virtual void DispatchEvent(const Event & event);

    virtual void OnMainLoop() = 0;
    static void OnLiftTimeout(Timer & timer);
    static void OnTiltTimeout(Timer & timer);
    static void OnCoverTypeTimeout(Timer & timer);
    static void OnResetTimeout(Timer & timer);

    Timer * mLiftTimer      = nullptr;
    Timer * mTiltTimer      = nullptr;
    Timer * mResetTimer     = nullptr;
    Timer * mCoverTypeTimer = nullptr;
    Button * mButtonUp      = nullptr;
    Button * mButtonDown    = nullptr;
    Event mLiftAction       = Event::None;
    Event mTiltAction       = Event::None;
    StateFlags mState;

private:
    void LiftUp();
    void LiftDown();
    void TiltUp();
    void TiltDown();
    void GotoLift();
    void GotoTilt();
    void CoverTypeCycle();
    void StopMotion();
};
