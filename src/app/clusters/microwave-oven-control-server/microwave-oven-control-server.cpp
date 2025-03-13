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

#include "app/data-model-provider/MetadataList.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

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
                   BitMask<MicrowaveOvenControl::Feature> aFeature, Clusters::OperationalState::Instance & aOpStateInstance,
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
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    VerifyOrReturnError(
        emberAfContainsServer(mEndpointId, mClusterId), CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.", long(mClusterId)));

    // Exactly one of the PowerAsNumber and PowerInWatts features must be supported, per spec.
    VerifyOrReturnError(
        mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) || mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl,
                     "Microwave Oven Control: feature bits error, feature must support one of PowerInWatts and PowerAsNumber"));

    // Check that the feature bits do not include both PowerAsNumber and PowerInWatts
    VerifyOrReturnError(
        !(mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber) &&
          mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts)),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(Zcl,
                     "Microwave Oven Control: feature bits error, feature could not support both PowerAsNumber and PowerInWatts"));

    // Per spec, the PowerNumberLimits feature is only allowed if the PowerAsNumber feature is supported.
    VerifyOrReturnError(
        !mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits) ||
            mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber),
        CHIP_ERROR_INVALID_ARGUMENT,
        ChipLogError(
            Zcl,
            "Microwave Oven Control: feature bits error, if feature supports PowerNumberLimits it must support PowerAsNumber"));

    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    // If the PowerInWatts feature is supported, get the count of supported watt levels so we can later
    // ensure incoming watt level values are valid.
    if (HasFeature(MicrowaveOvenControl::Feature::kPowerInWatts))
    {
        mSupportedWattLevels = GetCountOfSupportedWattLevels();
        VerifyOrReturnError(mSupportedWattLevels > 0, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "Microwave Oven Control: supported watt levels is empty"));
    }
    return CHIP_NO_ERROR;
}

bool Instance::HasFeature(MicrowaveOvenControl::Feature feature) const
{
    return mFeature.Has(feature);
}

uint8_t Instance::GetCountOfSupportedWattLevels() const
{
    uint8_t wattIndex = 0;
    uint16_t watt     = 0;
    while (mDelegate->GetWattSettingByIndex(wattIndex, watt) == CHIP_NO_ERROR)
    {
        wattIndex++;
    }
    return wattIndex;
}

uint32_t Instance::GetCookTimeSec() const
{
    return mCookTimeSec;
}

