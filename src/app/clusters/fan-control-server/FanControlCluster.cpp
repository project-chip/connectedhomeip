/*
 *
 * Copyright (c) 2022-2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/****************************************************************************
 * @file
 ***************************************************************************/

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FanControl/Attributes.h>
#include <clusters/FanControl/Commands.h>
#include <clusters/FanControl/Enums.h>
#include <clusters/FanControl/Metadata.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;

using Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

FanControlCluster::FanControlCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, FanControl::Id }), mFanModeSequence(config.mFanModeSequence),
    mSupportsStep(config.mSupportsStep), mSpeedMax(config.mSpeedMax), mRockSupport(config.mRockSupport),
    mWindSupport(config.mWindSupport), mOptionalAttributes(config.mOptionalAttributes), mFeatureMap(config.mFeatureMap),
    mDelegate(config.mDelegate)
{}

CHIP_ERROR FanControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    return CHIP_NO_ERROR;
}

void FanControlCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
}

Protocols::InteractionModel::Status FanControlCluster::SetFanModeToOff()
{
    if (mFanMode != FanModeEnum::kOff)
    {
        mFanMode = FanModeEnum::kOff;
        ApplyFanModeOffSideEffects();
    }
    return Status::Success;
}

void FanControlCluster::ApplyFanModeOffSideEffects()
{
    mPercentSetting.SetNonNull(0);
    NotifyAttributeChanged(PercentSetting::Id);
    NotifyAttributeChanged(PercentCurrent::Id);

    if (SupportsMultiSpeed())
    {
        mSpeedSetting.SetNonNull(0);
        NotifyAttributeChanged(SpeedSetting::Id);
        NotifyAttributeChanged(SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeLowSideEffects()
{
    mPercentSetting.SetNonNull(33); // Spec mandates 33%
    NotifyAttributeChanged(PercentSetting::Id);
    NotifyAttributeChanged(PercentCurrent::Id);

    if (SupportsMultiSpeed())
    {
        mSpeedSetting.SetNonNull(1); // Spec mandates Speed 1
        NotifyAttributeChanged(SpeedSetting::Id);
        NotifyAttributeChanged(SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeMediumSideEffects()
{
    mPercentSetting.SetNonNull(66); // Spec mandates 66%
    NotifyAttributeChanged(PercentSetting::Id);
    NotifyAttributeChanged(PercentCurrent::Id);

    if (SupportsMultiSpeed())
    {
        // Spec mandates ceil(SpeedMax / 2) for Medium
        uint8_t speedSetting = (mSpeedMax > 1) ? static_cast<uint8_t>((mSpeedMax + 1) / 2) : 1;
        mSpeedSetting.SetNonNull(speedSetting);
        NotifyAttributeChanged(SpeedSetting::Id);
        NotifyAttributeChanged(SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeHighSideEffects()
{
    mPercentSetting.SetNonNull(100); // Spec mandates 100%
    NotifyAttributeChanged(PercentSetting::Id);
    NotifyAttributeChanged(PercentCurrent::Id);

    if (SupportsMultiSpeed())
    {
        mSpeedSetting.SetNonNull(mSpeedMax); // Spec mandates SpeedMax
        NotifyAttributeChanged(SpeedSetting::Id);
        NotifyAttributeChanged(SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeAutoSideEffects()
{
    mPercentSetting.SetNull();
    NotifyAttributeChanged(PercentSetting::Id);

    if (SupportsMultiSpeed())
    {
        mSpeedSetting.SetNull();
        NotifyAttributeChanged(SpeedSetting::Id);
    }
}

void FanControlCluster::ApplyPercentSettingChanged()
{
    if (mPercentSetting.IsNull())
        return;

    if (mPercentSetting.Value() == 0)
    {
        SetFanModeToOff();
        return;
    }

    if (SupportsMultiSpeed())
    {
        uint8_t speedMax     = mSpeedMax;
        uint16_t percent     = mPercentSetting.Value();
        uint8_t speedSetting = static_cast<uint8_t>((speedMax * percent + 99) / 100);
        mSpeedSetting.SetNonNull(speedSetting);
        NotifyAttributeChanged(SpeedSetting::Id);
    }
}

void FanControlCluster::ApplySpeedSettingChanged()
{
    if (!SupportsMultiSpeed() || mSpeedSetting.IsNull())
        return;

    if (mSpeedSetting.Value() == 0)
    {
        SetFanModeToOff();
        return;
    }

    uint8_t speedMax             = mSpeedMax;
    chip::Percent percentSetting = static_cast<chip::Percent>((mSpeedSetting.Value() * 100) / speedMax);
    mPercentSetting.SetNonNull(percentSetting);
    NotifyAttributeChanged(PercentSetting::Id);
}

DataModel::ActionReturnStatus FanControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(FanControl::kRevision);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case FanMode::Id:
        return encoder.Encode(mFanMode);
    case FanModeSequence::Id:
        return encoder.Encode(mFanModeSequence);
    case PercentSetting::Id:
        return encoder.Encode(mPercentSetting);
    case PercentCurrent::Id:
        return encoder.Encode(mPercentSetting.IsNull() ? static_cast<chip::Percent>(0) : mPercentSetting.Value());
    case SpeedMax::Id:
        if (!SupportsMultiSpeed())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mSpeedMax);
    case SpeedSetting::Id:
        if (!SupportsMultiSpeed())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mSpeedSetting);
    case SpeedCurrent::Id:
        if (!SupportsMultiSpeed())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mSpeedSetting.IsNull() ? static_cast<uint8_t>(0) : mSpeedSetting.Value());
    case RockSupport::Id:
        if (!SupportsRocking())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mRockSupport);
    case RockSetting::Id:
        if (!SupportsRocking())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mRockSetting);
    case WindSupport::Id:
        if (!SupportsWind())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mWindSupport);
    case WindSetting::Id:
        if (!SupportsWind())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mWindSetting);
    case AirflowDirection::Id:
        if (!SupportsAirflowDirection())
            return Status::UnsupportedAttribute;
        return encoder.Encode(mAirflowDirection);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus FanControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case FanMode::Id: {
        FanModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetFanMode(value);
    }
    case PercentSetting::Id: {
        DataModel::Nullable<chip::Percent> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetPercentSetting(value);
    }
    case SpeedSetting::Id: {
        DataModel::Nullable<uint8_t> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetSpeedSetting(value);
    }
    case RockSetting::Id: {
        BitMask<RockBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetRockSetting(value);
    }
    case WindSetting::Id: {
        BitMask<WindBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetWindSetting(value);
    }
    case AirflowDirection::Id: {
        AirflowDirectionEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetAirflowDirection(value);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR FanControlCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry                = AttributeListBuilder::OptionalAttributeEntry;
    OptionalEntry optionalAttributes[] = {
        { SupportsMultiSpeed(), SpeedMax::kMetadataEntry },     { SupportsMultiSpeed(), SpeedSetting::kMetadataEntry },
        { SupportsMultiSpeed(), SpeedCurrent::kMetadataEntry }, { SupportsRocking(), RockSupport::kMetadataEntry },
        { SupportsRocking(), RockSetting::kMetadataEntry },     { SupportsWind(), WindSupport::kMetadataEntry },
        { SupportsWind(), WindSetting::kMetadataEntry },        { SupportsAirflowDirection(), AirflowDirection::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(FanControl::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR FanControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (SupportsStep())
    {
        static const DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
            Commands::Step::kMetadataEntry,
        };
        return builder.ReferenceExisting(kAcceptedCommands);
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> FanControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::Step::Id: {
        if (!SupportsStep())
            return Status::UnsupportedCommand;

        Commands::Step::DecodableType commandData;
        VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        bool wrapValue      = commandData.wrap.ValueOr(false);
        bool lowestOffValue = commandData.lowestOff.ValueOr(false);

        if (mDelegate == nullptr)
            return Status::Failure;
        return mDelegate->HandleStep(commandData.direction, wrapValue, lowestOffValue);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus FanControlCluster::SetFanMode(FanModeEnum value)
{
    uint8_t seq = chip::to_underlying(mFanModeSequence);
    if (value == FanModeEnum::kLow && seq >= 4)
        return Status::ConstraintError;
    if (value == FanModeEnum::kMedium && seq != 0 && seq != 2)
        return Status::ConstraintError;
    if (value == FanModeEnum::kAuto && !SupportsAuto())
        return Status::ConstraintError;

    FanModeEnum newMode = value;

    if (value == FanModeEnum::kOn)
    {
        newMode = FanModeEnum::kHigh;
    }
    else if (value == FanModeEnum::kSmart)
    {
        if (SupportsAuto() &&
            (mFanModeSequence == FanModeSequenceEnum::kOffLowHighAuto ||
             mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto))
        {
            newMode = FanModeEnum::kAuto;
        }
        else
        {
            newMode = FanModeEnum::kHigh;
        }
    }

    mFanMode = newMode;

    if (newMode == FanModeEnum::kOff)
    {
        ApplyFanModeOffSideEffects();
    }
    else if (newMode == FanModeEnum::kLow)
    {
        ApplyFanModeLowSideEffects();
    }
    else if (newMode == FanModeEnum::kMedium)
    {
        ApplyFanModeMediumSideEffects();
    }
    else if (newMode == FanModeEnum::kHigh)
    {
        ApplyFanModeHighSideEffects();
    }
    else if (newMode == FanModeEnum::kAuto)
    {
        ApplyFanModeAutoSideEffects();
    }

    return NotifyAttributeChangedIfSuccess(FanMode::Id, Status::Success);
}

DataModel::ActionReturnStatus FanControlCluster::SetPercentSetting(DataModel::Nullable<chip::Percent> value)
{
    if (value.IsNull())
    {
        if (SupportsAuto())
        {
            SetFanMode(FanModeEnum::kAuto);
        }
        return Status::Success;
    }

    mPercentSetting = value;
    ApplyPercentSettingChanged();
    return NotifyAttributeChangedIfSuccess(PercentSetting::Id, Status::Success);
}

DataModel::ActionReturnStatus FanControlCluster::SetSpeedSetting(DataModel::Nullable<uint8_t> value)
{
    if (!SupportsMultiSpeed())
        return Status::UnsupportedAttribute;

    if (value.IsNull())
    {
        if (SupportsAuto())
        {
            SetFanMode(FanModeEnum::kAuto);
        }
        return Status::Success;
    }

    if (value.Value() > mSpeedMax)
        return Status::ConstraintError;

    mSpeedSetting = value;
    ApplySpeedSettingChanged();
    return NotifyAttributeChangedIfSuccess(SpeedSetting::Id, Status::Success);
}

DataModel::ActionReturnStatus FanControlCluster::SetRockSetting(BitMask<RockBitmap> value)
{
    if (!SupportsRocking())
        return Status::UnsupportedAttribute;

    uint8_t rawValue   = value.Raw();
    uint8_t rawSupport = mRockSupport.Raw();
    if ((rawValue & rawSupport) != rawValue)
        return Status::ConstraintError;

    mRockSetting = value;
    return NotifyAttributeChangedIfSuccess(RockSetting::Id, Status::Success);
}

DataModel::ActionReturnStatus FanControlCluster::SetWindSetting(BitMask<WindBitmap> value)
{
    if (!SupportsWind())
        return Status::UnsupportedAttribute;

    uint8_t rawValue   = value.Raw();
    uint8_t rawSupport = mWindSupport.Raw();
    if ((rawValue & rawSupport) != rawValue)
        return Status::ConstraintError;

    mWindSetting = value;
    return NotifyAttributeChangedIfSuccess(WindSetting::Id, Status::Success);
}

DataModel::ActionReturnStatus FanControlCluster::SetAirflowDirection(AirflowDirectionEnum value)
{
    if (!SupportsAirflowDirection())
        return Status::UnsupportedAttribute;

    mAirflowDirection = value;
    return NotifyAttributeChangedIfSuccess(AirflowDirection::Id, Status::Success);
}

void FanControlCluster::SetDelegate(FanControl::Delegate * delegate)
{
    mDelegate = delegate;
}

} // namespace chip::app::Clusters
