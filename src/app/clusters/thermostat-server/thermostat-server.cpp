/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/af.h>

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters;

constexpr int16_t kDefaultAbsMinHeatSetpointLimit    = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultAbsMaxHeatSetpointLimit    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultMinHeatSetpointLimit       = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultMaxHeatSetpointLimit       = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit    = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit    = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultMinCoolSetpointLimit       = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultMaxCoolSetpointLimit       = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultHeatingSetpoint            = 2000;
constexpr int16_t kDefaultCoolingSetpoint            = 2600;
constexpr uint8_t kInvalidControlSequenceOfOperation = 0xff;
constexpr uint8_t kInvalidRequestedSystemMode        = 0xff;
constexpr int8_t kDefaultDeadBand                    = 25; // 2.5C is the default
// // translateZclTemp() and
// // translateDegFTemp() are from the Matter Application Clusters Specification
// // 4.5.4.1. Sample Conversion Code

// /*
//  * Function : translateZclTemp()
//  * Description : Converts the temperature setpoints in ZCL
//  * to the half degF format
//  * The half degF format is 8-bit unsigned,
//  * and represents 2x temperature value in
//  * Fahrenheit (to get 0.5 resolution).
//  * The format used in ZCL is 16-bit signed
//  * in Celsius and multiplied by 100
//  * to get 0.01 resolution.
//  * e.g. 2500 (25.00 deg C) ---> 0x9A (77 deg F)
//  * Input Para : Temperature in ZCL (degC) format
//  * Output Para: Temperature in half DegF format
//  */
// int8u translateZclTemp(int16s temperature)
// {
//     int32s x = temperature;
//     // rearrangement of
//     // = (x * (9/5) / 100 + 32) * 2;
//     // the added 250 is for proper rounding.
//     // a rounding technique that only works
//     // with positive numbers
//     return (int8u)((x * 9 * 2 + 250) / (5 * 100) + 64);
// }
// /*
//  * Function : translateDegFTemp
//  * Description : Converts the temperature in DegF
//  * protocol to the format
//  * expected by the cluster attribute
//  * Measured Value in the
//  * Temperature Measurement
//  * Information Attribute Set.
//  * The half deg F format is 8-bit
//  * unsigned, and represents
//  * 2x temperature value in
//  * Fahrenheit (to get 0.5 resolution).
//  * The format expected by cluster
//  * is 16-bit signed in Celsius and
//  * multiplied by 100 to get
//  * 0.01 resolution.
//  * e.g. 0x9A (77 deg F) ---> 2500 (25.00 deg C)
//  * Input Para : temperature in DegF format
//  * Output Para: temperature in ZCL format
//  */
// int16s translateDegFTemp(int8u temperature)
// {
//     int32s x = temperature;
//     // rearrangement of
//     // = 100 * (x/2 - 32) * 5/9
//     // *1000 (should be 100), +90, then /10,
//     // is for rounding.
//     return (int16s)(((x - 64) * 5 * 1000 + 90) / (10 * 2 * 9));
// }

void emberAfThermostatClusterServerInitCallback()
{
    // TODO
    // Get from the "real thermostat"
    // current mode
    // current occupied heating setpoint
    // current unoccupied heating setpoint
    // current occupied cooling setpoint
    // current unoccupied cooling setpoint
    // and update the zcl cluster values
    // This should be a callback defined function
    // with weak binding so that real thermostat
    // can get the values.
    // or should this just be the responsibility of the thermostat application?
}

// IMPORTANT NOTE:
// No Side effects are permitted in emberAfThermostatClusterServerPreAttributeChangedCallback
// If a setpoint changes is required as a result of setpoint limit change
// it does not happen here.  It is the responsibility of the device to adjust the setpoint(s)
// as required in emberAfThermostatClusterServerPostAttributeChangedCallback
// limit change validation assures that there is at least 1 setpoint that will be valid

#define FEATURE_MAP_HEAT 0x01
#define FEATURE_MAP_COOL 0x02
#define FEATURE_MAP_OCC 0x04
#define FEATURE_MAP_SCH 0x08
#define FEATURE_MAP_SB 0x10
#define FEATURE_MAP_AUTO 0x20

