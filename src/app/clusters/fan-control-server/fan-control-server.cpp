/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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
 * @brief Implementation for the Fan Control Server Cluster
 ***************************************************************************/

#include <assert.h>
#include <math.h>

#include "fan-control.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::FanControl::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

Instance::Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint32_t aFeature, Delegate * aDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), FanControl::Id),
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), FanControl::Id), mEndpointId(aEndpointId), mFeatureMap(aFeature),
    mFanModeSequence(aFanModeSequence), mDelegate(aDelegate)
{
    Init();
}

Instance::Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint8_t aSpeedMax, uint32_t aFeature,
                   Delegate * aDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), FanControl::Id),
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), FanControl::Id), mEndpointId(aEndpointId), mFeatureMap(aFeature),
    mFanModeSequence(aFanModeSequence), mSpeedMax(aSpeedMax), mDelegate(aDelegate)
{
    Init();
}

Instance::Instance(EndpointId aEndpointId, FanModeSequenceEnum aFanModeSequence, uint8_t aSpeedMax, RockBitmap aRockSupport,
                   WindBitmap aWindSupport, uint32_t aFeature, Delegate * aDelegate) :
    CommandHandlerInterface(Optional<EndpointId>(aEndpointId), FanControl::Id),
    AttributeAccessInterface(Optional<EndpointId>(aEndpointId), FanControl::Id), mEndpointId(aEndpointId), mFeatureMap(aFeature),
    mFanModeSequence(aFanModeSequence), mSpeedMax(aSpeedMax), mRockSupport(aRockSupport), mWindSupport(aWindSupport),
    mDelegate(aDelegate)
{
    Init();
}

CHIP_ERROR Instance::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, FanControl::Id))
    {
        ChipLogError(Zcl, "FanControl: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        mDelegate->SetParent(this);

        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
        VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
        loadPersistentAttributes(); // TODO

        // TODO Delegate Init()

        if (mFanMode == FanModeEnum::kOff)
        {
            // TODO: Set up other attributes and delegate callback
        }
        else if (mFanMode == FanModeEnum::kAuto)
        {
            // TODO: Set up other attributes and delegate callback
        }
        else
        {
            // TODO: Call FanMode callback
        }
    }

    return err;
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case FanMode::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFanMode));
        break;

    case FanModeSequence::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFanModeSequence));
        break;

    case PercentSetting::Id:
        if (mPercentSetting.IsNull())
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(mPercentSetting.Value()));
        }
        break;

    case PercentCurrent::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mPercentCurrent));
        break;

    case SpeedMax::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSpeedMax));
        break;

    case SpeedSetting::Id:
        if
        {
            mSpeedSetting.IsNull()
        }
        {
            ReturnErrorOnFailure(aEncoder.EncodeNull());
        }
        else
        {
            ReturnErrorOnFailure(aEncoder.Encode(mSpeedSetting.Value()));
        }
        break;

    case SpeedCurrent::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSpeedCurrent));
        break;

    case RockSupport::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mRockSupport));
        break;

    case RockSetting::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mRockSetting));
        break;

    case WindSupport::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mWindSupport));
        break;

    case WindSetting::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mWindSetting));
        break;

    case AirflowDirection::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mAirflowDirection));
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & attributePath, AttributeValueDecoder & aDecoder)
{
    Status status;

    switch (attributePath.mAttributeId)
    {
    case FanMode::Id:
        status = handleFanModeWrite(aDecoder);
        return StatusIB(status).ToChipError();

    case PercentSetting::Id:
        status = handlePercentSettingWrite(aDecoder);
        return StatusIB(status).ToChipError();

    case SpeedSetting::Id:
        if (SupportsMultiSpeed())
        {
            status = handleSpeedSettingWrite(aDecoder);
        }
        else
        {
            status = Status::UnsupportedAttribute;
        }

        return StatusIB(status).ToChipError();

    case RockSetting::Id:
        if (SupportsRock())
        {
            status = handleRockSettingWrite(aDecoder);
        }
        else
        {
            status = Status::UnsupportedAttribute;
        }

        return StatusIB(status).ToChipError();

    case WindSetting::Id:
        if (SupportsWind())
        {
            status = handleWindSettingWrite(aDecoder);
        }
        else
        {
            status = Status::UnsupportedAttribute;
        }

        return StatusIB(status).ToChipError();

    case AirflowDirection::Id:
        if (SupportsAirflowDirection())
        {
            status = handleAirflowDirectionWrite(aDecoder);
        }
        else
        {
            status = Status::UnsupportedAttribute;
        }

        return StatusIB(status).ToChipError();
    }
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::Step::Id:
        ChipLogDetail(Zcl, "FanControl: Handling Step Command");

        HandleCommand<Commands::Step::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { handleStep(ctx, commandData); });
    }
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    if (SupportsStep())
    {
        callback(FanControl::Commands::Step::Id, context);
    }
    return CHIP_NO_ERROR;
}

