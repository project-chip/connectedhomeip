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

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app;
using namespace chip::app::Clusters::WindowCovering;

inline void OnTriggerEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    WindowApp::Instance().PostEvent(WindowApp::EventId::WinkOff);
}

void OnTriggerEffect(Identify * identify)
{
    Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mEffectVariant != Clusters::Identify::EffectVariantEnum::kDefault)
    {
        ChipLogDetail(AppServer, "Identify Effect Variant unsupported. Using default");
    }

    switch (sIdentifyEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        WindowApp::Instance().PostEvent(WindowApp::EventId::WinkOn);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerEffectCompleted, identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
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
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
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
        chip::Server::GetInstance().ScheduleFactoryReset();
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
        else
        {
            GetCover().UpdateTargetPosition(OperationalState::MovingUpOrOpen, mTiltMode);
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
        else
        {
            GetCover().UpdateTargetPosition(OperationalState::MovingDownOrClose, mTiltMode);
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
    chip::BitMask<Mode> mode;
    chip::BitMask<ConfigStatus> configStatus;
    chip::BitMask<OperationalStatus> opStatus;

    if (nullptr == cover)
    {
        ChipLogProgress(Zcl, "Ep[%u] not supported AttributeId=%u\n", endpoint, (unsigned int) attribute);
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
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        opStatus = OperationalStatusGet(endpoint);
        OperationalStatusPrint(opStatus);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        break;
    /* RW Mode */
    case Attributes::Mode::Id:
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        mode = ModeGet(endpoint);
        ModePrint(mode);
        ModeSet(endpoint, mode); // refilter mode if needed
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        break;
    /* RO ConfigStatus: set by WC server */
    case Attributes::ConfigStatus::Id:
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        configStatus = ConfigStatusGet(endpoint);
        ConfigStatusPrint(configStatus);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        break;
    /* ### ATTRIBUTEs CHANGEs IGNORED ### */
    /* RO Type: not supposed to dynamically change */
    case Attributes::Type::Id:
    /* RO EndProductType: not supposed to dynamically change */
    case Attributes::EndProductType::Id:
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
        mDownSuppressed = true;
        Type type       = GetCover().CycleType();
        mTiltMode       = mTiltMode && (Type::kTiltBlindLiftAndTilt == type);
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

    // Preset Lift attributes
    Attributes::InstalledOpenLimitLift::Set(endpoint, LIFT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitLift::Set(endpoint, LIFT_CLOSED_LIMIT);

    // Preset Tilt attributes
    Attributes::InstalledOpenLimitTilt::Set(endpoint, TILT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitTilt::Set(endpoint, TILT_CLOSED_LIMIT);

    // Note: All Current Positions are preset via Zap config and kept accross reboot via NVM: no need to init them

    // Attribute: Id  0 Type
    TypeSet(endpoint, Type::kTiltBlindLiftAndTilt);

    // Attribute: Id  7 ConfigStatus
    chip::BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);
    configStatus.Set(ConfigStatus::kLiftEncoderControlled);
    configStatus.Set(ConfigStatus::kTiltEncoderControlled);
    ConfigStatusSet(endpoint, configStatus);

    chip::app::Clusters::WindowCovering::ConfigStatusUpdateFeatures(endpoint);

    // Attribute: Id 13 EndProductType
    EndProductTypeSet(endpoint, EndProductType::kInteriorBlind);

    // Attribute: Id 24 Mode
    chip::BitMask<Mode> mode;
    mode.Clear(Mode::kMotorDirectionReversed);
    mode.Clear(Mode::kMaintenanceMode);
    mode.Clear(Mode::kCalibrationMode);
    mode.Set(Mode::kLedFeedback);

    /* Mode also update ConfigStatus accordingly */
    ModeSet(endpoint, mode);

    // Attribute: Id 27 SafetyStatus (Optional)
    chip::BitFlags<SafetyStatus> safetyStatus(0x00); // 0 is no issues;
}

void WindowApp::Cover::Finish()
{
    WindowApp::Instance().DestroyTimer(mLiftTimer);
    WindowApp::Instance().DestroyTimer(mTiltTimer);
}

void WindowApp::Cover::LiftStepToward(OperationalState direction)
{
    Protocols::InteractionModel::Status status;
    chip::Percent100ths percent100ths;
    NPercent100ths current;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    status = Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if ((status == Protocols::InteractionModel::Status::Success) && !current.IsNull())
    {
        percent100ths = ComputePercent100thsStep(direction, current.Value(), LIFT_DELTA);
    }
    else
    {
        percent100ths = WC_PERCENT100THS_MIDDLE; // set at middle by default
    }

    LiftSchedulePositionSet(percent100ths);
}

void WindowApp::Cover::LiftUpdate(bool newTarget)
{
    NPercent100ths current, target;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    Attributes::TargetPositionLiftPercent100ths::Get(mEndpoint, target);
    Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);

    OperationalState opState = ComputeOperationalState(target, current);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    /* If Triggered by a TARGET update */
    if (newTarget)
    {
        mLiftTimer->Stop(); // Cancel previous motion if any
        mLiftOpState = opState;
    }

    if (mLiftOpState == opState)
    {
        /* Actuator still need to move, not reached/crossed Target yet */
        LiftStepToward(mLiftOpState);
    }
    else /* CURRENT reached TARGET or crossed it */
    {
        /* Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end */
        if (!target.IsNull())
            LiftSchedulePositionSet(target.Value());

        mLiftOpState = OperationalState::Stall;
    }

    LiftScheduleOperationalStateSet(mLiftOpState);

    if ((OperationalState::Stall != mLiftOpState) && mLiftTimer)
    {
        mLiftTimer->Start();
    }
}

void WindowApp::Cover::TiltStepToward(OperationalState direction)
{
    Protocols::InteractionModel::Status status;
    chip::Percent100ths percent100ths;
    NPercent100ths current;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    status = Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    if ((status == Protocols::InteractionModel::Status::Success) && !current.IsNull())
    {
        percent100ths = ComputePercent100thsStep(direction, current.Value(), TILT_DELTA);
    }
    else
    {
        percent100ths = WC_PERCENT100THS_MIDDLE; // set at middle by default
    }

    TiltSchedulePositionSet(percent100ths);
}

void WindowApp::Cover::TiltUpdate(bool newTarget)
{
    NPercent100ths current, target;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    Attributes::TargetPositionTiltPercent100ths::Get(mEndpoint, target);
    Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);

    OperationalState opState = ComputeOperationalState(target, current);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    /* If Triggered by a TARGET update */
    if (newTarget)
    {
        mTiltTimer->Stop(); // Cancel previous motion if any
        mTiltOpState = opState;
    }

    if (mTiltOpState == opState)
    {
        /* Actuator still need to move, not reached/crossed Target yet */
        TiltStepToward(mTiltOpState);
    }
    else /* CURRENT reached TARGET or crossed it */
    {
        /* Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end */
        if (!target.IsNull())
            TiltSchedulePositionSet(target.Value());

        mTiltOpState = OperationalState::Stall;
    }

    TiltScheduleOperationalStateSet(mTiltOpState);

    if ((OperationalState::Stall != mTiltOpState) && mTiltTimer)
    {
        mTiltTimer->Start();
    }
}

void WindowApp::Cover::StepToward(OperationalState direction, bool isTilt)
{
    if (isTilt)
    {
        TiltStepToward(direction);
    }
    else
    {
        LiftStepToward(direction);
    }
}

void WindowApp::Cover::UpdateTargetPosition(OperationalState direction, bool isTilt)
{
    Protocols::InteractionModel::Status status;
    NPercent100ths current;
    chip::Percent100ths target;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    if (isTilt)
    {
        status = Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, current);
        if ((status == Protocols::InteractionModel::Status::Success) && !current.IsNull())
        {

            target = ComputePercent100thsStep(direction, current.Value(), TILT_DELTA);
            (void) Attributes::TargetPositionTiltPercent100ths::Set(mEndpoint, target);
        }
    }
    else
    {
        status = Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, current);
        if ((status == Protocols::InteractionModel::Status::Success) && !current.IsNull())
        {

            target = ComputePercent100thsStep(direction, current.Value(), LIFT_DELTA);
            (void) Attributes::TargetPositionLiftPercent100ths::Set(mEndpoint, target);
        }
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
}

Type WindowApp::Cover::CycleType()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    Type type = TypeGet(mEndpoint);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    switch (type)
    {
    case Type::kRollerShade:
        type = Type::kDrapery;
        // tilt = false;
        break;
    case Type::kDrapery:
        type = Type::kTiltBlindLiftAndTilt;
        break;
    case Type::kTiltBlindLiftAndTilt:
        type = Type::kRollerShade;
        // tilt = false;
        break;
    default:
        type = Type::kTiltBlindLiftAndTilt;
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    TypeSet(mEndpoint, type);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

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

void WindowApp::Cover::SchedulePositionSet(chip::Percent100ths position, bool isTilt)
{
    CoverWorkData * data = chip::Platform::New<CoverWorkData>();
    VerifyOrReturn(data != nullptr, ChipLogProgress(Zcl, "Cover::SchedulePositionSet - Out of Memory for WorkData"));

    data->mEndpointId   = mEndpoint;
    data->percent100ths = position;
    data->isTilt        = isTilt;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(CallbackPositionSet, reinterpret_cast<intptr_t>(data));
}

void WindowApp::Cover::CallbackPositionSet(intptr_t arg)
{
    NPercent100ths position;
    WindowApp::Cover::CoverWorkData * data = reinterpret_cast<WindowApp::Cover::CoverWorkData *>(arg);
    position.SetNonNull(data->percent100ths);

    if (data->isTilt)
        TiltPositionSet(data->mEndpointId, position);
    else
        LiftPositionSet(data->mEndpointId, position);

    chip::Platform::Delete(data);
}

void WindowApp::Cover::ScheduleOperationalStateSet(OperationalState opState, bool isTilt)
{
    CoverWorkData * data = chip::Platform::New<CoverWorkData>();
    VerifyOrReturn(data != nullptr, ChipLogProgress(Zcl, "Cover::OperationalStatusSet - Out of Memory for WorkData"));

    data->mEndpointId = mEndpoint;
    data->opState     = opState;
    data->isTilt      = isTilt;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(CallbackOperationalStateSet, reinterpret_cast<intptr_t>(data));
}

void WindowApp::Cover::CallbackOperationalStateSet(intptr_t arg)
{
    WindowApp::Cover::CoverWorkData * data = reinterpret_cast<WindowApp::Cover::CoverWorkData *>(arg);

    OperationalStateSet(data->mEndpointId, data->isTilt ? OperationalStatus::kTilt : OperationalStatus::kLift, data->opState);

    chip::Platform::Delete(data);
}
