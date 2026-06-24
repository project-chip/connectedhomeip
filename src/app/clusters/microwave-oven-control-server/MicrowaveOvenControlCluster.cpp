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

#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/OperationalState/Commands.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using Status = Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

using namespace MicrowaveOvenControl;
using namespace MicrowaveOvenControl::Attributes;
using namespace MicrowaveOvenControl::Commands;

namespace {

constexpr uint32_t kDefaultCookTimeSec = 30u;
constexpr uint32_t kMinCookTimeSec     = 1u;
constexpr uint8_t kDefaultMinPowerNum  = 10u;
constexpr uint8_t kDefaultMaxPowerNum  = 100u;
constexpr uint8_t kDefaultPowerStepNum = 10u;

bool IsCookTimeSecondsInRange(uint32_t cookTimeSec, uint32_t maxCookTimeSec)
{
    return kMinCookTimeSec <= cookTimeSec && cookTimeSec <= maxCookTimeSec;
}

bool IsPowerSettingNumberInRange(uint8_t powerSettingNum, uint8_t minCookPowerNum, uint8_t maxCookPowerNum)
{
    return minCookPowerNum <= powerSettingNum && powerSettingNum <= maxCookPowerNum;
}

} // namespace

MicrowaveOvenControlCluster::MicrowaveOvenControlCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, MicrowaveOvenControl::Id }), mFeature(config.feature),
    mOptionalAttributeSet(config.optionalAttributeSet), mSupportsAddMoreTime(config.supportsAddMoreTime),
    mIntegrationDelegate(config.integrationDelegate), mAppDelegate(config.appDelegate), mCookTimeSec(kDefaultCookTimeSec)
{}

CHIP_ERROR MicrowaveOvenControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // If the PowerInWatts feature is supported, get the count of supported watt levels so we can later
    // ensure incoming watt level values are valid.
    if (mFeature.Has(Feature::kPowerInWatts))
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
    while (mAppDelegate.GetWattSettingByIndex(wattIndex, watt) == CHIP_NO_ERROR)
    {
        wattIndex++;
    }
    return wattIndex;
}

