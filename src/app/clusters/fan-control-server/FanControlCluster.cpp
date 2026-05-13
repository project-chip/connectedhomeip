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

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FanControl/Attributes.h>
#include <clusters/FanControl/Commands.h>
#include <clusters/FanControl/Enums.h>
#include <clusters/FanControl/EnumsCheck.h>
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
    mSpeedMax(config.mSpeedMax), mRockSupport(config.mRockSupport), mWindSupport(config.mWindSupport),
    mOptionalAttributes(config.mOptionalAttributes), mFeatureMap(config.mFeatureMap), mDelegate(config.mDelegate)
{}

void FanControlCluster::NotifyDelegateFanDriveState()
{
    if (mDelegate == nullptr)
    {
        return;
    }
    VerifyOrReturn(!mTemporarilyIgnoreFanDriveDelegateCallbacks);

    // Prevent potential callback loops
    mTemporarilyIgnoreFanDriveDelegateCallbacks = true;
    const FanDriveState state{ mFanMode, mPercentSetting, mPercentCurrent, mSpeedSetting, mSpeedCurrent };
    mDelegate->OnFanDriveStateChanged(state);
    mTemporarilyIgnoreFanDriveDelegateCallbacks = false;
}

CHIP_ERROR FanControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attrPersistence{ context.attributeStorage };
    FanModeEnum restoredFanMode = mFanMode;
    attrPersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, FanControl::Id, FanMode::Id), restoredFanMode,
                                          mFanMode);

    DataModel::ActionReturnStatus status = SetFanMode(restoredFanMode);
    if (!status.IsSuccess())
    {
        status = SetFanMode(FanModeEnum::kOff);
    }
    VerifyOrReturnError(status.IsSuccess(), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void FanControlCluster::CommitFanModeOffState()
{
    VerifyOrReturn(SetAttributeValue(mFanMode, FanModeEnum::kOff, FanMode::Id)); // noop if already off
    ApplyFanModeSideEffects(FanModeEnum::kOff);
    StoreFanModePersistence();
}

void FanControlCluster::ApplyFanModeSideEffects(FanModeEnum fanMode)
{
    switch (fanMode)
    {
    case FanModeEnum::kOff:
        SetAttributeValue(mPercentSetting, DataModel::MakeNullable<chip::Percent>(0), PercentSetting::Id);
        SetAttributeValue(mPercentCurrent, chip::Percent{ 0 }, PercentCurrent::Id);

        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedSetting, DataModel::MakeNullable<uint8_t>(0), SpeedSetting::Id);
            SetAttributeValue(mSpeedCurrent, static_cast<uint8_t>(0), SpeedCurrent::Id);
        }
        break;

    case FanModeEnum::kLow:
        SetAttributeValue(mPercentSetting, DataModel::MakeNullable<chip::Percent>(33), PercentSetting::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedSetting, DataModel::MakeNullable<uint8_t>(1), SpeedSetting::Id);
        }
        SetAttributeValue(mPercentCurrent, chip::Percent{ 33 }, PercentCurrent::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedCurrent, static_cast<uint8_t>(1), SpeedCurrent::Id);
        }
        break;

    case FanModeEnum::kMedium: {
        const uint8_t speedSetting = (mSpeedMax > 1) ? static_cast<uint8_t>((mSpeedMax + 1) / 2) : 1;

        SetAttributeValue(mPercentSetting, DataModel::MakeNullable<chip::Percent>(66), PercentSetting::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(speedSetting), SpeedSetting::Id);
        }
        SetAttributeValue(mPercentCurrent, chip::Percent{ 66 }, PercentCurrent::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedCurrent, speedSetting, SpeedCurrent::Id);
        }
        break;
    }

    case FanModeEnum::kHigh:
        SetAttributeValue(mPercentSetting, DataModel::MakeNullable<chip::Percent>(100), PercentSetting::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(mSpeedMax), SpeedSetting::Id);
        }
        SetAttributeValue(mPercentCurrent, chip::Percent{ 100 }, PercentCurrent::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedCurrent, mSpeedMax, SpeedCurrent::Id);
        }
        break;

    case FanModeEnum::kAuto:
        if (!mPercentSetting.IsNull())
        {
            SetAttributeValue(mPercentCurrent, mPercentSetting.Value(), PercentCurrent::Id);
        }
        SetAttributeValue(mPercentSetting, DataModel::Nullable<chip::Percent>{ DataModel::NullNullable }, PercentSetting::Id);

        if (SupportsMultiSpeed())
        {
            if (!mSpeedSetting.IsNull())
            {
                SetAttributeValue(mSpeedCurrent, mSpeedSetting.Value(), SpeedCurrent::Id);
            }
            SetAttributeValue(mSpeedSetting, DataModel::Nullable<uint8_t>{ DataModel::NullNullable }, SpeedSetting::Id);
        }
        break;

    default:
        break;
    }
}

