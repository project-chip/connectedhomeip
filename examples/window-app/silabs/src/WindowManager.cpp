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

#include "AppEvent.h"
#include "AppTask.h"
#include <AppConfig.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server/Server.h>

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <WindowManager.h>

#include <app/clusters/window-covering-server/window-covering-server.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#ifdef SL_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#include <platform/silabs/wifi/WifiInterface.h>
#endif

#ifdef DISPLAY_ENABLED
#include <LcdPainter.h>
#endif

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#define LCD_ICON_TIMEOUT 1000

using namespace chip::app::Clusters::WindowCovering;
using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
#define APP_ACTION_LED 1

#ifdef DIC_ENABLE
#define DECIMAL 10
#define MSG_SIZE 6
#include "dic.h"
#endif // DIC_ENABLE

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

WindowManager WindowManager::sWindow;

AppEvent CreateNewEvent(AppEvent::AppEventTypes type)
{
    AppEvent aEvent;
    aEvent.Type                = type;
    aEvent.Handler             = WindowManager::GeneralEventHandler;
    WindowManager * window     = static_cast<WindowManager *>(&WindowManager::sWindow);
    aEvent.WindowEvent.Context = window;
    return aEvent;
}

void WindowManager::Timer::Start()
{
    // Starts or restarts the function timer
    if (osTimerStart(mHandler, pdMS_TO_TICKS(100)) != osOK)
    {
        SILABS_LOG("Timer start() failed");
        appError(CHIP_ERROR_INTERNAL);
    }

    mIsActive = true;
}

void WindowManager::Timer::Timeout()
{
    mIsActive = false;
    if (mCallback)
    {
        mCallback(*this);
    }
}

WindowManager::Cover & WindowManager::GetCover()
{
    return mCoverList[mCurrentCover];
}

WindowManager::Cover * WindowManager::GetCover(chip::EndpointId endpoint)
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

void WindowManager::DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute)
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
        UpdateLED();
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
        UpdateLED();
        UpdateLCD();
        break;
    default:
        break;
    }
}

void WindowManager::HandleLongPress()
{

    AppEvent event;
    event.Handler             = GeneralEventHandler;
    WindowManager * window    = static_cast<WindowManager *>(&WindowManager::sWindow);
    event.WindowEvent.Context = window;

    if (mUpPressed && mDownPressed)
    {
        // Long press both buttons: Cycle between window coverings
        mUpSuppressed = mDownSuppressed = true;
        mCurrentCover                   = mCurrentCover < WINDOW_COVER_COUNT - 1 ? mCurrentCover + 1 : 0;
        event.Type                      = AppEvent::kEventType_CoverChange;
        AppTask::GetAppTask().PostEvent(&event);
        ChipLogDetail(AppServer, "App controls set to cover %d", mCurrentCover + 1);
    }
    else if (mUpPressed)
    {
        mUpSuppressed = true;
        if (!mResetWarning)
        {
            // Long press button up: Reset warning!
            event.Type = AppEvent::kEventType_ResetWarning;
            AppTask::GetAppTask().PostEvent(&event);
        }
    }
    else if (mDownPressed)
    {
        // Long press button down: Cycle between covering types
        mDownSuppressed = true;
        Type type       = GetCover().CycleType();
        mTiltMode       = mTiltMode && (Type::kTiltBlindLiftAndTilt == type);
        ChipLogDetail(AppServer, "Cover type changed to %d", to_underlying(type));
    }
}

void WindowManager::OnLongPressTimeout(WindowManager::Timer & timer)
{
    WindowManager * app = static_cast<WindowManager *>(timer.mContext);
    if (app)
    {
        app->HandleLongPress();
    }
}

