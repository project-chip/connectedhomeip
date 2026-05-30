/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ThermostatClusterSetpoints.h"
#include "Setpoint.h"
#include "Temperature.h"
#include "ThermostatCluster.h"

#include "Setpoints.h"
#include "app/data-model-provider/ActionReturnStatus.h"
#include "lib/support/Assertions.h"
#include "protocols/interaction_model/StatusCode.h"
#include <new>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

DataModel::ActionReturnStatus ThermostatCluster::HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId,
                                               temperature value,
                                               SetpointAttributes & changedAttributes)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        return setpoints.ChangeRangeHeating(setpoints.occupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case OccupiedCoolingSetpoint::Id:
        return setpoints.ChangeRangeCooling(setpoints.occupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.unoccupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.unoccupiedRange, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, value, changedAttributes);
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, value, changedAttributes);
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMinimum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, value, changedAttributes);
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeLimitMaximum(setpoints.userCoolLimits, setpoints.absoluteCoolLimits, value, changedAttributes);
    case MinSetpointDeadBand::Id:
        return Status::Success;
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ThermostatCluster::SetpointRaiseLower(const Commands::SetpointRaiseLower::DecodableType & commandData)
{

    auto & mode    = commandData.mode;
    int16_t amount = static_cast<int16_t>(commandData.amount * 10);

    Setpoints setpoints = mSetpoints;

    OccupancyBitmap isOccupied = (!setpoints.occupancySupported || mOccupancy.Has(OccupancyBitmap::kOccupied))
        ? OccupancyBitmap::kOccupied
        : OccupancyBitmap(0);

    auto & range = setpoints.GetRange(isOccupied);

    chip::Optional<temperature> heat;
    chip::Optional<temperature> cool;

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<temperature>(range.heating.Temperature() + amount));
        }
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kHeat:
        if (setpoints.heatSupported)
        {
            heat.SetValue(static_cast<temperature>(range.heating.Temperature() + amount));
        }
        break;
    case SetpointRaiseLowerModeEnum::kCool:
        if (setpoints.coolSupported)
        {
            cool.SetValue(static_cast<temperature>(range.cooling.Temperature() + amount));
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
    status = SaveSetpoints(setpoints, changedAttributes);
    if (status == Protocols::InteractionModel::Status::Success) {
        mSetpoints = setpoints;
    }

    return status;
}

Protocols::InteractionModel::Status ThermostatCluster::LoadSetpoints(Setpoints & setpoints, AttributePersistence & persistence)
{
    auto endpoint = mPath.mEndpointId;
    setpoints.autoSupported      = mFeatures.Has(Feature::kAutoMode);
    setpoints.heatSupported      = mFeatures.Has(Feature::kHeating);
    setpoints.coolSupported      = mFeatures.Has(Feature::kCooling);
    setpoints.occupancySupported = mFeatures.Has(Feature::kOccupancy);

    if (setpoints.autoSupported)
    {
        int8_t deadBand;
        persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinSetpointDeadBand::Id }, deadBand,
                                          static_cast<int8_t>(kDefaultDeadBand / 10));
        setpoints.deadBand = static_cast<int16_t>(deadBand * 10);
    }

    int16_t absMinHeatLimit = kDefaultAbsMinHeatSetpointLimit;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMinHeatSetpointLimit::Id }, absMinHeatLimit,
                                      static_cast<int16_t>(kDefaultAbsMinHeatSetpointLimit));
    setpoints.absoluteHeatLimits.minimum.SetTemperature(absMinHeatLimit);

    int16_t absMaxHeatLimit = kDefaultAbsMaxHeatSetpointLimit;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMaxHeatSetpointLimit::Id }, absMaxHeatLimit,
                                      static_cast<int16_t>(kDefaultAbsMaxHeatSetpointLimit));
    setpoints.absoluteHeatLimits.maximum.SetTemperature(absMaxHeatLimit);

    int16_t absMinCoolLimit = kDefaultAbsMinCoolSetpointLimit;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMinCoolSetpointLimit::Id }, absMinCoolLimit,
                                      static_cast<int16_t>(kDefaultAbsMinCoolSetpointLimit));
    setpoints.absoluteCoolLimits.minimum.SetTemperature(absMinCoolLimit);

    int16_t absMaxCoolLimit = kDefaultAbsMaxCoolSetpointLimit;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, AbsMaxCoolSetpointLimit::Id }, absMaxCoolLimit,
                                      static_cast<int16_t>(kDefaultAbsMaxCoolSetpointLimit));
    setpoints.absoluteCoolLimits.maximum.SetTemperature(absMaxCoolLimit);

    if (setpoints.heatSupported)
    {
        int16_t minHeatLimit;
        if (persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinHeatSetpointLimit::Id }, minHeatLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userHeatLimits.minimum.SetTemperature(minHeatLimit);
        }
        int16_t maxHeatLimit;
        if (persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MaxHeatSetpointLimit::Id }, maxHeatLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userHeatLimits.maximum.SetTemperature(maxHeatLimit);
        }
    }
    if (setpoints.coolSupported)
    {
        int16_t minCoolLimit;
        if (persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MinCoolSetpointLimit::Id }, minCoolLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userCoolLimits.minimum.SetTemperature(minCoolLimit);
        }
        int16_t maxCoolLimit;
        if (persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, MaxCoolSetpointLimit::Id }, maxCoolLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userCoolLimits.maximum.SetTemperature(maxCoolLimit);
        }
    }

    int16_t occupiedCooling;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, OccupiedCoolingSetpoint::Id }, occupiedCooling,
                                      static_cast<int16_t>(kDefaultCoolingSetpoint));
    setpoints.occupied.cooling.SetTemperature(occupiedCooling);

    int16_t occupiedHeating;
    persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, OccupiedHeatingSetpoint::Id }, occupiedHeating,
                                      static_cast<int16_t>(kDefaultHeatingSetpoint));
    setpoints.occupied.heating.SetTemperature(occupiedHeating);

    if (setpoints.occupancySupported)
    {
        int16_t unoccupiedCooling;
        persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, UnoccupiedCoolingSetpoint::Id },
                                          unoccupiedCooling, static_cast<int16_t>(kDefaultCoolingSetpoint));
        setpoints.unoccupied.cooling.SetTemperature(unoccupiedCooling);

        int16_t unoccupiedHeating;
        persistence.LoadNativeEndianValue({ endpoint, Thermostat::Id, UnoccupiedHeatingSetpoint::Id },
                                          unoccupiedHeating, static_cast<int16_t>(kDefaultHeatingSetpoint));
        setpoints.unoccupied.heating.SetTemperature(unoccupiedHeating);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::SaveSetpoint(Setpoint & oldSetpoint, Setpoint & newSetpoint) {
    VerifyOrReturnValue(oldSetpoint.AttributeId() == newSetpoint.AttributeId(), Status::InvalidCommand);
    VerifyOrReturnValue(oldSetpoint.Temperature() != newSetpoint.Temperature(), Status::Success);
    temperature newTemp = newSetpoint.Temperature();
    auto status = mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, Thermostat::Id, oldSetpoint.AttributeId()), ByteSpan(reinterpret_cast<const uint8_t *>(&newTemp), sizeof(newTemp)));
    if (status != CHIP_NO_ERROR) {
        return chip::Protocols::InteractionModel::ClusterStatusCode(status).GetStatus();
    }
    GenerateSetpointEvent(oldSetpoint.AttributeId(), oldSetpoint.Temperature(), newSetpoint.Temperature());
    oldSetpoint.SetTemperature(newSetpoint.Temperature());
    return Status::Success;
}

