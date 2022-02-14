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
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

inline void OnTriggerEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    WindowApp::Instance().PostEvent(WindowApp::EventId::WinkOff);
}

void OnTriggerEffect(Identify * identify)
{
    EmberAfIdentifyEffectIdentifier sIdentifyEffect = identify->mCurrentEffectIdentifier;

    ChipLogProgress(Zcl, "IDENTFY  OnTriggerEffect");

    if (identify->mCurrentEffectIdentifier == EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE)
    {
        ChipLogProgress(Zcl, "IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE - Not supported, use effect varriant %d",
                        identify->mEffectVariant);
        sIdentifyEffect = static_cast<EmberAfIdentifyEffectIdentifier>(identify->mEffectVariant);
    }

    switch (sIdentifyEffect)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        WindowApp::Instance().PostEvent(WindowApp::EventId::WinkOn);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerEffectCompleted, identify);
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectCompleted, identify);
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
    }
}

Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnTriggerEffect,
};

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
    chip::Server::GetInstance().Init();

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

#if CHIP_ENABLE_OPENTHREAD
    oldState.isThreadProvisioned = !ConnectivityMgr().IsThreadProvisioned();
#else
    oldState.isWiFiProvisioned = !ConnectivityMgr().IsWiFiStationProvisioned();
#endif
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
#if CHIP_ENABLE_OPENTHREAD
            mState.isThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            mState.isThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
#else
            mState.isWiFiProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            mState.isWiFiEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
#endif
            mState.haveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

#if CHIP_ENABLE_OPENTHREAD
        if (mState.isThreadProvisioned != oldState.isThreadProvisioned)
#else
        if (mState.isWiFiProvisioned != oldState.isWiFiProvisioned)
#endif
        {
            // Provisioned state changed
            DispatchEvent(EventId::ProvisionedStateChanged);
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
    case EventId::AttributeChange:
        DispatchEventAttributeChange(event.mEndpoint, event.mAttributeId);
        break;
    default:
        break;
    }
}

