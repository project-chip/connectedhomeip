/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using OperationalStateEnum = OperationalState::OperationalStateEnum;
using Status               = Protocols::InteractionModel::Status;

namespace {

constexpr uint32_t kMaxCookTime     = 65535;
constexpr uint32_t kMinCookTime     = 1;
constexpr uint8_t kDefaultCookTime  = 30;
constexpr uint8_t kDefaultCookPower = 100;

constexpr size_t kMicrowaveOvenControlInstanceTableSize =
    EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kMicrowaveOvenControlInstanceTableSize <= kEmberInvalidEndpointIndex,
              "Microwave Oven Control Instance table size error");

OperationalState::Instance * gInstanceTable[kMicrowaveOvenControlInstanceTableSize] = { nullptr };

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

/**
 * Get operational state instance
 */
OperationalState::Instance * GetOPInstance(EndpointId aEndpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id,
                                                       EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMicrowaveOvenControlInstanceTableSize ? nullptr : gInstanceTable[ep]);
}

/**
 * Set operational state instance
 */
void SetOPInstance(EndpointId aEndpoint, OperationalState::Instance * aInstance)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id,
                                                       EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kMicrowaveOvenControlInstanceTableSize)
    {
        gInstanceTable[ep] = aInstance;
    }
}

bool CheckCookTimeIsInRange(uint32_t cookTime)
{
    return (cookTime < kMinCookTime || cookTime > kMaxCookTime) ? false : true;
}

bool CheckPowerSettingIsInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower)
{
    return (powerSetting < minCookPower || powerSetting > maxCookPower) ? false : true;
}

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip

/**********************************************************
 * Callbacks Implementation
 *********************************************************/
/**
 * @brief Microwave Oven Control Cluster SetCookingParameters Command callback (from client)
 */
bool emberAfMicrowaveOvenControlClusterSetCookingParametersCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MicrowaveOvenControl::Commands::SetCookingParameters::DecodableType & commandData)
{
    uint8_t opState;
    EndpointId endpointId                                  = commandPath.mEndpointId;
    EmberAfStatus emberAfStatus                            = EMBER_ZCL_STATUS_SUCCESS;
    Status status                                          = Status::Success;
    OperationalState::Instance * gOperationalStateInstance = nullptr;
    auto & CookMode                                        = commandData.cookMode;
    auto & CookTime                                        = commandData.cookTime;
    auto & PowerSetting                                    = commandData.powerSetting;
    uint8_t kMinCookPower;
    uint8_t kMaxCookPower;

    // get Operational State instance and current state
    gOperationalStateInstance = GetOPInstance(endpointId);
    if (gOperationalStateInstance == nullptr)
    {
        status = Status::InvalidInState;
        ChipLogProgress(Zcl, "Server didn't create instance of Operational State");
        goto exit;
    }

    if (CookTime.HasValue() && (!CheckCookTimeIsInRange(CookTime.Value())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
        goto exit;
    }

    emberAfStatus = MinPower::Get(endpointId, &kMinCookPower);
    if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        status = app::ToInteractionModelStatus(emberAfStatus);
        ChipLogError(Zcl, "Failed to get MinPower");
        goto exit;
    }

    emberAfStatus = MaxPower::Get(endpointId, &kMaxCookPower);
    if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        status = app::ToInteractionModelStatus(emberAfStatus);
        ChipLogError(Zcl, "Failed to get MaxPower");
        goto exit;
    }

    if (PowerSetting.HasValue() && (!CheckPowerSettingIsInRange(PowerSetting.Value(), kMinCookPower, kMaxCookPower)))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookPower, cookPower value is out of range");
        goto exit;
    }

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped))
    {

        if (CookMode.HasValue())
        {
            // TODO: set Microwave Oven cooking mode by CookMode.Value().
        }
        else
        {
            // TODO: set Microwave Oven cooking mode to normal mode.
        }

        if (CookTime.HasValue())
        {
            // set Microwave Oven cooking time by getting input value.
            emberAfStatus = CookTime::Set(endpointId, CookTime.Value());
            ChipLogError(Zcl, "emberAfStatus value = %d", emberAfStatus);
            if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::InvalidCommand;
                ChipLogError(Zcl, "Failed to set cookTime with value = %ld", CookTime.Value());
                goto exit;
            }
        }
        else
        {
            // set Microwave Oven cooking time to 30 seconds(default).
            emberAfStatus = CookTime::Set(endpointId, kDefaultCookTime);
            if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::InvalidCommand;
                ChipLogError(Zcl, "Failed to set cookTime with value = %d", kDefaultCookTime);
                goto exit;
            }
        }

        if (PowerSetting.HasValue())
        {
            // set Microwave Oven cooking power by getting input value.
            emberAfStatus = PowerSetting::Set(endpointId, PowerSetting.Value());
            ChipLogError(Zcl, "emberAfStatus value = %d", emberAfStatus);
            if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::InvalidCommand;
                ChipLogError(Zcl, "Failed to set cooking power with value = %d", PowerSetting.Value());
                goto exit;
            }
        }
        else
        {
            // set Microwave Oven cooking power to max power(default).
            emberAfStatus = PowerSetting::Set(endpointId, kDefaultCookPower);
            if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::InvalidCommand;
                ChipLogError(Zcl, "Failed to set cooking power with value = %d", kDefaultCookPower);
                goto exit;
            }
        }
    }
    else
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return true;
}
/**
 * @brief Microwave Oven Control Cluster AddMoreTime Command callback (from client)
 */
bool emberAfMicrowaveOvenControlClusterAddMoreTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MicrowaveOvenControl::Commands::AddMoreTime::DecodableType & commandData)
{
    uint8_t opState;
    EndpointId endpointId                                  = commandPath.mEndpointId;
    EmberAfStatus emberAfStatus                            = EMBER_ZCL_STATUS_SUCCESS;
    Status status                                          = Status::Success;
    OperationalState::Instance * gOperationalStateInstance = nullptr;

    // get Operational State instance and current state
    gOperationalStateInstance = GetOPInstance(endpointId);
    if (gOperationalStateInstance == nullptr)
    {
        ChipLogProgress(Zcl, "Server didn't create instance of Operational State");
        status = Status::InvalidInState;
        goto exit;
    }

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kRunning) ||
        opState == to_underlying(OperationalStateEnum::kPaused))
    {
        // add cooking time by TimeToAdd command
        auto & TimeToAdd = commandData.timeToAdd;
        uint32_t currentCookTime;
        uint32_t addedCookTime;
        emberAfStatus = CookTime::Get(endpointId, &currentCookTime);
        if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
        {
            status = app::ToInteractionModelStatus(emberAfStatus);
            ChipLogError(Zcl, "Failed to get cuurent cooking time");
            goto exit;
        }

        addedCookTime = currentCookTime + TimeToAdd;
        // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
        if (addedCookTime < kMaxCookTime)
        {
            emberAfStatus = CookTime::Set(endpointId, addedCookTime);
            if (emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::InvalidInState;
                ChipLogError(Zcl, "Failed to set cookTime with value = %ld", addedCookTime);
                goto exit;
            }
        }
        else
        {
            status = Status::ConstraintError;
            ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
            goto exit;
        }
    }
    else // operational state is in error
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:

    commandObj->AddStatus(commandPath, status);
    return true;
}

/** @brief Microwave Oven Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void MatterMicrowaveOvenControlPluginServerInitCallback() {}
