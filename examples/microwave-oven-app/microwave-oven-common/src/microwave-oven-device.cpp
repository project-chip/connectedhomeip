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
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::MicrowaveOvenControl;
template <typename T>
using List              = chip::app::DataModel::List<T>;
using ModeTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;
using Status            = Protocols::InteractionModel::Status;

void ExampleMicrowaveOvenDevice::MicrowaveOvenInit()
{
    mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kStopped));
    mOperationalStateInstance.Init();
    mMicrowaveOvenModeInstance.Init();
    mMicrowaveOvenControlInstance.Init();

    // set default value for attribute SelectedWattIndex and WattRating
    if (mMicrowaveOvenControlInstance.HasFeature(MicrowaveOvenControl::Feature::kPowerInWatts))
    {
        static_assert(MATTER_ARRAY_SIZE(mWattSettingList) > 0, "Watt setting list is empty!");
        mSelectedWattIndex = MATTER_ARRAY_SIZE(mWattSettingList) - 1;
        mWattRating        = mWattSettingList[mSelectedWattIndex];
    }
    else
    {
        mWattRating = kExampleWatt5;
    }
}

/**
 * MicrowaveOvenControl cluster
 */
Protocols::InteractionModel::Status
ExampleMicrowaveOvenDevice::HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec, bool startAfterSetting,
                                                               Optional<uint8_t> powerSettingNum,
                                                               Optional<uint8_t> wattSettingIndex)
{
    // placeholder implementation
    Status status;
    // Update cook mode.
    if ((status = mMicrowaveOvenModeInstance.UpdateCurrentMode(cookMode)) != Status::Success)
    {
        return status;
    }

    mMicrowaveOvenControlInstance.SetCookTimeSec(cookTimeSec);

    // If using power as number, check if powerSettingNum has value before setting the power number.
    // If powerSetting field is missing in the command, the powerSettingNum passed here is handled to the max value
    // and user can use this value directly.
    if (powerSettingNum.HasValue())
    {
        mPowerSettingNum = powerSettingNum.Value();
    }

    // If using power in watt, check if wattSettingIndex has value before setting the watt rating and watt list index.
    // If wattSettinIndex field is missing in the command, the wattSettingIndex passed here is handled to the max value
    // and user can use this value directly.
    if (wattSettingIndex.HasValue())
    {
        mSelectedWattIndex = wattSettingIndex.Value();
        mWattRating        = mWattSettingList[mSelectedWattIndex];
    }

    if (startAfterSetting)
    {
        mOperationalStateInstance.SetOperationalState(to_underlying(OperationalStateEnum::kRunning));
    }
    return Status::Success;
}

Protocols::InteractionModel::Status ExampleMicrowaveOvenDevice::HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec)
{
    // placeholder implementation
    mMicrowaveOvenControlInstance.SetCookTimeSec(finalCookTimeSec);
    return Status::Success;
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mWattSettingList), CHIP_ERROR_NOT_FOUND);

    wattSetting = mWattSettingList[index];
    return CHIP_NO_ERROR;
}

/**
 * OperationalState cluster
 */
app::DataModel::Nullable<uint32_t> ExampleMicrowaveOvenDevice::GetCountdownTime()
{
    return DataModel::MakeNullable(mMicrowaveOvenControlInstance.GetCookTimeSec());
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetOperationalStateAtIndex(size_t index,
                                                                  OperationalState::GenericOperationalState & operationalState)
{
    if (index > mOperationalStateList.size() - 1)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    operationalState = mOperationalStateList[index];
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase)
{
    if (index >= mOperationalPhaseList.size())
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
}

void ExampleMicrowaveOvenDevice::HandlePauseStateCallback(OperationalState::GenericOperationalError & err)
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

void ExampleMicrowaveOvenDevice::HandleResumeStateCallback(OperationalState::GenericOperationalError & err)
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

void ExampleMicrowaveOvenDevice::HandleStartStateCallback(OperationalState::GenericOperationalError & err)
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

void ExampleMicrowaveOvenDevice::HandleStopStateCallback(OperationalState::GenericOperationalError & err)
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

/**
 * MicrowaveOvenMode Cluster
 */
CHIP_ERROR ExampleMicrowaveOvenDevice::Init()
{
    return CHIP_NO_ERROR;
}

// todo refactor code by making a parent class for all ModeInstance classes to reduce flash usage.
void ExampleMicrowaveOvenDevice::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    response.status = to_underlying(ModeBase::StatusCode::kSuccess);
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    value = kModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExampleMicrowaveOvenDevice::GetModeTagsByIndex(uint8_t modeIndex, List<ModeTagStructType> & tags)
{
    if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    if (tags.size() < kModeOptions[modeIndex].modeTags.size())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
    tags.reduce_size(kModeOptions[modeIndex].modeTags.size());

    return CHIP_NO_ERROR;
}

/*
 * An example to present device
 */
static constexpr EndpointId kDemoEndpointId = 1;

Platform::UniquePtr<ExampleMicrowaveOvenDevice> gMicrowaveOvenDevice;

void MatterMicrowaveOvenServerInit()
{
    gMicrowaveOvenDevice = Platform::MakeUnique<ExampleMicrowaveOvenDevice>(kDemoEndpointId);
    gMicrowaveOvenDevice.get()->MicrowaveOvenInit();
}

void MatterMicrowaveOvenServerShutdown()
{
    gMicrowaveOvenDevice = nullptr;
}
