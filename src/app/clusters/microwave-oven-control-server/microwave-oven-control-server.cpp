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
#include <app/InteractionModelEngine.h>
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using Status = Protocols::InteractionModel::Status;


namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, Clusters::OperationalState::Instance * aOpStateInstance) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId), AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId),
    mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(aClusterId), 
    mOpStateInstance(aOpStateInstance), mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
{
    mDelegate->SetInstance(this);
}

Instance::~Instance()
{
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, mClusterId))
    {
        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

uint32_t Instance::GetCookTime() const
{
    return mCookTime;
}

uint8_t Instance::GetPowerSetting() const
{
    return mPowerSettng;
}

void Instance::SetCookTime(uint32_t cookTime)
{
    uint32_t oldCookTime = mCookTime;
    mCookTime = cookTime;
    if (mCookTime != oldCookTime)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CookTime::Id));
    }
}

void Instance::SetPowerSetting(uint8_t powerSetting)
{
    uint8_t oldPowerSetting = mPowerSetting;
    mPowerSetting = powerSetting;
    if (mPowerSettng != oldPowerSetting)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::PowerSetting::Id));
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "OperationalState: Reading");
    switch (aPath.mAttributeId)
    {
    case MicrowaveOvenControl::Attributes::CookTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetCookTime()));
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerSetting::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetPowerSetting()));
    }
    break;

    case MicrowaveOvenControl::Attributes::MinPower::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMinPower()));
    }
    break;

    case MicrowaveOvenControl::Attributes::MaxPower::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMaxPower()));
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerStep::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetPowerStep()));
    }
    break;
    }
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SetCookingParameters::Id:
        ChipLogDetail(Zcl, "MicrowaveOvenControl:  Entering SetCookingParameters");

        CommandHandlerInterface::HandleCommand<Commands::SetCookingParameters::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSetCookingParameters(ctx, req); });
        break;

    case Commands::AddMoreTime::Id:
        ChipLogDetail(Zcl, "MicrowaveOvenControl:  Entering AddMoreTime");

        CommandHandlerInterface::HandleCommand<Commands::AddMoreTime::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleAddMoreTime(ctx, req); });
        break;
    }
}

void Instance::HandleSetCookingParameters(HandlerContext & ctx, const Commands::SetCookingParameters::DecodableType & req)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl:  HandleSetCookingParameters");
    Status status;
    uint8_t opState;
    auto & cookMode     = req.cookMode;
    auto & cookTime     = req.cookTime;
    auto & powerSetting = req.powerSetting;

    // check if the input cooking value is invalid
    if(cookMode.HasValue() && (!mMicrowaveOvenModeInstance->IsSupportedMode(cookMode.Value())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookMode, cookMode is not support");
        goto exit;
    }

    if (cookTime.HasValue() && (!IsCookTimeInRange(cookTime.Value())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
        goto exit;
    }

    if (powerSetting.HasValue() &&
        (!IsPowerSettingInRange(powerSetting.Value(), mDelegate->GetMinPower(), mDelegate->GetMaxPower())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookPower, cookPower value is out of range");
        goto exit;
    }

    //get current operational state 
    opState = mOpStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped))
    {
        uint8_t reqCookMode     = 0;
        uint32_t reqCookTime    = 0;
        uint8_t reqPowerSetting = 0;
        if (cookMode.HasValue())
        {
            reqCookMode = cookMode.Value();
        }
        else
        {
            // TODO: set Microwave Oven cooking mode to normal mode(default).
            reqCookMode = Clusters::ModeNormal;
        }

        if (cookTime.HasValue())
        {
            reqCookTime = cookTime.Value();
        }
        else
        {
            // set Microwave Oven cooking time to 30 seconds(default).
            reqCookTime = MicrowaveOvenControl::kDefaultCookTime;
        }

        if (powerSetting.HasValue())
        {
            reqPowerSetting = powerSetting.Value();
        }
        else
        {
            // set Microwave Oven cooking power to max power(default).
            reqPowerSetting = mDelegate->GetMaxPower();
        }
        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTime, reqPowerSetting);
        goto exit;
    }
    else
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl:  HandleAddMoreTime");
    Status status;
    uint8_t opState;

    //get current operational state 
    opState = mOpStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kRunning) ||
        opState == to_underlying(OperationalStateEnum::kPaused))
    {
        uint32_t finalCookTime;

        finalCookTime = GetCookTime() + req.timeToAdd;
        // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
        if (finalCookTime < kMaxCookTime)
        {
            status = mDelegate->HandleSetCookTimeCallback(finalCookTime);
            goto exit;
        }
        else
        {
            ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
            status = Status::ConstraintError;
            goto exit;
        }
    }
    else // operational state is in error
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool IsCookTimeInRange(uint32_t cookTime)
{
    return (cookTime < MicrowaveOvenControl::kMinCookTime || cookTime > MicrowaveOvenControl::kMaxCookTime) ? false : true;
}

bool IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower)
{
    return (powerSetting < minCookPower || powerSetting > maxCookPower) ? false : true;
}


} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Microwave Oven Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void MatterMicrowaveOvenControlPluginServerInitCallback() {}
