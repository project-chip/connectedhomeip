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
    mCurrentPosition.SetNonNull(static_cast<uint16_t>(0));
    mTargetPosition.SetNonNull(static_cast<uint16_t>(0));
    SetEndpoint(endpoint);
}

void WindowCoveringManager::HandleMovementTimer(System::Layer * layer, void * aAppState)
{
    WindowCoveringManager * manager = reinterpret_cast<WindowCoveringManager *>(aAppState);

    VerifyOrReturn(manager->mState != OperationalState::Stall);

    uint16_t currentPosition = manager->mCurrentPosition.Value();
    uint16_t targetPosition  = manager->mTargetPosition.Value();

    ChipLogProgress(NotSpecified, "HandleMovementTimer:currentPosition:%u, targetPosition:%u", currentPosition, targetPosition);

    if (OperationalState::MovingUpOrOpen == manager->mState)
    {
        if (currentPosition > targetPosition)
        {
            uint16_t tempPosition =
                currentPosition > kDefaultMovementStep ? static_cast<uint16_t>(currentPosition - kDefaultMovementStep) : 0;
            currentPosition = tempPosition > targetPosition ? tempPosition : targetPosition;

            manager->mCurrentPosition.SetNonNull(currentPosition);
        }
        else
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }
    }
    else
    {
        if (currentPosition < targetPosition)
        {
            uint16_t tempPosition = static_cast<uint16_t>(currentPosition + kDefaultMovementStep);
            currentPosition       = tempPosition < targetPosition ? tempPosition : targetPosition;
            manager->mCurrentPosition.SetNonNull(currentPosition);
        }
        else
        {
            ChipLogProgress(NotSpecified, "Reached the target position");
            return;
        }
    }

    if (HasFeature(manager->mEndpoint, Feature::kPositionAwareLift))
    {
        LiftPositionSet(manager->mEndpoint, manager->mCurrentPosition);
    }

    if (HasFeature(manager->mEndpoint, Feature::kPositionAwareTilt))
    {
        TiltPositionSet(manager->mEndpoint, manager->mCurrentPosition);
    }

    if (manager->mCurrentPosition != manager->mTargetPosition)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleMovementTimer, aAppState));
    }
}

CHIP_ERROR WindowCoveringManager::HandleMovement(WindowCoveringType type)
{
    mState = OperationalStateGet(mEndpoint, OperationalStatus::kGlobal);

    if (OperationalState::Stall == mState)
    {
        ChipLogProgress(NotSpecified, "Covering is currently not moving");
        return CHIP_NO_ERROR;
    }

    // Cancel ongoing window covering movement timer if any.
    DeviceLayer::SystemLayer().CancelTimer(HandleMovementTimer, this);

    // At least one of the Lift and Tilt features SHALL be supported.
    if (type == WindowCoveringType::Lift)
    {
        Attributes::CurrentPositionLiftPercent100ths::Get(mEndpoint, mCurrentPosition);
        Attributes::TargetPositionLiftPercent100ths::Get(mEndpoint, mTargetPosition);
    }
    else
    {
        Attributes::CurrentPositionTiltPercent100ths::Get(mEndpoint, mCurrentPosition);
        Attributes::TargetPositionTiltPercent100ths::Get(mEndpoint, mTargetPosition);
    }

    VerifyOrReturnError(!mCurrentPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mTargetPosition.IsNull(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mCurrentPosition != mTargetPosition, CHIP_ERROR_INCORRECT_STATE);

    return DeviceLayer::SystemLayer().StartTimer(kIncrementMovementTimeout, HandleMovementTimer, this);
}

CHIP_ERROR WindowCoveringManager::HandleStopMotion()
{

    DeviceLayer::SystemLayer().CancelTimer(HandleMovementTimer, this);
    return CHIP_NO_ERROR;
}
