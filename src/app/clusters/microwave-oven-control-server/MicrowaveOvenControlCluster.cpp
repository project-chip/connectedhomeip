/**
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/MicrowaveOvenControl/Metadata.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using Status = Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

namespace {

constexpr uint32_t kDefaultCookTimeSec = 30u;
constexpr uint32_t kMinCookTimeSec     = 1u;
constexpr uint8_t kDefaultMinPowerNum  = 10u;
constexpr uint8_t kDefaultMaxPowerNum  = 100u;
constexpr uint8_t kDefaultPowerStepNum = 10u;

} // namespace

MicrowaveOvenControlCluster::MicrowaveOvenControlCluster(EndpointId endpointId, BitMask<MicrowaveOvenControl::Feature> feature,
                                                         const OptionalAttributeSet & optionalAttributeSet, const Context context) :
    DefaultServerCluster({ endpointId, MicrowaveOvenControl::Id }),
    mFeature(feature), mOptionalAttributeSet(optionalAttributeSet), mDelegate(context.delegate),
    mOpStateInstance(context.opStateInstance), mMicrowaveOvenModeInstance(context.microwaveOvenModeInstance),
    mInteractionModelEngine(context.interactionModelEngine), mAcceptedCommands(context.acceptedCommands),
    mCookTimeSec(kDefaultCookTimeSec)
{}

CHIP_ERROR MicrowaveOvenControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // If the PowerInWatts feature is supported, get the count of supported watt levels so we can later
    // ensure incoming watt level values are valid.
    if (mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts))
    {
        mSupportedWattLevels = GetCountOfSupportedWattLevels();
        VerifyOrReturnError(mSupportedWattLevels > 0, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "Microwave Oven Control: supported watt levels is empty"));
    }

    return CHIP_NO_ERROR;
}

uint8_t MicrowaveOvenControlCluster::GetCountOfSupportedWattLevels() const
{
    uint8_t wattIndex = 0;
    uint16_t watt     = 0;
    while (mDelegate.GetWattSettingByIndex(wattIndex, watt) == CHIP_NO_ERROR)
    {
        wattIndex++;
    }
    return wattIndex;
}

uint32_t MicrowaveOvenControlCluster::GetCookTimeSec() const
{
    return mCookTimeSec;
}

void MicrowaveOvenControlCluster::SetCookTimeSec(uint32_t cookTimeSec)
{
    SetAttributeValue(mCookTimeSec, cookTimeSec, CookTime::Id);
}

DataModel::ActionReturnStatus MicrowaveOvenControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(MicrowaveOvenControl::kRevision);
    case CookTime::Id:
        return encoder.Encode(GetCookTimeSec());
    case MaxCookTime::Id:
        return encoder.Encode(mDelegate.GetMaxCookTimeSec());
    case PowerSetting::Id:
        return encoder.Encode(mDelegate.GetPowerSettingNum());
    case MinPower::Id:
        return encoder.Encode(mDelegate.GetMinPowerNum());
    case MaxPower::Id:
        return encoder.Encode(mDelegate.GetMaxPowerNum());
    case PowerStep::Id:
        return encoder.Encode(mDelegate.GetPowerStepNum());
    case SupportedWatts::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR {
            uint16_t wattRating = 0;
            uint8_t index       = 0;
            CHIP_ERROR err      = CHIP_NO_ERROR;
            while ((err = mDelegate.GetWattSettingByIndex(index, wattRating)) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encod.Encode(wattRating));
                index++;
            }
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                return CHIP_NO_ERROR;
            }
            return err;
        });
    case SelectedWattIndex::Id:
        return encoder.Encode(mDelegate.GetCurrentWattIndex());
    case WattRating::Id:
        return encoder.Encode(mDelegate.GetWattRating());
    case FeatureMap::Id:
        return encoder.Encode(mFeature.Raw());
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR MicrowaveOvenControlCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber), PowerSetting::kMetadataEntry },
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits), MinPower::kMetadataEntry },
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits), MaxPower::kMetadataEntry },
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits), PowerStep::kMetadataEntry },
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts), SupportedWatts::kMetadataEntry },
        { mFeature.Has(MicrowaveOvenControl::Feature::kPowerInWatts), SelectedWattIndex::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(WattRating::Id), WattRating::kMetadataEntry },
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

std::optional<DataModel::ActionReturnStatus> MicrowaveOvenControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case MicrowaveOvenControl::Commands::SetCookingParameters::Id: {
        MicrowaveOvenControl::Commands::SetCookingParameters::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetCookingParameters(handler, request.path, data);
    }
    case MicrowaveOvenControl::Commands::AddMoreTime::Id: {
        MicrowaveOvenControl::Commands::AddMoreTime::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleAddMoreTime(handler, request.path, data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR MicrowaveOvenControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mAcceptedCommands.test(MicrowaveOvenControl::Commands::SetCookingParameters::Id))
    {
        ReturnErrorOnFailure(builder.AppendElements({ MicrowaveOvenControl::Commands::SetCookingParameters::kMetadataEntry }));
    }

    if (mAcceptedCommands.test(MicrowaveOvenControl::Commands::AddMoreTime::Id))
    {
        ReturnErrorOnFailure(builder.AppendElements({ MicrowaveOvenControl::Commands::AddMoreTime::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

static bool IsCookTimeSecondsInRange(uint32_t cookTimeSec, uint32_t maxCookTimeSec)
{
    return kMinCookTimeSec <= cookTimeSec && cookTimeSec <= maxCookTimeSec;
}

static bool IsPowerSettingNumberInRange(uint8_t powerSettingNum, uint8_t minCookPowerNum, uint8_t maxCookPowerNum)
{
    return minCookPowerNum <= powerSettingNum && powerSettingNum <= maxCookPowerNum;
}

std::optional<DataModel::ActionReturnStatus> MicrowaveOvenControlCluster::HandleSetCookingParameters(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const MicrowaveOvenControl::Commands::SetCookingParameters::DecodableType & commandData)
{
    Status status = Status::Success;
    uint8_t opState;
    uint8_t modeValue;
    uint8_t reqCookMode;
    uint32_t reqCookTimeSec;
    bool reqStartAfterSetting;
    auto & cookMode          = commandData.cookMode;
    auto & cookTimeSec       = commandData.cookTime;
    auto & powerSetting      = commandData.powerSetting;
    auto & wattSettingIndex  = commandData.wattSettingIndex;
    auto & startAfterSetting = commandData.startAfterSetting;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState == to_underlying(OperationalStateEnum::kStopped), status = Status::InvalidInState);

    if (startAfterSetting.HasValue())
    {

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsList;

        TEMPORARY_RETURN_IGNORED mInteractionModelEngine.GetDataModelProvider()->AcceptedCommands(
            ConcreteClusterPath(commandPath.mEndpointId, OperationalState::Id), acceptedCommandsList);
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

    reqCookTimeSec = cookTimeSec.ValueOr(kDefaultCookTimeSec);
    VerifyOrExit(IsCookTimeSecondsInRange(reqCookTimeSec, mDelegate.GetMaxCookTimeSec()), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    if (mFeature.Has(MicrowaveOvenControl::Feature::kPowerAsNumber))
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

        if (mFeature.Has(MicrowaveOvenControl::Feature::kPowerNumberLimits))
        {
            maxPowerNum  = mDelegate.GetMaxPowerNum();
            minPowerNum  = mDelegate.GetMinPowerNum();
            powerStepNum = mDelegate.GetPowerStepNum();
        }
        reqPowerSettingNum = powerSetting.ValueOr(maxPowerNum);
        VerifyOrExit(IsPowerSettingNumberInRange(reqPowerSettingNum, minPowerNum, maxPowerNum), status = Status::ConstraintError;
                     ChipLogError(Zcl, "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value is out of range"));

        VerifyOrExit(
            (reqPowerSettingNum - minPowerNum) % powerStepNum == 0, status = Status::ConstraintError; ChipLogError(
                Zcl,
                "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value must be multiple of PowerStep number"));

        status = mDelegate.HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting,
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

        status = mDelegate.HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting, NullOptional,
                                                              MakeOptional(reqWattSettingIndex));
    }

exit:
    commandObj->AddStatus(commandPath, status);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
MicrowaveOvenControlCluster::HandleAddMoreTime(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const MicrowaveOvenControl::Commands::AddMoreTime::DecodableType & commandData)
{
    Status status = Status::Success;
    uint8_t opState;
    uint32_t finalCookTimeSec;

    opState = mOpStateInstance.GetCurrentOperationalState();
    VerifyOrExit(opState != to_underlying(OperationalStateEnum::kError), status = Status::InvalidInState);

    // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
    VerifyOrExit(commandData.timeToAdd + GetCookTimeSec() <= mDelegate.GetMaxCookTimeSec(), status = Status::ConstraintError;
                 ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    finalCookTimeSec = GetCookTimeSec() + commandData.timeToAdd;
    status           = mDelegate.HandleModifyCookTimeSecondsCallback(finalCookTimeSec);

exit:
    commandObj->AddStatus(commandPath, status);
    return std::nullopt;
}

} // namespace chip::app::Clusters
