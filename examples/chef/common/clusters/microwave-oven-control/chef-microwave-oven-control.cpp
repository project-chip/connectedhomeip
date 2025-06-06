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

#include "chef-microwave-oven-control.h"
#include "../../chef-operational-state-delegate-impl.h"
#include "../microwave-oven-mode/chef-microwave-oven-mode.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

using ModeBase::StatusCode;
using OperationalState::ErrorStateEnum;
using OperationalState::OperationalStateEnum;

using namespace chip::app::Clusters::MicrowaveOvenControl;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

template <typename T>
using List = chip::app::DataModel::List<T>;
using Status = Protocols::InteractionModel::Status;

#ifdef MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER

void ChefMicrowaveOvenDevice::MicrowaveOvenInit()
{
    OperationalState::GetOperationalStateInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
    OperationalState::GetOperationalStateInstance()->Init();
    MicrowaveOvenMode::GetInstance()->Init();
    mMicrowaveOvenControlInstance.Init();
}

Protocols::InteractionModel::Status
ChefMicrowaveOvenDevice::HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec, bool startAfterSetting,
                                                            Optional<uint8_t> powerSettingNum, Optional<uint8_t> wattSettingIndex)
{
    Status status;
    if ((status = MicrowaveOvenMode::GetInstance()->UpdateCurrentMode(cookMode)) != Status::Success)
    {
        return status;
    }

    mMicrowaveOvenControlInstance.SetCookTimeSec(cookTimeSec);

    if (powerSettingNum.HasValue())
    {
        mPowerSettingNum = powerSettingNum.Value();
    }

    if (startAfterSetting)
    {
        OperationalState::GetOperationalStateInstance()->SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    }
    return Status::Success;
}

Protocols::InteractionModel::Status ChefMicrowaveOvenDevice::HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec)
{
    mMicrowaveOvenControlInstance.SetCookTimeSec(finalCookTimeSec);
    return Status::Success;
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

app::DataModel::Nullable<uint32_t> ChefMicrowaveOvenDevice::GetCountdownTime()
{
    return OperationalState::GetOperationalStateDelegate()->GetCountdownTime();
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetOperationalStateAtIndex(
    size_t index, chip::app::Clusters::OperationalState::GenericOperationalState & operationalState)
{
    return OperationalState::GetOperationalStateDelegate()->GetOperationalStateAtIndex(index, operationalState);
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    return OperationalState::GetOperationalStateDelegate()->GetOperationalPhaseAtIndex(index, operationalPhase);
}

void ChefMicrowaveOvenDevice::HandlePauseStateCallback(chip::app::Clusters::OperationalState::GenericOperationalError & err)
{
    OperationalState::GetOperationalStateDelegate()->HandlePauseStateCallback(err);
}

void ChefMicrowaveOvenDevice::HandleResumeStateCallback(chip::app::Clusters::OperationalState::GenericOperationalError & err)
{
    OperationalState::GetOperationalStateDelegate()->HandleResumeStateCallback(err);
}

void ChefMicrowaveOvenDevice::HandleStartStateCallback(chip::app::Clusters::OperationalState::GenericOperationalError & err)
{
    OperationalState::GetOperationalStateDelegate()->HandleStartStateCallback(err);
}

void ChefMicrowaveOvenDevice::HandleStopStateCallback(chip::app::Clusters::OperationalState::GenericOperationalError & err)
{
    OperationalState::GetOperationalStateDelegate()->HandleStopStateCallback(err);
}

CHIP_ERROR ChefMicrowaveOvenDevice::Init()
{
    return MicrowaveOvenMode::GetDelegate()->Init();
}

void ChefMicrowaveOvenDevice::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    MicrowaveOvenMode::GetDelegate()->HandleChangeToMode(NewMode, response);
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(MicrowaveOvenMode::GetDelegate()->kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].label, label);
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(MicrowaveOvenMode::GetDelegate()->kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(MicrowaveOvenMode::GetDelegate()->kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].modeTags.begin(),
              MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(MicrowaveOvenMode::GetDelegate()->kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

static constexpr EndpointId kDemoEndpointId = 1;

Platform::UniquePtr<ChefMicrowaveOvenDevice> gMicrowaveOvenDevice;

void MatterMicrowaveOvenServerInit()
{
    gMicrowaveOvenDevice = Platform::MakeUnique<ChefMicrowaveOvenDevice>(kDemoEndpointId);
    gMicrowaveOvenDevice.get()->MicrowaveOvenInit();
}

void MatterMicrowaveOvenServerShutdown()
{
    gMicrowaveOvenDevice = nullptr;
}

#endif // MATTER_DM_PLUGIN_MICROWAVE_OVEN_CONTROL_SERVER