void WindowManager::Cover::Init(chip::EndpointId endpoint)
{
    mEndpoint  = endpoint;
    mLiftTimer = new Timer(COVER_LIFT_TILT_TIMEOUT, OnLiftTimeout, this);
    mTiltTimer = new Timer(COVER_LIFT_TILT_TIMEOUT, OnTiltTimeout, this);

    // Preset Lift attributes
    Attributes::InstalledOpenLimitLift::Set(endpoint, LIFT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitLift::Set(endpoint, LIFT_CLOSED_LIMIT);

    // Preset Tilt attributes
    Attributes::InstalledOpenLimitTilt::Set(endpoint, TILT_OPEN_LIMIT);
    Attributes::InstalledClosedLimitTilt::Set(endpoint, TILT_CLOSED_LIMIT);

    // Note: All Current Positions are preset via Zap config and kept across reboot via NVM: no need to init them

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

void WindowManager::Cover::ScheduleControlAction(ControlAction action, bool setNewTarget)
{
    VerifyOrReturn(action <= ControlAction::Tilt);
    // Allocate a CoverWorkData. It will be freed by the Worker callback
    CoverWorkData * data = new CoverWorkData(this, setNewTarget);
    VerifyOrDie(data != nullptr);

    AsyncWorkFunct workFunct = (action == ControlAction::Lift) ? LiftUpdateWorker : TiltUpdateWorker;
    if (PlatformMgr().ScheduleWork(workFunct, reinterpret_cast<intptr_t>(data)) != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to schedule cover control action");
        delete data;
    }
}

void WindowManager::Cover::LiftUpdateWorker(intptr_t arg)
{
    // Use a unique_prt so it's freed when leaving this context
    std::unique_ptr<CoverWorkData> data(reinterpret_cast<CoverWorkData *>(arg));
    Cover * cover = data->cover;

    NPercent100ths current, target;

    VerifyOrReturn(Attributes::TargetPositionLiftPercent100ths::Get(cover->mEndpoint, target) ==
                   Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(cover->mEndpoint, current) ==
                   Protocols::InteractionModel::Status::Success);

    OperationalState opState = ComputeOperationalState(target, current);

    // If Triggered by a TARGET update
    if (data->setNewTarget)
    {
        cover->mLiftTimer->Stop(); // Cancel previous motion if any
        cover->mLiftOpState = opState;
    }

    if (cover->mLiftOpState == opState)
    {
        // Actuator still needs to move, has not reached/crossed Target yet
        chip::Percent100ths percent100ths;

        if (!current.IsNull())
        {
            percent100ths = ComputePercent100thsStep(cover->mLiftOpState, current.Value(), LIFT_DELTA);
        }
        else
        {
            percent100ths = WC_PERCENT100THS_MIDDLE; // set at middle by default
        }

        cover->PositionSet(cover->mEndpoint, percent100ths, ControlAction::Lift);
    }
    else // CURRENT reached TARGET or crossed it
    {
        // Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end
        if (!target.IsNull())
            cover->PositionSet(cover->mEndpoint, target.Value(), ControlAction::Lift);

        cover->mLiftOpState = OperationalState::Stall;
    }

    OperationalStateSet(cover->mEndpoint, OperationalStatus::kLift, cover->mLiftOpState);

    if ((OperationalState::Stall != cover->mLiftOpState) && cover->mLiftTimer)
    {
        cover->mLiftTimer->Start();
    }
}

void WindowManager::Cover::TiltUpdateWorker(intptr_t arg)
{
    // Use a unique_prt so it's freed when leaving this context
    std::unique_ptr<CoverWorkData> data(reinterpret_cast<CoverWorkData *>(arg));
    Cover * cover = data->cover;

    NPercent100ths current, target;
    VerifyOrReturn(Attributes::TargetPositionTiltPercent100ths::Get(cover->mEndpoint, target) ==
                   Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(cover->mEndpoint, current) ==
                   Protocols::InteractionModel::Status::Success);

    OperationalState opState = ComputeOperationalState(target, current);

    // If Triggered by a TARGET update
    if (data->setNewTarget)
    {
        cover->mTiltTimer->Stop(); // Cancel previous motion if any
        cover->mTiltOpState = opState;
    }

    if (cover->mTiltOpState == opState)
    {
        // Actuator still needs to move, has not reached/crossed Target yet
        chip::Percent100ths percent100ths;

        if (!current.IsNull())
        {
            percent100ths = ComputePercent100thsStep(cover->mTiltOpState, current.Value(), TILT_DELTA);
        }
        else
        {
            percent100ths = WC_PERCENT100THS_MIDDLE; // set at middle by default
        }

        cover->PositionSet(cover->mEndpoint, percent100ths, ControlAction::Tilt);
    }
    else // CURRENT reached TARGET or crossed it
    {
        // Actuator finalize the movement AND CURRENT Must be equal to TARGET at the end
        if (!target.IsNull())
            cover->PositionSet(cover->mEndpoint, target.Value(), ControlAction::Tilt);

        cover->mTiltOpState = OperationalState::Stall;
    }

    OperationalStateSet(cover->mEndpoint, OperationalStatus::kTilt, cover->mTiltOpState);

    if ((OperationalState::Stall != cover->mTiltOpState) && cover->mTiltTimer)
    {
        cover->mTiltTimer->Start();
    }
}

void WindowManager::Cover::UpdateTargetPosition(OperationalState direction, ControlAction action)
{
    Protocols::InteractionModel::Status status;
    NPercent100ths current;
    chip::Percent100ths target;

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    if (action == ControlAction::Tilt)
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

Type WindowManager::Cover::CycleType()
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

void WindowManager::Cover::OnLiftTimeout(WindowManager::Timer & timer)
{
    WindowManager::Cover * cover = static_cast<WindowManager::Cover *>(timer.mContext);
    if (cover)
    {
        cover->LiftContinueToTarget();
    }
}

void WindowManager::Cover::OnTiltTimeout(WindowManager::Timer & timer)
{
    WindowManager::Cover * cover = static_cast<WindowManager::Cover *>(timer.mContext);
    if (cover)
    {
        cover->TiltContinueToTarget();
    }
}

void WindowManager::Cover::PositionSet(chip::EndpointId endpointId, chip::Percent100ths position, ControlAction action)
{
    NPercent100ths nullablePosition;
    nullablePosition.SetNonNull(position);
    if (action == ControlAction::Tilt)
    {
        TiltPositionSet(endpointId, nullablePosition);
#ifdef DIC_ENABLE
        uint16_t value = position;
        char buffer[MSG_SIZE];
        itoa(value, buffer, DECIMAL);
        dic_sendmsg("tilt/position set", (const char *) (buffer));
#endif // DIC_ENABLE
    }
    else
    {
        LiftPositionSet(endpointId, nullablePosition);
#ifdef DIC_ENABLE
        uint16_t value = position;
        char buffer[MSG_SIZE];
        itoa(value, buffer, DECIMAL);
        dic_sendmsg("lift/position set", (const char *) (buffer));
#endif // DIC_ENABLE
    }
}

//------------------------------------------------------------------------------
// Timers
//------------------------------------------------------------------------------

WindowManager::Timer::Timer(uint32_t timeoutInMs, Callback callback, void * context) : mCallback(callback), mContext(context)
{
    mHandler = osTimerNew(TimerCallback, // timer callback handler
                          osTimerOnce,   // no timer reload (one-shot timer)
                          this,          // pass the app task obj context
                          NULL           // No osTimerAttr_t to provide.
    );

    if (mHandler == NULL)
    {
        SILABS_LOG("Timer create failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

WindowManager::Timer::~Timer()
{
    if (mHandler)
    {
        osTimerDelete(mHandler);
        mHandler = nullptr;
    }
}

void WindowManager::Timer::Stop()
{
    mIsActive = false;
    if (osTimerStop(mHandler) == osError)
    {
        SILABS_LOG("Timer stop() failed");
        appError(CHIP_ERROR_INTERNAL);
    }
}

void WindowManager::Timer::TimerCallback(void * timerCbArg)
{
    Timer * timer = static_cast<Timer *>(timerCbArg);
    if (timer)
    {
        timer->Timeout();
    }
}

WindowManager & WindowManager::Instance()
{
    return WindowManager::sWindow;
}

WindowManager::WindowManager() {}

void WindowManager::OnIconTimeout(WindowManager::Timer & timer)
{
#ifdef DISPLAY_ENABLED
    sWindow.mIcon = LcdIcon::None;
    sWindow.UpdateLCD();
#endif
}

CHIP_ERROR WindowManager::Init()
{
    chip::DeviceLayer::PlatformMgr().LockChipStack();

#ifdef DISPLAY_ENABLED
    mIconTimer = new Timer(LCD_ICON_TIMEOUT, OnIconTimeout, this);
#endif
    // Timers
    mLongPressTimer = new Timer(LONG_PRESS_TIMEOUT, OnLongPressTimeout, this);

    // Coverings
    mCoverList[0].Init(WINDOW_COVER_ENDPOINT1);
    mCoverList[1].Init(WINDOW_COVER_ENDPOINT2);

    // Initialize LEDs
    LEDWidget::InitGpio();
    mActionLED.Init(APP_ACTION_LED);
    AppTask::GetAppTask().LinkAppLed(&mActionLED);

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    return CHIP_NO_ERROR;
}

void WindowManager::PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    AppEvent event     = CreateNewEvent(AppEvent::kEventType_AttributeChange);
    event.mEndpoint    = endpoint;
    event.mAttributeId = attributeId;
    AppTask::GetAppTask().PostEvent(&event);
}

void WindowManager::UpdateLED()
{
    Cover & cover = GetCover();
    if (mResetWarning)
    {
        mActionLED.Set(false);
        mActionLED.Blink(500);
    }
    else
    {
        // Action LED
        NPercent100ths current;
        LimitStatus liftLimit = LimitStatus::Intermediate;

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Attributes::CurrentPositionLiftPercent100ths::Get(cover.mEndpoint, current);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        if (!current.IsNull())
        {
            AbsoluteLimits limits = { .open = WC_PERCENT100THS_MIN_OPEN, .closed = WC_PERCENT100THS_MAX_CLOSED };
            liftLimit             = CheckLimitState(current.Value(), limits);
        }

        if (OperationalState::Stall != cover.mLiftOpState)
        {
            mActionLED.Blink(100);
        }
        else if (LimitStatus::IsUpOrOpen == liftLimit)
        {
            mActionLED.Set(true);
        }
        else if (LimitStatus::IsDownOrClose == liftLimit)
        {
            mActionLED.Set(false);
        }
        else
        {
            mActionLED.Blink(1000);
        }
    }
}

void WindowManager::UpdateLCD()
{
    // Update LCD
#ifdef DISPLAY_ENABLED
    if (BaseApplication::GetProvisionStatus())
    {
        Cover & cover = GetCover();
        chip::app::DataModel::Nullable<uint16_t> lift;
        chip::app::DataModel::Nullable<uint16_t> tilt;

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        Type type = TypeGet(cover.mEndpoint);

        Attributes::CurrentPositionLift::Get(cover.mEndpoint, lift);
        Attributes::CurrentPositionTilt::Get(cover.mEndpoint, tilt);
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();

        if (!tilt.IsNull() && !lift.IsNull())
        {
            LcdPainter::Paint(AppTask::GetAppTask().GetLCD(), type, lift.Value(), tilt.Value(), mIcon);
        }
    }
#endif // DISPLAY_ENABLED
}

// Silabs button callback from button event ISR
void WindowManager::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent event;

    if (btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        event = CreateNewEvent(button ? AppEvent::kEventType_DownPressed : AppEvent::kEventType_UpPressed);
    }
    else
    {
        event = CreateNewEvent(button ? AppEvent::kEventType_DownReleased : AppEvent::kEventType_UpReleased);
    }

    AppTask::GetAppTask().PostEvent(&event);
}

void WindowManager::GeneralEventHandler(AppEvent * aEvent)
{
    WindowManager * window = static_cast<WindowManager *>(aEvent->WindowEvent.Context);

    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ResetWarning:
        window->mResetWarning = true;
        AppTask::GetAppTask().StartFactoryResetSequence();
        window->UpdateLED();
        break;

    case AppEvent::kEventType_ResetCanceled:
        window->mResetWarning = false;
        AppTask::GetAppTask().CancelFactoryResetSequence();
        window->UpdateLED();
        break;

    case AppEvent::kEventType_UpPressed:
        window->mUpPressed = true;
        if (window->mLongPressTimer)
        {
            window->mLongPressTimer->Start();
        }
        break;

    case AppEvent::kEventType_UpReleased:
        window->mUpPressed = false;
        if (window->mLongPressTimer)
        {
            window->mLongPressTimer->Stop();
        }
        if (window->mResetWarning)
        {
            aEvent->Type = AppEvent::kEventType_ResetCanceled;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        if (window->mUpSuppressed)
        {
            window->mUpSuppressed = false;
        }
        else if (window->mDownPressed)
        {
            window->mTiltMode       = !(window->mTiltMode);
            window->mDownSuppressed = true;
            aEvent->Type            = AppEvent::kEventType_TiltModeChange;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        else
        {
            window->GetCover().UpdateTargetPosition(
                OperationalState::MovingUpOrOpen, ((window->mTiltMode) ? Cover::ControlAction::Tilt : Cover::ControlAction::Lift));
        }
        break;

    case AppEvent::kEventType_DownPressed:
        window->mDownPressed = true;
        if (window->mLongPressTimer)
        {
            window->mLongPressTimer->Start();
        }
        break;

    case AppEvent::kEventType_DownReleased:
        window->mDownPressed = false;
        if (window->mLongPressTimer)
        {
            window->mLongPressTimer->Stop();
        }
        if (window->mResetWarning)
        {
            aEvent->Type = AppEvent::kEventType_ResetCanceled;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        if (window->mDownSuppressed)
        {
            window->mDownSuppressed = false;
        }
        else if (window->mUpPressed)
        {
            window->mTiltMode     = !(window->mTiltMode);
            window->mUpSuppressed = true;
            aEvent->Type          = AppEvent::kEventType_TiltModeChange;
            AppTask::GetAppTask().PostEvent(aEvent);
        }
        else
        {
            window->GetCover().UpdateTargetPosition(
                OperationalState::MovingDownOrClose,
                ((window->mTiltMode) ? Cover::ControlAction::Tilt : Cover::ControlAction::Lift));
        }
        break;

    case AppEvent::kEventType_AttributeChange:
        window->DispatchEventAttributeChange(aEvent->mEndpoint, aEvent->mAttributeId);
        break;

#ifdef DISPLAY_ENABLED
    case AppEvent::kEventType_CoverTypeChange:
        window->UpdateLCD();
        break;
    case AppEvent::kEventType_CoverChange:
        if (window->mIconTimer != nullptr)
        {
            window->mIconTimer->Start();
        }
        window->mIcon = (window->GetCover().mEndpoint == 1) ? LcdIcon::One : LcdIcon::Two;
        window->UpdateLCD();
        break;
    case AppEvent::kEventType_TiltModeChange:
        ChipLogDetail(AppServer, "App control mode changed to %s", window->mTiltMode ? "Tilt" : "Lift");
        if (window->mIconTimer != nullptr)
        {
            window->mIconTimer->Start();
        }
        window->mIcon = window->mTiltMode ? LcdIcon::Tilt : LcdIcon::Lift;
        window->UpdateLCD();
        break;
#endif

    default:
        break;
    }
}
