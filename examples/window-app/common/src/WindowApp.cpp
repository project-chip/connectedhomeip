/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <AppConfig.h>
#include <Service.h>
#include <WindowApp.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::DeviceLayer;

namespace example {
CHIP_ERROR WindowApp::Init()
{
    emberAfWindowCoveringClusterPrint("#### WindowApp::Init()");

    // Init ZCL Data Model
    InitServer();
    ConfigurationMgr().LogDeviceConfig();
    SetDeviceName("EFR32WindowCoverDemo._chip._udp.local.");

    // Timers
    mLiftTimer      = CreateTimer("Timer:Lift", COVER_LIFT_TILT_TIMEOUT, OnLiftTimeout, this);
    mTiltTimer      = CreateTimer("Timer:Tilt", COVER_LIFT_TILT_TIMEOUT, OnTiltTimeout, this);
    mCoverTypeTimer = CreateTimer("Timer:Reset", WINDOW_COVER_TYPE_CYCLE_TIMEOUT, OnCoverTypeTimeout, this);
    mResetTimer     = CreateTimer("Timer:Reset", FACTORY_RESET_WINDOW_TIMEOUT, OnResetTimeout, this);

    //
    // Cover Initial state
    //
    WindowCover & cover = WindowCover::Instance();

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
    emberAfWindowCoveringClusterPrint("#### WindowApp::Run()");
    uint64_t lastChangeTimeUS = 0;
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
            OnProvisionedStateChanged(mState.isThreadProvisioned);
        }

        if (mState.haveServiceConnectivity != oldState.haveServiceConnectivity)
        {
            // Provisioned state changed
            OnConnectivityStateChanged(mState.haveServiceConnectivity);
        }

        if (mState.haveBLEConnections != oldState.haveBLEConnections)
        {
            // Provisioned state changed
            OnBLEConnectionsChanged(mState.haveBLEConnections);
        }

        uint64_t nowUS            = chip::System::Platform::Layer::GetClock_Monotonic();
        uint64_t nextChangeTimeUS = lastChangeTimeUS + 5 * 1000 * 1000UL;

        if (nowUS > nextChangeTimeUS)
        {
            PublishService();
            lastChangeTimeUS = nowUS;
        }

        OnMainLoop();
        oldState = mState;
    }

    return CHIP_NO_ERROR;
}

