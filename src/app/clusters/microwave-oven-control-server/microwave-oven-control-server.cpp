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
using namespace chip::app::Clusters::MicrowaveOvenMode;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using Status = Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId,
                   Clusters::OperationalState::Instance & aOpStateInstance,
                   Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance) :
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
    uint8_t modeValue;
    uint8_t reqCookMode;
    uint32_t reqCookTime;
    uint8_t reqPowerSetting;
    auto & cookMode     = req.cookMode;
    auto & cookTime     = req.cookTime;
    auto & powerSetting = req.powerSetting;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState == to_underlying(OperationalStateEnum::kStopped), status = Status::InvalidInState);

    VerifyOrExit(cookMode.HasValue() || cookTime.HasValue() || powerSetting.HasValue(), status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, all command fields are missing "));

    modeValue = 0;
    VerifyOrExit(mMicrowaveOvenModeInstance.GetModeValueByModeTag(to_underlying(MicrowaveOvenMode::ModeTag::kNormal), modeValue) ==
                     CHIP_NO_ERROR,
                 status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, Normal mode is not found"));

    reqCookMode = cookMode.ValueOr(modeValue);
    VerifyOrExit(mMicrowaveOvenModeInstance.IsSupportedMode(reqCookMode), status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, cookMode is not supported"));

    reqCookTime = cookTime.ValueOr(MicrowaveOvenControl::kDefaultCookTime);
    VerifyOrExit(IsCookTimeInRange(reqCookTime), status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    reqPowerSetting = powerSetting.ValueOr(mDelegate->GetMaxPower());
    VerifyOrExit(IsPowerSettingInRange(reqPowerSetting, mDelegate->GetMinPower(), mDelegate->GetMaxPower()),
                 status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookPower, cookPower value is out of range"));

    status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTime, reqPowerSetting);

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req)
{
    ChipLogDetail(Zcl, "Microwave Oven Control: HandleAddMoreTime");
    Status status;
    uint8_t opState;
    uint32_t finalCookTime;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState != to_underlying(OperationalStateEnum::kError), status = Status::InvalidInState);

    // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
    VerifyOrExit(req.timeToAdd <= kMaxCookTime - GetCookTime(), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    finalCookTime = GetCookTime() + req.timeToAdd;
    status        = mDelegate->HandleModifyCookTimeCallback(finalCookTime);

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool IsCookTimeInRange(uint32_t cookTime)
{
    return MicrowaveOvenControl::kMinCookTime <= cookTime && cookTime <= MicrowaveOvenControl::kMaxCookTime;
}

bool IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower)
{
    return minCookPower <= powerSetting && powerSetting <= maxCookPower;
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
