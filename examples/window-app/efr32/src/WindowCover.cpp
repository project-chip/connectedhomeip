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

#include <AppTask.h>
#include <WindowCover.h>

#define LIFT_DELTA (LCD_COVER_SIZE / 10)
#define TILT_DELTA 1
#define TIMER_DELAY_MS 500

WindowCover::WindowCover()
{
    mLiftTimer.Init(this, "Timer:lift", TIMER_DELAY_MS, LiftTimerCallback);
    mTiltTimer.Init(this, "Timer:tilt", TIMER_DELAY_MS, TiltTimerCallback);
}

WindowCover::WindowCover(CoverType type, uint16_t liftOpenLimit, uint16_t liftClosedLimit, uint16_t tiltOpenLimit,
                         uint16_t tiltClosedLimit) :
    mType(type),
    mLiftOpenLimit(liftOpenLimit), mLiftClosedLimit(liftClosedLimit), mLift(liftClosedLimit), mTiltOpenLimit(tiltOpenLimit),
    mTiltClosedLimit(tiltClosedLimit), mTilt(tiltClosedLimit)
{}

void WindowCover::StatusSet(uint8_t status)
{
    if (status != mStatus)
    {
        mStatus = status;
        PostEvent(AppEvent::EventType::CoverStatusChange);
    }
}

uint8_t WindowCover::StatusGet(void)
{
    return mStatus;
}

void WindowCover::TypeSet(CoverType type)
{
    if (type != mType)
    {
        mType = type;
        switch (mType)
        {
        // Lift only
        case CoverType::Rollershade:
        case CoverType::Rollershade_2_motor:
        case CoverType::Rollershade_exterior_2_motor:
        case CoverType::Drapery:
        case CoverType::Awning:
            TiltModeSet(false);
            break;
        // Tilt only
        case CoverType::Shutter:
        case CoverType::Tilt_blind:
        case CoverType::Projector_screen:
            TiltModeSet(true);
            break;
        // Lift & Tilt
        case CoverType::Tilt_Lift_blind:
        default:
            break;
        }
        PostEvent(AppEvent::EventType::CoverTypeChange);
    }
}

void WindowCover::TypeCycle()
{
    switch (mType)
    {
    case CoverType::Rollershade:
        TypeSet(CoverType::Drapery);
        break;
    case CoverType::Drapery:
        TypeSet(CoverType::Tilt_Lift_blind);
        break;
    case CoverType::Tilt_Lift_blind:
        TypeSet(CoverType::Rollershade);
        break;
    default:
        TypeSet(CoverType::Tilt_Lift_blind);
        break;
    }
}

WindowCover::CoverType WindowCover::TypeGet()
{
    return mType;
}

uint16_t WindowCover::LiftOpenLimitGet()
{
    return mLiftOpenLimit;
}

uint16_t WindowCover::LiftClosedLimitGet()
{
    return mLiftClosedLimit;
}

void WindowCover::LiftSet(uint16_t lift)
{
    if (lift > mLiftClosedLimit)
    {
        lift = mLiftClosedLimit;
    }
    else if (lift < mLiftOpenLimit)
    {
        lift = mLiftOpenLimit;
    }
    if (lift != mLift)
    {
        AppEvent::EventType event = lift > mLift ? AppEvent::EventType::CoverLiftUp : AppEvent::EventType::CoverLiftDown;
        mLift                     = lift;
        PostEvent(event);
    }
}

uint16_t WindowCover::LiftGet()
{
    return mLift;
}

void WindowCover::LiftPercentSet(uint8_t percentage)
{
    LiftSet(PercentToLift(percentage));
}

uint8_t WindowCover::LiftPercentGet()
{
    return LiftToPercent(mLift);
}

void WindowCover::LiftUp()
{
    LiftSet(mLift + LIFT_DELTA);
}

void WindowCover::LiftDown()
{
    if (mLift >= LIFT_DELTA)
    {
        LiftSet(mLift - LIFT_DELTA);
    }
    else
    {
        LiftSet(mLiftOpenLimit);
    }
}

