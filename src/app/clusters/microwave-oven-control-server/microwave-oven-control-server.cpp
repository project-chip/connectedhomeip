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

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                   Clusters::OperationalState::Instance * aOpStateInstance,
                   Clusters::ModeBase::Instance * aMicrowaveOvenModeInstance) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId),
    AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mClusterId(aClusterId), mOpStateInstance(aOpStateInstance), mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
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
    if (!mOpStateInstance)
    {
        ChipLogError(Zcl, "Microwave Oven Control: Operational State instance is NULL");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!mMicrowaveOvenModeInstance)
    {
        ChipLogError(Zcl, "Microwave Oven Control: Microwave Oven Mode instance is NULL");
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
    return mPowerSetting;
}

void Instance::SetCookTime(uint32_t cookTime)
{
    uint32_t oldCookTime = mCookTime;
    mCookTime            = cookTime;
    if (mCookTime != oldCookTime)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CookTime::Id);
    }
}

void Instance::SetPowerSetting(uint8_t powerSetting)
{
    uint8_t oldPowerSetting = mPowerSetting;
    mPowerSetting           = powerSetting;
    if (mPowerSetting != oldPowerSetting)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::PowerSetting::Id);
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
    ChipLogDetail(Zcl, "Microwave Oven Control: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SetCookingParameters::Id:
        ChipLogDetail(Zcl, "Microwave Oven Control: Entering SetCookingParameters");

        CommandHandlerInterface::HandleCommand<Commands::SetCookingParameters::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSetCookingParameters(ctx, req); });
        break;

    case Commands::AddMoreTime::Id:
        ChipLogDetail(Zcl, "Microwave Oven Control: Entering AddMoreTime");

        CommandHandlerInterface::HandleCommand<Commands::AddMoreTime::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleAddMoreTime(ctx, req); });
        break;
    }
}

void Instance::HandleSetCookingParameters(HandlerContext & ctx, const Commands::SetCookingParameters::DecodableType & req)
{
    ChipLogDetail(Zcl, "Microwave Oven Control: HandleSetCookingParameters");
    Status status;
    uint8_t opState;
    uint32_t reqCookTime;
    uint8_t reqPowerSetting;
    auto & cookMode     = req.cookMode;
    auto & cookTime     = req.cookTime;
    auto & powerSetting = req.powerSetting;

    VerifyOrExit((cookMode.HasValue() || cookTime.HasValue() || powerSetting.HasValue()), status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, command fields are null "));

    VerifyOrExit(!(cookMode.HasValue() && (!mMicrowaveOvenModeInstance->IsSupportedMode(cookMode.Value()))),
                 status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, cookMode is not supported"));

    VerifyOrExit(!(cookTime.HasValue() && (!IsCookTimeInRange(cookTime.Value()))), status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    VerifyOrExit(!(powerSetting.HasValue() &&
                   (!IsPowerSettingInRange(powerSetting.Value(), mDelegate->GetMinPower(), mDelegate->GetMaxPower()))),
                 status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookPower, cookPower value is out of range"));

    opState = mOpStateInstance->GetCurrentOperationalState();
    VerifyOrExit(opState == to_underlying(OperationalStateEnum::kStopped), status = Status::InvalidInState);

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
    status = mDelegate->HandleSetCookingParametersCallback(cookMode, reqCookTime, reqPowerSetting);

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req)
{
    ChipLogDetail(Zcl, "Microwave Oven Control: HandleAddMoreTime");
    Status status;
    uint8_t opState;
    uint32_t finalCookTime;

    opState = mOpStateInstance->GetCurrentOperationalState();
    VerifyOrExit(opState != to_underlying(OperationalStateEnum::kError), status = Status::InvalidInState);

    finalCookTime = GetCookTime() + req.timeToAdd;
    // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
    VerifyOrExit(finalCookTime < kMaxCookTime, status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    status = mDelegate->HandleModifyCookTimeCallback(finalCookTime);
    
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