Status chip::app::Clusters::FanControl::Instance::UpdateFanMode(FanModeEnum fanMode)
{
    if (fanMode == FanModeEnum::kOn)
    {
        ChipLogProgress(Zcl, "FanControl: Fan Mode On deprecated");
        fanMode = FanModeEnum::kHigh;
    }
    else if (fanMode == FanModeEnum::kSmart)
    {
        ChipLogProgress(Zcl, "FanControl: Fan Mode Smart deprecated");
        if (SupportsAuto(attributePath.mEndpointId) &&
            ((mFanModeSequence == FanModeSequenceEnum::kOffLowHighAuto) ||
             (mFanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto)))
        {
            fanMode = FanModeEnum::kAuto;
        }
        else
        {
            fanMode = FanModeEnum::kHigh;
        }
    }

    if (mFanMode != fanMode)
    {
        mFanMode = fanMode;
        ChipLogProgress(Zcl, "FanControl: fanMode change: %" PRIu8 "", fanMode);

        // TODO: Write Value to Persistent Storage
        MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::FanMode::Id);
    }

    return Status::Success;
}

Status Instance::UpdatePercentSetting(DataModel::Nullable<Percent> percentSetting)
{
    Status status;

    if (percentSetting.IsNull())
    {
        status = Status::WriteIgnored;
    }
    else
    {
        if (percentSetting > mPercentMax)
        {
            status = Status::ConstraintError;
        }
        else
        {
            if (mPercentSetting != percentSetting)
            {
                mPercentSetting = percentSetting;
                ChipLogProgress(Zcl, "FanControl: percentSetting change: %" PRIu8 "", percentSetting);
                MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::PercentSetting::Id);
            }
            status = Status::Success;
        }
    }
    return status;
}

Status Instance::UpdatePercentCurrent(Percent percentCurrent)
{
    Status status;

    if (percentCurrent > mPercentMax)
    {
        status = Status::ConstraintError;
    }
    else
    {
        if (percentCurrent > mPercentMax)
        {
            status = Status::ConstraintError;
        }
        else
        {
            if (mPercentCurrent != percentCurrent)
            {
                mPercentCurrent = percentCurrent;
                ChipLogProgress(Zcl, "FanControl: percentCurrent change: %" PRIu8 "", percentCurrent);
                MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::PercentCurrent::Id);
            }
            status = Status::Success;
        }
    }
    return status;
}

Status Instance::UpdateSpeedSetting(DataModel::Nullable<uint8_t> speedSetting)
{
    Status status;

    if (speedSetting.IsNull())
    {
        status = Status::WriteIgnored;
    }
    else
    {
        if (speedSetting > mSpeedMax)
        {
            status = Status::ConstraintError;
        }
        else
        {
            mSpeedSetting = speedSetting;
            status        = Status::Success;
            ChipLogProgress(Zcl, "FanControl: speedSetting change: %" PRIu8 "", speedSetting);
            MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::SpeedSetting::Id);
        }
    }

    return status;
}

Status Instance::UpdateSpeedCurrent(uint8_t speedCurrent)
{
    Status status;

    if (speedCurrent > mSpeedMax)
    {
        status = Status::ConstraintError;
    }
    else
    {
        mSpeedCurrent = speedCurrent;
        status        = Status::Success;
        ChipLogProgress(Zcl, "FanControl: speedCurrent change: %" PRIu8 "", speedCurrent);
        MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::SpeedCurrent::Id);
    }

    return status;
}