namespace {

FanModeEnum ComputeFanModeFromPercent(chip::Percent percent, FanModeSequenceEnum fanModeSequence)
{
    if (percent == 0)
    {
        return FanModeEnum::kOff;
    }

    const bool hasThreeSpeeds =
        (fanModeSequence == FanModeSequenceEnum::kOffLowMedHigh || fanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto);
    const bool hasLow = (fanModeSequence != FanModeSequenceEnum::kOffHigh && fanModeSequence != FanModeSequenceEnum::kOffHighAuto);

    if (hasThreeSpeeds)
    {
        if (percent <= 33)
        {
            return FanModeEnum::kLow;
        }
        if (percent <= 66)
        {
            return FanModeEnum::kMedium;
        }
        return FanModeEnum::kHigh;
    }
    if (hasLow)
    {
        return (percent <= 50) ? FanModeEnum::kLow : FanModeEnum::kHigh;
    }
    return FanModeEnum::kHigh;
}

} // namespace

void FanControlCluster::ApplyPercentSettingChanged()
{
    if (mPercentSetting.IsNull())
    {
        return;
    }

    if (mPercentSetting.Value() == 0)
    {
        CommitFanModeOffState();
        return;
    }

    // Apply MultiSpeed before FanMode so delegates that react to FanMode (and read SpeedSetting)
    // observe a consistent pair; otherwise SpeedSetting can still be stale from the prior mode.
    if (SupportsMultiSpeed())
    {
        uint16_t percent     = mPercentSetting.Value();
        uint8_t speedSetting = static_cast<uint8_t>((mSpeedMax * percent + 99) / 100);
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(speedSetting), SpeedSetting::Id);
        SetAttributeValue(mSpeedCurrent, speedSetting, SpeedCurrent::Id);
    }

    FanModeEnum newMode = ComputeFanModeFromPercent(mPercentSetting.Value(), mFanModeSequence);
    if (SetAttributeValue(mFanMode, newMode, FanMode::Id))
    {
        StoreFanModePersistence();
    }

    SetAttributeValue(mPercentCurrent, mPercentSetting.Value(), PercentCurrent::Id);
}

void FanControlCluster::ApplySpeedSettingChanged()
{
    if (!SupportsMultiSpeed() || mSpeedSetting.IsNull())
    {
        return;
    }

    if (mSpeedSetting.Value() == 0)
    {
        CommitFanModeOffState();
        return;
    }

    if (mSpeedMax == 0)
    {
        ChipLogError(Zcl, "FanControlCluster: mSpeedMax is 0; cannot compute PercentSetting");
        return;
    }
    uint8_t speedSetting  = mSpeedSetting.Value();
    chip::Percent percent = static_cast<chip::Percent>((speedSetting * 100) / mSpeedMax);
    SetAttributeValue(mPercentSetting, DataModel::MakeNullable(percent), PercentSetting::Id);

    FanModeEnum newMode = ComputeFanModeFromPercent(percent, mFanModeSequence);
    if (SetAttributeValue(mFanMode, newMode, FanMode::Id))
    {
        StoreFanModePersistence();
    }

    SetAttributeValue(mPercentCurrent, percent, PercentCurrent::Id);
    SetAttributeValue(mSpeedCurrent, speedSetting, SpeedCurrent::Id);
}