void WindowCover::LiftGotoValue(uint16_t lift)
{
    if (lift > mLiftClosedLimit)
    {
        lift = mLiftClosedLimit;
    }
    else if (lift < mLiftOpenLimit)
    {
        lift = mLiftOpenLimit;
    }
    mLiftAction = lift > mLift ? CoverAction::LiftUp : CoverAction::LiftDown;
    mLiftTarget = lift;
    mLiftTimer.Start();
    PostEvent(AppEvent::EventType::CoverStart);
}

void WindowCover::LiftGotoPercent(uint8_t percentage)
{
    LiftGotoValue(PercentToLift(percentage));
}

uint8_t WindowCover::LiftToPercent(uint16_t lift)
{
    return (uint8_t)(100 * (lift - mLiftOpenLimit) / (mLiftClosedLimit - mLiftOpenLimit));
}

uint16_t WindowCover::PercentToLift(uint8_t liftPercent)
{
    return mLiftOpenLimit + (mLiftClosedLimit - mLiftOpenLimit) * liftPercent / 100;
}

uint16_t WindowCover::TiltOpenLimitGet()
{
    return mTiltOpenLimit;
}

uint16_t WindowCover::TiltClosedLimitGet()
{
    return mTiltClosedLimit;
}

void WindowCover::TiltSet(uint16_t tilt)
{
    if (tilt > mTiltClosedLimit)
    {
        tilt = mTiltClosedLimit;
    }
    else if (tilt < mTiltOpenLimit)
    {
        tilt = mTiltOpenLimit;
    }
    if (tilt != mTilt)
    {
        AppEvent::EventType event = tilt > mTilt ? AppEvent::EventType::CoverTiltUp : AppEvent::EventType::CoverTiltDown;
        mTilt                     = tilt;
        PostEvent(event);
    }
}

uint16_t WindowCover::TiltGet()
{
    return mTilt;
}

void WindowCover::TiltPercentSet(uint8_t percentage)
{
    TiltSet(PercentToTilt(percentage));
}

uint8_t WindowCover::TiltPercentGet()
{
    return TiltToPercent(mTilt);
}

void WindowCover::TiltUp()
{
    TiltSet(mTilt + TILT_DELTA);
}

void WindowCover::TiltDown()
{
    if (mTilt >= TILT_DELTA)
    {
        TiltSet(mTilt - TILT_DELTA);
    }
    else
    {
        TiltSet(mTiltOpenLimit);
    }
}

void WindowCover::TiltGotoValue(uint16_t tilt)
{
    if (tilt > mTiltClosedLimit)
    {
        tilt = mTiltClosedLimit;
    }
    else if (tilt < mTiltOpenLimit)
    {
        tilt = mTiltOpenLimit;
    }
    mTiltAction = tilt > mTilt ? CoverAction::TiltUp : CoverAction::TiltDown;
    mTiltTarget = tilt;
    mTiltTimer.Start();
    PostEvent(AppEvent::EventType::CoverStart);
}

void WindowCover::TiltGotoPercent(uint8_t percentage)
{
    TiltGotoValue(PercentToTilt(percentage));
}

uint8_t WindowCover::TiltToPercent(uint16_t tilt)
{
    return (uint8_t)(100 * (tilt - mTiltOpenLimit) / (mTiltClosedLimit - mTiltOpenLimit));
}

uint16_t WindowCover::PercentToTilt(uint8_t tiltPercent)
{
    return mTiltOpenLimit + (mTiltClosedLimit - mTiltOpenLimit) * tiltPercent / 100;
}

void WindowCover::Open()
{
    LiftGotoValue(mLiftOpenLimit);
    TiltGotoValue(mTiltOpenLimit);
}

void WindowCover::Close()
{
    LiftGotoValue(mLiftClosedLimit);
    TiltGotoValue(mTiltClosedLimit);
}

void WindowCover::Stop()
{
    mLiftTimer.Stop();
    mTiltTimer.Stop();
    PostEvent(AppEvent::EventType::CoverStop);
}

void WindowCover::TiltModeSet(bool mode)
{
    if (mode != mTiltMode)
    {
        mTiltMode = mode;
        PostEvent(AppEvent::EventType::CoverTiltModeChange);
    }
}

bool WindowCover::TiltModeGet()
{
    return mTiltMode;
}

void WindowCover::ToggleTiltMode()
{
    mTiltMode = !mTiltMode;
    PostEvent(AppEvent::EventType::CoverTiltModeChange);
}

