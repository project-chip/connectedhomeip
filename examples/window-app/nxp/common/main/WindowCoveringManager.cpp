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

#include "WindowCoveringManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip::app;
using namespace chip::app::Clusters::WindowCovering;

static constexpr uint32_t sMoveTimeoutMs{ 100 };
constexpr uint16_t sPercentDelta   = 500;
constexpr uint8_t kDefaultMinLevel = 0;
constexpr uint8_t kDefaultMaxLevel = 254;

WindowCoveringManager::WindowCoveringManager()
{
	//Initialize window covering here
	ChipLogProgress(DeviceLayer, "########## Window Covering Manager Initialized. ##########");
}

void WindowCoveringManager::DriveCurrentLiftPosition(intptr_t)
{
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current) == Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::TargetPositionLiftPercent100ths::Get(Endpoint(), target) == Protocols::InteractionModel::Status::Success);

    UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));

    ChipLogProgress(DeviceLayer, "++++++++++++ DriveCurrentLiftPosition: Target position: %d percent closed, Type: Lift ++++++++++++",target.Value()/100);

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Lift));
	
	LiftPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    WindowCoveringMgr().mInLiftMove = false;

    VerifyOrReturn(Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), current) == Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(WindowCoveringType::Lift, current, target))
    {
        // continue to move
		ChipLogProgress(DeviceLayer, "************************ continue to move ************************");
        StartTimer(WindowCoveringType::Lift, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-lift-movement after the target is completed
        UpdateOperationalStatus(WindowCoveringType::Lift, ComputeOperationalState(target, current));
		ChipLogProgress(DeviceLayer, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		ChipLogProgress(DeviceLayer, "************ Move(WindowCoveringType::Lift) Completed ************");
		ChipLogProgress(DeviceLayer, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
}

chip::Percent100ths WindowCoveringManager::CalculateSingleStep(WindowCoveringType aMoveType)
{
    Protocols::InteractionModel::Status status;
    chip::Percent100ths percent100ths{};
	chip::Percent100ths percentToSet{};
    NPercent100ths current{};
	NPercent100ths target_lift{};
	NPercent100ths target_tilt{};
    OperationalState opState = OperationalState::Stall;

	if (Attributes::TargetPositionLiftPercent100ths::Get(Endpoint(), target_lift) != Protocols::InteractionModel::Status::Success)
	{
		ChipLogError(DeviceLayer, "Cannot read the target lift position.");
	}
	
	if (Attributes::TargetPositionTiltPercent100ths::Get(Endpoint(), target_tilt) != Protocols::InteractionModel::Status::Success)
	{
		ChipLogError(DeviceLayer, "Cannot read the target tilt position.");
	}
	
	Percent100ths targetLiftPosition  = target_lift.Value();
	Percent100ths targetTiltPosition  = target_tilt.Value();


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

    if ((status == Protocols::InteractionModel::Status::Success) && !current.IsNull())
    {
        percent100ths = ComputePercent100thsStep(opState, current.Value(), sPercentDelta);
    }
    else
    {
        ChipLogError(DeviceLayer, "Cannot read the current lift position. Error: %d", static_cast<uint8_t>(status));
    }
	
	if (aMoveType == WindowCoveringType::Lift)
    {
		if (abs(percent100ths - targetLiftPosition) < sPercentDelta)
		{
			percentToSet = targetLiftPosition;
		}else{
			percentToSet = percent100ths;
		}
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
		if (abs(percent100ths - targetTiltPosition) < sPercentDelta)
		{
			percentToSet = targetTiltPosition;
		}else{
			percentToSet = percent100ths;
		}
    }
	
    return percentToSet;
}

bool WindowCoveringManager::TargetCompleted(WindowCoveringType aMoveType, NPercent100ths aCurrent, NPercent100ths aTarget)
{
    return (OperationalState::Stall == ComputeOperationalState(aTarget, aCurrent));
}

void WindowCoveringManager::StartTimer(WindowCoveringType aMoveType, uint32_t aTimeoutMs)
{
    WindowCoveringType * moveType = chip::Platform::New<WindowCoveringType>();
    VerifyOrReturn(moveType != nullptr);

    *moveType = aMoveType;
    (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutMs), MoveTimerTimeoutCallback,
                                                       reinterpret_cast<void *>(moveType));
}

void WindowCoveringManager::MoveTimerTimeoutCallback(chip::System::Layer * systemLayer, void * appState)
{
    WindowCoveringType * moveType = reinterpret_cast<WindowCoveringType *>(appState);
    VerifyOrReturn(moveType != nullptr);

    if (*moveType == WindowCoveringType::Lift)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCoveringManager::DriveCurrentLiftPosition);
    }
    else if (*moveType == WindowCoveringType::Tilt)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(WindowCoveringManager::DriveCurrentTiltPosition);
    }

    chip::Platform::Delete(moveType);
}

