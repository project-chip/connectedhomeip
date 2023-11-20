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
#include "microwave-oven-device.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using OperationalStateEnum = chip::app::Clusters::OperationalState::OperationalStateEnum;
using Status               = Protocols::InteractionModel::Status;

Protocols::InteractionModel::Status
MicrowaveOvenDevice::HandleMicrowaveOvenSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTime, uint8_t powerSetting)
{
    // placeholder implementation
    // TODO: set Microwave Oven cooking mode by cookMode.Value().
    mMicrowaveOvenControlInstance.SetCookTime(cookTime);
    mMicrowaveOvenControlInstance.SetPowerSetting(powerSetting);
    return Status::Success;
}

Protocols::InteractionModel::Status MicrowaveOvenDevice::HandleMicrowaveOvenAddMoreTimeCallback(uint32_t addedCookTime)
{
    // placeholder implementation
    mMicrowaveOvenControlInstance.SetCookTime(addedCookTime);
    return Status::Success;
}

void MicrowaveOvenDevice::HandleMicrowaveOvenOpStatePasueCallback(OperationalState::GenericOperationalError & err)
{
    // placeholder implementation
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kPaused));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDevice::HandleMicrowaveOvenOpStateResumeCallback(OperationalState::GenericOperationalError & err)
{
    // placeholder implementation
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDevice::HandleMicrowaveOvenOpStateStartCallback(OperationalState::GenericOperationalError & err)
{
    // placeholder implementation
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

void MicrowaveOvenDevice::HandleMicrowaveOvenOpStateStopCallback(OperationalState::GenericOperationalError & err)
{
    // placeholder implementation
    auto error = mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
    if (error == CHIP_NO_ERROR)
    {
        err.Set(to_underlying(ErrorStateEnum::kNoError));
    }
    else
    {
        err.Set(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    }
}

app::DataModel::Nullable<uint32_t> MicrowaveOvenDevice::HandleMicrowaveOvenOpStateGetCountdownTime()
{
    return static_cast<app::DataModel::Nullable<uint32_t>>(mMicrowaveOvenControlInstance.GetCookTime());
}

void MicrowaveOvenDevice::Init(EndpointId aEndpoint)
{
    mOperationalStateInstance.SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    mOperationalStateInstance.Init();
    MicrowaveOvenControl::SetOPInstance(aEndpoint, &mOperationalStateInstance);
    mMicrowaveOvenControlInstance.Init();
}
