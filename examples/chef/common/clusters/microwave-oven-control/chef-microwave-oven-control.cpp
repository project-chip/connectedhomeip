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
#include "../../CodeUtils.h"
#include "../../DeviceTypes.h"
#include <app/util/attribute-storage.h>
#include <devices/Types.h>

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

#if MATTER_DM_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0

ChefMicrowaveOvenDevice::ChefMicrowaveOvenDevice(EndpointId aClustersEndpoint,
                                                 OperationalState::Instance * operationalStateInstancePtr,
                                                 OperationalStateDelegate * operationalStateDelegatePtr,
                                                 ModeBase::Instance * microwaveOvenModeInstancePtr) :
    mOperationalStateDelegatePtr(operationalStateDelegatePtr),
    mOperationalStateInstancePtr(operationalStateInstancePtr), mMicrowaveOvenModeInstancePtr(microwaveOvenModeInstancePtr)
{
    if (mOperationalStateInstancePtr && mMicrowaveOvenModeInstancePtr && mMicrowaveOvenModeInstancePtr)
    {
        mMicrowaveOvenControlInstance = std::make_unique<MicrowaveOvenControl::Instance>(
            this, aClustersEndpoint, MicrowaveOvenControl::Id,
            BitMask<MicrowaveOvenControl::Feature>(MicrowaveOvenControl::Feature::kPowerAsNumber,
                                                   MicrowaveOvenControl::Feature::kPowerNumberLimits),
            *mOperationalStateInstancePtr, *mMicrowaveOvenModeInstancePtr);
        VerifyOrLogChipError(mMicrowaveOvenControlInstance->Init());
    }
}

/**
 * MicrowaveOvenControl cluster
 */
Protocols::InteractionModel::Status
ChefMicrowaveOvenDevice::HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec, bool startAfterSetting,
                                                            Optional<uint8_t> powerSettingNum, Optional<uint8_t> wattSettingIndex)
{
    Status status;
    // Update cook mode.
    if (mMicrowaveOvenModeInstancePtr)
    {
        if ((status = mMicrowaveOvenModeInstancePtr->UpdateCurrentMode(cookMode)) != Status::Success)
        {
            return status;
        }
    }

    HandleModifyCookTimeSecondsCallback(cookTimeSec);

    // If using power as number, check if powerSettingNum has value before setting the power number.
    // If powerSetting field is missing in the command, the powerSettingNum passed here is handled to the max value
    // and user can use this value directly.
    if (powerSettingNum.HasValue())
    {
        mPowerSettingNum = powerSettingNum.Value();
    }

    if (startAfterSetting && mOperationalStateDelegatePtr)
    {
        GenericOperationalError noError(static_cast<uint8_t>(ErrorStateEnum::kNoError));
        mOperationalStateDelegatePtr->HandleStartStateCallback(noError);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ChefMicrowaveOvenDevice::HandleModifyCookTimeSecondsCallback(uint32_t finalCookTimeSec)
{
    if (mOperationalStateInstancePtr && mOperationalStateDelegatePtr)
    {
        // Can not shorten existing cycle.
        if (mOperationalStateInstancePtr->GetCurrentOperationalState() != to_underlying(OperationalStateEnum::kStopped) &&
            finalCookTimeSec < mOperationalStateDelegatePtr->GetCountdownTime().ValueOr(0))
        {
            return Status::InvalidInState;
        }
    }

    if (mMicrowaveOvenControlInstance)
    {
        mMicrowaveOvenControlInstance->SetCookTimeSec(finalCookTimeSec);
    }

    if (mOperationalStateDelegatePtr)
    {
        mOperationalStateDelegatePtr->mCountDownTime.SetNonNull(finalCookTimeSec);
    }

    if (mOperationalStateInstancePtr)
    {
        mOperationalStateInstancePtr->UpdateCountdownTimeFromDelegate();
    }

    return Status::Success;
}

CHIP_ERROR ChefMicrowaveOvenDevice::GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting)
{
    VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mWattSettingList), CHIP_ERROR_NOT_FOUND);

    wattSetting = mWattSettingList[index];
    return CHIP_NO_ERROR;
}

namespace {
constexpr size_t kMicrowaveOvenDeviceSize = MATTER_DM_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;
static_assert(kMicrowaveOvenDeviceSize <= kEmberInvalidEndpointIndex, "MicrowaveOvenDevice table size error");

std::unique_ptr<ChefMicrowaveOvenDevice> gMicrowaveOvenDevice[kMicrowaveOvenDeviceSize];

/**
 * Check if endpoint is a valid MicrowaveOven device.
 * @param endpoint Endpoint
 * @return True if this endpoint has required bits to be a microwave device. False otherwise.
 */
bool checkEndpointIsValidMicrowave(EndpointId endpoint)
{
    if (!emberAfContainsServer(endpoint, MicrowaveOvenControl::Id))
    {
        return false;
    }

    if (!emberAfContainsServer(endpoint, MicrowaveOvenMode::Id))
    {
        return false;
    }
    if (!emberAfContainsServer(endpoint, OperationalState::Id))
    {
        return false;
    }
    if (!chef::DeviceTypes::EndpointHasDeviceType(endpoint, Device::kMicrowaveOvenDeviceTypeId))
    {
        return false;
    }
    return true;
}
} // namespace

void InitChefMicrowaveOvenControlCluster()
{
    ChipLogProgress(NotSpecified, "Deferred Initializing MicrowaveOvenControl cluster for all relevant endpoints.");
    for (uint16_t i = 0; i < emberAfEndpointCount(); ++i)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(i);
        if (checkEndpointIsValidMicrowave(endpoint))
        {
            uint16_t epIndex = emberAfGetClusterServerEndpointIndex(endpoint, MicrowaveOvenControl::Id, kMicrowaveOvenDeviceSize);
            if (epIndex < kMicrowaveOvenDeviceSize)
            {
                // Check if an instance already exists for this endpoint.
                if (gMicrowaveOvenDevice[epIndex])
                {
                    continue;
                }
                gMicrowaveOvenDevice[epIndex] = std::make_unique<ChefMicrowaveOvenDevice>(
                    endpoint, GetOperationalStateInstance(endpoint), GetOperationalStateDelegate(endpoint),
                    ChefMicrowaveOvenMode::GetInstance(endpoint));
            }
        }
    }
}

#endif // MATTER_DM_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
