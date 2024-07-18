/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "WindowCovering.h"
#include "AppConfig.h"
#include "PWMDevice.h"

#include <dk_buttons_and_leds.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

static const struct pwm_dt_spec sLiftPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));
static const struct pwm_dt_spec sTiltPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led2));

static constexpr uint32_t sMoveTimeoutMs{ 200 };

WindowCovering::WindowCovering()
{
    mLiftLED.Init(LIFT_STATE_LED);
    mTiltLED.Init(TILT_STATE_LED);

    if (mLiftIndicator.Init(&sLiftPwmDevice, 0, 255) != 0)
    {
        LOG_ERR("Cannot initialize the lift indicator");
    }
    if (mTiltIndicator.Init(&sTiltPwmDevice, 0, 255) != 0)
    {
        LOG_ERR("Cannot initialize the tilt indicator");
    }
}

void WindowCovering::DriveCurrentLiftPosition(intptr_t)
{
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::TargetPositionLiftPercent100ths::Get(Endpoint(), target) ==
                   chip::Protocols::InteractionModel::Status::Success);

    OperationalState state = ComputeOperationalState(target, current);
    UpdateOperationalStatus(MoveType::LIFT, state);

    chip::Percent100ths position = CalculateNextPosition(MoveType::LIFT);

    if (state == OperationalState::MovingUpOrOpen)
    {
        positionToSet.SetNonNull(position > target.Value() ? position : target.Value());
    }
    else if (state == OperationalState::MovingDownOrClose)
    {
        positionToSet.SetNonNull(position < target.Value() ? position : target.Value());
    }
    else
    {
        positionToSet.SetNonNull(current.Value());
    }

    LiftPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInLiftMove = false;

    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(MoveType::LIFT, current, target))
    {
        // continue to move
        StartTimer(MoveType::LIFT, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-lift-movement after the target is completed
        UpdateOperationalStatus(MoveType::LIFT, ComputeOperationalState(target, current));
    }
}

chip::Percent100ths WindowCovering::CalculateNextPosition(MoveType aMoveType)
{
    chip::Protocols::InteractionModel::Status status{};
    chip::Percent100ths percent100ths{};
    NPercent100ths current{};
    OperationalState opState{};

    if (aMoveType == MoveType::LIFT)
    {
        status  = Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current);
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kLift);
    }
    else if (aMoveType == MoveType::TILT)
    {
        status  = Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current);
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kTilt);
    }

    if ((status == chip::Protocols::InteractionModel::Status::Success) && !current.IsNull())
    {
        static constexpr auto sPercentDelta{ WC_PERCENT100THS_MAX_CLOSED / 20 };
        percent100ths = ComputePercent100thsStep(opState, current.Value(), sPercentDelta);
    }
    else
    {
        LOG_ERR("Cannot read the current lift position. Error: %d", static_cast<uint8_t>(status));
    }

    return percent100ths;
}

bool WindowCovering::TargetCompleted(MoveType aMoveType, NPercent100ths aCurrent, NPercent100ths aTarget)
{
    return (OperationalState::Stall == ComputeOperationalState(aTarget, aCurrent));
}

void WindowCovering::StartTimer(MoveType aMoveType, uint32_t aTimeoutMs)
{
    MoveType * moveType = chip::Platform::New<MoveType>();
    VerifyOrReturn(moveType != nullptr);

    *moveType = aMoveType;
    (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutMs), MoveTimerTimeoutCallback,
                                                       reinterpret_cast<void *>(moveType));
}

void WindowCovering::MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState)
{
    MoveType * moveType = reinterpret_cast<MoveType *>(appState);
    VerifyOrReturn(moveType != nullptr);

    if (*moveType == MoveType::LIFT)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCovering::DriveCurrentLiftPosition);
    }
    else if (*moveType == MoveType::TILT)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCovering::DriveCurrentTiltPosition);
    }

    chip::Platform::Delete(moveType);
}

void WindowCovering::DriveCurrentTiltPosition(intptr_t)
{
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::TargetPositionTiltPercent100ths::Get(Endpoint(), target) ==
                   chip::Protocols::InteractionModel::Status::Success);

    OperationalState state = ComputeOperationalState(target, current);
    UpdateOperationalStatus(MoveType::TILT, state);

    chip::Percent100ths position = CalculateNextPosition(MoveType::TILT);

    if (state == OperationalState::MovingUpOrOpen)
    {
        positionToSet.SetNonNull(position > target.Value() ? position : target.Value());
    }
    else if (state == OperationalState::MovingDownOrClose)
    {
        positionToSet.SetNonNull(position < target.Value() ? position : target.Value());
    }
    else
    {
        positionToSet.SetNonNull(current.Value());
    }

    TiltPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInTiltMove = false;

    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(MoveType::TILT, current, target))
    {
        // continue to move
        StartTimer(MoveType::TILT, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-tilt-movement after the target is completed
        UpdateOperationalStatus(MoveType::TILT, ComputeOperationalState(target, current));
    }
}

