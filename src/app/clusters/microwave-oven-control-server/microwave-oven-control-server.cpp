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
using Status = Protocols::InteractionModel::Status;

#define EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT (1)

namespace{

constexpr size_t kMicrowaveOvenControlInstanceTableSize =
    EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kMicrowaveOvenControlInstanceTableSize <= kEmberInvalidEndpointIndex, "Microwave Oven Control Instance table size error");

OperationalState::Instance * gInstanceTable[kMicrowaveOvenControlInstanceTableSize] = { nullptr };

OperationalState::Instance * gOperationalStateInstance = nullptr;



} // anonymous namespace


namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {


/**
 * Get Operational State instance for Microwave Oven Control Cluster to know what the current state is.
 */
OperationalState::Instance * GetInstance(EndpointId aEndpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id, EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMicrowaveOvenControlInstanceTableSize ? nullptr : gInstanceTable[ep]);
}

/*
* Set Operational State instance from the application layer.
*/
void SetInstance(EndpointId aEndpoint, OperationalState::Instance * aInstance)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id, EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kMicrowaveOvenControlInstanceTableSize)
    {
        gInstanceTable[ep] = aInstance;
    }
}



}
}
}
}

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
    EndpointId endpointId = commandPath.mEndpointId;
    Status status = Status::Success;
    EmberAfStatus emberAfStatus = EMBER_ZCL_STATUS_SUCCESS;

    ChipLogProgress(Zcl,"lpy microwave debug 11111111111111111111111111111111111111111");
    ChipLogProgress(Zcl,"lpy microwave debug endpointId = %d", endpointId);
    
    if(gOperationalStateInstance == nullptr){
        gOperationalStateInstance = GetInstance(endpointId);
    }
    opState = gOperationalStateInstance->GetCurrentOperationalState();
    /*
    if(opState == to_underlying(OperationalStateEnum::kStopped))
    {
        auto & CookMode = commandData.CookMode;
        auto & CookTime = commandData.CooTime;
        auto & PowerSetting = commandData.PowerSetting;  

        if(CookMode.HasValue())
        {
            //TODO: set Microwave Oven Cooking mode by CookMode.Value().
        }
        else
        {
            //TODO: set Microwave Oven Cooking mode to normal mode.
        }

        if(CookTime.HasValue())
        {
            //set Microwave Oven cooking time by getting input value.
            if(CookTime.Vlaue() < 1 && CookTime.Vlaue() > 65535)
            {
                status = Status::InvalidCommand;
            }
            else
            {
                emberAfStatus = CookTime::Set(endpointId, CookTime.Vlaue());
                if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
                {
                    
                }
            }
        }
        else
        {
            //set Microwave Oven Cooking time to 30(default).
            emberAfStatus = CookTime::Set(endpointId, 30);
        }

        if(CookMode.HasValue())
        {
            //TODO: set Microwave Oven Cooking mode by CookMode.Value()
        }
        else
        {
            //TODO: set Microwave Oven Cooking mode to normal mode
        }

    }
    else
    {
        status = Status::InvalidInState;
        goto exit;
    }
    */
    ChipLogProgress(Zcl,"lpy microwave debug state = %d", opState);
    uint32_t cookTime;
    emberAfStatus = CookTime::Get(endpointId, &cookTime);
    if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl,"lpy microwave debug error ");   
    }
    ChipLogProgress(Zcl,"lpy microwave debug cookTime = %d", cookTime);
    uint8_t powerSetting;
    emberAfStatus = PowerSetting::Get(endpointId, &powerSetting);
    if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl,"lpy microwave debug error ");   
    }
    ChipLogProgress(Zcl,"lpy microwave debug powerSetting = %d", powerSetting);
    uint8_t minPower;
    emberAfStatus = MinPower::Get(endpointId, &minPower);
    if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl,"lpy microwave debug error ");   
    }
    ChipLogProgress(Zcl,"lpy microwave debug minPower = %d", minPower);  
    uint8_t maxPower;
    emberAfStatus = MaxPower::Get(endpointId, &maxPower);
    if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl,"lpy microwave debug error ");   
    }
    ChipLogProgress(Zcl,"lpy microwave debug maxPower = %d", maxPower);  
    uint8_t powerStep;
    emberAfStatus = PowerStep::Get(endpointId, &powerStep);
    if(emberAfStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogProgress(Zcl,"lpy microwave debug error ");   
    }
    ChipLogProgress(Zcl,"lpy microwave debug powerStep = %d", powerStep);    
    ChipLogProgress(Zcl,"lpy microwave debug 2222222222222222222222222222222222222222");
    return true;
}
/**
 * @brief Microwave Oven Control Cluster AddMoreTime Command callback (from client)
 */
bool emberAfMicrowaveOvenControlClusterAddMoreTimeCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::MicrowaveOvenControl::Commands::AddMoreTime::DecodableType & commandData)
{
    uint8_t opState;  //
    ChipLogProgress(Zcl,"lpy microwave debug 33333333333333333333333333333333333333333");
    if(gOperationalStateInstance == nullptr){
        gOperationalStateInstance = GetInstance(commandPath.mEndpointId);
    }
    opState = gOperationalStateInstance->GetCurrentOperationalState();
    ChipLogProgress(Zcl,"lpy microwave debug opState = %d", opState);
    ChipLogProgress(Zcl,"lpy microwave debug 44444444444444444444444444444444444444444");
    return true;
}

/** @brief Microwave Oven Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void MatterMicrowaveOvenControlPluginServerInitCallback()
{
    
}
