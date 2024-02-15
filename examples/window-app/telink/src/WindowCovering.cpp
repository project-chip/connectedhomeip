/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/af.h>
#include <platform/CHIPDeviceLayer.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

static const struct pwm_dt_spec sLiftPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct pwm_dt_spec sTiltPwmDevice = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1));

static constexpr uint32_t sMoveTimeoutMs{ 200 };
constexpr uint16_t sPercentDelta   = 500;
constexpr uint8_t kDefaultMinLevel = 0;
constexpr uint8_t kDefaultMaxLevel = 254;

WindowCovering::WindowCovering()
{
    if (mLiftIndicator.Init(&sLiftPwmDevice, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMinLevel) != CHIP_NO_ERROR)
    {
        LOG_ERR("Cannot initialize the lift indicator");
    }
    if (mTiltIndicator.Init(&sTiltPwmDevice, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMinLevel) != CHIP_NO_ERROR)
    {
        LOG_ERR("Cannot initialize the tilt indicator");
    }
    mLiftIndicator.InitiateAction(PWMDevice::ON_ACTION, 0, nullptr);
    mTiltIndicator.InitiateAction(PWMDevice::ON_ACTION, 0, nullptr);
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

    UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Lift));
    LiftPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInLiftMove = false;

    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(WindowCoveringType::Lift, current, target))
    {
        // continue to move
        StartTimer(WindowCoveringType::Lift, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-lift-movement after the target is completed
        UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));
    }
}

chip::Percent100ths WindowCovering::CalculateSingleStep(WindowCoveringType aMoveType)
{
    chip::Protocols::InteractionModel::Status status{};
    chip::Percent100ths percent100ths{};
    NPercent100ths current{};
    OperationalState opState = OperationalState::Stall;

    if (aMoveType == WindowCoveringType::Lift)
    {
        status  = Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current);
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kLift);
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        status  = Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current);
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kTilt);
    }

    if ((status == chip::Protocols::InteractionModel::Status::Success) && !current.IsNull())
    {
        percent100ths = ComputePercent100thsStep(opState, current.Value(), sPercentDelta);
    }
    else
    {
        LOG_ERR("Cannot read the current lift position. Error: %d", static_cast<uint8_t>(status));
    }

    return percent100ths;
}

bool WindowCovering::TargetCompleted(WindowCoveringType aMoveType, NPercent100ths aCurrent, NPercent100ths aTarget)
{
    return (OperationalState::Stall == ComputeOperationalState(aTarget, aCurrent));
}

void WindowCovering::StartTimer(WindowCoveringType aMoveType, uint32_t aTimeoutMs)
{
    WindowCoveringType * moveType = chip::Platform::New<WindowCoveringType>();
    VerifyOrReturn(moveType != nullptr);

    *moveType = aMoveType;
    (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutMs), MoveTimerTimeoutCallback,
                                                       reinterpret_cast<void *>(moveType));
}

void WindowCovering::MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState)
{
    WindowCoveringType * moveType = reinterpret_cast<WindowCoveringType *>(appState);
    VerifyOrReturn(moveType != nullptr);

    if (*moveType == WindowCoveringType::Lift)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCovering::DriveCurrentLiftPosition);
    }
    else if (*moveType == WindowCoveringType::Tilt)
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

    UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Tilt));
    TiltPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInTiltMove = false;

    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current) ==
                   chip::Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(WindowCoveringType::Tilt, current, target))
    {
        // continue to move
        StartTimer(WindowCoveringType::Tilt, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-tilt-movement after the target is completed
        UpdateOperationalStatus(WindowCoveringType::Tilt, ComputeOperationalState(target, current));
    }
}

void WindowCovering::StartMove(WindowCoveringType aMoveType)
{
    switch (aMoveType)
    {
    case WindowCoveringType::Lift:
        if (!mInLiftMove)
        {
            mInLiftMove = true;
            StartTimer(aMoveType, sMoveTimeoutMs);
        }
        break;
    case WindowCoveringType::Tilt:
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
    SetTargetPosition(aDirection, CalculateSingleStep(mCurrentUIMoveType));
}

void WindowCovering::UpdateOperationalStatus(WindowCoveringType aMoveType, OperationalState aDirection)
{
    switch (aMoveType)
    {
    case WindowCoveringType::Lift:
        OperationalStateSet(Endpoint(), OperationalStatus::kLift, aDirection);
        break;
    case WindowCoveringType::Tilt:
        OperationalStateSet(Endpoint(), OperationalStatus::kTilt, aDirection);
        break;
    case WindowCoveringType::Reserved:
        break;
    default:
        break;
    }
}

void WindowCovering::SetTargetPosition(OperationalState aDirection, chip::Percent100ths aPosition)
{
    chip::Protocols::InteractionModel::Status status{};
    if (Instance().mCurrentUIMoveType == WindowCoveringType::Lift)
    {
        status = Attributes::TargetPositionLiftPercent100ths::Set(Endpoint(), aPosition);
    }
    else if (Instance().mCurrentUIMoveType == WindowCoveringType::Tilt)
    {
        status = Attributes::TargetPositionTiltPercent100ths::Set(Endpoint(), aPosition);
    }

    if (status != chip::Protocols::InteractionModel::Status::Success)
    {
        LOG_ERR("Cannot set the target position. Error: %d", static_cast<uint8_t>(status));
    }
}

void WindowCovering::PositionLEDUpdate(WindowCoveringType aMoveType)
{
    chip::Protocols::InteractionModel::Status status{};
    NPercent100ths currentPosition{};

    if (aMoveType == WindowCoveringType::Lift)
    {
        status = Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), currentPosition);
        if (chip::Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
            Instance().SetBrightness(WindowCoveringType::Lift, currentPosition.Value());
        }
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        status = Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), currentPosition);
        if (chip::Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
            Instance().SetBrightness(WindowCoveringType::Tilt, currentPosition.Value());
        }
    }
}

void WindowCovering::SetBrightness(WindowCoveringType aMoveType, uint16_t aPosition)
{
    uint8_t brightness = PositionToBrightness(aPosition, aMoveType);
    if (aMoveType == WindowCoveringType::Lift)
    {
        mLiftIndicator.InitiateAction(PWMDevice::LEVEL_ACTION, 0, &brightness);
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        mTiltIndicator.InitiateAction(PWMDevice::LEVEL_ACTION, 0, &brightness);
    }
}

uint8_t WindowCovering::PositionToBrightness(uint16_t aPosition, WindowCoveringType aMoveType)
{
    AbsoluteLimits pwmLimits{};

    if (aMoveType == WindowCoveringType::Lift)
    {
        pwmLimits.open   = mLiftIndicator.GetMinLevel();
        pwmLimits.closed = mLiftIndicator.GetMaxLevel();
    }
    else if (aMoveType == WindowCoveringType::Tilt)
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
}