void WindowCover::StepUp()
{
    if (mTiltMode)
    {
        TiltUp();
    }
    else
    {
        LiftUp();
    }
}

void WindowCover::StepDown()
{
    if (mTiltMode)
    {
        TiltDown();
    }
    else
    {
        LiftDown();
    }
}

bool WindowCover::IsOpen(void)
{
    switch (mType)
    {
    // Lift only
    case CoverType::Rollershade:
    case CoverType::Rollershade_2_motor:
    case CoverType::Rollershade_exterior_2_motor:
    case CoverType::Drapery:
    case CoverType::Awning:
        return mLift <= mLiftOpenLimit;
    // Tilt only
    case CoverType::Shutter:
    case CoverType::Tilt_blind:
    case CoverType::Projector_screen:
        return mTilt <= mTiltOpenLimit;
    // Lift & Tilt
    case CoverType::Tilt_Lift_blind:
    default:
        return mLift <= mLiftOpenLimit && mTilt <= mTiltOpenLimit;
    }
    return false;
}

bool WindowCover::IsClosed(void)
{
    switch (mType)
    {
    // Lift only
    case CoverType::Rollershade:
    case CoverType::Rollershade_2_motor:
    case CoverType::Rollershade_exterior_2_motor:
    case CoverType::Drapery:
    case CoverType::Awning:
        return mLift >= mLiftClosedLimit;
    // Tilt only
    case CoverType::Shutter:
    case CoverType::Tilt_blind:
    case CoverType::Projector_screen:
        return mTilt >= mTiltClosedLimit;
    // Lift & Tilt
    case CoverType::Tilt_Lift_blind:
    default:
        return mLift >= mLiftClosedLimit && mTilt >= mTiltClosedLimit;
    }
    return false;
}

bool WindowCover::IsMoving(void)
{
    return mLiftTimer.IsActive() || mTiltTimer.IsActive();
}

const char * WindowCover::TypeString(const WindowCover::CoverType type)
{
    switch (type)
    {
    case CoverType::Rollershade:
        return "Rollershade";
    case CoverType::Rollershade_2_motor:
        return "Rollershade_2_motor";
    case CoverType::Rollershade_exterior_2_motor:
        return "Rollershade_exterior_2_motor";
    case CoverType::Drapery:
        return "Drapery";
    case CoverType::Awning:
        return "Awning";
    case CoverType::Shutter:
        return "Shutter";
    case CoverType::Tilt_blind:
        return "Tilt_blind";
    case CoverType::Tilt_Lift_blind:
        return "Tilt_Lift_blind";
    case CoverType::Projector_screen:
        return "Projector_screen";
    default:
        return "?";
    }
}

void WindowCover::LiftTimerCallback(AppTimer & timer, void * context)
{
    WindowCover * cover = (WindowCover *) context;
    switch (cover->mLiftAction)
    {
    case CoverAction::LiftDown:
        if (cover->mLift > cover->mLiftTarget)
        {
            cover->LiftDown();
            timer.Start();
        }
        break;
    case CoverAction::LiftUp:
        if (cover->mLift < cover->mLiftTarget)
        {
            cover->LiftUp();
            timer.Start();
        }
        break;
    case CoverAction::None:
    default:
        timer.Stop();
        break;
    }

    if (!timer.IsActive())
    {
        cover->mLiftAction = CoverAction::None;
        cover->PostEvent(AppEvent::EventType::CoverStop);
    }
}

void WindowCover::TiltTimerCallback(AppTimer & timer, void * context)
{
    WindowCover * cover = (WindowCover *) context;
    switch (cover->mTiltAction)
    {
    case CoverAction::TiltDown:
        if (cover->mTilt > cover->mTiltTarget)
        {
            cover->TiltDown();
            timer.Start();
        }
        break;
    case CoverAction::TiltUp:
        if (cover->mTilt < cover->mTiltTarget)
        {
            cover->TiltUp();
            timer.Start();
        }
        break;
    case CoverAction::None:
    default:
        timer.Stop();
        break;
    }

    if (!timer.IsActive())
    {
        cover->mTiltAction = CoverAction::None;
        cover->PostEvent(AppEvent::EventType::CoverStop);
    }
}

void WindowCover::PostEvent(AppEvent::EventType event)
{
    AppTask::Instance().PostEvent(AppEvent(event, this));
}
