/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Fan Control Server Cluster (Code-Driven)
 ***************************************************************************/

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/FanControl/Attributes.h>
#include <lib/support/TypeTraits.h>
#include <clusters/FanControl/Commands.h>
#include <clusters/FanControl/Enums.h>
#include <clusters/FanControl/Metadata.h>
#include <lib/support/CodeUtils.h>
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
    DefaultServerCluster({ config.mEndpointId, FanControl::Id }),
    mFanModeSequence(config.mFanModeSequence), mSupportsStep(config.mSupportsStep), mSpeedMax(config.mSpeedMax),
    mRockSupport(config.mRockSupport), mWindSupport(config.mWindSupport), mOptionalAttributes(config.mOptionalAttributes),
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

void FanControlCluster::ApplyFanModeAutoSideEffects()
{
    // Spec 4.4.6.1.2: PercentSetting and SpeedSetting set to null.
    // PercentCurrent and SpeedCurrent SHALL continue to indicate the current state (not modified here).
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
        // Spec 4.4.6.3.1: speed = ceil(SpeedMax * (percent * 0.01))
        uint8_t speedMax   = mSpeedMax;
        uint16_t percent   = mPercentSetting.Value();
        uint8_t speedSetting = static_cast<uint8_t>((speedMax * percent + 99) / 100);
        mSpeedWriteInProgress = true;
        mSpeedSetting.SetNonNull(speedSetting);
        mSpeedWriteInProgress = false;
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

    // Spec 4.4.6.6.1: percent = floor(speed/SpeedMax * 100)
    uint8_t speedMax     = mSpeedMax;
    float speed          = static_cast<float>(mSpeedSetting.Value());
    chip::Percent percentSetting = static_cast<chip::Percent>(speed / speedMax * 100);
    mPercentWriteInProgress = true;
    mPercentSetting.SetNonNull(percentSetting);
    mPercentWriteInProgress = false;
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
        return encoder.Encode<uint32_t>(0);
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

        if (mFanModeWriteInProgress)
            return Status::WriteIgnored;

        uint8_t seq = chip::to_underlying(mFanModeSequence);
        if (value == FanModeEnum::kLow && seq >= 4)
            return Status::ConstraintError;
        if (value == FanModeEnum::kMedium && seq != 0 && seq != 2)
            return Status::ConstraintError;
        if (value == FanModeEnum::kAuto && !SupportsAuto())
            return Status::ConstraintError;

        if (value == FanModeEnum::kOn)
        {
            mFanMode = FanModeEnum::kHigh;
            return NotifyAttributeChangedIfSuccess(FanMode::Id, Status::Success);
        }
        if (value == FanModeEnum::kSmart)
        {
            if (SupportsAuto() &&
                (mFanModeSequence == FanModeSequenceEnum::kOffLowHighAuto ||
                 mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto))
            {
                mFanMode = FanModeEnum::kAuto;
            }
            else
            {
                mFanMode = FanModeEnum::kHigh;
            }
            return NotifyAttributeChangedIfSuccess(FanMode::Id, Status::Success);
        }

        mFanMode = value;
        if (value == FanModeEnum::kOff)
            ApplyFanModeOffSideEffects();
        else if (value == FanModeEnum::kAuto)
            ApplyFanModeAutoSideEffects();

        return NotifyAttributeChangedIfSuccess(FanMode::Id, Status::Success);
    }
    case PercentSetting::Id: {
        DataModel::Nullable<chip::Percent> value;
        ReturnErrorOnFailure(decoder.Decode(value));

        if (mPercentWriteInProgress)
            return Status::WriteIgnored;

        if (value.IsNull())
            return Status::Success; // Spec: "If the client writes null, the attribute value SHALL NOT change"

        mPercentSetting = value;
        ApplyPercentSettingChanged();
        return NotifyAttributeChangedIfSuccess(PercentSetting::Id, Status::Success);
    }
    case SpeedSetting::Id: {
        if (!SupportsMultiSpeed())
            return Status::UnsupportedAttribute;
        if (mSpeedWriteInProgress)
            return Status::WriteIgnored;

        DataModel::Nullable<uint8_t> value;
        ReturnErrorOnFailure(decoder.Decode(value));

        if (value.IsNull())
            return Status::Success; // Spec: "If the client writes null, the attribute value SHALL NOT change"

        if (value.Value() > mSpeedMax)
            return Status::ConstraintError;

        mSpeedSetting = value;
        ApplySpeedSettingChanged();
        return NotifyAttributeChangedIfSuccess(SpeedSetting::Id, Status::Success);
    }
    case RockSetting::Id: {
        if (!SupportsRocking())
            return Status::UnsupportedAttribute;
        BitMask<RockBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        uint8_t rawValue   = value.Raw();
        uint8_t rawSupport = mRockSupport.Raw();
        if ((rawValue & rawSupport) != rawValue)
            return Status::ConstraintError;
        if (rawValue != 0)
        {
            uint8_t lowest = 0;
            for (uint8_t mask = 0x01; mask != 0; mask <<= 1)
            {
                if ((rawValue & mask) && (rawSupport & mask))
                {
                    lowest = mask;
                    break;
                }
            }
            mRockSetting = BitMask<RockBitmap>(lowest);
        }
        else
        {
            mRockSetting = value;
        }
        return NotifyAttributeChangedIfSuccess(RockSetting::Id, Status::Success);
    }
    case WindSetting::Id: {
        if (!SupportsWind())
            return Status::UnsupportedAttribute;
        BitMask<WindBitmap> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        uint8_t rawValue   = value.Raw();
        uint8_t rawSupport = mWindSupport.Raw();
        if ((rawValue & rawSupport) != rawValue)
            return Status::ConstraintError;
        if (rawValue != 0)
        {
            uint8_t lowest = 0;
            for (uint8_t mask = 0x01; mask != 0; mask <<= 1)
            {
                if ((rawValue & mask) && (rawSupport & mask))
                {
                    lowest = mask;
                    break;
                }
            }
            mWindSetting = BitMask<WindBitmap>(lowest);
        }
        else
        {
            mWindSetting = value;
        }
        return NotifyAttributeChangedIfSuccess(WindSetting::Id, Status::Success);
    }
    case AirflowDirection::Id: {
        if (!SupportsAirflowDirection())
            return Status::UnsupportedAttribute;
        AirflowDirectionEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        mAirflowDirection = value;
        return NotifyAttributeChangedIfSuccess(AirflowDirection::Id, Status::Success);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR FanControlCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry = AttributeListBuilder::OptionalAttributeEntry;
    OptionalEntry optionalAttributes[] = {
        { SupportsMultiSpeed(), SpeedMax::kMetadataEntry },
        { SupportsMultiSpeed(), SpeedSetting::kMetadataEntry },
        { SupportsMultiSpeed(), SpeedCurrent::kMetadataEntry },
        { SupportsRocking(), RockSupport::kMetadataEntry },
        { SupportsRocking(), RockSetting::kMetadataEntry },
        { SupportsWind(), WindSupport::kMetadataEntry },
        { SupportsWind(), WindSetting::kMetadataEntry },
        { SupportsAirflowDirection(), AirflowDirection::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(FanControl::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR FanControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (SupportsStep())
    {
        DataModel::AcceptedCommandEntry entry = Commands::Step::kMetadataEntry;
        return builder.Append(entry);
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

        return mDelegate.HandleStep(commandData.direction, wrapValue, lowestOffValue);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

} // namespace chip::app::Clusters
