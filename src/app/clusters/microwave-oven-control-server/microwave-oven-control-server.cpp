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

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature,
                   Clusters::OperationalState::Instance & aOpStateInstance,
                   Clusters::ModeBase::Instance & aMicrowaveOvenModeInstance) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId),
    AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mClusterId(aClusterId), mFeature(aFeature), mOpStateInstance(aOpStateInstance),
    mMicrowaveOvenModeInstance(aMicrowaveOvenModeInstance)
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
    // Check if the feature bit is mutually exclusive
    if (mFeature != to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber) &&
        mFeature != to_underlying(MicrowaveOvenControl::Feature::kPowerInWatts))
    {
        ChipLogError(Zcl, "Microwave Oven Control: feature bits error, one and only one of the feature bits must be true");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerInWatts)))
    {
        mSupportedPowerLevel = GetSupportedPowerLevel();
    }
    return CHIP_NO_ERROR;
}

bool Instance::HasFeature(uint32_t feature) const
{
    return (mFeature & feature) != 0;
}

uint8_t Instance::GetSupportedPowerLevel() const
{
    uint8_t wattIndex = 0;
    uint16_t watt     = 0;
    while (mDelegate->GetWattSettingByIndex(wattIndex, watt) != CHIP_ERROR_NOT_FOUND)
    {
        wattIndex++;
    }
    return wattIndex;
}

uint32_t Instance::GetCookTime() const
{
    return mCookTime;
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

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "Microwave Oven Control: Reading");
    switch (aPath.mAttributeId)
    {
    case MicrowaveOvenControl::Attributes::CookTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetCookTime()));
    }
    break;

    case MicrowaveOvenControl::Attributes::MaxCookTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMaxCookTime()));
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerSetting::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber)))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetPowerSetting()));
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::MinPower::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber)))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMinPower()));
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::MaxPower::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber)))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMaxPower()));
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerStep::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber)))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetPowerStep()));
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::SupportedWatts::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerInWatts)))
        {
            aEncoder.EncodeList([delegate = mDelegate](const auto & encoder) -> CHIP_ERROR {
                uint16_t wattRatting;
                uint8_t index  = 0;
                CHIP_ERROR err = CHIP_NO_ERROR;
                while ((err = delegate->GetWattSettingByIndex(index, wattRatting)) == CHIP_NO_ERROR)
                {
                    ReturnErrorOnFailure(encoder.Encode(wattRatting));
                    index++;
                }
                if (err == CHIP_ERROR_NOT_FOUND)
                {
                    return CHIP_NO_ERROR;
                }
                return err;
            });
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::SelectedWattIndex::Id: {
        if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerInWatts)))
        {
            ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetCurrentWattIndex()));
        }
        else
        {
            ChipLogError(Zcl, "Microwave Oven Control: feature is not supported");
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
    }
    break;

    case MicrowaveOvenControl::Attributes::WattRating::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetCurrentWattRating()));
    }
    break;

    case MicrowaveOvenControl::Attributes::FeatureMap::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
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
    uint8_t reqWattSettingIndex;
    bool reqStartAfterSetting;
    auto & cookMode          = req.cookMode;
    auto & cookTime          = req.cookTime;
    auto & powerSetting      = req.powerSetting;
    auto & wattSettingIndex  = req.wattSettingIndex;
    auto & startAfterSetting = req.startAfterSetting;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState == to_underlying(OperationalStateEnum::kStopped), status = Status::InvalidInState);

    if (startAfterSetting.HasValue())
    {
        VerifyOrExit(
            ServerClusterCommandExists(
                ConcreteCommandPath(mEndpointId, OperationalState::Id, OperationalState::Commands::Start::Id)) == Status::Success,
            status = Status::InvalidCommand;
            ChipLogError(
                Zcl,
                "Microwave Oven Control: Failed to set cooking parameters, Start command of operational state is not supported"));
    }
    reqStartAfterSetting = startAfterSetting.ValueOr(false);

    modeValue = 0;
    VerifyOrExit(mMicrowaveOvenModeInstance.GetModeValueByModeTag(to_underlying(MicrowaveOvenMode::ModeTag::kNormal), modeValue) ==
                     CHIP_NO_ERROR,
                 status = Status::InvalidCommand;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, Normal mode is not found"));

    reqCookMode = cookMode.ValueOr(modeValue);
    VerifyOrExit(mMicrowaveOvenModeInstance.IsSupportedMode(reqCookMode), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, cookMode is not supported"));

    reqCookTime = cookTime.ValueOr(MicrowaveOvenControl::kDefaultCookTime);
    VerifyOrExit(IsCookTimeInRange(reqCookTime, mDelegate->GetMaxCookTime()), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    if (HasFeature(to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber))) // power as number
    {
        VerifyOrExit(!wattSettingIndex.HasValue(), status = Status::InvalidCommand; ChipLogError(
            Zcl, "Microwave Oven Control: Failed to set cooking parameters, feature is not support fot this field"));

        VerifyOrExit(
            cookMode.HasValue() || cookTime.HasValue() || powerSetting.HasValue(), status = Status::InvalidCommand;
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, all command fields are missing "));

        reqPowerSetting = powerSetting.ValueOr(mDelegate->GetMaxPower());
        VerifyOrExit(IsPowerSettingInRange(reqPowerSetting, mDelegate->GetMinPower(), mDelegate->GetMaxPower()),
                     status = Status::ConstraintError;
                     ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookPower, cookPower value is out of range"));

        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTime, reqPowerSetting, reqStartAfterSetting,
                                                               to_underlying(MicrowaveOvenControl::Feature::kPowerAsANumber));
    }
    else // power in watts
    {
        VerifyOrExit(!powerSetting.HasValue(), status = Status::InvalidCommand; ChipLogError(
            Zcl, "Microwave Oven Control: Failed to set cooking parameters, feature is not support fot this field "));

        VerifyOrExit(
            cookMode.HasValue() || cookTime.HasValue() || wattSettingIndex.HasValue(), status = Status::InvalidCommand;
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, all command fields are missing "));

        reqWattSettingIndex = wattSettingIndex.ValueOr(mSupportedPowerLevel - 1);
        VerifyOrExit((reqWattSettingIndex <= (mSupportedPowerLevel - 1)) && mSupportedPowerLevel > 0,
                     status = Status::ConstraintError;
                     ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, cookMode is not supported"));

        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTime, reqWattSettingIndex, reqStartAfterSetting,
                                                               to_underlying(MicrowaveOvenControl::Feature::kPowerInWatts));
    }

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
    VerifyOrExit(req.timeToAdd <= mDelegate->GetMaxCookTime() - GetCookTime(), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    finalCookTime = GetCookTime() + req.timeToAdd;
    status        = mDelegate->HandleModifyCookTimeCallback(finalCookTime);

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool IsCookTimeInRange(uint32_t cookTime, uint32_t maxCookTime)
{
    return MicrowaveOvenControl::kMinCookTime <= cookTime && cookTime <= maxCookTime;
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
 */
void MatterMicrowaveOvenControlPluginServerInitCallback() {}