Status Instance::UpdateRockSetting(RockBitmap rockSetting)
{
    Status status;
    if (rockSetting.Raw() & mRockSupport.Raw() == rockSetting.Raw())
    {
        if (mRockSetting != rockSetting)
        {
            mRockSetting = rockSetting;
            ChipLogProgress(Zcl, "FanControl: rockSetting change: %" PRIu8 "", rockSetting);
            MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::RockSetting::Id);
        }
        status = Status::Success;
    }
    else
    {
        status = Status::ConstraintError;
    }

    return status;
}

Status Instance::UpdateWindSetting(WindBitmap windSetting)
{
    Status status;
    if (windSetting.Raw() & mWindSupport.Raw() == windSetting.Raw())
    {
        if (mWindSetting != windSetting)
        {
            mWindSetting = windSetting;
            ChipLogProgress(Zcl, "FanControl: windSetting change: %" PRIu8 "", windSetting);
            MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::WindSetting::Id);
        }
        status = Status::Success;
    }
    else
    {
        status = Status::ConstraintError;
    }
}

Status Instance::UpdateAirflowDirection(AirflowDirectionEnum airflowDirection)
{
    if (mAirflowDirection != airflowDirection)
    {
        mAirflowDirection = airflowDirection;
        ChipLogProgress(Zcl, "FanControl: airflowDirection change: %" PRIu8 "", airflowDirection);
        MatterReportingAttributeChangeCallback(mEndpointId, FanControl::Id, Attributes::AirflowDirection::Id);
    }

    return Status::Success;
}

Status Instance::UpdatePercentageAndSpeedSetting(DataModel::Nullable<Percent> percentSetting)
{
    Status status;
    status = UpdatePercentSetting(percentSetting);
    if (status == Status::Success)
    {
        if ((status == Status::Success) && SupportsMultiSpeed())
        {
            status = UpdateSpeedSetting(getSpeedFromPercent(percentSetting.Value(), mSpeedMax));
        }
    }
    return status;
}

Status Instance::UpdatePercentageAndSpeedSetting(DataModel::Nullable<uint8_t> speedSetting)
{
    Status status;
    status = UpdateSpeedSetting(speedSetting);
    if (status == Status::Success)
    {
        if (status == Status::Success)
        {
            status = UpdatePercentSetting(getPercentFromSpeed(speedSetting.Value(), mSpeedMax));
        }
    }
    return status
}

Status Instance::UpdatePercentageAndSpeedCurrent(Percent percentCurrent)
{
    Status status;
    status = UpdatePercentCurrent(percentCurrent);
    if (status == Status::Success)
    {
        if ((status == Status::Success) && SupportsMultiSpeed())
        {
            status = UpdateSpeedCurrent(getSpeedFromPercent(percentCurrent.Value(), mSpeedMax));
        }
    }
    return status;
}

Status Instance::UpdatePercentageAndSpeedCurrent(uint8_t speedCurrent)
{
    Status status;
    status = UpdateSpeedCurrent(speedCurrent);
    if (status == Status::Success)
    {
        if (status == Status::Success)
        {
            status = UpdatePercentCurrent(getPercentFromSpeed(speedCurrent.Value(), mSpeedMax));
        }
    }
    return status
}

Status Instance::nullifyPercentSetting()
{
    mPercentSetting.SetNull();
}

Status Instance::nullifySpeedSetting()
{
    mSpeedSetting.SetNull();
}

Status Instance::handleFanModeWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    FanModeEnum fanMode;

    if (aDecoder.Decode(fanMode) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mFanMode != fanMode)
        {
            status = UpdateFanMode(fanMode);
            if (status == Status::Success)
            {
                // Setting the FanMode value to Off SHALL set the values of PercentSetting and
                // SpeedSetting attributes to 0 (zero).
                if (fanMode == FanModeEnum::kOff)
                {
                    status = UpdatePercentSetting(0);

                    if (status == Status::Success)
                    {
                        status = UpdateSpeedSetting(0);
                    }

                    if (status == Status::Success)
                    {
                        // Call into Delegate to handle updates
                        if (mDelegate->HandleFanModeOff(mPercentSetting, mSpeedSetting) != CHIP_NO_ERROR)
                        {
                            status = Status::Failure;
                        }
                    }
                }
                // Setting the FanMode value to Auto SHALL set the values of PercentSetting and
                // SpeedSetting attributes to null.
                else if (fanMode == FanModeEnum::kAuto)
                {
                    status = nullifyPercentSetting();

                    if (status == Status::Success)
                    {
                        status = nullifySpeedSetting();
                    }

                    if (status == Status::Success)
                    {
                        // Call into Delegate to handle updates
                        if (mDelegate->HandleFanModeAuto(mPercentSetting, mSpeedSetting) != CHIP_NO_ERROR)
                        {
                            status = Status::Failure;
                        }
                    }
                }
                else
                {
                    // Call into Delegate to handle updates
                    if (mDelegate->HandleFanMode(fanMode) != CHIP_NO_ERROR)
                    {
                        status = Status::Failure;
                    }
                }
            }
        }
    }

    return status;
}

