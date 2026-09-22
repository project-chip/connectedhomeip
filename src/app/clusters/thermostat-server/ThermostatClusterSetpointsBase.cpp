/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/support/Assertions.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

#include <algorithm>
#include <cstdint>

#include "Setpoint.h"
#include "Temperature.h"
#include "ThermostatClusterBase.h"
#include "ThermostatClusterSetpointsBase.h"

#include "Setpoints.h"

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

const OptionalAttributes & ThermostatSetpointsBase::GetOptionalAttributes() const
{
    return mCluster.GetOptionalAttributes();
}

const BitFlags<Thermostat::Feature> & ThermostatSetpointsBase::Features() const
{
    return mCluster.Features();
}

Setpoints ThermostatSetpointsBase::GetSetpoints()
{
    Setpoints setpoints;
    auto features                = mCluster.Features();
    setpoints.autoSupported      = features.Has(Feature::kAutoMode);
    setpoints.heatSupported      = features.Has(Feature::kHeating);
    setpoints.coolSupported      = features.Has(Feature::kCooling);
    setpoints.occupancySupported = features.Has(Feature::kOccupancy);
    return setpoints;
}

void ThermostatSetpointsBase::GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp) const
{
    if (!mCluster.Features().Has(Feature::kEvents))
    {
        return;
    }
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        mCluster.GenerateSetpointChangeEvent(SystemModeEnum::kHeat, OccupancyBitmap::kOccupied, MakeOptional(oldTemp), newTemp);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        mCluster.GenerateSetpointChangeEvent(SystemModeEnum::kHeat, BitMask<OccupancyBitmap>(), MakeOptional(oldTemp), newTemp);
        break;
    case OccupiedCoolingSetpoint::Id:
        mCluster.GenerateSetpointChangeEvent(SystemModeEnum::kCool, OccupancyBitmap::kOccupied, MakeOptional(oldTemp), newTemp);
        break;
    case UnoccupiedCoolingSetpoint::Id:
        mCluster.GenerateSetpointChangeEvent(SystemModeEnum::kCool, BitMask<OccupancyBitmap>(), MakeOptional(oldTemp), newTemp);
        break;
    }
}

void ThermostatSetpointsBase::NotifyAttributesChanged(const SetpointAttributes & changedAttributes)
{
    for (AttributeId id = 0; id < 32; ++id)
    {
        if (changedAttributes.Has(id))
        {
            mCluster.NotifyAttributeChanged(id);
        }
    }
}

bool ThermostatSetpointsBase::IsOperationalSetpointAttribute(AttributeId attributeId)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
    case UnoccupiedHeatingSetpoint::Id:
    case OccupiedCoolingSetpoint::Id:
    case UnoccupiedCoolingSetpoint::Id:
        return true;
    default:
        return false;
    }
}

