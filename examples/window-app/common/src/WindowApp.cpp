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

#include <AppConfig.h>
#include <WindowApp.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

void WindowApp::Timer::Timeout()
{
    mIsActive = false;
    if (mCallback)
    {
        mCallback(*this);
    }
}

void WindowApp::Button::Press()
{
    Instance().PostEvent(Button::Id::Up == mId ? WindowApp::Event::UpPressed : WindowApp::Event::DownPressed);
}

void WindowApp::Button::Release()
{
    Instance().PostEvent(Button::Id::Up == mId ? WindowApp::Event::UpReleased : WindowApp::Event::DownReleased);
}

CHIP_ERROR WindowApp::Init()
{
    // Init ZCL Data Model
    InitServer();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ConfigurationMgr().LogDeviceConfig();

    // Timers
    mLiftTimer      = CreateTimer("Timer:Lift", COVER_LIFT_TILT_TIMEOUT, OnLiftTimeout, this);
    mTiltTimer      = CreateTimer("Timer:Tilt", COVER_LIFT_TILT_TIMEOUT, OnTiltTimeout, this);
    mCoverTypeTimer = CreateTimer("Timer:CoverType", WINDOW_COVER_TYPE_CYCLE_TIMEOUT, OnCoverTypeTimeout, this);
    mResetTimer     = CreateTimer("Timer:Reset", FACTORY_RESET_WINDOW_TIMEOUT, OnResetTimeout, this);

    // Buttons
    mButtonUp   = CreateButton(Button::Id::Up, "UP");
    mButtonDown = CreateButton(Button::Id::Down, "DOWN");

    //
    // Cover Initial state
    //

    WindowCover & cover = WindowCover::Instance();

    // Initialize Actuators
    cover.Lift().OpenLimitSet(LIFT_OPEN_LIMIT);
    cover.Lift().ClosedLimitSet(LIFT_CLOSED_LIMIT);
    cover.Lift().PositionValueSet(LIFT_CLOSED_LIMIT);
    cover.Tilt().OpenLimitSet(TILT_OPEN_LIMIT);
    cover.Tilt().ClosedLimitSet(TILT_CLOSED_LIMIT);
    cover.Tilt().PositionValueSet(TILT_CLOSED_LIMIT);

    // Attribute: Id  0 Type
    cover.TypeSet(EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT);

    // Attribute: Id  7 ConfigStatus
    WindowCover::ConfigStatus configStatus = { .operational             = 1,
                                               .online                  = 1,
                                               .liftIsReversed          = 0,
                                               .liftIsPA                = 1,
                                               .tiltIsPA                = 1,
                                               .liftIsEncoderControlled = 1,
                                               .tiltIsEncoderControlled = 1 };
    cover.ConfigStatusSet(configStatus);

    // Attribute: Id 10 OperationalStatus
    WindowCover::OperationalStatus operationalStatus = { .global = WindowCover::OperationalState::Stall,
                                                         .lift   = WindowCover::OperationalState::Stall,
                                                         .tilt   = WindowCover::OperationalState::Stall };
    cover.OperationalStatusSet(operationalStatus);

    // Attribute: Id 13 EndProductType
    cover.EndProductTypeSet(EMBER_ZCL_WC_END_PRODUCT_TYPE_INTERIOR_BLIND);

    // Attribute: Id 24 Mode
    WindowCover::Mode mode = { .motorDirReversed = 0, .calibrationMode = 1, .maintenanceMode = 1, .ledDisplay = 1 };
    cover.ModeSet(mode);

    // Attribute: Id 27 SafetyStatus (Optional)
    WindowCover::SafetyStatus safetyStatus = { 0x00 }; // 0 is no issues;
    cover.SafetyStatusSet(safetyStatus);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowApp::Run()
{
    StateFlags oldState;

    oldState.isThreadProvisioned = !ConnectivityMgr().IsThreadProvisioned();

    while (true)
    {
        ProcessEvents();

        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            mState.isThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            mState.isThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            mState.haveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            mState.haveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        if (mState.isThreadProvisioned != oldState.isThreadProvisioned)
        {
            // Provisioned state changed
            DispatchEvent(WindowApp::Event::ProvisionedStateChanged);
        }

        if (mState.haveServiceConnectivity != oldState.haveServiceConnectivity)
        {
            // Provisioned state changed
            DispatchEvent(WindowApp::Event::ConnectivityStateChanged);
        }

        if (mState.haveBLEConnections != oldState.haveBLEConnections)
        {
            // Provisioned state changed
            DispatchEvent(WindowApp::Event::BLEConnectionsChanged);
        }

        OnMainLoop();
        oldState = mState;
    }

    return CHIP_NO_ERROR;
}

void WindowApp::Finish()
{
    DestroyTimer(mLiftTimer);
    DestroyTimer(mTiltTimer);
    DestroyTimer(mCoverTypeTimer);
    DestroyTimer(mResetTimer);
    DestroyButton(mButtonUp);
    DestroyButton(mButtonDown);
}

void WindowApp::DispatchEvent(const WindowApp::Event & event)
{
    switch (event)
    {
    case WindowApp::Event::ResetPressed:
        mState.longPress = true;
        if (mState.resetWarning)
        {
            mState.resetWarning = false;
            PostEvent(WindowApp::Event::Reset);
        }
        else if (mResetTimer)
        {
            mState.resetWarning = true;
            PostEvent(WindowApp::Event::ResetWarning);
            mResetTimer->Start();
        }
        break;

    case WindowApp::Event::Reset:
        ConfigurationMgr().InitiateFactoryReset();
        break;

    case WindowApp::Event::UpPressed:
        mState.upPressed = true;
        if (mState.downPressed)
        {
            mState.longPress = true;
            mState.tiltMode  = !mState.tiltMode;
            PostEvent(WindowApp::Event::TiltModeChange);
        }
        else if (mResetTimer)
        {
            // Start the reset timer
            mResetTimer->Start();
        }
        if (mCoverTypeTimer)
        {
            mCoverTypeTimer->Stop();
        }
        break;

    case WindowApp::Event::UpReleased:
        mState.upPressed = false;
        if (mResetTimer)
        {
            if (mState.resetWarning)
            {
                mState.resetWarning = false;
                PostEvent(WindowApp::Event::ResetCanceled);
            }
            mResetTimer->Stop();
        }
        if (mState.downPressed)
        {
            // Ignore
        }
        else if (mState.longPress)
        {
            mState.longPress = false;
        }
        else if (mState.tiltMode)
        {
            TiltUp();
        }
        else
        {
            LiftUp();
        }
        break;

    case WindowApp::Event::DownPressed:
        mState.downPressed = true;
        if (mState.upPressed)
        {
            mState.longPress = true;
            mState.tiltMode  = !mState.tiltMode;
            PostEvent(WindowApp::Event::TiltModeChange);
        }
        else if (mCoverTypeTimer)
        {
            mCoverTypeTimer->Start();
        }
        break;

    case WindowApp::Event::DownReleased:
        mState.downPressed = false;
        if (mCoverTypeTimer)
        {
            mCoverTypeTimer->Stop();
        }
        if (mState.upPressed)
        {
            // Ignore
        }
        else if (mState.longPress)
        {
            mState.longPress = false;
        }
        else if (mState.tiltMode)
        {
            TiltDown();
        }
        else
        {
            LiftDown();
        }
        break;

    case WindowApp::Event::CoverTypeChange:
        if (mState.downPressed && mCoverTypeTimer)
        {
            // Keep cycling
            mCoverTypeTimer->Start();
        }
        break;

    case WindowApp::Event::LiftUp:
    case WindowApp::Event::LiftDown:
        mLiftAction = event;
        GotoLift();
        break;

    case WindowApp::Event::TiltUp:
    case WindowApp::Event::TiltDown:
        mTiltAction = event;
        GotoTilt();
        break;
    case WindowApp::Event::StopMotion:
        StopMotion();
        break;
    default:
        break;
    }
}

void WindowApp::DestroyTimer(Timer * timer)
{
    if (timer)
    {
        delete timer;
    }
}

void WindowApp::DestroyButton(Button * btn)
{
    if (btn)
    {
        delete btn;
    }
}

void WindowApp::LiftUp()
{
    WindowCover::LiftActuator & lift = WindowCover::Instance().Lift();
    uint16_t percent100ths           = lift.PositionGet();
    if (percent100ths < 9000)
    {
        percent100ths += 1000;
    }
    else
    {
        percent100ths = 10000;
    }
    lift.PositionSet(percent100ths);
}

void WindowApp::LiftDown()
{
    WindowCover::LiftActuator & lift = WindowCover::Instance().Lift();
    uint16_t percent100ths           = lift.PositionGet();
    if (percent100ths > 1000)
    {
        percent100ths -= 1000;
    }
    else
    {
        percent100ths = 0;
    }
    lift.PositionSet(percent100ths);
}

void WindowApp::TiltUp()
{
    WindowCover::TiltActuator & tilt = WindowCover::Instance().Tilt();
    uint16_t percent100ths           = tilt.PositionGet();
    if (percent100ths < 9000)
    {
        percent100ths += 1000;
    }
    else
    {
        percent100ths = 10000;
    }
    tilt.PositionSet(percent100ths);
}

void WindowApp::TiltDown()
{
    WindowCover::TiltActuator & tilt = WindowCover::Instance().Tilt();
    uint16_t percent100ths           = tilt.PositionGet();
    if (percent100ths > 1000)
    {
        percent100ths -= 1000;
    }
    else
    {
        percent100ths = 0;
    }
    tilt.PositionSet(percent100ths);
}

void WindowApp::GotoLift()
{
    WindowCover::LiftActuator & lift = WindowCover::Instance().Lift();
    uint16_t current                 = lift.PositionGet();
    uint16_t target                  = lift.TargetGet();

    if (WindowApp::Event::LiftUp == mLiftAction)
    {
        if (current < target)
        {
            LiftUp();
        }
        else
        {
            mLiftAction = WindowApp::Event::None;
        }
    }
    else
    {
        if (current > target)
        {
            LiftDown();
        }
        else
        {
            mLiftAction = WindowApp::Event::None;
        }
    }

    if (WindowApp::Event::None != mLiftAction && mLiftTimer)
    {
        mLiftTimer->Start();
    }
}

void WindowApp::GotoTilt()
{
    WindowCover::TiltActuator & tilt = WindowCover::Instance().Tilt();
    uint16_t current                 = tilt.PositionGet();
    uint16_t target                  = tilt.TargetGet();

    if (WindowApp::Event::TiltUp == mTiltAction)
    {
        if (current < target)
        {
            TiltUp();
        }
        else
        {
            mTiltAction = WindowApp::Event::None;
        }
    }
    else
    {
        if (current > target)
        {
            TiltDown();
        }
        else
        {
            mTiltAction = WindowApp::Event::None;
        }
    }

    if (WindowApp::Event::None != mTiltAction && mTiltTimer)
    {
        mTiltTimer->Start();
    }
}

void WindowApp::CoverTypeCycle()
{
    WindowCover & cover = WindowCover::Instance();
    EmberAfWcType type  = cover.TypeGet();
    bool tilt           = mState.tiltMode;

    switch (type)
    {
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE:
        type = EMBER_ZCL_WC_TYPE_DRAPERY;
        tilt = false;
        break;
    case EMBER_ZCL_WC_TYPE_DRAPERY:
        type = EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT;
        break;
    case EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT:
        type = EMBER_ZCL_WC_TYPE_ROLLERSHADE;
        tilt = false;
        break;
    default:
        type = EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT;
    }
    cover.TypeSet(type);
    if (tilt != mState.tiltMode)
    {
        mState.tiltMode = tilt;
        PostEvent(WindowApp::Event::TiltModeChange);
    }
}

void WindowApp::StopMotion()
{
    mLiftAction = WindowApp::Event::None;
    if (mLiftTimer)
    {
        mLiftTimer->Stop();
    }
    mTiltAction = WindowApp::Event::None;
    if (mTiltTimer)
    {
        mTiltTimer->Stop();
    }
}

void WindowApp::OnLiftTimeout(WindowApp::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->GotoLift();
    }
}

void WindowApp::OnTiltTimeout(WindowApp::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->GotoTilt();
    }
}

void WindowApp::OnCoverTypeTimeout(WindowApp::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->CoverTypeCycle();
    }
}

void WindowApp::OnResetTimeout(WindowApp::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->PostEvent(WindowApp::Event::ResetPressed);
    }
}
