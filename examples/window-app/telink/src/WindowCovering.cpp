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
#include "PWMManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/window-covering-server/CodegenIntegration.h>
#include <platform/CHIPDeviceLayer.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WindowCovering;

static constexpr uint32_t sMoveTimeoutMs{ 200 };
constexpr uint16_t sPercentDelta   = 500;
constexpr uint8_t kDefaultMinLevel = 0;
constexpr uint8_t kDefaultMaxLevel = 254;

WindowCovering::WindowCovering()
{
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, false);
    PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, false);
}

void WindowCovering::DriveCurrentLiftPosition(intptr_t)
{
    auto wc = FindClusterOnEndpoint(Endpoint());
    VerifyOrReturn(wc != nullptr);
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    current = wc->GetCurrentPositionLiftPercent100ths();
    target  = wc->GetTargetPositionLiftPercent100ths();

    UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Lift));
    LiftPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInLiftMove = false;

    current = wc->GetCurrentPositionLiftPercent100ths();

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
    chip::Percent100ths percent100ths{};
    NPercent100ths current{};
    OperationalState opState = OperationalState::Stall;

    auto wc = FindClusterOnEndpoint(Endpoint());
    VerifyOrReturnValue(wc != nullptr, percent100ths);

    if (aMoveType == WindowCoveringType::Lift)
    {
        current = wc->GetCurrentPositionLiftPercent100ths();
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kLift);
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        current = wc->GetCurrentPositionTiltPercent100ths();
        opState = OperationalStateGet(Endpoint(), OperationalStatus::kTilt);
    }

    if (!current.IsNull())
    {
        percent100ths = ComputePercent100thsStep(opState, current.Value(), sPercentDelta);
    }
    else
    {
        LOG_ERR("Cannot read the current lift position");
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

    *moveType      = aMoveType;
    CHIP_ERROR err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutMs),
                                                                 MoveTimerTimeoutCallback, reinterpret_cast<void *>(moveType));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "WC timer err: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void WindowCovering::MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    WindowCoveringType * moveType = reinterpret_cast<WindowCoveringType *>(appState);
    VerifyOrReturn(moveType != nullptr);

    if (*moveType == WindowCoveringType::Lift)
    {
        err = chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCovering::DriveCurrentLiftPosition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "WC lift work err: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else if (*moveType == WindowCoveringType::Tilt)
    {
        err = chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCovering::DriveCurrentTiltPosition);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "WC tilt work err: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    chip::Platform::Delete(moveType);
}

void WindowCovering::DriveCurrentTiltPosition(intptr_t)
{
    auto wc = FindClusterOnEndpoint(Endpoint());
    VerifyOrReturn(wc != nullptr);
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    current = wc->GetCurrentPositionTiltPercent100ths();
    target  = wc->GetTargetPositionTiltPercent100ths();

    UpdateOperationalStatus(WindowCoveringType::Tilt, ComputeOperationalState(target, current));

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Tilt));
    TiltPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    Instance().mInTiltMove = false;

    current = wc->GetCurrentPositionTiltPercent100ths();

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
    auto wc = FindClusterOnEndpoint(Endpoint());
    VerifyOrReturn(wc != nullptr, LOG_ERR("Cannot set the target position: cluster not found"));

    if (Instance().mCurrentUIMoveType == WindowCoveringType::Lift)
    {
        wc->SetTargetPositionLiftPercent100ths(chip::app::DataModel::MakeNullable(aPosition));
    }
    else if (Instance().mCurrentUIMoveType == WindowCoveringType::Tilt)
    {
        wc->SetTargetPositionTiltPercent100ths(chip::app::DataModel::MakeNullable(aPosition));
    }
}

void WindowCovering::PositionLEDUpdate(WindowCoveringType aMoveType)
{
    auto wc = FindClusterOnEndpoint(Endpoint());
    VerifyOrReturn(wc != nullptr);
    NPercent100ths currentPosition{};

    if (aMoveType == WindowCoveringType::Lift)
    {
        currentPosition = wc->GetCurrentPositionLiftPercent100ths();
        if (!currentPosition.IsNull())
        {
            Instance().SetBrightness(WindowCoveringType::Lift, currentPosition.Value());
        }
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        currentPosition = wc->GetCurrentPositionTiltPercent100ths();
        if (!currentPosition.IsNull())
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
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Red, (((uint32_t) brightness * 1000) / UINT8_MAX));
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        PwmManager::getInstance().setPwm(PwmManager::EAppPwm_Green, (((uint32_t) brightness * 1000) / UINT8_MAX));
    }
}

uint8_t WindowCovering::PositionToBrightness(uint16_t aPosition, WindowCoveringType aMoveType)
{
    AbsoluteLimits pwmLimits{};

    if (aMoveType == WindowCoveringType::Lift)
    {
        pwmLimits.open   = kDefaultMinLevel;
        pwmLimits.closed = kDefaultMaxLevel;
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        pwmLimits.open   = kDefaultMinLevel;
        pwmLimits.closed = kDefaultMaxLevel;
    }

    return Percent100thsToValue(pwmLimits, aPosition);
}

void WindowCovering::SchedulePostAttributeChange(chip::EndpointId aEndpoint, chip::AttributeId aAttributeId)
{
    AttributeUpdateData * data = chip::Platform::New<AttributeUpdateData>();
    VerifyOrReturn(data != nullptr);

    data->mEndpoint    = aEndpoint;
    data->mAttributeId = aAttributeId;

    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(DoPostAttributeChange, reinterpret_cast<intptr_t>(data));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Attr work err: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void WindowCovering::DoPostAttributeChange(intptr_t aArg)
{
    AttributeUpdateData * data = reinterpret_cast<AttributeUpdateData *>(aArg);
    VerifyOrReturn(data != nullptr);

    PostAttributeChange(data->mEndpoint, data->mAttributeId);
}