void WindowCoveringManager::DriveCurrentTiltPosition(intptr_t)
{
    NPercent100ths current{};
    NPercent100ths target{};
    NPercent100ths positionToSet{};

    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current) == Protocols::InteractionModel::Status::Success);
    VerifyOrReturn(Attributes::TargetPositionTiltPercent100ths::Get(Endpoint(), target) == Protocols::InteractionModel::Status::Success);

    UpdateOperationalStatus(WindowCoveringType::Tilt, ComputeOperationalState(target, current));
	
	ChipLogProgress(DeviceLayer, "++++++++++++ DriveCurrentTiltPosition: Target position: %d percent closed, Type: Tilt ++++++++++++",target.Value()/100);

    positionToSet.SetNonNull(CalculateSingleStep(WindowCoveringType::Tilt));

    TiltPositionSet(Endpoint(), positionToSet);

    // assume single move completed
    WindowCoveringMgr().mInTiltMove = false;

    VerifyOrReturn(Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), current) == Protocols::InteractionModel::Status::Success);

    if (!TargetCompleted(WindowCoveringType::Tilt, current, target))
    {
        // continue to move
        StartTimer(WindowCoveringType::Tilt, sMoveTimeoutMs);
    }
    else
    {
        // the OperationalStatus should indicate no-tilt-movement after the target is completed
        UpdateOperationalStatus(WindowCoveringType::Tilt, ComputeOperationalState(target, current));
		ChipLogProgress(DeviceLayer, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		ChipLogProgress(DeviceLayer, "************ Move(WindowCoveringType::Tilt) Completed ************");
		ChipLogProgress(DeviceLayer, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
}

void WindowCoveringManager::StartMove(WindowCoveringType aMoveType)
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

void WindowCoveringManager::SetSingleStepTarget(OperationalState aDirection)
{
    UpdateOperationalStatus(mCurrentUIMoveType, aDirection);
    SetTargetPosition(aDirection, CalculateSingleStep(mCurrentUIMoveType));
}

void WindowCoveringManager::UpdateOperationalStatus(WindowCoveringType aMoveType, OperationalState aDirection)
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

void WindowCoveringManager::SetTargetPosition(OperationalState aDirection, chip::Percent100ths aPosition)
{
    Protocols::InteractionModel::Status status;
    if (WindowCoveringMgr().mCurrentUIMoveType == WindowCoveringType::Lift)
    {
        status = Attributes::TargetPositionLiftPercent100ths::Set(Endpoint(), aPosition);
    }
    else if (WindowCoveringMgr().mCurrentUIMoveType == WindowCoveringType::Tilt)
    {
        status = Attributes::TargetPositionTiltPercent100ths::Set(Endpoint(), aPosition);
    }

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer,"Cannot set the target position. Error: %d", static_cast<uint8_t>(status));
    }
}

void WindowCoveringManager::PositionUpdate(WindowCoveringType aMoveType)
{
    Protocols::InteractionModel::Status status;
    NPercent100ths currentPosition{};

    if (aMoveType == WindowCoveringType::Lift)
    {
        status = Attributes::CurrentPositionLiftPercent100ths::Get(Endpoint(), currentPosition);
        if (Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
			ChipLogProgress(DeviceLayer, "############ Current position: %d percent closed, Type: Lift ############",currentPosition.Value()/100);
        }
    }
    else if (aMoveType == WindowCoveringType::Tilt)
    {
        status = Attributes::CurrentPositionTiltPercent100ths::Get(Endpoint(), currentPosition);
        if (Protocols::InteractionModel::Status::Success == status && !currentPosition.IsNull())
        {
            ChipLogProgress(DeviceLayer, "############ Current position: %d percent closed, Type: Tilt ############",currentPosition.Value()/100);
        }
    }
}

void WindowCoveringManager::SchedulePostAttributeChange(chip::EndpointId aEndpoint, chip::AttributeId aAttributeId)
{
    AttributeUpdateData * data = chip::Platform::New<AttributeUpdateData>();
    VerifyOrReturn(data != nullptr);

    data->mEndpoint    = aEndpoint;
    data->mAttributeId = aAttributeId;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(DoPostAttributeChange, reinterpret_cast<intptr_t>(data));
}

void WindowCoveringManager::DoPostAttributeChange(intptr_t aArg)
{
    AttributeUpdateData * data = reinterpret_cast<AttributeUpdateData *>(aArg);
    VerifyOrReturn(data != nullptr);

    PostAttributeChange(data->mEndpoint, data->mAttributeId);
}