void WindowApp::DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute)
{
    Cover * cover = GetCover(endpoint);

    if (nullptr == cover)
    {
        emberAfWindowCoveringClusterPrint("Ep[%u] not supported AttributeId=%u\n", endpoint, (unsigned int) attribute);
        return;
    }

    switch (attribute)
    {
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionLiftPercent100ths::Id:
        cover->LiftGoToTarget();
        break;
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionTiltPercent100ths::Id:
        cover->TiltGoToTarget();
        break;
    /* RO OperationalStatus */
    case Attributes::OperationalStatus::Id:
        emberAfWindowCoveringClusterPrint("Global OpState: %02X\n", (unsigned int) OperationalStatusGet(endpoint).global);
        break;
    /* RW Mode */
    case Attributes::Mode::Id:
        emberAfWindowCoveringClusterPrint("Mode set: ignored");
        break;
    /* ### ATTRIBUTEs CHANGEs IGNORED ### */
    /* RO Type: not supposed to dynamically change */
    case Attributes::Type::Id:
    /* RO EndProductType: not supposed to dynamically change */
    case Attributes::EndProductType::Id:
    /* RO ConfigStatus: set by WC server */
    case Attributes::ConfigStatus::Id:
    /* RO SafetyStatus: set by WC server */
    case Attributes::SafetyStatus::Id:
    /* ============= Positions for Position Aware ============= */
    case Attributes::CurrentPositionLiftPercent100ths::Id:
    case Attributes::CurrentPositionTiltPercent100ths::Id:
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

    Attributes::InstalledOpenLimitLift::Set(endpoint, LIFT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitLift::Set(endpoint, LIFT_CLOSED_LIMIT);
    LiftPositionSet(endpoint, LiftToPercent100ths(endpoint, LIFT_CLOSED_LIMIT));
    Attributes::InstalledOpenLimitTilt::Set(endpoint, TILT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitTilt::Set(endpoint, TILT_CLOSED_LIMIT);
    TiltPositionSet(endpoint, TiltToPercent100ths(endpoint, TILT_CLOSED_LIMIT));

    // Attribute: Id  0 Type
    TypeSet(endpoint, EMBER_ZCL_WC_TYPE_TILT_BLIND_LIFT_AND_TILT);

    // Attribute: Id  7 ConfigStatus
    ConfigStatus configStatus = { .operational             = 1,
                                  .online                  = 1,
                                  .liftIsReversed          = 0,
                                  .liftIsPA                = HasFeaturePaLift(endpoint),
                                  .tiltIsPA                = HasFeaturePaTilt(endpoint),
                                  .liftIsEncoderControlled = 1,
                                  .tiltIsEncoderControlled = 1 };
    ConfigStatusSet(endpoint, configStatus);

    OperationalStatusSetWithGlobalUpdated(endpoint, mOperationalStatus);

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

void WindowApp::Cover::LiftDown()
{
    EmberAfStatus status;
    chip::app::DataModel::Nullable<chip::Percent100ths> current;
    chip::Percent100ths percent100ths = 5000; // set at middle

    status = Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);

    if ((status == EMBER_ZCL_STATUS_SUCCESS) && !current.IsNull())
        percent100ths = current.Value();

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

void WindowApp::Cover::LiftUp()
{
    EmberAfStatus status;
    chip::app::DataModel::Nullable<chip::Percent100ths> current;
    chip::Percent100ths percent100ths = 5000; // set at middle

    status = Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);

    if ((status == EMBER_ZCL_STATUS_SUCCESS) && !current.IsNull())
        percent100ths = current.Value();

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

void WindowApp::Cover::LiftUpdate(bool newTarget)
{
    NPercent100ths current, target;
    Attributes::TargetPositionLiftPercent100ths::Get(mEndpoint, target);
    Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);

    OperationalStatus opStatus = OperationalStatusGet(mEndpoint);
    OperationalState opState   = ComputeOperationalState(target, current);

    /* If Triggered by a TARGET update */
    if (newTarget)
    {
        mLiftTimer->Stop(); // Cancel previous motion if any
        mLiftOpState = opState;
    }

    if (mLiftOpState == opState)
    {
        /* Actuator still need to move, not reached/crossed Target yet */
        switch (mLiftOpState)
        {
        case OperationalState::MovingDownOrClose:
            LiftDown();
            break;
        case OperationalState::MovingUpOrOpen:
            LiftUp();
            break;
        default:
            break;
        }
    }
    else /* CURRENT reached TARGET or crossed it */
    {
        /* Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end */
        Attributes::CurrentPositionLiftPercent100ths::Set(mEndpoint, target);
        mLiftOpState = OperationalState::Stall;
    }

    opStatus.lift = mLiftOpState;
    OperationalStatusSetWithGlobalUpdated(mEndpoint, opStatus);

    if ((OperationalState::Stall != mLiftOpState) && mLiftTimer)
    {
        mLiftTimer->Start();
    }
}

void WindowApp::Cover::TiltDown()
{
    EmberAfStatus status;
    chip::app::DataModel::Nullable<chip::Percent100ths> current;
    chip::Percent100ths percent100ths = 5000; // set at middle

    status = Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);

    if ((status == EMBER_ZCL_STATUS_SUCCESS) && !current.IsNull())
        percent100ths = current.Value();

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

void WindowApp::Cover::TiltUp()
{
    EmberAfStatus status;
    chip::app::DataModel::Nullable<chip::Percent100ths> current;
    chip::Percent100ths percent100ths = 5000; // set at middle

    status = Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);

    if ((status == EMBER_ZCL_STATUS_SUCCESS) && !current.IsNull())
        percent100ths = current.Value();

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

void WindowApp::Cover::TiltUpdate(bool newTarget)
{
    NPercent100ths current, target;
    Attributes::TargetPositionTiltPercent100ths::Get(mEndpoint, target);
    Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);

    OperationalStatus opStatus = OperationalStatusGet(mEndpoint);
    OperationalState opState   = ComputeOperationalState(target, current);

    /* If Triggered by a TARGET update */
    if (newTarget)
    {
        mTiltTimer->Stop(); // Cancel previous motion if any
        mTiltOpState = opState;
    }

    if (mTiltOpState == opState)
    {
        /* Actuator still need to move, not reached/crossed Target yet */
        switch (mTiltOpState)
        {
        case OperationalState::MovingDownOrClose:
            TiltDown();
            break;
        case OperationalState::MovingUpOrOpen:
            TiltUp();
            break;
        default:
            break;
        }
    }
    else /* CURRENT reached TARGET or crossed it */
    {
        /* Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end */
        Attributes::CurrentPositionTiltPercent100ths::Set(mEndpoint, target);
        mTiltOpState = OperationalState::Stall;
    }

    opStatus.tilt = mTiltOpState;
    OperationalStatusSetWithGlobalUpdated(mEndpoint, opStatus);

    if ((OperationalState::Stall != mTiltOpState) && mTiltTimer)
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

void WindowApp::Cover::OnLiftTimeout(WindowApp::Timer & timer)
{
    WindowApp::Cover * cover = static_cast<WindowApp::Cover *>(timer.mContext);
    if (cover)
    {
        cover->LiftContinueToTarget();
    }
}

void WindowApp::Cover::OnTiltTimeout(WindowApp::Timer & timer)
{
    WindowApp::Cover * cover = static_cast<WindowApp::Cover *>(timer.mContext);
    if (cover)
    {
        cover->TiltContinueToTarget();
    }
}