void Instance::SetCookTimeSec(uint32_t cookTimeSec)
{
    uint32_t oldCookTimeSec = mCookTimeSec;
    mCookTimeSec            = cookTimeSec;
    if (mCookTimeSec != oldCookTimeSec)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::CookTime::Id);
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "Microwave Oven Control: Reading");
    switch (aPath.mAttributeId)
    {
    case MicrowaveOvenControl::Attributes::CookTime::Id:
        return aEncoder.Encode(GetCookTimeSec());

    case MicrowaveOvenControl::Attributes::MaxCookTime::Id:
        return aEncoder.Encode(mDelegate->GetMaxCookTimeSec());

    case MicrowaveOvenControl::Attributes::PowerSetting::Id:
        VerifyOrReturnError(HasFeature(MicrowaveOvenControl::Feature::kPowerAsNumber), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Microwave Oven Control: can not get PowerSetting number, feature is not supported"));

        return aEncoder.Encode(mDelegate->GetPowerSettingNum());

    case MicrowaveOvenControl::Attributes::MinPower::Id:
        VerifyOrReturnError(HasFeature(MicrowaveOvenControl::Feature::kPowerAsNumber), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Microwave Oven Control: can not get MinPower number, feature is not supported"));

        return aEncoder.Encode(HasFeature(MicrowaveOvenControl::Feature::kPowerNumberLimits) ? mDelegate->GetMinPowerNum()
                                                                                             : kDefaultMinPowerNum);

    case MicrowaveOvenControl::Attributes::MaxPower::Id:
        VerifyOrReturnError(HasFeature(MicrowaveOvenControl::Feature::kPowerAsNumber), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Microwave Oven Control: can not get MaxPower number, feature is not supported"));

        return aEncoder.Encode(HasFeature(MicrowaveOvenControl::Feature::kPowerNumberLimits) ? mDelegate->GetMaxPowerNum()
                                                                                             : kDefaultMaxPowerNum);

    case MicrowaveOvenControl::Attributes::PowerStep::Id:
        VerifyOrReturnError(HasFeature(MicrowaveOvenControl::Feature::kPowerAsNumber), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Microwave Oven Control: can not get PowerStep number, feature is not supported"));

        return aEncoder.Encode(HasFeature(MicrowaveOvenControl::Feature::kPowerNumberLimits) ? mDelegate->GetPowerStepNum()
                                                                                             : kDefaultPowerStepNum);

    case MicrowaveOvenControl::Attributes::SupportedWatts::Id:
        VerifyOrReturnError(HasFeature(MicrowaveOvenControl::Feature::kPowerInWatts), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                            ChipLogError(Zcl, "Microwave Oven Control: can not get SuppoertWatts list, feature is not supported"));

        return aEncoder.EncodeList([delegate = mDelegate](const auto & encoder) -> CHIP_ERROR {
            uint16_t wattRating;
            uint8_t index  = 0;
            CHIP_ERROR err = CHIP_NO_ERROR;
            while ((err = delegate->GetWattSettingByIndex(index, wattRating)) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encoder.Encode(wattRating));
                index++;
            }
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                return CHIP_NO_ERROR;
            }
            return err;
        });

    case MicrowaveOvenControl::Attributes::SelectedWattIndex::Id:
        VerifyOrReturnError(
            HasFeature(MicrowaveOvenControl::Feature::kPowerInWatts), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
            ChipLogError(Zcl, "Microwave Oven Control: can not get SelectedWattIndex number, feature is not supported"));

        return aEncoder.Encode(mDelegate->GetCurrentWattIndex());

    case MicrowaveOvenControl::Attributes::WattRating::Id:
        return aEncoder.Encode(mDelegate->GetWattRating());

    case MicrowaveOvenControl::Attributes::FeatureMap::Id:
        return aEncoder.Encode(mFeature.Raw());
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
    Status status = Status::Success;
    uint8_t opState;
    uint8_t modeValue;
    uint8_t reqCookMode;
    uint32_t reqCookTimeSec;
    bool reqStartAfterSetting;
    auto & cookMode          = req.cookMode;
    auto & cookTimeSec       = req.cookTime;
    auto & powerSetting      = req.powerSetting;
    auto & wattSettingIndex  = req.wattSettingIndex;
    auto & startAfterSetting = req.startAfterSetting;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState == to_underlying(OperationalStateEnum::kStopped), status = Status::InvalidInState);

    if (startAfterSetting.HasValue())
    {

        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsList;

        InteractionModelEngine::GetInstance()->GetDataModelProvider()->AcceptedCommands(
            ConcreteClusterPath(mEndpointId, OperationalState::Id), acceptedCommandsList);
        auto acceptedCommands = acceptedCommandsList.TakeBuffer();

        bool commandExists =
            std::find_if(acceptedCommands.begin(), acceptedCommands.end(), [](const DataModel::AcceptedCommandEntry & entry) {
                return entry.commandId == OperationalState::Commands::Start::Id;
            }) != acceptedCommands.end();

        VerifyOrExit(
            commandExists, status = Status::InvalidCommand; ChipLogError(
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

    reqCookTimeSec = cookTimeSec.ValueOr(MicrowaveOvenControl::kDefaultCookTimeSec);
    VerifyOrExit(IsCookTimeSecondsInRange(reqCookTimeSec, mDelegate->GetMaxCookTimeSec()), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    if (HasFeature(MicrowaveOvenControl::Feature::kPowerAsNumber))
    {
        // if using power as number, check if the param is invalid and set PowerSetting number.
        uint8_t reqPowerSettingNum;
        uint8_t maxPowerNum  = kDefaultMaxPowerNum;
        uint8_t minPowerNum  = kDefaultMinPowerNum;
        uint8_t powerStepNum = kDefaultPowerStepNum;
        VerifyOrExit(!wattSettingIndex.HasValue(), status = Status::InvalidCommand; ChipLogError(
            Zcl, "Microwave Oven Control: Failed to set cooking parameters, should have no value for wattSettingIndex"));

        VerifyOrExit(
            cookMode.HasValue() || cookTimeSec.HasValue() || powerSetting.HasValue(), status = Status::InvalidCommand;
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, all command fields are missing "));

        if (HasFeature(MicrowaveOvenControl::Feature::kPowerNumberLimits))
        {
            maxPowerNum  = mDelegate->GetMaxPowerNum();
            minPowerNum  = mDelegate->GetMinPowerNum();
            powerStepNum = mDelegate->GetPowerStepNum();
        }
        reqPowerSettingNum = powerSetting.ValueOr(maxPowerNum);
        VerifyOrExit(IsPowerSettingNumberInRange(reqPowerSettingNum, minPowerNum, maxPowerNum), status = Status::ConstraintError;
                     ChipLogError(Zcl, "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value is out of range"));

        VerifyOrExit(
            (reqPowerSettingNum - minPowerNum) % powerStepNum == 0, status = Status::ConstraintError; ChipLogError(
                Zcl,
                "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value must be multiple of PowerStep number"));

        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting,
                                                               MakeOptional(reqPowerSettingNum), NullOptional);
    }
    else
    {
        // if using power in watt, check if the param is invalid and set wattSettingIndex number.
        uint8_t reqWattSettingIndex;
        VerifyOrExit(!powerSetting.HasValue(), status = Status::InvalidCommand; ChipLogError(
            Zcl, "Microwave Oven Control: Failed to set cooking parameters, should have no value for powerSetting "));

        VerifyOrExit(
            cookMode.HasValue() || cookTimeSec.HasValue() || wattSettingIndex.HasValue(), status = Status::InvalidCommand;
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, all command fields are missing "));

        // count of supported watt levels must greater than 0
        VerifyOrExit(
            mSupportedWattLevels > 0,
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set wattSettingIndex, count of supported watt levels is 0"));
        uint8_t maxWattSettingIndex = static_cast<uint8_t>(mSupportedWattLevels - 1);
        reqWattSettingIndex         = wattSettingIndex.ValueOr(maxWattSettingIndex);
        VerifyOrExit(reqWattSettingIndex <= maxWattSettingIndex, status = Status::ConstraintError;
                     ChipLogError(Zcl, "Microwave Oven Control: Failed to set wattSettingIndex, wattSettingIndex is out of range"));

        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting, NullOptional,
                                                               MakeOptional(reqWattSettingIndex));
    }

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req)
{
    ChipLogDetail(Zcl, "Microwave Oven Control: HandleAddMoreTime");
    Status status;
    uint8_t opState;
    uint32_t finalCookTimeSec;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState != to_underlying(OperationalStateEnum::kError), status = Status::InvalidInState);

    // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
    VerifyOrExit(req.timeToAdd + GetCookTimeSec() <= mDelegate->GetMaxCookTimeSec(), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    finalCookTimeSec = GetCookTimeSec() + req.timeToAdd;
    status           = mDelegate->HandleModifyCookTimeSecondsCallback(finalCookTimeSec);

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool IsCookTimeSecondsInRange(uint32_t cookTimeSec, uint32_t maxCookTimeSec)
{
    return MicrowaveOvenControl::kMinCookTimeSec <= cookTimeSec && cookTimeSec <= maxCookTimeSec;
}

bool IsPowerSettingNumberInRange(uint8_t powerSettingNum, uint8_t minCookPowerNum, uint8_t maxCookPowerNum)
{
    return minCookPowerNum <= powerSettingNum && powerSettingNum <= maxCookPowerNum;
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
