/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "WindowCoveringManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::System::Clock::Literals;

namespace {

constexpr const System::Clock::Milliseconds32 kIncrementMovementTimeout = 700_ms32;
constexpr const uint16_t kDefaultMovementStep                           = 2000;

} // namespace

void WindowCoveringManager::Init(EndpointId endpoint)
{
    mState = OperationalState::Stall;
    mCurrentLiftPosition.SetNonNull(static_cast<uint16_t>(0));
    mTargetLiftPosition.SetNonNull(static_cast<uint16_t>(0));
    mCurrentTiltPosition.SetNonNull(static_cast<uint16_t>(0));
    mTargetTiltPosition.SetNonNull(static_cast<uint16_t>(0));
    SetEndpoint(endpoint);
}

void WindowCoveringManager::HandleLiftMovementTimer(System::Layer * layer, void * aAppState)
{
    WindowCoveringManager * manager = reinterpret_cast<WindowCoveringManager *>(aAppState);

    VerifyOrReturn(manager->mState != OperationalState::Stall);

    Percent100ths currentPosition = manager->mCurrentLiftPosition.Value();
    Percent100ths targetPosition  = manager->mTargetLiftPosition.Value();

    ChipLogProgress(NotSpecified, "HandleLiftMovementTimer:currentPosition:%u, targetPosition:%u", currentPosition, targetPosition);

    if (OperationalState::MovingUpOrOpen == manager->mState)
    {
        if (ComputeOperationalState(targetPosition, currentPosition) != OperationalState::MovingUpOrOpen)
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }

        Percent100ths tempPosition =
            ComputePercent100thsStep(OperationalState::MovingUpOrOpen, currentPosition, kDefaultMovementStep);
        currentPosition = tempPosition > targetPosition ? tempPosition : targetPosition;
        manager->mCurrentLiftPosition.SetNonNull(currentPosition);
    }
    else
    {
        if (ComputeOperationalState(targetPosition, currentPosition) != OperationalState::MovingDownOrClose)
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }

        Percent100ths tempPosition =
            ComputePercent100thsStep(OperationalState::MovingDownOrClose, currentPosition, kDefaultMovementStep);
        currentPosition = tempPosition < targetPosition ? tempPosition : targetPosition;
        manager->mCurrentLiftPosition.SetNonNull(currentPosition);
    }

    LiftPositionSet(manager->mEndpoint, manager->mCurrentLiftPosition);

    if (manager->mCurrentLiftPosition != manager->mTargetLiftPosition)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleLiftMovementTimer, aAppState));
    }
}

void WindowCoveringManager::HandleTiltMovementTimer(System::Layer * layer, void * aAppState)
{
    WindowCoveringManager * manager = reinterpret_cast<WindowCoveringManager *>(aAppState);

    VerifyOrReturn(manager->mState != OperationalState::Stall);

    Percent100ths currentPosition = manager->mCurrentTiltPosition.Value();
    Percent100ths targetPosition  = manager->mTargetTiltPosition.Value();

    ChipLogProgress(NotSpecified, "HandleTiltMovementTimer:currentPosition:%u, targetPosition:%u", currentPosition, targetPosition);

    if (OperationalState::MovingUpOrOpen == manager->mState)
    {
        if (ComputeOperationalState(targetPosition, currentPosition) != OperationalState::MovingUpOrOpen)
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }

        Percent100ths tempPosition =
            ComputePercent100thsStep(OperationalState::MovingUpOrOpen, currentPosition, kDefaultMovementStep);
        currentPosition = tempPosition > targetPosition ? tempPosition : targetPosition;
        manager->mCurrentTiltPosition.SetNonNull(currentPosition);
    }
    else
    {
        if (ComputeOperationalState(targetPosition, currentPosition) != OperationalState::MovingDownOrClose)
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }

        Percent100ths tempPosition =
            ComputePercent100thsStep(OperationalState::MovingDownOrClose, currentPosition, kDefaultMovementStep);
        currentPosition = tempPosition < targetPosition ? tempPosition : targetPosition;
        manager->mCurrentTiltPosition.SetNonNull(currentPosition);
    }

    TiltPositionSet(manager->mEndpoint, manager->mCurrentTiltPosition);

    if (manager->mCurrentTiltPosition != manager->mTargetTiltPosition)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleTiltMovementTimer, aAppState));
    }
}

CHIP_ERROR WindowCoveringManager::HandleMovement(WindowCoveringType type)
{
    if (type == WindowCoveringType::Lift)
    {
        mState = OperationalStateGet(mEndpoint, OperationalStatus::kLift);
    }
    else
    {
        mState = OperationalStateGet(mEndpoint, OperationalStatus::kTilt);
    }

    if (OperationalState::Stall == mState)
    {
        ChipLogProgress(NotSpecified, "Covering is currently not moving");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    // At least one of the Lift and Tilt features SHALL be supported.
    if (type == WindowCoveringType::Lift)
    {
        // Cancel ongoing window covering movement timer if any.
        DeviceLayer::SystemLayer().CancelTimer(HandleLiftMovementTimer, this);

        Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, mCurrentLiftPosition);
        Attributes::TargetPositionLiftPercent100ths::Get(mEndpoint, mTargetLiftPosition);

        VerifyOrReturnError(!mCurrentLiftPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(!mTargetLiftPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mCurrentLiftPosition != mTargetLiftPosition, CHIP_ERROR_INCORRECT_STATE);

        err = DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleLiftMovementTimer, this);
    }
    else
    {
        // Cancel ongoing window covering movement timer if any.
        DeviceLayer::SystemLayer().CancelTimer(HandleTiltMovementTimer, this);

        Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, mCurrentTiltPosition);
        Attributes::TargetPositionTiltPercent100ths::Get(mEndpoint, mTargetTiltPosition);

        VerifyOrReturnError(!mCurrentTiltPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(!mTargetTiltPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mCurrentTiltPosition != mTargetTiltPosition, CHIP_ERROR_INCORRECT_STATE);

        err = DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleTiltMovementTimer, this);
    }

    return err;
}

CHIP_ERROR WindowCoveringManager::HandleStopMotion()
{

    DeviceLayer::SystemLayer().CancelTimer(HandleLiftMovementTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleTiltMovementTimer, this);
    return CHIP_NO_ERROR;
}
