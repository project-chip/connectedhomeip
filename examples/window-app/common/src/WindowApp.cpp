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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

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
    EventId event = Button::Id::Up == mId ? EventId::UpPressed : EventId::DownPressed;
    Instance().PostEvent(WindowApp::Event(event));
}

void WindowApp::Button::Release()
{
    Instance().PostEvent(Button::Id::Up == mId ? EventId::UpReleased : EventId::DownReleased);
}

WindowApp::Cover & WindowApp::GetCover()
{
    return mCoverList[mCurrentCover];
}

WindowApp::Cover * WindowApp::GetCover(chip::EndpointId endpoint)
{
    for (uint16_t i = 0; i < WINDOW_COVER_COUNT; ++i)
    {
        if (mCoverList[i].mEndpoint == endpoint)
        {
            return &mCoverList[i];
        }
    }
    return nullptr;
}

CHIP_ERROR WindowApp::Init()
{
    // Init ZCL Data Model
    InitServer();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ConfigurationMgr().LogDeviceConfig();

    // Timers
    mLongPressTimer = CreateTimer("Timer:LongPress", LONG_PRESS_TIMEOUT, OnLongPressTimeout, this);

    // Buttons
    mButtonUp   = CreateButton(Button::Id::Up, "UP");
    mButtonDown = CreateButton(Button::Id::Down, "DOWN");

    // Coverings
    mCoverList[0].Init(WINDOW_COVER_ENDPOINT1);
    mCoverList[1].Init(WINDOW_COVER_ENDPOINT2);

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
            DispatchEvent(EventId::ProvisionedStateChanged);
        }

        if (mState.haveServiceConnectivity != oldState.haveServiceConnectivity)
        {
            // Provisioned state changed
            DispatchEvent(EventId::ConnectivityStateChanged);
        }

        if (mState.haveBLEConnections != oldState.haveBLEConnections)
        {
            // Provisioned state changed
            DispatchEvent(EventId::BLEConnectionsChanged);
        }

        OnMainLoop();
        oldState = mState;
    }

    return CHIP_NO_ERROR;
}

void WindowApp::Finish()
{
    DestroyTimer(mLongPressTimer);
    DestroyButton(mButtonUp);
    DestroyButton(mButtonDown);
    for (uint16_t i = 0; i < WINDOW_COVER_COUNT; ++i)
    {
        mCoverList[i].Finish();
    }
}

