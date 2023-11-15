/*
 *
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
#include <microwave-oven-delegates-impl.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::OperationalState;
using OperationalStateEnum = chip::app::Clusters::OperationalState::OperationalStateEnum;
using Status               = Protocols::InteractionModel::Status;


namespace {

constexpr uint32_t kMaxCookTime                                = 65535;
constexpr uint32_t kMinCookTime                                = 1;

OperationalState::Instance * gOperationalStateInstance         = nullptr;
MicrowaveOvenControl::Instance * gMicrowaveOvenControlInstance = nullptr;
MicrowaveOvenDelegates * gMicrowaveOvenDelegates               = nullptr;

} // anonymous namespace

//Operational State command callbacks
app::DataModel::Nullable<uint32_t> MicrowaveOvenDelegates::GetCountdownTime()
{
    VerifyOrDie(gMicrowaveOvenControlInstance != nullptr);
    return static_cast<app::DataModel::Nullable<uint32_t>>(gMicrowaveOvenControlInstance->GetCookTime());
}

CHIP_ERROR MicrowaveOvenDelegates::GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState)
{
    if (index > mOperationalStateList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR MicrowaveOvenDelegates::GetOperationalPhaseAtIndex(size_t index, OperationalState::GenericOperationalPhase & operationalPhase)
{
    if (index > mOperationalPhaseList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalPhase = mOperationalPhaseList[index];
    return CHIP_NO_ERROR;
}

void MicrowaveOvenDelegates::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
{
    VerifyOrDie(gOperationalStateInstance != nullptr);
    // placeholder implementation
    auto error = gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDelegates::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
{
    VerifyOrDie(gOperationalStateInstance != nullptr);
    // placeholder implementation
    auto error = gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDelegates::HandleStartStateCallback(OperationalState::GenericOperationalError & err)
{
    VerifyOrDie(gOperationalStateInstance != nullptr);
    // placeholder implementation
    auto error = gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDelegates::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
{
    VerifyOrDie(gOperationalStateInstance != nullptr);
    // placeholder implementation
    auto error = gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}


//Microwave Oven Control command callbacks
Protocols::InteractionModel::Status MicrowaveOvenDelegates::HandleSetCookingParametersCallback(Optional<uint8_t> cookMode,Optional<uint32_t> cookTime,Optional<uint8_t> powerSetting)
{
    VerifyOrDie(gOperationalStateInstance != nullptr && gMicrowaveOvenControlInstance != nullptr);
    uint8_t opState;
    
    //TODO: check if the input cook mode value is invalid

    if(cookTime.HasValue() && (!IsCookTimeInRange(cookTime.Value())))
    {
        ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
        return Status::InvalidCommand;
    }

    if(powerSetting.HasValue() && (!IsPowerSettingInRange(powerSetting.Value(),GetMinPower(),GetMaxPower())))
    {
        ChipLogError(Zcl, "Failed to set cookPower, cookPower value is out of range");
        return Status::InvalidCommand;
    }

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    ChipLogError(Zcl, "Current Operational State = %d", opState);
    if (opState == to_underlying(OperationalStateEnum::kStopped))
    {
        if (cookMode.HasValue())
        {
            // TODO: set Microwave Oven cooking mode by cookMode.Value().
        }
        else
        {
            // TODO: set Microwave Oven cooking mode to normal mode.
        }

        if (cookTime.HasValue())
        {
            gMicrowaveOvenControlInstance->SetCookTime(cookTime.Value());
        }
        else
        {
            // set Microwave Oven cooking time to 30 seconds(default).
            gMicrowaveOvenControlInstance->SetCookTime(30);
        }

        if (powerSetting.HasValue())
        {
            gMicrowaveOvenControlInstance->SetPowerSetting(powerSetting.Value());
        }
        else
        {
            // set Microwave Oven cooking power to max power(default).
            gMicrowaveOvenControlInstance->SetPowerSetting(GetMaxPower());
        }
        return Status::Success;
    }
    else
    {
        return Status::InvalidInState;
    }
}

Protocols::InteractionModel::Status MicrowaveOvenDelegates::HandleAddMoreTimeCallback(uint32_t timeToAdd)
{
    VerifyOrDie(gOperationalStateInstance != nullptr && gMicrowaveOvenControlInstance != nullptr);
    uint8_t opState;

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    ChipLogError(Zcl, "Current Operational State = %d", opState);
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kRunning) ||
        opState == to_underlying(OperationalStateEnum::kPaused))
    {
        uint32_t addedCookTime;

        addedCookTime = gMicrowaveOvenControlInstance->GetCookTime() + timeToAdd;
        // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
        if (addedCookTime < kMaxCookTime)
        {
            gMicrowaveOvenControlInstance->SetCookTime(addedCookTime);
            return Status::Success;
        }
        else
        {
            ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
            return Status::ConstraintError;
        }
    }
    else // operational state is in error
    {
        return Status::InvalidInState;
    }
}

bool MicrowaveOvenDelegates::IsCookTimeInRange(uint32_t cookTime)
{
    return (cookTime < kMinCookTime || cookTime > kMaxCookTime) ? false : true;
}

bool MicrowaveOvenDelegates::IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower)
{
    return (powerSetting < minCookPower || powerSetting > maxCookPower) ? false : true;
}

void MicrowaveOvenControl::Shutdown()
{
    if (gOperationalStateInstance != nullptr)
    {
        delete gOperationalStateInstance;
        gOperationalStateInstance = nullptr;
    }
    if (gMicrowaveOvenControlInstance != nullptr)
    {
        delete gMicrowaveOvenControlInstance;
        gMicrowaveOvenControlInstance = nullptr;
    }
    
    if (gMicrowaveOvenDelegates != nullptr)
    {
        delete gMicrowaveOvenDelegates;
        gMicrowaveOvenDelegates = nullptr;
    }
}


void emberAfMicrowaveOvenControlClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(gOperationalStateInstance == nullptr && gMicrowaveOvenDelegates  == nullptr && gMicrowaveOvenControlInstance == nullptr);
    gMicrowaveOvenDelegates           = new MicrowaveOvenDelegates;
    EndpointId aEndpoint = 0x01;
    gOperationalStateInstance         = new OperationalState::Instance(gMicrowaveOvenDelegates, aEndpoint, Clusters::OperationalState::Id);
    gMicrowaveOvenControlInstance     = new MicrowaveOvenControl::Instance(gMicrowaveOvenDelegates, aEndpoint, Clusters::MicrowaveOvenControl::Id);
    
    //Initialize instance of Operational state cluster
    gOperationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    gOperationalStateInstance->Init();

    //Initialize instance of Microwave Oven Control cluster
    gMicrowaveOvenControlInstance->Init();
}

void  emberAfOperationalStateClusterInitCallback(chip::EndpointId endpointId){}