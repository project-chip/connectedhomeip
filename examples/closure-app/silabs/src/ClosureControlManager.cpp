/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <ClosureControlManager.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

using Protocols::InteractionModel::Status;

namespace {

// Mock Error List generated for sample application usage.
const ClosureErrorEnum kCurrentErrorList[] = {
    ClosureErrorEnum::kBlocked,
    ClosureErrorEnum::kInternalInterference,
    ClosureErrorEnum::kMaintenanceRequired,
    ClosureErrorEnum::kTemperatureLimited,
};

PositioningEnum GetStatePositionFromTarget(TargetPositionEnum tagPosition)
{
    switch (tagPosition)
    {
    case TargetPositionEnum::kCloseInFull:
        return PositioningEnum::kFullyClosed;
    case TargetPositionEnum::kOpenInFull:
        return PositioningEnum::kFullyOpened;
    case TargetPositionEnum::kPedestrian:
        return PositioningEnum::kOpenedForPedestrian;
    case TargetPositionEnum::kVentilation:
        return PositioningEnum::kOpenedForVentilation;
    case TargetPositionEnum::kSignature:
        return PositioningEnum::kOpenedAtSignature;
    default:
        break;
    }
    return PositioningEnum::kUnknownEnumValue;
}

} // namespace

void ClosureControlManager::SetClosureControlInstance(ClosureControl::Instance & instance)
{
    mpClosureControlInstance = &instance;
}

ClosureControlManager ClosureControlManager::sClosureCtrlMgr;

/*********************************************************************************
 *
 * Methods implementing the ClosureControl::Delegate interace
 *
 *********************************************************************************/

// Return default value, will add timers and attribute handling in next phase
DataModel::Nullable<uint32_t> ClosureControlManager::GetCountdownTime()
{
    // TODO: handle countdown timer
    return DataModel::NullNullable;
}

// TODO: try to Single API for start , read and end
CHIP_ERROR ClosureControlManager::StartCurrentErrorListRead()
{
    // Notify device that errorlist is being read and data should locked
    return CHIP_NO_ERROR;
}

// TODO: Return emualted error list, will add event handling along with Events
CHIP_ERROR ClosureControlManager::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    VerifyOrReturnError(Index < MATTER_ARRAY_SIZE(kCurrentErrorList), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    closureError = kCurrentErrorList[Index];
    return CHIP_NO_ERROR;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlManager::EndCurrentErrorListRead()
{
    // Notify device that errorlist is being read completed and lock on data is removed
    return CHIP_NO_ERROR;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::Stop()
{
    // TODO: device should stop the action
    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::MoveTo(const Optional<TargetPositionEnum> & tag,
                                                                  const Optional<TargetLatchEnum> & latch,
                                                                  const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    MainStateEnum state              = mpClosureControlInstance->GetMainState();
    GenericOverallState overallState = mpClosureControlInstance->GetOverallState();

    if (tag.HasValue())
    {
        if ((state == MainStateEnum::kStopped) || (state == MainStateEnum::kError))
        {
            // TODO: set target for motion
        }

        if ((state == MainStateEnum::kWaitingForMotion) || (state == MainStateEnum::kMoving))
        {
            // TODO: change target for motion
        }

        overallState.positioning.SetValue(GetStatePositionFromTarget(tag.Value()));
    }

    if (latch.HasValue())
    {
        // TODO: device to perform latch operation

        if (latch.Value() == TargetLatchEnum::kLatch)
        {
            overallState.latching.SetValue(LatchingEnum::kLatchedAndSecured);
        }
        if (latch.Value() == TargetLatchEnum::kUnlatch)
        {
            overallState.latching.SetValue(LatchingEnum::kNotLatched);
        }
    }

    if (speed.HasValue())
    {
        overallState.speed = speed;
        // TODO: device to change speed
    }

    mpClosureControlInstance->SetOverallState(overallState);
    // TODO: move the device or device to wait for ready to move and the move the device

    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::Calibrate()
{
    // TODO: Calibrate Device
    return Status::Success;
}

void ClosureControlManager::ClosureControlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId)
{
    switch (attributeId)
    {
    case Attributes::CountdownTime::Id:
        // Display CountdownTime in UI
        break;
    case Attributes::MainState::Id:
        // Display Mainstate in UI
        break;
    case Attributes::CurrentErrorList::Id:
        // Display ErrorList in UI
        break;
    case Attributes::OverallState::Id:
        // Display Overallstate in UI
        break;
    case Attributes::OverallTarget::Id:
        // Display TargetState in UI
        break;
    default:
        return;
    }
}

bool ClosureControlManager::IsLatchManual()
{
    // TODO: Check the latch is manual or not on device
    return false;
}

bool ClosureControlManager::CheckErrorOnDevice()
{
    // TODO: Check for errors on the device
    return false;
}

bool ClosureControlManager::IsDeviceReadyToMove()
{
    // TODO: Check if device is ready to move or should wait.
    return true;
}