void WindowApp::DispatchEvent(const WindowApp::Event & event)
{
    Cover * cover = nullptr;

    switch (event.mId)
    {
    case EventId::ResetWarning:
        mResetWarning = true;
        if (mLongPressTimer)
        {
            mLongPressTimer->Start();
        }
        break;

    case EventId::ResetCanceled:
        mResetWarning = false;
        break;

    case EventId::Reset:
        ConfigurationMgr().InitiateFactoryReset();
        break;

    case EventId::UpPressed:
        mUpPressed = true;
        if (mLongPressTimer)
        {
            mLongPressTimer->Start();
        }
        break;

    case EventId::UpReleased:
        mUpPressed = false;
        if (mLongPressTimer)
        {
            mLongPressTimer->Stop();
        }
        if (mResetWarning)
        {
            PostEvent(EventId::ResetCanceled);
        }
        if (mUpSuppressed)
        {
            mUpSuppressed = false;
        }
        else if (mDownPressed)
        {
            mTiltMode     = !mTiltMode;
            mUpSuppressed = mDownSuppressed = true;
            PostEvent(EventId::TiltModeChange);
        }
        else if (mTiltMode)
        {
            GetCover().TiltUp();
        }
        else
        {
            GetCover().LiftUp();
        }
        break;

    case EventId::DownPressed:
        mDownPressed = true;
        if (mLongPressTimer)
        {
            mLongPressTimer->Start();
        }
        break;

    case EventId::DownReleased:
        mDownPressed = false;
        if (mLongPressTimer)
        {
            mLongPressTimer->Stop();
        }
        if (mResetWarning)
        {
            PostEvent(EventId::ResetCanceled);
        }
        if (mDownSuppressed)
        {
            mDownSuppressed = false;
        }
        else if (mUpPressed)
        {
            mTiltMode     = !mTiltMode;
            mUpSuppressed = mDownSuppressed = true;
            PostEvent(EventId::TiltModeChange);
        }
        else if (mTiltMode)
        {
            GetCover().TiltDown();
        }
        else
        {
            GetCover().LiftDown();
        }
        break;

    case EventId::LiftUp:
    case EventId::LiftDown:
        cover = GetCover(event.mEndpoint);
        if (cover)
        {
            cover->GotoLift(event.mId);
        }
        break;

    case EventId::TiltUp:
    case EventId::TiltDown:
        cover = GetCover(event.mEndpoint);
        if (cover)
        {
            cover->GotoTilt(event.mId);
        }
        break;

    case EventId::StopMotion:
        cover = GetCover(event.mEndpoint);
        if (cover)
        {
            cover->StopMotion();
        }
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

void WindowApp::HandleLongPress()
{
    if (mUpPressed && mDownPressed)
    {
        // Long press both buttons: Cycle between window coverings
        mUpSuppressed = mDownSuppressed = true;
        mCurrentCover                   = mCurrentCover < WINDOW_COVER_COUNT - 1 ? mCurrentCover + 1 : 0;
        PostEvent(EventId::CoverChange);
    }
    else if (mUpPressed)
    {
        mUpSuppressed = true;
        if (mResetWarning)
        {
            // Double long press button up: Reset now, you were warned!
            PostEvent(EventId::Reset);
        }
        else
        {
            // Long press button up: Reset warning!
            PostEvent(EventId::ResetWarning);
        }
    }
    else if (mDownPressed)
    {
        // Long press button down: Cycle between covering types
        mDownSuppressed          = true;
        EmberAfWcType cover_type = GetCover().CycleType();
        mTiltMode                = mTiltMode && (EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT == cover_type);
    }
}

void WindowApp::OnLongPressTimeout(WindowApp::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->HandleLongPress();
    }
}

void WindowApp::Cover::Init(chip::EndpointId endpoint)
{
    mEndpoint  = endpoint;
    mLiftTimer = WindowApp::Instance().CreateTimer("Timer:Lift", COVER_LIFT_TILT_TIMEOUT, OnLiftTimeout, this);
    mTiltTimer = WindowApp::Instance().CreateTimer("Timer:Tilt", COVER_LIFT_TILT_TIMEOUT, OnTiltTimeout, this);

    Attributes::SetInstalledOpenLimitLift(endpoint, LIFT_OPEN_LIMIT);
    Attributes::SetInstalledClosedLimitLift(endpoint, LIFT_CLOSED_LIMIT);
    LiftPositionSet(endpoint, LiftToPercent100ths(endpoint, LIFT_CLOSED_LIMIT));
    Attributes::SetInstalledOpenLimitTilt(endpoint, TILT_OPEN_LIMIT);
    Attributes::SetInstalledClosedLimitTilt(endpoint, TILT_CLOSED_LIMIT);
    TiltPositionSet(endpoint, TiltToPercent100ths(endpoint, TILT_CLOSED_LIMIT));

    // Attribute: Id  0 Type
    TypeSet(endpoint, EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT);

    // Attribute: Id  7 ConfigStatus
    ConfigStatus configStatus = { .operational             = 1,
                                  .online                  = 1,
                                  .liftIsReversed          = 0,
                                  .liftIsPA                = 1,
                                  .tiltIsPA                = 1,
                                  .liftIsEncoderControlled = 1,
                                  .tiltIsEncoderControlled = 1 };
    ConfigStatusSet(endpoint, configStatus);

    // Attribute: Id 10 OperationalStatus
    OperationalStatus operationalStatus = { .global = OperationalState::Stall,
                                            .lift   = OperationalState::Stall,
                                            .tilt   = OperationalState::Stall };
    OperationalStatusSet(endpoint, operationalStatus);

    // Attribute: Id 13 EndProductType
    EndProductTypeSet(endpoint, EMBER_ZCL_WC_END_PRODUCT_TYPE_INTERIOR_BLIND);

    // Attribute: Id 24 Mode
    Mode mode = { .motorDirReversed = 0, .calibrationMode = 1, .maintenanceMode = 1, .ledDisplay = 1 };
    ModeSet(endpoint, mode);

    // Attribute: Id 27 SafetyStatus (Optional)
    SafetyStatus safetyStatus = { 0x00 }; // 0 is no issues;
    SafetyStatusSet(endpoint, safetyStatus);
}

void WindowApp::Cover::Finish()
{
    WindowApp::Instance().DestroyTimer(mLiftTimer);
    WindowApp::Instance().DestroyTimer(mTiltTimer);
}

void WindowApp::Cover::LiftUp()
{
    uint16_t percent100ths = 0;

    Attributes::GetCurrentPositionLiftPercent100ths(mEndpoint, &percent100ths);
    if (percent100ths < 9000)
    {
        percent100ths += 1000;
    }
    else
    {
        percent100ths = 10000;
    }
    LiftPositionSet(mEndpoint, percent100ths);
}

void WindowApp::Cover::LiftDown()
{
    uint16_t percent100ths = 0;

    Attributes::GetCurrentPositionLiftPercent100ths(mEndpoint, &percent100ths);
    if (percent100ths > 1000)
    {
        percent100ths -= 1000;
    }
    else
    {
        percent100ths = 0;
    }
    LiftPositionSet(mEndpoint, percent100ths);
}

void WindowApp::Cover::GotoLift(EventId action)
{
    uint16_t current = 0;
    uint16_t target  = 0;
    Attributes::GetTargetPositionLiftPercent100ths(mEndpoint, &target);
    Attributes::GetCurrentPositionLiftPercent100ths(mEndpoint, &current);

    if (EventId::None != action)
    {
        mLiftAction = action;
    }

    if (EventId::LiftUp == mLiftAction)
    {
        if (current < target)
        {
            LiftUp();
        }
        else
        {
            mLiftAction = EventId::None;
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
            mLiftAction = EventId::None;
        }
    }

    if (EventId::None != mLiftAction && mLiftTimer)
    {
        mLiftTimer->Start();
    }
}

void WindowApp::Cover::TiltUp()
{
    uint16_t percent100ths = 0;
    Attributes::GetCurrentPositionTiltPercent100ths(mEndpoint, &percent100ths);
    if (percent100ths < 9000)
    {
        percent100ths += 1000;
    }
    else
    {
        percent100ths = 10000;
    }
    TiltPositionSet(mEndpoint, percent100ths);
}

void WindowApp::Cover::TiltDown()
{
    uint16_t percent100ths = 0;
    Attributes::GetCurrentPositionTiltPercent100ths(mEndpoint, &percent100ths);
    if (percent100ths > 1000)
    {
        percent100ths -= 1000;
    }
    else
    {
        percent100ths = 0;
    }
    TiltPositionSet(mEndpoint, percent100ths);
}

void WindowApp::Cover::GotoTilt(EventId action)
{
    uint16_t current = 0;
    uint16_t target  = 0;

    Attributes::GetTargetPositionTiltPercent100ths(mEndpoint, &target);
    Attributes::GetCurrentPositionTiltPercent100ths(mEndpoint, &current);

    if (EventId::None != action)
    {
        mTiltAction = action;
    }

    if (EventId::TiltUp == mTiltAction)
    {
        if (current < target)
        {
            TiltUp();
        }
        else
        {
            mTiltAction = EventId::None;
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
            mTiltAction = EventId::None;
        }
    }

    if (EventId::None != mTiltAction && mTiltTimer)
    {
        mTiltTimer->Start();
    }
}

EmberAfWcType WindowApp::Cover::CycleType()
{
    EmberAfWcType type = TypeGet(mEndpoint);

    switch (type)
    {
    case EMBER_ZCL_WC_TYPE_ROLLERSHADE:
        type = EMBER_ZCL_WC_TYPE_DRAPERY;
        // tilt = false;
        break;
    case EMBER_ZCL_WC_TYPE_DRAPERY:
        type = EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT;
        break;
    case EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT:
        type = EMBER_ZCL_WC_TYPE_ROLLERSHADE;
        // tilt = false;
        break;
    default:
        type = EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT;
    }
    TypeSet(mEndpoint, type);
    return type;
}

void WindowApp::Cover::StopMotion()
{
    mLiftAction = EventId::None;
    if (mLiftTimer)
    {
        mLiftTimer->Stop();
    }
    mTiltAction = EventId::None;
    if (mTiltTimer)
    {
        mTiltTimer->Stop();
    }
}

void WindowApp::Cover::OnLiftTimeout(WindowApp::Timer & timer)
{
    WindowApp::Cover * cover = static_cast<WindowApp::Cover *>(timer.mContext);
    if (cover)
    {
        cover->GotoLift();
    }
}

void WindowApp::Cover::OnTiltTimeout(WindowApp::Timer & timer)
{
    WindowApp::Cover * cover = static_cast<WindowApp::Cover *>(timer.mContext);
    if (cover)
    {
        cover->GotoTilt();
    }
}
