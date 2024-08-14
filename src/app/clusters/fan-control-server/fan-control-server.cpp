/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;

using Protocols::InteractionModel::Status;

namespace {

constexpr size_t kFanControlDelegateTableSize =
    MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kFanControlDelegateTableSize <= kEmberInvalidEndpointIndex, "FanControl Delegate table size error");

Delegate * gDelegateTable[kFanControlDelegateTableSize] = { nullptr };

struct EmberAfFanControlState {
    bool moveStarted;
    DataModel::Nullable<chip::Percent> endPercent;
    QuieterReportingAttribute<uint16_t> quietPercentCurrent{ DataModel::Nullable<chip::Percent>(0) };
};

static EmberAfFanControlState stateTable[kFanControlDelegateTableSize];

static EmberAfFanControlState * getState(EndpointId endpoint);

static Status SetPercentCurrentQuietReport(EndpointId endpoint, EmberAfFanControlState * state,
                                         DataModel::Nullable<chip::Percent> newValue, bool isStartOrEndOfTransition);

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

Delegate * GetDelegate(EndpointId aEndpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kFanControlDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(aEndpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kFanControlDelegateTableSize)
    {
        gDelegateTable[ep] = aDelegate;
    }
}

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {

// Indicates if the write operation is from the cluster server itself
bool gWriteFromClusterLogic = false;

Status SetFanModeToOff(EndpointId endpointId)
{
    FanModeEnum currentFanMode;
    Status status = FanMode::Get(endpointId, &currentFanMode);
    VerifyOrReturnError(Status::Success == status, status);

    if (currentFanMode != FanModeEnum::kOff)
    {
        status = FanMode::Set(endpointId, FanModeEnum::kOff);
    }

    return status;
}

bool HasFeature(EndpointId endpoint, Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

inline bool SupportsMultiSpeed(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kMultiSpeed);
}

inline bool SupportsAuto(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kAuto);
}

inline bool SupportsRocking(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kRocking);
}

inline bool SupportsWind(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kWind);
}

inline bool SupportsStep(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kStep);
}

inline bool SupportsAirflowDirection(EndpointId endpointId)
{
    return HasFeature(endpointId, Feature::kAirflowDirection);
}

} // anonymous namespace


void emberAfFanControlClusterServerInitCallback(EndpointId endpoint)
{
    EmberAfFanControlState * state = getState(endpoint);
    auto now = System::SystemClock().GetMonotonicTimestamp();

    if (state == nullptr)
    {
        ChipLogProgress(Zcl, "ERR: Fan control cluster not available on ep%d", endpoint);
        return;
    }

    state->quietPercentCurrent.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);

    DataModel::Nullable<chip::Percent> percentCurrent;
    Status status = Attributes::PercentCurrent::Get(endpoint, percentCurrent);
    state->moveStarted = false;
    if (status == Status::Success)
    {
        state->endPercent = percentCurrent;
        state->quietPercentCurrent.SetValue(percentCurrent, now);
    }
}

// =============================================================================
// Pre-change callbacks for cluster attributes
// =============================================================================

using Status = Protocols::InteractionModel::Status;