CHIP_ERROR MicrowaveOvenControlCluster::SetCookTimeSec(uint32_t cookTimeSec)
{
    VerifyOrReturnError(IsCookTimeSecondsInRange(cookTimeSec, mAppDelegate.GetMaxCookTimeSec()),
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));

    SetAttributeValue(mCookTimeSec, cookTimeSec, CookTime::Id);
    return CHIP_NO_ERROR;
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
        return encoder.Encode(mAppDelegate.GetMaxCookTimeSec());
    case PowerSetting::Id:
        return encoder.Encode(mAppDelegate.GetPowerSettingNum());
    case MinPower::Id:
        return encoder.Encode(mAppDelegate.GetMinPowerNum());
    case MaxPower::Id:
        return encoder.Encode(mAppDelegate.GetMaxPowerNum());
    case PowerStep::Id:
        return encoder.Encode(mAppDelegate.GetPowerStepNum());
    case SupportedWatts::Id:
        return encoder.EncodeList([this](const auto & encod) -> CHIP_ERROR {
            uint16_t wattRating = 0;
            uint8_t index       = 0;
            CHIP_ERROR err      = CHIP_NO_ERROR;
            while ((err = mAppDelegate.GetWattSettingByIndex(index, wattRating)) == CHIP_NO_ERROR)
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
        return encoder.Encode(mAppDelegate.GetCurrentWattIndex());
    case WattRating::Id:
        return encoder.Encode(mAppDelegate.GetWattRating());
    case FeatureMap::Id:
        return encoder.Encode(mFeature);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR MicrowaveOvenControlCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeature.Has(Feature::kPowerAsNumber), PowerSetting::kMetadataEntry },
        { mFeature.Has(Feature::kPowerNumberLimits), MinPower::kMetadataEntry },
        { mFeature.Has(Feature::kPowerNumberLimits), MaxPower::kMetadataEntry },
        { mFeature.Has(Feature::kPowerNumberLimits), PowerStep::kMetadataEntry },
        { mFeature.Has(Feature::kPowerInWatts), SupportedWatts::kMetadataEntry },
        { mFeature.Has(Feature::kPowerInWatts), SelectedWattIndex::kMetadataEntry },
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
    case Commands::SetCookingParameters::Id: {
        Commands::SetCookingParameters::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleSetCookingParameters(request.path, data);
    }
    case Commands::AddMoreTime::Id: {
        Commands::AddMoreTime::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return HandleAddMoreTime(data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR MicrowaveOvenControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.AppendElements({ Commands::SetCookingParameters::kMetadataEntry }));

    if (mSupportsAddMoreTime)
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::AddMoreTime::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
MicrowaveOvenControlCluster::HandleSetCookingParameters(const ConcreteCommandPath & commandPath,
                                                        const Commands::SetCookingParameters::DecodableType & commandData)
{
    Status status            = Status::Success;
    auto & cookMode          = commandData.cookMode;
    auto & cookTimeSec       = commandData.cookTime;
    auto & powerSetting      = commandData.powerSetting;
    auto & wattSettingIndex  = commandData.wattSettingIndex;
    auto & startAfterSetting = commandData.startAfterSetting;

    uint8_t opState = mIntegrationDelegate.GetCurrentOperationalState();
    VerifyOrReturnError(opState == to_underlying(OperationalState::OperationalStateEnum::kStopped), Status::InvalidInState);

    if (startAfterSetting.HasValue())
    {
        VerifyOrReturnError(
            mIntegrationDelegate.IsSupportedOperationalStateCommand(commandPath.mEndpointId, OperationalState::Commands::Start::Id),
            Status::InvalidCommand,
            ChipLogError(
                Zcl,
                "Microwave Oven Control: Failed to set cooking parameters, Start command of operational state is not supported"));
    }
    bool reqStartAfterSetting = startAfterSetting.ValueOr(false);

    uint8_t modeValue = 0;
    VerifyOrReturnError(mIntegrationDelegate.GetNormalOperatingMode(modeValue) == CHIP_NO_ERROR, Status::InvalidCommand,
                        ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, Normal mode is not found"));

    uint8_t reqCookMode = cookMode.ValueOr(modeValue);
    VerifyOrReturnError(mIntegrationDelegate.IsSupportedMode(reqCookMode), Status::ConstraintError,
                        ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookMode, cookMode is not supported"));

    uint32_t reqCookTimeSec = cookTimeSec.ValueOr(kDefaultCookTimeSec);
    VerifyOrReturnError(IsCookTimeSecondsInRange(reqCookTimeSec, mAppDelegate.GetMaxCookTimeSec()), Status::ConstraintError,
                        ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    if (mFeature.Has(Feature::kPowerAsNumber))
    {
        // if using power as number, check if the param is invalid and set PowerSetting number.
        uint8_t maxPowerNum  = kDefaultMaxPowerNum;
        uint8_t minPowerNum  = kDefaultMinPowerNum;
        uint8_t powerStepNum = kDefaultPowerStepNum;

        VerifyOrReturnError(
            !wattSettingIndex.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl,
                         "Microwave Oven Control: Failed to set cooking parameters, should have no value for wattSettingIndex"));

        if (mFeature.Has(Feature::kPowerNumberLimits))
        {
            maxPowerNum  = mAppDelegate.GetMaxPowerNum();
            minPowerNum  = mAppDelegate.GetMinPowerNum();
            powerStepNum = mAppDelegate.GetPowerStepNum();
        }

        uint8_t reqPowerSettingNum = powerSetting.ValueOr(maxPowerNum);
        VerifyOrReturnError(
            IsPowerSettingNumberInRange(reqPowerSettingNum, minPowerNum, maxPowerNum), Status::ConstraintError,
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value is out of range"));

        VerifyOrReturnError(
            (reqPowerSettingNum - minPowerNum) % powerStepNum == 0, Status::ConstraintError,
            ChipLogError(
                Zcl,
                "Microwave Oven Control: Failed to set PowerSetting, PowerSetting value must be multiple of PowerStep number"));

        // Snapshot PowerSetting before the delegate call so this cluster server can notify that the
        // attribute changed when the delegate updates its internal value.  The delegate owns storage for
        // this attribute, but to notify the change is the server's responsibility.
        uint8_t oldPowerSettingNum = mAppDelegate.GetPowerSettingNum();

        status = mAppDelegate.HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting,
                                                                 MakeOptional(reqPowerSettingNum), NullOptional);

        if (oldPowerSettingNum != mAppDelegate.GetPowerSettingNum())
        {
            NotifyAttributeChanged(PowerSetting::Id);
        }
    }
    else
    {
        // if using power in watt, check if the param is invalid and set wattSettingIndex number.
        VerifyOrReturnError(
            !powerSetting.HasValue(), Status::InvalidCommand,
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set cooking parameters, should have no value for powerSetting "));

        // count of supported watt levels must greater than 0
        VerifyOrReturnError(
            mSupportedWattLevels > 0, Status::ConstraintError,
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set wattSettingIndex, count of supported watt levels is 0"));

        uint8_t maxWattSettingIndex = static_cast<uint8_t>(mSupportedWattLevels - 1);
        uint8_t reqWattSettingIndex = wattSettingIndex.ValueOr(maxWattSettingIndex);
        VerifyOrReturnError(
            reqWattSettingIndex <= maxWattSettingIndex, Status::ConstraintError,
            ChipLogError(Zcl, "Microwave Oven Control: Failed to set wattSettingIndex, wattSettingIndex is out of range"));

        // Snapshot WattSettingIndex and WattRating before the delegate call so this cluster server can notify that the
        // attribute changed when the delegate updates its internal value. The delegate owns storage for these attributes,
        // but to notify the change is the server's responsibility.
        uint8_t oldWattSettingIndex = mAppDelegate.GetCurrentWattIndex();
        uint16_t oldWattRating      = mAppDelegate.GetWattRating();

        status = mAppDelegate.HandleSetCookingParametersCallback(reqCookMode, reqCookTimeSec, reqStartAfterSetting, NullOptional,
                                                                 MakeOptional(reqWattSettingIndex));

        if (oldWattSettingIndex != mAppDelegate.GetCurrentWattIndex())
        {
            NotifyAttributeChanged(SelectedWattIndex::Id);
        }
        if (oldWattRating != mAppDelegate.GetWattRating())
        {
            NotifyAttributeChanged(WattRating::Id);
        }
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus>
MicrowaveOvenControlCluster::HandleAddMoreTime(const Commands::AddMoreTime::DecodableType & commandData)
{
    uint8_t opState = mIntegrationDelegate.GetCurrentOperationalState();
    VerifyOrReturnError(opState != to_underlying(OperationalState::OperationalStateEnum::kError), Status::InvalidInState);

    // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
    VerifyOrReturnError(commandData.timeToAdd <= mAppDelegate.GetMaxCookTimeSec() &&
                            GetCookTimeSec() <= mAppDelegate.GetMaxCookTimeSec() - commandData.timeToAdd,
                        Status::ConstraintError,
                        ChipLogError(Zcl, "Microwave Oven Control: Failed to set cookTime, cookTime value is out of range"));

    uint32_t finalCookTimeSec = GetCookTimeSec() + commandData.timeToAdd;
    return mAppDelegate.HandleModifyCookTimeSecondsCallback(finalCookTimeSec);
}

} // namespace chip::app::Clusters