void WindowCovering::StartMove(MoveType aMoveType)
{
    switch (aMoveType)
    {
    case MoveType::LIFT:
        if (!mInLiftMove)
        {
            mInLiftMove = true;
            StartTimer(aMoveType, sMoveTimeoutMs);
        }
        break;
    case MoveType::TILT:
        if (!mInTiltMove)
        {
            mInTiltMove = true;
            StartTimer(aMoveType, sMoveTimeoutMs);
        }
        break;
    default:
        break;
    };
}

void WindowCovering::SetSingleStepTarget(OperationalState aDirection)
{
    UpdateOperationalStatus(mCurrentUIMoveType, aDirection);
    SetTargetPosition(aDirection, CalculateNextPosition(mCurrentUIMoveType));
}

void WindowCovering::UpdateOperationalStatus(MoveType aMoveType, OperationalState aDirection)
{
    switch (aMoveType)
    {
    case MoveType::LIFT:
        OperationalStateSet(Endpoint(), OperationalStatus::kLift, aDirection);
        break;
    case MoveType::TILT:
        OperationalStateSet(Endpoint(), OperationalStatus::kTilt, aDirection);
        break;
    case MoveType::NONE:
        break;
    default:
        break;
    }
}

void WindowCovering::SetTargetPosition(OperationalState aDirection, chip::Percent100ths aPosition)
{
    chip::Protocols::InteractionModel::Status status{};
    if (Instance().mCurrentUIMoveType == MoveType::LIFT)
    {
        status = Attributes::TargetPositionLiftPercent100ths::Set(Endpoint(), aPosition);
    }
    else if (Instance().mCurrentUIMoveType == MoveType::TILT)
    {
        status = Attributes::TargetPositionTiltPercent100ths::Set(Endpoint(), aPosition);
    }

    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Cannot set the target position. Error: %d", static_cast<uint8_t>(status));
    }
}

void WindowCovering::PositionLEDUpdate(MoveType aMoveType)
{
    chip::Protocols::InteractionModel::Status status{};
    NPercent100ths currentPosition{};

    if (aMoveType == MoveType::LIFT)
    {
        status = Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), currentPosition);
        if (chip::Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
            Instance().SetBrightness(MoveType::LIFT, currentPosition.Value());
        }
    }
    else if (aMoveType == MoveType::TILT)
    {
        status = Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), currentPosition);
        if (chip::Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
            Instance().SetBrightness(MoveType::TILT, currentPosition.Value());
        }
    }
}

void WindowCovering::SetBrightness(MoveType aMoveType, uint16_t aPosition)
{
    uint8_t brightness = PositionToBrightness(aPosition, aMoveType);
    if (aMoveType == MoveType::LIFT)
    {
        mLiftIndicator.InitiateAction(PWMDevice::LEVEL_ACTION, 0, &brightness);
    }
    else if (aMoveType == MoveType::TILT)
    {
        mTiltIndicator.InitiateAction(PWMDevice::LEVEL_ACTION, 0, &brightness);
    }
}

uint8_t WindowCovering::PositionToBrightness(uint16_t aPosition, MoveType aMoveType)
{
    AbsoluteLimits pwmLimits{};

    if (aMoveType == MoveType::LIFT)
    {
        pwmLimits.open   = mLiftIndicator.GetMinLevel();
        pwmLimits.closed = mLiftIndicator.GetMaxLevel();
    }
    else if (aMoveType == MoveType::TILT)
    {
        pwmLimits.open   = mTiltIndicator.GetMinLevel();
        pwmLimits.closed = mTiltIndicator.GetMaxLevel();
    }

    return Percent100thsToValue(pwmLimits, aPosition);
}

void WindowCovering::SchedulePostAttributeChange(chip::EndpointId aEndpoint, chip::AttributeId aAttributeId)
{
    AttributeUpdateData * data = chip::Platform::New<AttributeUpdateData>();
    VerifyOrReturn(data != nullptr);

    data->mEndpoint    = aEndpoint;
    data->mAttributeId = aAttributeId;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(DoPostAttributeChange, reinterpret_cast<intptr_t>(data));
}

void WindowCovering::DoPostAttributeChange(intptr_t aArg)
{
    AttributeUpdateData * data = reinterpret_cast<AttributeUpdateData *>(aArg);
    VerifyOrReturn(data != nullptr);

    PostAttributeChange(data->mEndpoint, data->mAttributeId);

    chip::Platform::Delete(data);
}