Status Instance::handlePercentSettingWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    DataModel::DecodableType<Percent> percentSetting;

    if (aDecoder.Decode(percentSetting) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mPercentSetting != percentSetting)
        {
            status = UpdatePercentageAndSpeedSetting(percentSetting);

            // Call into Delegate to handle updates
            if ((status == Status::Success) &&
                (mDelegate->HandlePercentSpeedSettingChange(mPercentSetting, mSpeedSetting) != CHIP_NO_ERROR))
            {
                status = Status::Failure;
            }
        }
    }

    return status;
}

Status Instance::handleSpeedSettingWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    DataModel::DecodableType<uint8_t> speedSetting;

    if (aDecoder.Decode(speedSetting) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mSpeedSetting != speedSetting)
        {
            status = UpdatePercentageAndSpeedSetting(speedSetting);

            // Call into Delegate to handle updates
            if ((status == Status::Success) &&
                (mDelegate->HandlePercentSpeedSettingChange(mPercentSetting, mSpeedSetting) != CHIP_NO_ERROR))
            {
                status = Status::Failure;
            }
        }
    }

    return status;
}

Status Instance::handleRockSettingWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    DataModel::DecodableType<RockBitmap> rockSetting;

    if (aDecoder.Decode(rockSetting) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mRockSetting != rockSetting)
        {
            status = UpdateRockSetting(rockSetting);
            if (status == Status::Success)
            {
                // Call into Delegate to handle updates
                if (mDelegate->HandleRockSettingChange(mRockSetting) != CHIP_NO_ERROR)
                {
                    status = Status::Failure;
                }
            }
        }
    }

    return status;
}

Status Instance::handleWindSettingWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    DataModel::DecodableType<WindBitmap> windSetting;

    if (aDecoder.Decode(windSetting) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mWindSetting != windSetting)
        {
            status = UpdateWindSetting(windSetting);
            if (status == Status::Success)
            {
                // Call into Delegate to handle updates
                if (mDelegate->HandleWindSettingChange(mWindSetting) != CHIP_NO_ERROR)
                {
                    status = Status::Failure;
                }
            }
        }
    }

    return status;
}

Status Instance::handleAirflowDirectionWrite(AttributeValueDecoder & aDecoder)
{
    Status status = Status::Success;
    DataModel::DecodableType<AirflowDirectionEnum> airflowDirection;

    if (aDecoder.Decode(airflowDirection) != CHIP_NO_ERROR)
    {
        status = Status::InvalidValue;
    }
    else
    {
        if (mAirflowDirection != airflowDirection)
        {
            status = UpdateAirflowDirection(airflowDirection);
            if (status == Status::Success)
            {
                // Call into Delegate to handle updates
                if (mDelegate->HandleAirflowDirectionChange(mAirflowDirection) != CHIP_NO_ERROR)
                {
                    status = Status::Failure;
                }
            }
        }
    }

    return status;
}

void Instance::handleStep(HandlerContext & ctx, const Commands::Step::DecodableType & commandData)
{
    auto direction = commandData.direction;
    auto wrap      = commandData.wrap;
    auto lowestOff = commandData.lowestOff;

    Protocols::InteractionModel::Status status = Status::Success;

    if (!SupportsStep(commandPath.mEndpointId))
    {
        status = Status::UnsupportedCommand;
    }
    else
    {
        status = mDelegate->HandleStep(direction, wrap, lowestOff);
    }
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

static uint8_t Instance::getSpeedFromPercent(float percent, uint8_t speedMax)
{
    return static_cast<uint8_t>(ceil(speedMax * (percent * 0.01)));
}

static Percent Instance::getPercentFromSpeed(uint8_t speed, uint8_t speedMax)
{
    return static_cast<Percent>(ceil(speed / speedMax * 100));
}

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
