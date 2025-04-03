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

// Mock Error List generated for sample application usage.
const ClosureErrorEnum kCurrentErrorList[] = {
    {
        ClosureErrorEnum::kBlocked,
    },
    {
        ClosureErrorEnum::kInternalInterference,
    },
    {
        ClosureErrorEnum::kMaintenanceRequired,
    },
    {
        ClosureErrorEnum::kTemperatureLimited,
    },
};

PositioningEnum getStatePositionFromTarget(TagPositionEnum tagPosition)
{
    switch (tagPosition) {
        case TagPositionEnum::kCloseInFull:
            return PositioningEnum::kFullyClosed;
        case TagPositionEnum::kOpenInFull:
            return PositioningEnum::kFullyOpened;
        case TagPositionEnum::kPedestrian:
            return PositioningEnum::kOpenedForPedestrian;
        case TagPositionEnum::kVentilation:
            return PositioningEnum::kOpenedForVentilation;
        case TagPositionEnum::kSignature:
            return PositioningEnum::kOpenedAtSignature;
        default:
            break;
    }
    return PositioningEnum::kUnknownEnumValue;
}

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

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlManager::StartCurrentErrorListRead()
{
    // Notify device that errorlist is being read and data should locked
    return CHIP_NO_ERROR;
}

// TODO: Return emualted error list, will add event handling along with Events
CHIP_ERROR ClosureControlManager::GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError)
{
    VerifyOrReturnError(Index < MATTER_ARRAY_SIZE(kCurrentErrorList),CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
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
Protocols::InteractionModel::Status ClosureControlManager::MoveTo(const Optional<TagPositionEnum> & tag,
                                                                  const Optional<TagLatchEnum> & latch,
                                                                  const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    MainStateEnum state              = mpClosureControlInstance->GetMainState();
    GenericOverallState overallState = mpClosureControlInstance->GetOverallState();

    if (tag.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(tag.Value()) != TagPositionEnum::kUnknownEnumValue,
                            Status::ConstraintError);
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kPositioning), Status::Success);
        ChipLogDetail(DataManagement, "ClosureControlManager::positioning");

        // TODO: CheckErrorondevice() -> if device error state move state to error and give failure
        // TODO: IsDeviceReadytoMove()

        if ((state == MainStateEnum::kStopped) || (state == MainStateEnum::kError))
        {
            // TODO: set target for motion
        }

        if ((state == MainStateEnum::kWaitingForMotion) || (state == MainStateEnum::kMoving))
        {
            // TODO: change target for motion
        }

        overallState.positioning.SetValue(getStatePositionFromTarget(tag.Value()));
    }

    if (latch.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(latch.Value()) == TagLatchEnum::kUnknownEnumValue,
                            Status::ConstraintError);
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kMotionLatching), Status::Success);
        ChipLogDetail(DataManagement, "ClosureControlManager::latch");
        VerifyOrReturnValue(IsManualLatch(), Status::InvalidAction);

        // TODO: device to perform latch operation

        if (latch.Value() == TagLatchEnum::kLatch)
        {
            overallState.latching.SetValue(LatchingEnum::kLatchedAndSecured);
        }
        if (latch.Value() == TagLatchEnum::kUnlatch)
        {
            overallState.latching.SetValue(LatchingEnum::kNotLatched);
        }
    }

    if (speed.HasValue())
    {
        VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(speed.Value()) == Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                            Status::ConstraintError);
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kSpeed), Status::Success);
        ChipLogDetail(DataManagement, "ClosureControlManager::speed");
        if (!(latch.HasValue() || tag.HasValue()))
        {
            if ((state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
            {
                overallState.speed = speed;
                // TODO: device to change speed
            }
            else
            {
                overallState.speed.SetValue(Globals::ThreeLevelAutoEnum::kAuto);
            }
        }
    }

    mpClosureControlInstance->SetOverallState(overallState);

    if (IsDeviceReadytoMove())
    {
        mpClosureControlInstance->SetMainState(MainStateEnum::kMoving);
        // TODO: move the device
    }
    else
    {
        mpClosureControlInstance->SetMainState(MainStateEnum::kWaitingForMotion);
        // TODO: device to wait for ready to move and the move the device.
    }

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

bool ClosureControlManager::IsManualLatch()
{
    // TODO: Check the latch is manual or not on device
    return false;
}

bool ClosureControlManager::IsDeviceReadytoMove()
{
    // TODO: Check if device is ready to move or should wait.
    return true;
}