void WindowApp::DispatchEvent(const WindowEvent & event)
{
    emberAfWindowCoveringClusterPrint("#### WindowApp::DispatchEvent(%u)", event);

    switch (event)
    {
    case WindowEvent::Reset:
        mState.longPress = true;
        if (mState.resetWarning)
        {
            mState.resetWarning = false;
            OnReset();
        }
        else if (mResetTimer)
        {
            mState.resetWarning = true;
            OnResetWarning();
            mResetTimer->Start();
        }
        break;

    case WindowEvent::UpPressed:
        mState.upPressed = true;
        if (mState.downPressed)
        {
            mState.longPress = true;
            mState.tiltMode  = !mState.tiltMode;
            OnTiltModeChanged();
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

    case WindowEvent::UpReleased:
        mState.upPressed = false;
        if (mResetTimer)
        {
            if (mState.resetWarning)
            {
                mState.resetWarning = false;
                OnResetCanceled();
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
            OnTiltUp();
        }
        else
        {
            OnLiftUp();
        }
        break;

    case WindowEvent::DownPressed:
        mState.downPressed = true;
        if (mState.upPressed)
        {
            mState.longPress = true;
            mState.tiltMode  = !mState.tiltMode;
            OnTiltModeChanged();
        }
        else if (mCoverTypeTimer)
        {
            mCoverTypeTimer->Start();
        }
        break;

    case WindowEvent::DownReleased:
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
            OnTiltDown();
        }
        else
        {
            OnLiftDown();
        }
        break;

    case WindowEvent::CoverTypeChange:
        if (mState.downPressed && mCoverTypeTimer)
        {
            // Keep cycling
            mCoverTypeTimer->Start();
        }
        OnCoverTypeChanged();
        break;

    case WindowEvent::CoverLiftChanged:
        OnLiftChanged();
        break;

    case WindowEvent::CoverLiftUp:
    case WindowEvent::CoverLiftDown:
        mLiftAction = event;
        GotoLift();
        break;

    case WindowEvent::CoverTiltChanged:
        OnTiltChanged();
        break;

    case WindowEvent::CoverTiltUp:
    case WindowEvent::CoverTiltDown:
        mTiltAction = event;
        GotoTilt();
        break;
    default:
        break;
    }
}

void WindowApp::OnLiftUp()
{
    WindowCover::Actuator & lift = WindowCover::Instance().Lift();
    uint16_t percent             = lift.PositionGet(WindowCover::PositionUnits::Percentage);
    if (percent < 90)
    {
        percent += 10;
    }
    else
    {
        percent = 100;
    }
    lift.PositionSet(percent, WindowCover::PositionUnits::Percentage);
}

void WindowApp::OnLiftDown()
{
    WindowCover::Actuator & lift = WindowCover::Instance().Lift();
    uint16_t percent             = lift.PositionGet(WindowCover::PositionUnits::Percentage);
    if (percent > 10)
    {
        percent -= 10;
    }
    else
    {
        percent = 0;
    }
    lift.PositionSet(percent, WindowCover::PositionUnits::Percentage);
}

void WindowApp::OnTiltUp()
{
    WindowCover::Actuator & tilt = WindowCover::Instance().Tilt();
    uint16_t percent             = tilt.PositionGet(WindowCover::PositionUnits::Percentage);
    if (percent < 90)
    {
        percent += 10;
    }
    else
    {
        percent = 100;
    }
    tilt.PositionSet(percent, WindowCover::PositionUnits::Percentage);
}

void WindowApp::OnTiltDown()
{
    WindowCover::Actuator & tilt = WindowCover::Instance().Tilt();
    uint16_t percent             = tilt.PositionGet(WindowCover::PositionUnits::Percentage);
    if (percent > 10)
    {
        percent -= 10;
    }
    else
    {
        percent = 0;
    }
    tilt.PositionSet(percent, WindowCover::PositionUnits::Percentage);
}

void WindowApp::OnLiftTimeout(example::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->GotoLift();
    }
}

void WindowApp::OnTiltTimeout(example::Timer & timer)
{
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->GotoTilt();
    }
}

void WindowApp::OnCoverTypeTimeout(example::Timer & timer)
{
    emberAfWindowCoveringClusterPrint("WindowApp::OnCoverTypeTimeout");

    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->CoverTypeCycle();
    }
}

void WindowApp::OnResetTimeout(example::Timer & timer)
{
    emberAfWindowCoveringClusterPrint("WindowApp::OnResetTimeout");
    WindowApp * app = static_cast<WindowApp *>(timer.mContext);
    if (app)
    {
        app->PostEvent(WindowEvent::Reset);
    }
}

void WindowApp::GotoLift()
{
    WindowCover::Actuator & lift = WindowCover::Instance().Lift();
    uint16_t current             = lift.PositionGet(WindowCover::PositionUnits::Percentage100ths);
    uint16_t target              = lift.TargetGet(WindowCover::PositionUnits::Percentage100ths);

    if (WindowEvent::CoverLiftUp == mLiftAction)
    {
        if (current < target)
        {
            OnLiftUp();
        }
        else
        {
            mLiftAction = WindowEvent::None;
        }
    }
    else
    {
        if (current > target)
        {
            OnLiftDown();
        }
        else
        {
            mLiftAction = WindowEvent::None;
        }
    }

    if (WindowEvent::None != mLiftAction && mLiftTimer)
    {
        mLiftTimer->Start();
    }
}

void WindowApp::GotoTilt()
{
    WindowCover::Actuator & lift = WindowCover::Instance().Tilt();
    uint16_t current             = lift.PositionGet(WindowCover::PositionUnits::Percentage100ths);
    uint16_t target              = lift.TargetGet(WindowCover::PositionUnits::Percentage100ths);

    if (WindowEvent::CoverTiltUp == mTiltAction)
    {
        if (current < target)
        {
            OnTiltUp();
        }
        else
        {
            mTiltAction = WindowEvent::None;
        }
    }
    else
    {
        if (current > target)
        {
            OnTiltDown();
        }
        else
        {
            mTiltAction = WindowEvent::None;
        }
    }

    if (WindowEvent::None != mTiltAction && mTiltTimer)
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
        OnTiltModeChanged();
    }
}

} // namespace example
