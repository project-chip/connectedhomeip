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
#include "ThermostatCluster.h"

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

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_DEFAULT FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    // Optional User supplied limits
    int16_t MinHeatSetpointLimit = kDefaultMinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit = kDefaultMaxHeatSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    Status status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinHeatSetpointLimit::Get(endpoint, &AbsMinHeatSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = AbsMaxHeatSetpointLimit::Get(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = MinHeatSetpointLimit::Get(endpoint, &MinHeatSetpointLimit);
    if (status != Status::Success)
    {
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = MaxHeatSetpointLimit::Get(endpoint, &MaxHeatSetpointLimit);
    if (status != Status::Success)
    {
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacturer imposed limits

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinHeatSetpointLimit < AbsMinHeatSetpointLimit)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit > AbsMaxHeatSetpointLimit)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (HeatingSetpoint < MinHeatSetpointLimit)
        HeatingSetpoint = MinHeatSetpointLimit;

    if (HeatingSetpoint > MaxHeatSetpointLimit)
        HeatingSetpoint = MaxHeatSetpointLimit;

    return HeatingSetpoint;
}

int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint)
{
    // Optional Mfg supplied limits
    int16_t AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    // Optional User supplied limits
    int16_t MinCoolSetpointLimit = kDefaultMinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit = kDefaultMaxCoolSetpointLimit;

    // Attempt to read the setpoint limits
    // Absmin/max are manufacturer limits
    // min/max are user imposed min/max

    // Note that the limits are initialized above per the spec limits
    // if they are not present Get() will not update the value so the defaults are used
    Status status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = AbsMinCoolSetpointLimit::Get(endpoint, &AbsMinCoolSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = AbsMaxCoolSetpointLimit::Get(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = MinCoolSetpointLimit::Get(endpoint, &MinCoolSetpointLimit);
    if (status != Status::Success)
    {
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = MaxCoolSetpointLimit::Get(endpoint, &MaxCoolSetpointLimit);
    if (status != Status::Success)
    {
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

    // Make sure the user imposed limits are within the manufacture imposed limits
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3725
    // Spec does not specify the behavior is the requested setpoint exceeds the limit allowed
    // This implementation clamps at the limit.

    // resolution of 3725 is to clamp.

    if (MinCoolSetpointLimit < AbsMinCoolSetpointLimit)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit > AbsMaxCoolSetpointLimit)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (CoolingSetpoint < MinCoolSetpointLimit)
        CoolingSetpoint = MinCoolSetpointLimit;

    if (CoolingSetpoint > MaxCoolSetpointLimit)
        CoolingSetpoint = MaxCoolSetpointLimit;

    return CoolingSetpoint;
}

Status GetSetpointLimits(EndpointId endpoint, SetpointLimits & setpointLimits)
{
    uint32_t flags;
    if (FeatureMap::Get(endpoint, &flags) != Status::Success)
    {
        ChipLogError(Zcl, "GetSetpointLimits: could not get feature flags");
        flags = FEATURE_MAP_DEFAULT;
    }

    auto featureMap                   = BitMask<Feature, uint32_t>(flags);
    setpointLimits.autoSupported      = featureMap.Has(Feature::kAutoMode);
    setpointLimits.heatSupported      = featureMap.Has(Feature::kHeating);
    setpointLimits.coolSupported      = featureMap.Has(Feature::kCooling);
    setpointLimits.occupancySupported = featureMap.Has(Feature::kOccupancy);

    if (setpointLimits.autoSupported)
    {
        int8_t deadBand;
        if (MinSetpointDeadBand::Get(endpoint, &deadBand) != Status::Success)
        {
            deadBand = kDefaultDeadBand;
        }
        setpointLimits.deadBand = static_cast<int16_t>(deadBand * 10);
    }

    if (AbsMinCoolSetpointLimit::Get(endpoint, &setpointLimits.absMinCoolSetpointLimit) != Status::Success)
        setpointLimits.absMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (AbsMaxCoolSetpointLimit::Get(endpoint, &setpointLimits.absMaxCoolSetpointLimit) != Status::Success)
        setpointLimits.absMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (MinCoolSetpointLimit::Get(endpoint, &setpointLimits.minCoolSetpointLimit) != Status::Success)
        setpointLimits.minCoolSetpointLimit = setpointLimits.absMinCoolSetpointLimit;

    if (MaxCoolSetpointLimit::Get(endpoint, &setpointLimits.maxCoolSetpointLimit) != Status::Success)
        setpointLimits.maxCoolSetpointLimit = setpointLimits.absMaxCoolSetpointLimit;

    if (AbsMinHeatSetpointLimit::Get(endpoint, &setpointLimits.absMinHeatSetpointLimit) != Status::Success)
        setpointLimits.absMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (AbsMaxHeatSetpointLimit::Get(endpoint, &setpointLimits.absMaxHeatSetpointLimit) != Status::Success)
        setpointLimits.absMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (MinHeatSetpointLimit::Get(endpoint, &setpointLimits.minHeatSetpointLimit) != Status::Success)
        setpointLimits.minHeatSetpointLimit = setpointLimits.absMinHeatSetpointLimit;

    if (MaxHeatSetpointLimit::Get(endpoint, &setpointLimits.maxHeatSetpointLimit) != Status::Success)
        setpointLimits.maxHeatSetpointLimit = setpointLimits.absMaxHeatSetpointLimit;

    if (setpointLimits.coolSupported)
    {
        if (OccupiedCoolingSetpoint::Get(endpoint, &setpointLimits.occupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.heatSupported)
    {
        if (OccupiedHeatingSetpoint::Get(endpoint, &setpointLimits.occupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.coolSupported && setpointLimits.occupancySupported)
    {
        if (UnoccupiedCoolingSetpoint::Get(endpoint, &setpointLimits.unoccupiedCoolingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return Status::Failure;
        }
    }

    if (setpointLimits.heatSupported && setpointLimits.occupancySupported)
    {
        if (UnoccupiedHeatingSetpoint::Get(endpoint, &setpointLimits.unoccupiedHeatingSetpoint) != Status::Success)
        {
            // We're substituting the failure code here for backwards-compatibility reasons
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return Status::Failure;
        }
    }
    return Status::Success;
}

Status CheckHeatingSetpointDeadband(bool autoSupported, int16_t newCoolingSetpoint, int16_t minHeatingSetpoint, int16_t deadband)
{
    if (!autoSupported)
    {
        return Status::Success;
    }
    int16_t maxValidHeatingSetpoint = static_cast<int16_t>(newCoolingSetpoint - deadband);
    if (maxValidHeatingSetpoint < minHeatingSetpoint)
    {
        // If we need to adjust the heating setpoint to preserve the deadband, it will go below the min heat setpoint
        return Status::InvalidValue;
    }
    // It's possible to adjust the heating setpoint, if needed
    return Status::Success;
}

Status CheckCoolingSetpointDeadband(bool autoSupported, int16_t newHeatingSetpoint, int16_t maxCoolingSetpoint, int16_t deadband)
{
    if (!autoSupported)
    {
        return Status::Success;
    }
    int16_t minValidCoolingSetpoint = static_cast<int16_t>(newHeatingSetpoint + deadband);
    if (minValidCoolingSetpoint > maxCoolingSetpoint)
    {
        // If we need to adjust the cooling setpoint to preserve the deadband, it will go above the max cool setpoint
        return Status::InvalidValue;
    }
    // It's possible to adjust the cooling setpoint, if needed
    return Status::Success;
}

typedef Status (*SetpointSetter)(EndpointId endpoint, int16_t value);

void EnsureCoolingSetpointDeadband(EndpointId endpoint, int16_t currentCoolingSetpoint, int16_t newHeatingSetpoint,
                                   int16_t maxCoolingSetpoint, int16_t deadband, SetpointSetter setter)
{
    int16_t minValidCoolingSetpoint = static_cast<int16_t>(newHeatingSetpoint + deadband);
    if (currentCoolingSetpoint >= minValidCoolingSetpoint)
    {
        // The current cooling setpoint doesn't violate the deadband
        return;
    }
    if (minValidCoolingSetpoint > maxCoolingSetpoint)
    {
        // Adjusting the cool setpoint to preserve the deadband would violate the max cool setpoint
        // This should have been caught in CheckCoolingSetpointDeadband, so log and exit
        ChipLogError(Zcl, "Failed ensuring cooling setpoint deadband");
        return;
    }
    // Adjust the cool setpoint to preserve deadband
    auto status = setter(endpoint, minValidCoolingSetpoint);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Error: EnsureCoolingSetpointDeadband failed!");
    }
}

void EnsureHeatingSetpointDeadband(EndpointId endpoint, int16_t currentHeatingSetpoint, int16_t newCoolingSetpoint,
                                   int16_t minHeatingSetpoint, int16_t deadband, SetpointSetter setter)
{
    int16_t maxValidHeatingSetpoint = static_cast<int16_t>(newCoolingSetpoint - deadband);
    if (currentHeatingSetpoint <= maxValidHeatingSetpoint)
    {
        // The current heating setpoint doesn't violate the deadband
        return;
    }
    if (maxValidHeatingSetpoint < minHeatingSetpoint)
    {
        // Adjusting the heating setpoint to preserve the deadband would violate the min heating setpoint
        // This should have been caught in CheckHeatingSetpointDeadband, so log and exit
        ChipLogError(Zcl, "Failed ensuring heating setpoint deadband");
        return;
    }
    // Adjust the heating setpoint to preserve deadband
    auto status = setter(endpoint, maxValidHeatingSetpoint);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "Error: EnsureHeatingSetpointDeadband failed!");
    }
}

void EnsureDeadband(const ConcreteAttributePath & attributePath)
{
    auto endpoint = attributePath.mEndpointId;
    SetpointLimits setpointLimits;
    auto status = GetSetpointLimits(endpoint, setpointLimits);
    if (status != Status::Success)
    {
        return;
    }
    if (!setpointLimits.autoSupported)
    {
        return;
    }
    switch (attributePath.mAttributeId)
    {
    case OccupiedHeatingSetpoint::Id:
        EnsureCoolingSetpointDeadband(endpoint, setpointLimits.occupiedCoolingSetpoint, setpointLimits.occupiedHeatingSetpoint,
                                      setpointLimits.maxCoolSetpointLimit, setpointLimits.deadBand, OccupiedCoolingSetpoint::Set);
        break;
    case OccupiedCoolingSetpoint::Id:
        EnsureHeatingSetpointDeadband(endpoint, setpointLimits.occupiedHeatingSetpoint, setpointLimits.occupiedCoolingSetpoint,
                                      setpointLimits.minHeatSetpointLimit, setpointLimits.deadBand, OccupiedHeatingSetpoint::Set);
        break;
    case UnoccupiedHeatingSetpoint::Id:
        EnsureCoolingSetpointDeadband(endpoint, setpointLimits.unoccupiedCoolingSetpoint, setpointLimits.unoccupiedHeatingSetpoint,
                                      setpointLimits.maxCoolSetpointLimit, setpointLimits.deadBand, UnoccupiedCoolingSetpoint::Set);
        break;
    case UnoccupiedCoolingSetpoint::Id:
        EnsureHeatingSetpointDeadband(endpoint, setpointLimits.unoccupiedHeatingSetpoint, setpointLimits.unoccupiedCoolingSetpoint,
                                      setpointLimits.minHeatSetpointLimit, setpointLimits.deadBand, UnoccupiedHeatingSetpoint::Set);
        break;
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterSetpointRaiseLowerCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::SetpointRaiseLower::DecodableType & commandData)
{
    auto & mode   = commandData.mode;
    auto & amount = commandData.amount;

    EndpointId aEndpointId = commandPath.mEndpointId;

    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    Status status                     = Status::Failure;
    Status WriteCoolingSetpointStatus = Status::Failure;
    Status WriteHeatingSetpointStatus = Status::Failure;
    int16_t DeadBandTemp              = 0;
    int8_t DeadBand                   = 0;
    uint32_t OurFeatureMap;
    bool AutoSupported = false;
    bool HeatSupported = false;
    bool CoolSupported = false;

    if (FeatureMap::Get(aEndpointId, &OurFeatureMap) != Status::Success)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
        AutoSupported = true;

    if (OurFeatureMap & 1 << 0)
        HeatSupported = true;

    if (OurFeatureMap & 1 << 1)
        CoolSupported = true;

    if (AutoSupported)
    {
        if (MinSetpointDeadBand::Get(aEndpointId, &DeadBand) != Status::Success)
            DeadBand = kDefaultDeadBand;
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }

    switch (mode)
    {
    case SetpointRaiseLowerModeEnum::kBoth:
        if (HeatSupported && CoolSupported)
        {
            int16_t DesiredCoolingSetpoint, CoolLimit, DesiredHeatingSetpoint, HeatLimit;
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                DesiredCoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolLimit              = static_cast<int16_t>(DesiredCoolingSetpoint -
                                                 EnforceCoolingSetpointLimits(DesiredCoolingSetpoint, aEndpointId));
                {
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
                    {
                        DesiredHeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                        HeatLimit              = static_cast<int16_t>(DesiredHeatingSetpoint -
                                                         EnforceHeatingSetpointLimits(DesiredHeatingSetpoint, aEndpointId));
                        {
                            if (CoolLimit != 0 || HeatLimit != 0)
                            {
                                if (abs(CoolLimit) <= abs(HeatLimit))
                                {
                                    // We are limited by the Heating Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - HeatLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - HeatLimit);
                                }
                                else
                                {
                                    // We are limited by Cooling Limit
                                    DesiredHeatingSetpoint = static_cast<int16_t>(DesiredHeatingSetpoint - CoolLimit);
                                    DesiredCoolingSetpoint = static_cast<int16_t>(DesiredCoolingSetpoint - CoolLimit);
                                }
                            }
                            WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, DesiredCoolingSetpoint);
                            if (WriteCoolingSetpointStatus != Status::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, DesiredHeatingSetpoint);
                            if (WriteHeatingSetpointStatus != Status::Success)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                        }
                    }
                }
            }
        }

        if (CoolSupported && !HeatSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                WriteCoolingSetpointStatus = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                if (WriteCoolingSetpointStatus != Status::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                }
            }
        }

        if (HeatSupported && !CoolSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
            {
                HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                WriteHeatingSetpointStatus = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                if (WriteHeatingSetpointStatus != Status::Success)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                }
            }
        }

        if ((!HeatSupported || WriteHeatingSetpointStatus == Status::Success) &&
            (!CoolSupported || WriteCoolingSetpointStatus == Status::Success))
            status = Status::Success;
        break;

    case SetpointRaiseLowerModeEnum::kCool:
        if (CoolSupported)
        {
            if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
            {
                CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            HeatingSetpoint = static_cast<int16_t>(CoolingSetpoint - DeadBandTemp);
                            if (HeatingSetpoint == EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == Status::Success)
                                {
                                    if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == Status::Success)
                                        status = Status::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust heating setpoint to maintain dead band!");
                                status = Status::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
                }
                else
                {
                    status = OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
        }
        else
            status = Status::InvalidCommand;
        break;

    case SetpointRaiseLowerModeEnum::kHeat:
        if (HeatSupported)
        {
            if (OccupiedHeatingSetpoint::Get(aEndpointId, &HeatingSetpoint) == Status::Success)
            {
                HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (OccupiedCoolingSetpoint::Get(aEndpointId, &CoolingSetpoint) == Status::Success)
                    {
                        if (CoolingSetpoint - HeatingSetpoint < DeadBandTemp)
                        {
                            // Dead Band Violation
                            // Try to adjust it
                            CoolingSetpoint = static_cast<int16_t>(HeatingSetpoint + DeadBandTemp);
                            if (CoolingSetpoint == EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId))
                            {
                                // Desired cooling setpoint is enforcable
                                // Set the new cooling and heating setpoints
                                if (OccupiedCoolingSetpoint::Set(aEndpointId, CoolingSetpoint) == Status::Success)
                                {
                                    if (OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint) == Status::Success)
                                        status = Status::Success;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust cooling setpoint to maintain dead band!");
                                status = Status::InvalidCommand;
                            }
                        }
                        else
                            status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
                }
                else
                {
                    status = OccupiedHeatingSetpoint::Set(aEndpointId, HeatingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
        }
        else
            status = Status::InvalidCommand;
        break;

    default:
        status = Status::InvalidCommand;
        break;
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}