#define FEATURE_MAP_OVERIDE FEATURE_MAP_HEAT | FEATURE_MAP_COOL | FEATURE_MAP_AUTO

EmberAfStatus emberAfThermostatClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                        EmberAfAttributeType attributeType, uint16_t size,
                                                                        uint8_t * value)
{
    int16_t requested;

    // Limits will be needed for all checks
    // so we just get them all now
    int16_t AbsMinHeatSetpointLimit;
    int16_t AbsMaxHeatSetpointLimit;
    int16_t MinHeatSetpointLimit;
    int16_t MaxHeatSetpointLimit;
    int16_t AbsMinCoolSetpointLimit;
    int16_t AbsMaxCoolSetpointLimit;
    int16_t MinCoolSetpointLimit;
    int16_t MaxCoolSetpointLimit;
    int8_t DeadBand         = 0;
    int16_t DeadBandTemp    = 0;
    uint32_t FeatureMap     = 0;
    bool AutoSupported      = false;
    bool HeatSupported      = false;
    bool CoolSupported      = false;
    bool OccupancySupported = false;
    int16_t OccupiedCoolingSetpoint;
    int16_t OccupiedHeatingSetpoint;
    int16_t UnoccupiedCoolingSetpoint;
    int16_t UnoccupiedHeatingSetpoint;

// TODO re-enable reading reaturemap once https://github.com/project-chip/connectedhomeip/pull/9725 is merged
#ifndef FEATURE_MAP_OVERIDE
    emberAfReadServerAttribute(endpoint, Thermostat::Id, chip::app::Clusters::Globals::Attributes::Ids::FeatureMap,
                               (uint8_t *) &FeatureMap, sizeof(FeatureMap));
#else
    FeatureMap = FEATURE_MAP_OVERIDE;
#endif

    if (FeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
    {
        AutoSupported = true;
        if (GetMinSetpointDeadBand(endpoint, &DeadBand) != EMBER_ZCL_STATUS_SUCCESS)
        {
            DeadBand = kDefaultDeadBand;
        }
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }
    if (FeatureMap & 1 << 0)
        HeatSupported = true;

    if (FeatureMap & 1 << 1)
        CoolSupported = true;

    if (FeatureMap & 1 << 2)
        OccupancySupported = true;

    if (GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        AbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;

    if (GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        AbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;

    if (GetMinCoolSetpointLimit(endpoint, &MinCoolSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;

    if (GetMaxCoolSetpointLimit(endpoint, &MaxCoolSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;

    if (GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        AbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;

    if (GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        AbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;

    if (GetMinHeatSetpointLimit(endpoint, &MinHeatSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;

    if (GetMaxHeatSetpointLimit(endpoint, &MaxHeatSetpointLimit) != EMBER_ZCL_STATUS_SUCCESS)
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;

    if (CoolSupported)
        if (GetOccupiedCoolingSetpoint(endpoint, &OccupiedCoolingSetpoint) != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Cooling Setpoint");
            return EMBER_ZCL_STATUS_FAILURE;
        }

    if (HeatSupported)
        if (GetOccupiedHeatingSetpoint(endpoint, &OccupiedHeatingSetpoint) != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(Zcl, "Error: Can not read Occupied Heating Setpoint");
            return EMBER_ZCL_STATUS_FAILURE;
        }

    if (CoolSupported && OccupancySupported)
        if (GetUnoccupiedCoolingSetpoint(endpoint, &UnoccupiedCoolingSetpoint) != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Cooling Setpoint");
            return EMBER_ZCL_STATUS_FAILURE;
        }

    if (HeatSupported && OccupancySupported)
        if (GetUnoccupiedHeatingSetpoint(endpoint, &UnoccupiedHeatingSetpoint) != EMBER_ZCL_STATUS_SUCCESS)
        {
            ChipLogError(Zcl, "Error: Can not read Unoccupied Heating Setpoint");
            return EMBER_ZCL_STATUS_FAILURE;
        }

    switch (attributeId)
    {
    case Ids::OccupiedHeatingSetpoint:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
                 requested > MaxHeatSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested > OccupiedCoolingSetpoint - DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::OccupiedCoolingSetpoint:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
                 requested > MaxCoolSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested < OccupiedHeatingSetpoint + DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::UnoccupiedHeatingSetpoint:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(HeatSupported && OccupancySupported))
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit ||
                 requested > MaxHeatSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested > UnoccupiedCoolingSetpoint - DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::UnoccupiedCoolingSetpoint:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!(CoolSupported && OccupancySupported))
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit ||
                 requested > MaxCoolSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested < UnoccupiedHeatingSetpoint + DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::MinHeatSetpointLimit:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinHeatSetpointLimit || requested > MaxHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested > MinCoolSetpointLimit - DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::MaxHeatSetpointLimit:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!HeatSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinHeatSetpointLimit || requested < MinHeatSetpointLimit || requested > AbsMaxHeatSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested > MaxCoolSetpointLimit - DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::MinCoolSetpointLimit:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinCoolSetpointLimit || requested > MaxCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested < MinHeatSetpointLimit + DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::MaxCoolSetpointLimit:
        requested = static_cast<int16_t>(chip::Encoding::LittleEndian::Get16(value));
        if (!CoolSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < AbsMinCoolSetpointLimit || requested < MinCoolSetpointLimit || requested > AbsMaxCoolSetpointLimit)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        else if (AutoSupported)
        {
            if (requested < MaxHeatSetpointLimit + DeadBandTemp)
                return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::MinSetpointDeadBand:
        requested = *value;
        if (!AutoSupported)
            return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        else if (requested < 0 || requested > 25)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::ControlSequenceOfOperation:
        uint8_t requestedCSO;
        requestedCSO = *value;
        if (requestedCSO > EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_AND_HEATING_WITH_REHEAT)
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        return EMBER_ZCL_STATUS_SUCCESS;

    case Ids::SystemMode: {
        uint8_t ControlSequenceOfOperation = kInvalidControlSequenceOfOperation;
        uint8_t RequestedSystemMode        = kInvalidRequestedSystemMode;
        GetControlSequenceOfOperation(endpoint, &ControlSequenceOfOperation);
        RequestedSystemMode = *value;
        if (ControlSequenceOfOperation > EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_AND_HEATING_WITH_REHEAT ||
            RequestedSystemMode > EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY)
        {
            return EMBER_ZCL_STATUS_INVALID_VALUE;
        }
        else
        {
            switch (ControlSequenceOfOperation)
            {
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_ONLY:
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_COOLING_WITH_REHEAT:
                if (RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_HEAT ||
                    RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_EMERGENCY_HEATING)
                    return EMBER_ZCL_STATUS_INVALID_VALUE;
                else
                    return EMBER_ZCL_STATUS_SUCCESS;

            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_HEATING_ONLY:
            case EMBER_ZCL_THERMOSTAT_CONTROL_SEQUENCE_HEATING_WITH_REHEAT:
                if (RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_COOL ||
                    RequestedSystemMode == EMBER_ZCL_THERMOSTAT_SYSTEM_MODE_PRECOOLING)
                    return EMBER_ZCL_STATUS_INVALID_VALUE;
                else
                    return EMBER_ZCL_STATUS_SUCCESS;
            default:
                return EMBER_ZCL_STATUS_SUCCESS;
            }
        }
    }
    default:
        return EMBER_ZCL_STATUS_SUCCESS;
    }
}

bool emberAfThermostatClusterClearWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj)
{
    // TODO
    return false;
}
bool emberAfThermostatClusterGetRelayStatusLogCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterGetWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                       uint8_t daysToReturn, uint8_t modeToReturn)
{
    // TODO
    return false;
}

bool emberAfThermostatClusterSetWeeklyScheduleCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                       uint8_t numberOfTransitionsForSequence, uint8_t daysOfWeekForSequence,
                                                       uint8_t modeForSequence, uint8_t * payload)
{
    // TODO
    return false;
}

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
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used
    EmberAfStatus status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = GetAbsMinHeatSetpointLimit(endpoint, &AbsMinHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMinHeatSetpointLimit missing using default");
    }

    status = GetAbsMaxHeatSetpointLimit(endpoint, &AbsMaxHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMaxHeatSetpointLimit missing using default");
    }
    status = GetMinHeatSetpointLimit(endpoint, &MinHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MinHeatSetpointLimit missing using default");
        MinHeatSetpointLimit = AbsMinHeatSetpointLimit;
    }

    status = GetMaxHeatSetpointLimit(endpoint, &MaxHeatSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MaxHeatSetpointLimit missing using AbsMaxHeatSetpointLimit");
        MaxHeatSetpointLimit = AbsMaxHeatSetpointLimit;
    }

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
    // if they are not present emberAfReadAttribute() will not update the value so the defaults are used
    EmberAfStatus status;

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/3724
    // behavior is not specified when Abs * values are not present and user values are present
    // implemented behavior accepts the user values without regard to default Abs values.

    // Per global matter data model policy
    // if a attribute is not present then it's default shall be used.

    status = GetAbsMinCoolSetpointLimit(endpoint, &AbsMinCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMinCoolSetpointLimit missing using default");
    }

    status = GetAbsMaxCoolSetpointLimit(endpoint, &AbsMaxCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: AbsMaxCoolSetpointLimit missing using default");
    }

    status = GetMinCoolSetpointLimit(endpoint, &MinCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MinCoolSetpointLimit missing using AbsMinCoolSetpointLimit");
        MinCoolSetpointLimit = AbsMinCoolSetpointLimit;
    }

    status = GetMaxCoolSetpointLimit(endpoint, &MaxCoolSetpointLimit);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MaxCoolSetpointLimit missing using AbsMaxCoolSetpointLimit");
        MaxCoolSetpointLimit = AbsMaxCoolSetpointLimit;
    }

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
bool emberAfThermostatClusterSetpointRaiseLowerCallback(EndpointId aEndpointId, chip::app::CommandHandler * commandObj,
                                                        uint8_t mode, int8_t amount)
{
    int16_t HeatingSetpoint = kDefaultHeatingSetpoint, CoolingSetpoint = kDefaultCoolingSetpoint; // Set to defaults to be safe
    EmberAfStatus status                     = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus WriteCoolingSetpointStatus = EMBER_ZCL_STATUS_FAILURE;
    EmberAfStatus WriteHeatingSetpointStatus = EMBER_ZCL_STATUS_FAILURE;
    bool AutoSupported                       = false;
    bool HeatSupported                       = false;
    bool CoolSupported                       = false;
    int16_t DeadBandTemp                     = 0;
    int8_t DeadBand                          = 0;
    uint32_t FeatureMap                      = 0;

    // TODO re-enable reading reaturemap once https://github.com/project-chip/connectedhomeip/pull/9725 is merged
#ifndef FEATURE_MAP_OVERIDE
    emberAfReadServerAttribute(endpoint, Thermostat::Id, chip::app::Clusters::Globals::Attributes::Ids::FeatureMap,
                               (uint8_t *) &FeatureMap, sizeof(FeatureMap));
#else
    FeatureMap = FEATURE_MAP_OVERIDE;
#endif

    if (FeatureMap & 1 << 5) // Bit 5 is Auto Mode supported
    {
        AutoSupported = true;
        if (GetMinSetpointDeadBand(aEndpointId, &DeadBand) != EMBER_ZCL_STATUS_SUCCESS)
            DeadBand = kDefaultDeadBand;
        DeadBandTemp = static_cast<int16_t>(DeadBand * 10);
    }
    if (FeatureMap & 1 << 0)
        HeatSupported = true;

    if (FeatureMap & 1 << 1)
        CoolSupported = true;
    switch (mode)
    {
    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_AND_COOL_SETPOINTS:
        if (HeatSupported && CoolSupported)
        {
            int16_t DesiredCoolingSetpoint, CoolLimit, DesiredHeatingSetpoint, HeatLimit;
            if (GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
            {
                DesiredCoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolLimit              = static_cast<int16_t>(DesiredCoolingSetpoint -
                                                 EnforceCoolingSetpointLimits(DesiredCoolingSetpoint, aEndpointId));
                {
                    if (GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
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
                            WriteCoolingSetpointStatus = SetOccupiedCoolingSetpoint(aEndpointId, DesiredCoolingSetpoint);
                            if (WriteCoolingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
                            {
                                ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            WriteHeatingSetpointStatus = SetOccupiedHeatingSetpoint(aEndpointId, DesiredHeatingSetpoint);
                            if (WriteHeatingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
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
            if (GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
            {
                CoolingSetpoint            = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint            = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                WriteCoolingSetpointStatus = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
                if (WriteCoolingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                }
            }
        }

        if (HeatSupported && !CoolSupported)
        {
            if (GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
            {
                HeatingSetpoint            = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint            = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                WriteHeatingSetpointStatus = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
                if (WriteHeatingSetpointStatus != EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                }
            }
        }

        if ((!HeatSupported || WriteHeatingSetpointStatus == EMBER_ZCL_STATUS_SUCCESS) &&
            (!CoolSupported || WriteCoolingSetpointStatus == EMBER_ZCL_STATUS_SUCCESS))
            status = EMBER_ZCL_STATUS_SUCCESS;
        break;

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_COOL_SETPOINT:
        if (CoolSupported)
        {
            if (GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
            {
                CoolingSetpoint = static_cast<int16_t>(CoolingSetpoint + amount * 10);
                CoolingSetpoint = EnforceCoolingSetpointLimits(CoolingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
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
                                if (SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
                                {
                                    if (SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
                                        status = EMBER_ZCL_STATUS_SUCCESS;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedHeatingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust heating setpoint to maintain dead band!");
                                status = EMBER_ZCL_STATUS_INVALID_ARGUMENT;
                            }
                        }
                        else
                            status = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
                }
                else
                {
                    status = SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
        }
        else
            status = EMBER_ZCL_STATUS_INVALID_ARGUMENT;
        break;

    case EMBER_ZCL_SETPOINT_ADJUST_MODE_HEAT_SETPOINT:
        if (HeatSupported)
        {
            if (GetOccupiedHeatingSetpoint(aEndpointId, &HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
            {
                HeatingSetpoint = static_cast<int16_t>(HeatingSetpoint + amount * 10);
                HeatingSetpoint = EnforceHeatingSetpointLimits(HeatingSetpoint, aEndpointId);
                if (AutoSupported)
                {
                    // Need to check if we can move the cooling setpoint while maintaining the dead band
                    if (GetOccupiedCoolingSetpoint(aEndpointId, &CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
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
                                if (SetOccupiedCoolingSetpoint(aEndpointId, CoolingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
                                {
                                    if (SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint) == EMBER_ZCL_STATUS_SUCCESS)
                                        status = EMBER_ZCL_STATUS_SUCCESS;
                                }
                                else
                                    ChipLogError(Zcl, "Error: SetOccupiedCoolingSetpoint failed!");
                            }
                            else
                            {
                                ChipLogError(Zcl, "Error: Could Not adjust cooling setpoint to maintain dead band!");
                                status = EMBER_ZCL_STATUS_INVALID_ARGUMENT;
                            }
                        }
                        else
                            status = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
                    }
                    else
                        ChipLogError(Zcl, "Error: GetOccupiedCoolingSetpoint failed!");
                }
                else
                {
                    status = SetOccupiedHeatingSetpoint(aEndpointId, HeatingSetpoint);
                }
            }
            else
                ChipLogError(Zcl, "Error: GetOccupiedHeatingSetpoint failed!");
        }
        else
            status = EMBER_ZCL_STATUS_INVALID_ARGUMENT;
        break;

    default:
        status = EMBER_ZCL_STATUS_INVALID_ARGUMENT;
        break;
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