DataModel::ActionReturnStatus FanControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(FanControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case FanMode::Id:
        return encoder.Encode(mFanMode);
    case FanModeSequence::Id:
        return encoder.Encode(mFanModeSequence);
    case PercentSetting::Id:
        return encoder.Encode(mPercentSetting);
    case PercentCurrent::Id:
        return encoder.Encode(mPercentCurrent);
    case SpeedMax::Id:
        return encoder.Encode(mSpeedMax);
    case SpeedSetting::Id:
        return encoder.Encode(mSpeedSetting);
    case SpeedCurrent::Id:
        return encoder.Encode(mSpeedCurrent);
    case RockSupport::Id:
        return encoder.Encode(mRockSupport);
    case RockSetting::Id:
        return encoder.Encode(mRockSetting);
    case WindSupport::Id:
        return encoder.Encode(mWindSupport);
    case WindSetting::Id:
        return encoder.Encode(mWindSetting);
    case AirflowDirection::Id:
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
        Commands::Step::DecodableType commandData;
        VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        if (EnsureKnownEnumValue(commandData.direction) == StepDirectionEnum::kUnknownEnumValue)
        {
            return Status::ConstraintError;
        }

        bool wrapValue      = commandData.wrap.ValueOr(false);
        bool lowestOffValue = commandData.lowestOff.ValueOr(true);

        if (mDelegate == nullptr)
        {
            return Status::Failure;
        }
        return mDelegate->HandleStep(commandData.direction, wrapValue, lowestOffValue);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

bool FanControlCluster::IsFanModeSupportedBySequence(FanModeEnum value) const
{
    if (value == FanModeEnum::kLow &&
        (mFanModeSequence == FanModeSequenceEnum::kOffHighAuto || mFanModeSequence == FanModeSequenceEnum::kOffHigh))
    {
        return false;
    }
    if (value == FanModeEnum::kMedium &&
        !(mFanModeSequence == FanModeSequenceEnum::kOffLowMedHigh || mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto))
    {
        return false;
    }
    if (value == FanModeEnum::kAuto && !SupportsAuto())
    {
        return false;
    }
    return true;
}

DataModel::ActionReturnStatus FanControlCluster::SetFanMode(FanModeEnum value)
{
    if (EnsureKnownEnumValue(value) == FanModeEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }
    if (!IsFanModeSupportedBySequence(value))
    {
        // TODO: Add spec-compliant ConstraintError for unsupported FanMode vs FanModeSequence once
        // REPL tests in matter-test-scripts are updated. https://github.com/project-chip/matter-test-scripts/issues/780
        return Status::InvalidInState;
    }

    FanModeEnum newMode = value;

    if (value == FanModeEnum::kOn)
    {
        newMode = FanModeEnum::kHigh;
    }
    else if (value == FanModeEnum::kSmart)
    {
        if (SupportsAuto())
        {
            newMode = FanModeEnum::kAuto;
        }
        else
        {
            newMode = FanModeEnum::kHigh;
        }
    }

    if (!SetAttributeValue(mFanMode, newMode, FanMode::Id))
    {
        return Status::Success;
    }

    ApplyFanModeSideEffects(newMode);

    StoreFanModePersistence();

    NotifyDelegateFanDriveState();

    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetPercentSetting(DataModel::Nullable<chip::Percent> value)
{
    if (value.IsNull())
    {
        if (mFanMode != FanModeEnum::kAuto)
        {
            return Status::InvalidInState;
        }
        return Status::Success;
    }

    if (value.Value() > 100)
    {
        return Status::ConstraintError;
    }

    if (!SetAttributeValue(mPercentSetting, value, PercentSetting::Id))
    {
        return Status::Success;
    }

    ApplyPercentSettingChanged();
    NotifyDelegateFanDriveState();
    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetSpeedSetting(DataModel::Nullable<uint8_t> value)
{
    if (value.IsNull())
    {
        // Null is only valid in Auto mode (same rule as PercentSetting; see TestFanControl.yaml).
        if (mFanMode != FanModeEnum::kAuto)
        {
            return Status::InvalidInState;
        }
        return Status::Success;
    }

    if (value.Value() > mSpeedMax)
    {
        return Status::ConstraintError;
    }

    if (!SetAttributeValue(mSpeedSetting, value, SpeedSetting::Id))
    {
        return Status::Success;
    }

    ApplySpeedSettingChanged();
    NotifyDelegateFanDriveState();
    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetRockSetting(BitMask<RockBitmap> value)
{
    uint8_t rawValue   = value.Raw();
    uint8_t rawSupport = mRockSupport.Raw();
    if ((rawValue & rawSupport) != rawValue)
    {
        return Status::ConstraintError;
    }

    if (!SetAttributeValue(mRockSetting, value, RockSetting::Id))
    {
        return Status::Success;
    }
    if (mDelegate != nullptr)
    {
        mDelegate->OnRockSettingChanged(mRockSetting);
    }
    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetWindSetting(BitMask<WindBitmap> value)
{
    uint8_t rawValue   = value.Raw();
    uint8_t rawSupport = mWindSupport.Raw();
    if ((rawValue & rawSupport) != rawValue)
    {
        return Status::ConstraintError;
    }

    if (!SetAttributeValue(mWindSetting, value, WindSetting::Id))
    {
        return Status::Success;
    }
    if (mDelegate != nullptr)
    {
        mDelegate->OnWindSettingChanged(mWindSetting);
    }
    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetAirflowDirection(AirflowDirectionEnum value)
{
    if (EnsureKnownEnumValue(value) == AirflowDirectionEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }

    if (!SetAttributeValue(mAirflowDirection, value, AirflowDirection::Id))
    {
        return Status::Success;
    }
    if (mDelegate != nullptr)
    {
        mDelegate->OnAirflowDirectionChanged(mAirflowDirection);
    }
    return Status::Success;
}

bool FanControlCluster::SetPercentCurrent(chip::Percent value)
{
    return SetAttributeValue(mPercentCurrent, value, PercentCurrent::Id);
}

bool FanControlCluster::SetSpeedCurrent(uint8_t value)
{
    if (!SupportsMultiSpeed())
    {
        return false;
    }
    return SetAttributeValue(mSpeedCurrent, value, SpeedCurrent::Id);
}

void FanControlCluster::StoreFanModePersistence()
{
    VerifyOrReturn(mContext != nullptr);
    const FanModeEnum value = mFanMode;
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, FanControl::Id, FanMode::Id),
                                                            ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
}

void FanControlCluster::SetDelegate(FanControl::Delegate * delegate)
{
    mDelegate = delegate;
}

} // namespace chip::app::Clusters
