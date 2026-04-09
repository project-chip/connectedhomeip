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
    mSupportsStep(config.mSupportsStep), mSpeedMax(config.mSpeedMax), mRockSupport(config.mRockSupport),
    mWindSupport(config.mWindSupport), mOptionalAttributes(config.mOptionalAttributes), mFeatureMap(config.mFeatureMap),
    mDelegate(config.mDelegate)
{}

void FanControlCluster::NotifyDelegateFanDriveState()
{
    if (mDelegate == nullptr)
    {
        return;
    }
    mDelegate->OnFanModeChanged(mFanMode);
    mDelegate->OnPercentSettingChanged(mPercentSetting);
    mDelegate->OnPercentCurrentChanged(mPercentCurrent);
    if (SupportsMultiSpeed())
    {
        mDelegate->OnSpeedSettingChanged(mSpeedSetting);
        mDelegate->OnSpeedCurrentChanged(mSpeedCurrent);
    }
}

CHIP_ERROR FanControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attrPersistence{ context.attributeStorage };
    FanModeEnum restoredFanMode = mFanMode;
    attrPersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, FanControl::Id, FanMode::Id), restoredFanMode,
                                          mFanMode);

    if (EnsureKnownEnumValue(restoredFanMode) == FanModeEnum::kUnknownEnumValue)
    {
        restoredFanMode = mFanMode;
    }

    DataModel::ActionReturnStatus status = SetFanMode(restoredFanMode, /* syncOnOffDelegate = */ false);
    if (!status.IsSuccess())
    {
        status = SetFanMode(FanModeEnum::kOff, /* syncOnOffDelegate = */ false);
    }
    VerifyOrReturnError(status.IsSuccess(), CHIP_ERROR_INTERNAL);

    if (mDelegate != nullptr)
    {
        mDelegate->OnPersistenceRestored();
    }

    return CHIP_NO_ERROR;
}

void FanControlCluster::SetFanModeToOff()
{
    VerifyOrReturn(SetAttributeValue(mFanMode, FanModeEnum::kOff, FanMode::Id)); // noop if already off
    ApplyFanModeOffSideEffects();
    StoreFanModePersistence();
    NotifyDelegateFanDriveState();
    if (mDelegate != nullptr)
    {
        mDelegate->OnFanStateChanged(false);
    }
}

