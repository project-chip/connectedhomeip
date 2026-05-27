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

Status ThermostatCluster::HandleSetpointChange(Setpoints & setpoints, const AttributeId attributeId,
                                               temperature value,
                                               SetpointAttributes & changedAttributes)
{
    switch (attributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        return setpoints.ChangeRangeHeating(setpoints.occupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case OccupiedCoolingSetpoint::Id:
        return setpoints.ChangeRangeCooling(setpoints.occupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedHeatingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeHeating(setpoints.unoccupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case UnoccupiedCoolingSetpoint::Id:
        if (!setpoints.occupancySupported)
        {
            return Status::UnsupportedAttribute;
        }
        return setpoints.ChangeRangeCooling(setpoints.unoccupied, value, Setpoints::ClampMode::kDontClamp, changedAttributes);
    case MinHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, value, changedAttributes);
    case MaxHeatSetpointLimit::Id:
        if (!setpoints.heatSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, value, changedAttributes);
    case MinCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMinimum(setpoints.userCoolLimits, value, changedAttributes);
    case MaxCoolSetpointLimit::Id:
        if (!setpoints.coolSupported)
            return Status::UnsupportedAttribute;
        return setpoints.ChangeLimitMaximum(setpoints.userCoolLimits, value, changedAttributes);
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
    if (status == Status::Success)
    {
        mSetpoints.~Setpoints();
        new (&mSetpoints) Setpoints(setpoints);
        return SaveSetpoints(setpoints, changedAttributes);
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

#define ReturnStatusOnFailure(expr)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __status = StatusIB(expr);                                                                                            \
        if (!__status.IsSuccess())                                                                                                 \
        {                                                                                                                          \
            return __status.mStatus;                                                                                               \
        }                                                                                                                          \
    } while (false)

Protocols::InteractionModel::Status ThermostatCluster::SaveSetpoints(Setpoints setpoints,
                                                                     SetpointAttributes changedAttributes)
{
    EndpointId endpoint = mPath.mEndpointId;

    if (changedAttributes.Has(OccupiedHeatingSetpoint::Id))
    {
        int16_t value = setpoints.occupied.heating.Temperature();
        ReturnStatusOnFailure(
            mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, OccupiedHeatingSetpoint::Id },
                                                  ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
        NotifyAttributeChanged(OccupiedHeatingSetpoint::Id);
    }
    if (changedAttributes.Has(OccupiedCoolingSetpoint::Id))
    {
        int16_t value = setpoints.occupied.cooling.Temperature();
        ReturnStatusOnFailure(
            mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, OccupiedCoolingSetpoint::Id },
                                                  ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
        NotifyAttributeChanged(OccupiedCoolingSetpoint::Id);
    }
    if (setpoints.occupancySupported)
    {
        if (changedAttributes.Has(UnoccupiedHeatingSetpoint::Id))
        {
            int16_t value = setpoints.unoccupied.heating.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, UnoccupiedHeatingSetpoint::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(UnoccupiedHeatingSetpoint::Id);
        }
        if (changedAttributes.Has(UnoccupiedCoolingSetpoint::Id))
        {
            int16_t value = setpoints.unoccupied.cooling.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, UnoccupiedCoolingSetpoint::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(UnoccupiedCoolingSetpoint::Id);
        }
    }
    if (setpoints.heatSupported)
    {
        if (changedAttributes.Has(MinHeatSetpointLimit::Id) && setpoints.userHeatLimits.minimum.HasTemperature())
        {
            int16_t value = setpoints.userHeatLimits.minimum.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, MinHeatSetpointLimit::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(MinHeatSetpointLimit::Id);
        }
        if (changedAttributes.Has(MaxHeatSetpointLimit::Id) && setpoints.userHeatLimits.maximum.HasTemperature())
        {
            int16_t value = setpoints.userHeatLimits.maximum.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, MaxHeatSetpointLimit::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(MaxHeatSetpointLimit::Id);
        }
    }
    if (setpoints.coolSupported)
    {
        if (changedAttributes.Has(MinCoolSetpointLimit::Id) && setpoints.userCoolLimits.minimum.HasTemperature())
        {
            int16_t value = setpoints.userCoolLimits.minimum.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, MinCoolSetpointLimit::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(MinCoolSetpointLimit::Id);
        }
        if (changedAttributes.Has(MaxCoolSetpointLimit::Id) && setpoints.userCoolLimits.maximum.HasTemperature())
        {
            int16_t value = setpoints.userCoolLimits.maximum.Temperature();
            ReturnStatusOnFailure(
                mContext->attributeStorage.WriteValue({ endpoint, Thermostat::Id, MaxCoolSetpointLimit::Id },
                                                      ByteSpan(reinterpret_cast<const uint8_t *>(&value), sizeof(value))));
            NotifyAttributeChanged(MaxCoolSetpointLimit::Id);
        }
    }
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatCluster::ChangeSetpointAttribute(const AttributeId attributeId, int16_t temperature)
{
    Setpoints setpoints = mSetpoints;
    SetpointAttributes changedAttributes;

    auto status = HandleSetpointChange(setpoints, attributeId, temperature, changedAttributes);
    if (status == Status::Success)
    {
        mSetpoints.~Setpoints();
        new (&mSetpoints) Setpoints(setpoints);
        return SaveSetpoints(setpoints, changedAttributes);
    }
    return status;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
