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

ClosureControlManager::ClosureControlManager(EndpointId clustersEndpoint) : mpClosureControlInstance(nullptr) {}

void ClosureControlManager::SetClosureControlInstance(ClosureControl::Instance & instance)
{
    mpClosureControlInstance = &instance;
}

/*********************************************************************************
 *
 * Methods implementing the ClosureControl::Delegate interace
 *
 *********************************************************************************/

// Return default value, will add timers and attribute handling in next phase
DataModel::Nullable<uint32_t> ClosureControlManager::GetCountdownTime()
{
    //TODO: handle countdown timer
    return DataModel::NullNullable;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlManager::StartCurrentErrorListRead()
{
    //Notify device that errorlist is being read and data should locked
    return CHIP_NO_ERROR;
}

// Return default value, will add attribute handling in next phase
CHIP_ERROR ClosureControlManager::EndCurrentErrorListRead()
{
    //Notify device that errorlist is being read completed and lock on data is removed
    return CHIP_NO_ERROR;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::Stop()
{
    MainStateEnum state = mpClosureControlInstance->GetMainState();
    
    VerifyOrReturnValue(CheckCommandStateCompatiblilty(Commands::Stop::Id,state), Status::InvalidInState);
    VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kMotionLatching),Status::Success);
    
    //TODO: For products supporting Positioning(PS) with a 100.00% resolution, the Stop() command MAY be ignored and  the device SHALL respond SUCCESS
    
    if ((state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
    {
        mpClosureControlInstance->SetMainState(MainStateEnum::kStopped); 
    }
    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::MoveTo(const Optional<TagPositionEnum> & tag,
                                                                   const Optional<TagLatchEnum> & latch,
                                                                   const Optional<Globals::ThreeLevelAutoEnum> & speed)
{
    MainStateEnum state = mpClosureControlInstance->GetMainState();
    
    VerifyOrReturnValue(CheckCommandStateCompatiblilty(Commands::MoveTo::Id,state), Status::InvalidInState);
    VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(tag.Value()) == TagPositionEnum::kUnknownEnumValue,Status::ConstraintError);
    VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(latch.Value()) == TagLatchEnum::kUnknownEnumValue,Status::ConstraintError);
    VerifyOrReturnValue(Clusters::EnsureKnownEnumValue(speed.Value()) == Globals::ThreeLevelAutoEnum::kUnknownEnumValue,Status::ConstraintError);
    
    GenericOverallState overallstate = mpClosureControlInstance->GetOverallState();

    if(tag.HasValue()) {
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kPositioning),Status::Success);
        
        //TODO: not supported by the device (linked feature is disabled) then a status code NOT_FOUND SHALL be returned.
        
        //TODO: IsMotionRequired() -> check for any error prohibiting movement
        //TODO: IsDeviceReadytoMove()
        
        if ((state == MainStateEnum::kStopped) || (state == MainStateEnum::kError)) {
            //TODO: DoMotion()
        }
        
        if ((state == MainStateEnum::kWaitingForMotion) || (state == MainStateEnum::kMoving) )
        {
            //TODO: Changetarget
        }

        //TODO - Update overallstate from target state ( overallstate.positioning = tag;) - add enumMap
        mpClosureControlInstance->SetOverallState(overallstate);
        
    }

    if(latch.HasValue()){
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kMotionLatching),Status::Success);
        VerifyOrReturnValue(IsManualLatch(),Status::InvalidAction);
        
        //TODO: Latch Device
        
        //TODO - Update overallstate from target state ( overallstate.latching = latch;) - add enum Map
        mpClosureControlInstance->SetOverallState(overallstate);
    }
    
    if(speed.HasValue()){
        VerifyOrReturnValue(mpClosureControlInstance->HasFeature(Feature::kSpeed),Status::Success);
        
        if(!(latch.HasValue() || tag.HasValue())) {
            if ((state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion) )
            {
                overallstate.speed = speed;
                mpClosureControlInstance->SetOverallState(overallstate);
                //TODO: change speed of device
            }
        }
        
        //TODO: if Speed(SP) feature is enabled while in Moving or WaitingForMotion states then:
        //if the Speed can be adjusted then OverallState. Speed is updated with the Speed Field.
        //if the Speed is not applicable then OverallState. Speed is updated with Auto.
    }
    
    if(IsDeviceReadytoMove()) {
        mpClosureControlInstance->SetMainState(MainStateEnum::kMoving);
    } else {
        mpClosureControlInstance->SetMainState(MainStateEnum::kWaitingForMotion);
    }
    
    return Status::Success;
}

// Return default success, will add command handling in next phase
Protocols::InteractionModel::Status ClosureControlManager::Calibrate()
{
    MainStateEnum state = mpClosureControlInstance->GetMainState();
    VerifyOrReturnValue(CheckCommandStateCompatiblilty(Commands::Calibrate::Id,state), Status::InvalidInState);
    
    //TODO: This command SHALL be effective over Closure elements on the same Endpoint and Sub-Endpoint of the wrapping device type. Procedures of how to achieve the calibration are manufacturers specific
    if ((state == MainStateEnum::kStopped))
    {
        //TODO: Calibrate Device
        mpClosureControlInstance->SetMainState(MainStateEnum::kCalibrating);
    }
    
    return Status::Success;
}

void ClosureControlManager::ClosureControlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size) {
    switch (attributeId)
    {
    case Attributes::CountdownTime::Id:
        //Display CountdownTime in UI
        break;
    case Attributes::MainState::Id:
        //Display Mainstate in UI
        break;
    case Attributes::CurrentErrorList::Id:
        //Display ErrorList in UI
        break;
    case Attributes::OverallState::Id:
        //Display Overallstate in UI
        break;
    case Attributes::OverallTarget::Id: 
        //Display TargetState in UI
        break;
    default:
        return;
    }
}

bool ClosureControlManager::CheckCommandStateCompatiblilty(CommandId cmd,MainStateEnum state){
    
    if ((state == MainStateEnum::kDisengaged) || (state == MainStateEnum::kProtected) || (state == MainStateEnum::kSetupRequired))
    {
        return false;
    }
    
    switch (cmd)
    {
    case Commands::Stop::Id:
        VerifyOrReturnValue(state == MainStateEnum::kError,false);
        break;
    case Commands::MoveTo::Id:
        VerifyOrReturnValue(state == MainStateEnum::kCalibrating,false);
        break;
    case Commands::Calibrate::Id:
        VerifyOrReturnValue(((state != MainStateEnum::kCalibrating) && (state != MainStateEnum::kStopped)),false);
        break;
    default:
        return false;
    }
    
    return false;
}

bool ClosureControlManager::IsManualLatch(){
    //TODO: Check the latch is manual or not on device
    return false;
}

bool ClosureControlManager::IsDeviceReadytoMove(){
    //TODO: Check if device is ready to move or should wait.
    return true;
}