void FanControlCluster::ApplyFanModeOffSideEffects()
{
    SetAttributeValue(mPercentSetting, DataModel::MakeNullable(static_cast<chip::Percent>(0)), PercentSetting::Id);
    SetAttributeValue(mPercentCurrent, static_cast<chip::Percent>(0), PercentCurrent::Id);

    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(static_cast<uint8_t>(0)), SpeedSetting::Id);
        SetAttributeValue(mSpeedCurrent, static_cast<uint8_t>(0), SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeLowSideEffects()
{
    SetAttributeValue(mPercentSetting, DataModel::MakeNullable(static_cast<chip::Percent>(33)), PercentSetting::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(static_cast<uint8_t>(1)), SpeedSetting::Id);
    }
    VerifyOrReturn(mIsOnOffOn);
    SetAttributeValue(mPercentCurrent, static_cast<chip::Percent>(33), PercentCurrent::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedCurrent, static_cast<uint8_t>(1), SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeMediumSideEffects()
{
    const uint8_t speedSetting = (mSpeedMax > 1) ? static_cast<uint8_t>((mSpeedMax + 1) / 2) : 1;

    SetAttributeValue(mPercentSetting, DataModel::MakeNullable(static_cast<chip::Percent>(66)), PercentSetting::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(speedSetting), SpeedSetting::Id);
    }
    VerifyOrReturn(mIsOnOffOn);
    SetAttributeValue(mPercentCurrent, static_cast<chip::Percent>(66), PercentCurrent::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedCurrent, speedSetting, SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeHighSideEffects()
{
    SetAttributeValue(mPercentSetting, DataModel::MakeNullable(static_cast<chip::Percent>(100)), PercentSetting::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(mSpeedMax), SpeedSetting::Id);
    }
    VerifyOrReturn(mIsOnOffOn);
    SetAttributeValue(mPercentCurrent, static_cast<chip::Percent>(100), PercentCurrent::Id);
    if (SupportsMultiSpeed())
    {
        SetAttributeValue(mSpeedCurrent, mSpeedMax, SpeedCurrent::Id);
    }
}

void FanControlCluster::ApplyFanModeAutoSideEffects()
{
    if (mIsOnOffOn && !mPercentSetting.IsNull())
    {
        SetAttributeValue(mPercentCurrent, mPercentSetting.Value(), PercentCurrent::Id);
    }
    SetAttributeValue(mPercentSetting, DataModel::Nullable<chip::Percent>{ DataModel::NullNullable }, PercentSetting::Id);

    if (SupportsMultiSpeed())
    {
        if (mIsOnOffOn && !mSpeedSetting.IsNull())
        {
            SetAttributeValue(mSpeedCurrent, mSpeedSetting.Value(), SpeedCurrent::Id);
        }
        SetAttributeValue(mSpeedSetting, DataModel::Nullable<uint8_t>{ DataModel::NullNullable }, SpeedSetting::Id);
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
        SetFanModeToOff();
        return;
    }

    FanModeEnum newMode = ComputeFanModeFromPercent(mPercentSetting.Value(), mFanModeSequence);
    if (SetAttributeValue(mFanMode, newMode, FanMode::Id))
    {
        StoreFanModePersistence();
    }

    if (SupportsMultiSpeed())
    {
        uint16_t percent     = mPercentSetting.Value();
        uint8_t speedSetting = static_cast<uint8_t>((mSpeedMax * percent + 99) / 100);
        SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(speedSetting), SpeedSetting::Id);
        if (mIsOnOffOn)
        {
            SetAttributeValue(mSpeedCurrent, speedSetting, SpeedCurrent::Id);
        }
    }

    VerifyOrReturn(mIsOnOffOn);

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
        SetFanModeToOff();
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

    VerifyOrReturn(mIsOnOffOn);

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
        bool lowestOffValue = commandData.lowestOff.ValueOr(false);

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

DataModel::ActionReturnStatus FanControlCluster::SetFanMode(FanModeEnum value, bool syncOnOffDelegate)
{
    if (EnsureKnownEnumValue(value) == FanModeEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }
    if (value == FanModeEnum::kLow &&
        (mFanModeSequence == FanModeSequenceEnum::kOffHighAuto || mFanModeSequence == FanModeSequenceEnum::kOffHigh))
    {
        return Status::InvalidInState;
    }
    if (value == FanModeEnum::kMedium &&
        !(mFanModeSequence == FanModeSequenceEnum::kOffLowMedHigh || mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto))
    {
        return Status::InvalidInState;
    }
    if (value == FanModeEnum::kAuto && !SupportsAuto())
    {
        return Status::InvalidInState;
    }

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

    SetAttributeValue(mFanMode, newMode, FanMode::Id);

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

    StoreFanModePersistence();

    NotifyDelegateFanDriveState();

    // Sync OnOff cluster: always notify on turn-off; only notify on turn-on when OnOff is
    // already on. If OnOff is off, changing the fan mode should not implicitly turn the
    // device on — the user must explicitly turn it on via the OnOff cluster.
    if (syncOnOffDelegate && mDelegate != nullptr && (newMode == FanModeEnum::kOff || mIsOnOffOn))
    {
        mDelegate->OnFanStateChanged(newMode != FanModeEnum::kOff);
    }

    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetPercentSetting(DataModel::Nullable<chip::Percent> value)
{
    if (value.IsNull())
    {
        return Status::InvalidInState;
    }

    if (value.Value() > 100)
    {
        return Status::ConstraintError;
    }

    SetAttributeValue(mPercentSetting, value, PercentSetting::Id);
    ApplyPercentSettingChanged();
    if (!mIsOnOffOn)
    {
        NotifyAttributeChanged(PercentCurrent::Id);
    }
    NotifyDelegateFanDriveState();
    return Status::Success;
}

DataModel::ActionReturnStatus FanControlCluster::SetSpeedSetting(DataModel::Nullable<uint8_t> value)
{
    if (value.IsNull())
    {
        return Status::InvalidInState;
    }

    if (value.Value() > mSpeedMax)
    {
        return Status::ConstraintError;
    }

    SetAttributeValue(mSpeedSetting, value, SpeedSetting::Id);
    ApplySpeedSettingChanged();
    if (!mIsOnOffOn)
    {
        NotifyAttributeChanged(SpeedCurrent::Id);
    }
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

void FanControlCluster::SetOnOffState(bool isOn)
{
    mIsOnOffOn = isOn;
    if (!isOn)
    {
        SetAttributeValue(mPercentCurrent, static_cast<chip::Percent>(0), PercentCurrent::Id);
        if (SupportsMultiSpeed())
        {
            SetAttributeValue(mSpeedCurrent, static_cast<uint8_t>(0), SpeedCurrent::Id);
        }
    }
    else
    {
        if (!mPercentSetting.IsNull() && mPercentSetting.Value() == 0)
        {
            SetAttributeValue(mPercentSetting, DataModel::MakeNullable(static_cast<chip::Percent>(100)), PercentSetting::Id);
        }

        SetAttributeValue(mPercentCurrent, mPercentSetting.ValueOr(100), PercentCurrent::Id);

        if (SupportsMultiSpeed())
        {
            if (!mSpeedSetting.IsNull() && mSpeedSetting.Value() == 0)
            {
                SetAttributeValue(mSpeedSetting, DataModel::MakeNullable(mSpeedMax), SpeedSetting::Id);
            }
            SetAttributeValue(mSpeedCurrent, mSpeedSetting.ValueOr(mSpeedMax), SpeedCurrent::Id);
        }

        if (mFanMode == FanModeEnum::kOff)
        {
            const FanModeEnum newMode = mPercentSetting.IsNull() ? (SupportsAuto() ? FanModeEnum::kAuto : FanModeEnum::kHigh)
                                                                 : ComputeFanModeFromPercent(mPercentCurrent, mFanModeSequence);
            if (SetAttributeValue(mFanMode, newMode, FanMode::Id))
            {
                StoreFanModePersistence();
            }
        }
    }

    NotifyDelegateFanDriveState();
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