void ThermostatSetpointsBase::UpdateSetpointChangeAttributes(const Setpoints & oldSetpoints, const Setpoints & newSetpoints,
                                                             const SetpointAttributes & changedAttributes,
                                                             bool initiatedByOperationalSetpointWrite)
{
    // A setpoint *limit* write (or a deadband adjustment) can indirectly clamp an operational setpoint via
    // Setpoints::Fix(), which would otherwise show up in `changedAttributes` exactly like a direct write. Only a
    // direct write to (or SetpointRaiseLower command targeting) an operational setpoint counts as a setpoint
    // change for tracking purposes.
    if (!initiatedByOperationalSetpointWrite)
    {
        return;
    }

    const auto & optionalAttributes = GetOptionalAttributes();
    if (!optionalAttributes.SetpointChangeSource && !optionalAttributes.SetpointChangeAmount &&
        !optionalAttributes.SetpointChangeSourceTimestamp)
    {
        return;
    }

    const Setpoint * oldOperationalSetpoints[] = { &oldSetpoints.occupiedRange.heating, &oldSetpoints.occupiedRange.cooling,
                                                   &oldSetpoints.unoccupiedRange.heating, &oldSetpoints.unoccupiedRange.cooling };
    const Setpoint * newOperationalSetpoints[] = { &newSetpoints.occupiedRange.heating, &newSetpoints.occupiedRange.cooling,
                                                   &newSetpoints.unoccupiedRange.heating, &newSetpoints.unoccupiedRange.cooling };

    // If more than one operational setpoint changed in the same operation (e.g. a SetpointRaiseLower command with
    // mode kBoth), report the delta of the first one, in a fixed order. Summing signed deltas across multiple
    // setpoints could cancel out (opposite-direction changes) or double-count (same-direction changes), either of
    // which would misrepresent how much the setpoint actually moved.
    const Setpoint * changedOldSetpoint = nullptr;
    const Setpoint * changedNewSetpoint = nullptr;
    for (size_t i = 0; i < MATTER_ARRAY_SIZE(newOperationalSetpoints); ++i)
    {
        if (changedAttributes.Has(newOperationalSetpoints[i]->AttributeId()))
        {
            changedOldSetpoint = oldOperationalSetpoints[i];
            changedNewSetpoint = newOperationalSetpoints[i];
            break;
        }
    }

    if (changedNewSetpoint == nullptr)
    {
        return;
    }

    if (optionalAttributes.SetpointChangeSource)
    {
        mSetpointChangeSource = mCluster.mDelegate.GetSetpointChangeSource();
        mCluster.NotifyAttributeChanged(Attributes::SetpointChangeSource::Id);
    }

    if (optionalAttributes.SetpointChangeAmount)
    {
        int32_t amount =
            static_cast<int32_t>(changedNewSetpoint->Temperature()) - static_cast<int32_t>(changedOldSetpoint->Temperature());
        amount = std::clamp(amount, static_cast<int32_t>(INT16_MIN), static_cast<int32_t>(INT16_MAX));
        mSetpointChangeAmount.SetNonNull(static_cast<int16_t>(amount));
        mCluster.NotifyAttributeChanged(Attributes::SetpointChangeAmount::Id);
    }

    if (optionalAttributes.SetpointChangeSourceTimestamp)
    {
        uint32_t matterEpochS;
        if (System::Clock::GetClock_MatterEpochS(matterEpochS) == CHIP_NO_ERROR)
        {
            mSetpointChangeSourceTimestamp = matterEpochS;
            mCluster.NotifyAttributeChanged(Attributes::SetpointChangeSourceTimestamp::Id);
        }
        else
        {
            ChipLogError(Zcl, "Failed to read the current time while updating SetpointChangeSourceTimestamp; leaving it stale");
        }
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatSetpointsBase::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                    TLV::TLVReader & input_arguments,
                                                                                    CommandHandler * handler)
{

    switch (request.path.mCommandId)
    {
    case SetpointRaiseLower::Id: {
        Commands::SetpointRaiseLower::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));

        Setpoints currentSetpoints = GetSetpoints();
        Setpoints setpoints        = currentSetpoints;

        OccupancyBitmap isOccupied = mCluster.IsOccupied() ? OccupancyBitmap::kOccupied : OccupancyBitmap(0);

        auto & range      = setpoints.GetRange(isOccupied);
        temperature delta = static_cast<temperature>(request_data.amount * 10);

        chip::Optional<temperature> heat;
        chip::Optional<temperature> cool;

        switch (request_data.mode)
        {
        case SetpointRaiseLowerModeEnum::kBoth:
            if (setpoints.heatSupported)
            {
                heat.SetValue(static_cast<temperature>(range.heating.Temperature() + delta));
            }
            if (setpoints.coolSupported)
            {
                cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + delta));
            }
            break;
        case SetpointRaiseLowerModeEnum::kHeat:
            if (setpoints.heatSupported)
            {
                heat.SetValue(static_cast<temperature>(range.heating.Temperature() + delta));
            }
            break;
        case SetpointRaiseLowerModeEnum::kCool:
            if (setpoints.coolSupported)
            {
                cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + delta));
            }
            break;
        default:
            return Status::InvalidCommand;
        }

        if (!heat.HasValue() && !cool.HasValue())
        {
            return Status::InvalidCommand;
        }
        SetpointAttributes changedAttributes;
        auto status = setpoints.ChangeRange(range, heat, cool, Setpoints::ClampMode::kClamp, changedAttributes);
        if (status != Status::Success)
        {
            return status;
        }
        return SaveSetpoints(setpoints, changedAttributes, /* initiatedByOperationalSetpointWrite = */ true);
    }
    }

    return std::nullopt;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