DataModel::ActionReturnStatus ThermostatCluster::SaveSetpoints(Setpoints & setpoints,
                                                                     SetpointAttributes changedAttributes)
{
    if (!setpoints.Valid()) {
        return Status::ConstraintError;
    }

    Status status;
    if (setpoints.heatSupported)
    {
        if (changedAttributes.Has(MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.minimum.HasTemperature())
        {
            status = SaveSetpoint(mSetpoints.userHeatLimits.minimum, setpoints.userHeatLimits.minimum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.maximum.HasTemperature())
        {
            status = SaveSetpoint(mSetpoints.userHeatLimits.maximum, setpoints.userHeatLimits.maximum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(OccupiedHeatingSetpoint::Id))
        {
            status = SaveSetpoint(mSetpoints.occupied.heating, setpoints.occupied.heating);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (setpoints.occupancySupported)
        {
            if (changedAttributes.Has(UnoccupiedHeatingSetpoint::Id))
            {
                status = SaveSetpoint(mSetpoints.unoccupied.heating, setpoints.unoccupied.heating);
                VerifyOrReturnValue(status == Status::Success, status);
            }
        }   
    }
    if (setpoints.coolSupported)
    {
        if (changedAttributes.Has(MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.minimum.HasTemperature())
        {
            status = SaveSetpoint(mSetpoints.userCoolLimits.minimum, setpoints.userCoolLimits.minimum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.maximum.HasTemperature())
        {
            status = SaveSetpoint(mSetpoints.userCoolLimits.maximum, setpoints.userCoolLimits.maximum);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (changedAttributes.Has(OccupiedCoolingSetpoint::Id))
        {
            status = SaveSetpoint(mSetpoints.occupied.cooling, setpoints.occupied.cooling);
            VerifyOrReturnValue(status == Status::Success, status);
        }
        if (setpoints.occupancySupported)
        {
            if (changedAttributes.Has(UnoccupiedCoolingSetpoint::Id))
            {
                status = SaveSetpoint(mSetpoints.unoccupied.cooling, setpoints.unoccupied.cooling);
                VerifyOrReturnValue(status == Status::Success, status);
            }
        } 
    }
    return Status::Success;
>>>>>>> 5d457bc605 (Initial conversion of Thermostat cluster to code-driven)
}

DataModel::ActionReturnStatus ThermostatCluster::ChangeSetpointAttribute(const AttributeId attributeId, temperature temp)
{
    Setpoints setpoints = mSetpoints;
    SetpointAttributes changedAttributes;

    auto status = HandleSetpointChange(setpoints, attributeId, temp, changedAttributes);
    if (status == Status::Success)
    {
        status = SaveSetpoints(setpoints, changedAttributes);
    }
    return status;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