Protocols::InteractionModel::Status
MatterFanControlClusterServerPreAttributeChangedCallback(const ConcreteAttributePath & attributePath,
                                                         EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    Protocols::InteractionModel::Status res;
    uint16_t ep = GetEndPoint(attributePath.mEndpointId, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);

    switch (attributePath.mAttributeId)
    {
    case FanMode::Id: {
        if (*value == to_underlying(FanModeEnum::kOn))
        {
            FanMode::Set(attributePath.mEndpointId, FanModeEnum::kHigh);
            res = Status::WriteIgnored;
        }
        else if (*value == to_underlying(FanModeEnum::kSmart))
        {
            FanModeSequenceEnum fanModeSequence;
            Status status = FanModeSequence::Get(attributePath.mEndpointId, &fanModeSequence);
            VerifyOrReturnError(Status::Success == status, Status::Failure);

            if (SupportsAuto(attributePath.mEndpointId) &&
                ((fanModeSequence == FanModeSequenceEnum::kOffLowHighAuto) ||
                 (fanModeSequence == FanModeSequenceEnum::kOffLowMedHighAuto)))
            {
                FanMode::Set(attributePath.mEndpointId, FanModeEnum::kAuto);
            }
            else
            {
                FanMode::Set(attributePath.mEndpointId, FanModeEnum::kHigh);
            }
            res = Status::WriteIgnored;
        }
        else
        {
            res = Status::Success;
        }
        break;
    }
    case SpeedSetting::Id: {
        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            // Check if the SpeedSetting is null.
            if (NumericAttributeTraits<uint8_t>::IsNullValue(*value))
            {

                if (gWriteFromClusterLogic)
                {
                    res                    = Status::Success;
                    gWriteFromClusterLogic = false;
                }
                else
                {
                    res = Status::WriteIgnored;
                }
            }
            else
            {
                uint8_t speedMax;
                Status status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
                VerifyOrReturnError(Status::Success == status, Status::ConstraintError);

                if (*value <= speedMax)
                {
                    res = Status::Success;
                }
                else
                {
                    res = Status::ConstraintError;
                }
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case PercentSetting::Id: {
        // Check if the PercentSetting is null.
        if (NumericAttributeTraits<Percent>::IsNullValue(*value))
        {
            if (gWriteFromClusterLogic)
            {
                res                    = Status::Success;
                gWriteFromClusterLogic = false;
            }
            else
            {
                res = Status::WriteIgnored;
            }
        }
        else
        {
            // record a movement start for this endpoint (might need to be a potential move as move could be rejected via other logic???)
            percentCurrentQTracking[ep].moveStarted = true;
            percentCurrentQTracking[ep].endPercent = (DataModel::Nullable<chip::Percent>)*value;
            res = Status::Success;
        }
        break;
    }
    case RockSetting::Id: {
        if (SupportsRocking(attributePath.mEndpointId))
        {
            BitMask<RockBitmap> rockSupport;
            Status status = RockSupport::Get(attributePath.mEndpointId, &rockSupport);
            VerifyOrReturnError(Status::Success == status, Status::ConstraintError);
            auto rawRockSupport = rockSupport.Raw();
            if ((*value & rawRockSupport) == *value)
            {
                res = Status::Success;
            }
            else
            {
                res = Status::ConstraintError;
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case WindSetting::Id: {
        if (SupportsWind(attributePath.mEndpointId))
        {
            BitMask<WindBitmap> windSupport;
            Status status = WindSupport::Get(attributePath.mEndpointId, &windSupport);
            VerifyOrReturnError(Status::Success == status, Status::ConstraintError);
            auto rawWindSupport = windSupport.Raw();
            if ((*value & rawWindSupport) == *value)
            {
                res = Status::Success;
            }
            else
            {
                res = Status::ConstraintError;
            }
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case AirflowDirection::Id: {
        if (SupportsAirflowDirection(attributePath.mEndpointId))
        {
            res = Status::Success;
        }
        else
        {
            res = Status::UnsupportedAttribute;
        }
        break;
    }
    case PercentCurrent::Id: {
        res = SetPercentCurrentQuietReport(attributePath.mEndpointId, value);
        break;
    }
    default:
        res = Status::Success;
        break;
    }

    return res;
}

void MatterFanControlClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    switch (attributePath.mAttributeId)
    {
    case FanMode::Id: {
        FanModeEnum mode;
        Status status = FanMode::Get(attributePath.mEndpointId, &mode);
        VerifyOrReturn(Status::Success == status);

        // Setting the FanMode value to Off SHALL set the values of PercentSetting, PercentCurrent,
        // SpeedSetting, SpeedCurrent attributes to 0 (zero).
        if (mode == FanModeEnum::kOff)
        {
            status = PercentSetting::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to write PercentSetting with error: 0x%02x", to_underlying(status)));

            status = PercentCurrent::Set(attributePath.mEndpointId, 0);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to write PercentCurrent with error: 0x%02x", to_underlying(status)));

            if (SupportsMultiSpeed(attributePath.mEndpointId))
            {
                status = SpeedSetting::Set(attributePath.mEndpointId, 0);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to write SpeedSetting with error: 0x%02x", to_underlying(status)));

                status = SpeedCurrent::Set(attributePath.mEndpointId, 0);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to write SpeedCurrent with error: 0x%02x", to_underlying(status)));
            }
        }

        // Setting the attribute value to Auto SHALL set the values of PercentSetting, SpeedSetting (if present)
        // to null.
        if (mode == FanModeEnum::kAuto)
        {
            gWriteFromClusterLogic = true;
            status                 = PercentSetting::SetNull(attributePath.mEndpointId);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to write PercentSetting with error: 0x%02x", to_underlying(status)));

            if (SupportsMultiSpeed(attributePath.mEndpointId))
            {
                gWriteFromClusterLogic = true;
                status                 = SpeedSetting::SetNull(attributePath.mEndpointId);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to write SpeedSetting with error: 0x%02x", to_underlying(status)));
            }
        }
        break;
    }
    case PercentSetting::Id: {
        DataModel::Nullable<Percent> percentSetting;
        Status status = PercentSetting::Get(attributePath.mEndpointId, percentSetting);
        VerifyOrReturn(Status::Success == status && !percentSetting.IsNull());

        // If PercentSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
        if (percentSetting.Value() == 0)
        {
            status = SetFanModeToOff(attributePath.mEndpointId);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to set FanMode to off with error: 0x%02x", to_underlying(status)));
        }

        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            // Adjust SpeedSetting from a percent value change for PercentSetting
            // speed = ceil( SpeedMax * (percent * 0.01) )
            uint8_t speedMax;
            status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", to_underlying(status)));

            DataModel::Nullable<uint8_t> currentSpeedSetting;
            status = SpeedSetting::Get(attributePath.mEndpointId, currentSpeedSetting);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to get SpeedSetting with error: 0x%02x", to_underlying(status)));

            uint16_t percent = percentSetting.Value();
            // Plus 99 then integer divide by 100 instead of multiplying 0.01 to avoid floating point precision error
            uint8_t speedSetting = static_cast<uint8_t>((speedMax * percent + 99) / 100);

            if (currentSpeedSetting.IsNull() || speedSetting != currentSpeedSetting.Value())
            {
                status = SpeedSetting::Set(attributePath.mEndpointId, speedSetting);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to set SpeedSetting with error: 0x%02x", to_underlying(status)));
            }
        }
        break;
    }
    case SpeedSetting::Id: {
        if (SupportsMultiSpeed(attributePath.mEndpointId))
        {
            DataModel::Nullable<uint8_t> speedSetting;
            Status status = SpeedSetting::Get(attributePath.mEndpointId, speedSetting);
            VerifyOrReturn(Status::Success == status && !speedSetting.IsNull());

            // If SpeedSetting is set to 0, the server SHALL set the FanMode attribute value to Off.
            if (speedSetting.Value() == 0)
            {
                status = SetFanModeToOff(attributePath.mEndpointId);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to set FanMode to off with error: 0x%02x", to_underlying(status)));
            }

            // Adjust PercentSetting from a speed value change for SpeedSetting
            // percent = floor( speed/SpeedMax * 100 )
            uint8_t speedMax;
            status = SpeedMax::Get(attributePath.mEndpointId, &speedMax);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to get SpeedMax with error: 0x%02x", to_underlying(status)));

            DataModel::Nullable<Percent> currentPercentSetting;
            status = PercentSetting::Get(attributePath.mEndpointId, currentPercentSetting);
            VerifyOrReturn(Status::Success == status,
                           ChipLogError(Zcl, "Failed to get PercentSetting with error: 0x%02x", to_underlying(status)));

            float speed            = speedSetting.Value();
            Percent percentSetting = static_cast<Percent>(speed / speedMax * 100);

            if (currentPercentSetting.IsNull() || percentSetting != currentPercentSetting.Value())
            {
                status = PercentSetting::Set(attributePath.mEndpointId, percentSetting);
                VerifyOrReturn(Status::Success == status,
                               ChipLogError(Zcl, "Failed to set PercentSetting with error: 0x%02x", to_underlying(status)));
            }
        }
        break;
    }
    default:
        break;
    }
}

bool emberAfFanControlClusterStepCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::Step::DecodableType & commandData)
{
    Protocols::InteractionModel::Status status = Status::Success;

    ChipLogProgress(Zcl, "FanControl emberAfFanControlClusterStepCallback: Endpoint %u", commandPath.mEndpointId);

    if (!SupportsStep(commandPath.mEndpointId))
    {
        ChipLogProgress(Zcl, "FanControl does not support Step:%u", commandPath.mEndpointId);
        status = Status::UnsupportedCommand;
    }
    else
    {
        EndpointId endpoint         = commandPath.mEndpointId;
        StepDirectionEnum direction = commandData.direction;

        bool wrapValue      = commandData.wrap.ValueOr(false);
        bool lowestOffValue = commandData.lowestOff.ValueOr(false);

        Delegate * delegate = GetDelegate(endpoint);
        if (delegate)
        {
            status = delegate->HandleStep(direction, wrapValue, lowestOffValue);
        }
        else
        {
            ChipLogProgress(Zcl, "FanControl has no delegate set for endpoint:%u", endpoint);
            status = Status::Failure;
        }
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

static EmberAfFanControlState * getState(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, FanControl::Id, MATTER_DM_FAN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kFanControlDelegateTableSize ? nullptr : &stateTable[ep]);
}

/*
 * @brief
 * This function is used to update the percent current attribute
 * while respecting its defined quiet reporting quality:
 * The attribute will be reported:
 * - At most once per second, or
 * - At the start of the movement/transition, or
 * - At the end of the movement/transition, or
 * - When it changes from null to any other value and vice versa.
 *
 * @param endpoint: endpoint on which the percentCurrent attribute must be updated.
 * @param newValue: Value to update the attribute with
 * @return WriteIgnored in setting the attribute value or the IM error code for the failure.
 */
static Status SetPercentCurrentQuietReport(EndpointId endpoint, DataModel::Nullable<chip::Percent> newValue)
{
    AttributeDirtyState dirtyState;
    auto now = System::SystemClock().GetMonotonicTimestamp();
    EmberAfFanControlState * state = getState(endpoint);

    bool isStartOrEndOfTransition = state->moveStarted || 
        ((DataModel::Nullable<chip::Percent>)*value == state->endPercent)
    state->moveStarted = false;

    if (isStartOrEndOfTransition)
    {
        // At the start or end of the movement/transition we must report
        auto predicate = [](const decltype(state->quietPercentCurrent)::SufficientChangePredicateCandidate &) -> bool {
            return true;
        };
        dirtyState = state->quietPercentCurrent.SetValue(newValue, now, predicate);
    }
    else
    {
        // During transtions, reports should be at most once per second
        System::Clock::Milliseconds64 reportInterval = System::Clock::Milliseconds64(1000);
        auto predicate = state->quietPercentCurrent.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
        dirtyState     = state->quietPercentCurrent.SetValue(newValue, now, predicate);
    }

    MarkAttributeDirty markDirty = MarkAttributeDirty::kNo;
    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        markDirty = MarkAttributeDirty::kYes;
    }

    Status  res = Attributes::PercentCurrent::Set(endpoint, state->quietPercentCurrent.value(), markDirty);
    if (res == Status::Success)
    {
        res = Status::WriteIgnored;
    }
    return res;
}
