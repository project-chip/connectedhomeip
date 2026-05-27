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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "Setpoint.h"
#include "SetpointAttributes.h"
#include "SetpointLimits.h"
#include "SetpointRange.h"
#include "Temperature.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints
{
public:
    enum class ClampMode : uint8_t
    {
        kDontClamp,
        kClamp,
    };

    bool autoSupported      = false;
    bool heatSupported      = false;
    bool coolSupported      = false;
    bool occupancySupported = false;
    bool eventsSupported    = false;

    AbsoluteSetpointLimits absoluteHeatLimits;
    AbsoluteSetpointLimits absoluteCoolLimits;

    UserSetpointLimits userHeatLimits;
    UserSetpointLimits userCoolLimits;

    SetpointRange occupied;
    SetpointRange unoccupied;

    temperature deadBand;

    Setpoints() :
        absoluteHeatLimits(AbsoluteSetpoint(Attributes::AbsMinHeatSetpointLimit::Id, kDefaultAbsMinHeatSetpointLimit),
                           AbsoluteSetpoint(Attributes::AbsMaxHeatSetpointLimit::Id, kDefaultAbsMaxHeatSetpointLimit)),
        absoluteCoolLimits(AbsoluteSetpoint(Attributes::AbsMinCoolSetpointLimit::Id, kDefaultAbsMinCoolSetpointLimit),
                           AbsoluteSetpoint(Attributes::AbsMaxCoolSetpointLimit::Id, kDefaultAbsMaxCoolSetpointLimit)),
        userHeatLimits(OptionalSetpoint(Attributes::MinHeatSetpointLimit::Id, absoluteHeatLimits.minimum),
                       OptionalSetpoint(Attributes::MaxHeatSetpointLimit::Id, absoluteHeatLimits.maximum), absoluteHeatLimits),
        userCoolLimits(OptionalSetpoint(Attributes::MinCoolSetpointLimit::Id, absoluteCoolLimits.minimum),
                       OptionalSetpoint(Attributes::MaxCoolSetpointLimit::Id, absoluteCoolLimits.maximum), absoluteCoolLimits),
        occupied(AbsoluteSetpoint(Attributes::OccupiedHeatingSetpoint::Id, kDefaultHeatingSetpoint),
                 AbsoluteSetpoint(Attributes::OccupiedCoolingSetpoint::Id, kDefaultCoolingSetpoint)),
        unoccupied(AbsoluteSetpoint(Attributes::UnoccupiedHeatingSetpoint::Id, kDefaultHeatingSetpoint),
                   AbsoluteSetpoint(Attributes::UnoccupiedCoolingSetpoint::Id, kDefaultCoolingSetpoint)),
        deadBand(kDefaultDeadBand)
    {}

    Setpoints(const Setpoints & spl) :
        autoSupported(spl.autoSupported), heatSupported(spl.heatSupported), coolSupported(spl.coolSupported),
        occupancySupported(spl.occupancySupported), eventsSupported(spl.eventsSupported),
        absoluteHeatLimits(spl.absoluteHeatLimits), absoluteCoolLimits(spl.absoluteCoolLimits),
        userHeatLimits(spl.userHeatLimits, absoluteHeatLimits), userCoolLimits(spl.userCoolLimits, absoluteCoolLimits),
        occupied(spl.occupied), unoccupied(spl.unoccupied), deadBand(spl.deadBand)
    {}

    /*
    Checks to make sure that the setpoints follow the rules from the Matter spec
    */
    bool Valid();

    /*
    Get the appropriate setpoint range, based on occupancy.
    */
    SetpointRange & GetRange(chip::app::Clusters::Thermostat::OccupancyBitmap occupancy);

    /*
    Get the appropriate setpoint limits, based on mode
    */
    SetpointLimits<AbsoluteSetpoint> GetLimits(chip::app::Clusters::Thermostat::SystemModeEnum mode);

    /*
     * Change the heating value of a given setpoint range.
     *
     * @param range The setpoint range to modify.
     * @param heat The new heating setpoint value to set for the given range.
     * @param clamp Whether to clamp the setpoint range.
     * @param changedAttributes The set of attributes changed by this operation.
     * @return The status of the operation.
     */
    Protocols::InteractionModel::Status ChangeRangeHeating(SetpointRange & range, temperature heat, ClampMode clamp,
                                                           SetpointAttributes & changedAttributes);

    /**
     * Change the cooling value of a given setpoint range.
     *
     * @param range The setpoint range to modify.
     * @param cool The new cooling setpoint value to set for the given range.
     * @param clamp Whether to clamp the setpoint range.
     * @param changedAttributes The set of attributes changed by this operation.
     * @return The status of the operation.
     */
    Protocols::InteractionModel::Status ChangeRangeCooling(SetpointRange & range, temperature cool, ClampMode clamp,
                                                           SetpointAttributes & changedAttributes);

    /**
     * Change either or both of the values of a given setpoint range.
     *
     * @param range The setpoint range to modify.
     * @param heat The optional new heating setpoint value to set for the given range.
     * @param cool The optional new cooling setpoint value to set for the given range.
     * @param clamp Whether to clamp the setpoint range to the limits associated with it.
     * @param changedAttributes The set of attributes changed by this operation.
     * @return The status of the operation.
     */
    Protocols::InteractionModel::Status ChangeRange(SetpointRange & range, Optional<temperature> heat, Optional<temperature> cool,
                                                    ClampMode clamp, SetpointAttributes & changedAttributes);

    /**
     * Change the minimum value of a given setpoint limit
     *
     * @param limits The setpoint limits to modify.
     * @param min The new minimum setpoint value to set for the given limits.
     * @param changedAttributes The set of attributes changed by this operation.
     * @return The status of the operation.
     */
    Protocols::InteractionModel::Status ChangeLimitMinimum(UserSetpointLimits & limits, temperature min,
                                                           SetpointAttributes & changedAttributes);

    /**
     * Change the maximum value of a given setpoint limit
     *
     * @param limits The setpoint limits to modify.
     * @param max The new maximum setpoint value to set for the given limits.
     * @param changedAttributes The set of attributes changed by this operation.
     * @return The status of the operation.
     */
    Protocols::InteractionModel::Status ChangeLimitMaximum(UserSetpointLimits & limits, temperature max,
                                                           SetpointAttributes & changedAttributes);

    /**
     * Attempt to fix any violations of the setpoint rules
     * @param changedAttributes The set of attributes that were changed prior to this operation.
     * @return The status of the operation; Success if the setpoints are now valid, ConstraintError if it was not possible to fix
     * them
     */
    Protocols::InteractionModel::Status Fix(SetpointAttributes & changedAttributes);

private:
    /*
    Change either the min or max value of a setpoint limits
    @param limits The setpoint limits to modify.
    @param min The new minimum setpoint value to set for the given limits.
    @param max The new maximum setpoint value to set for the given limits.
    @param changedAttributes The set of attributes changed by this operation.
    @return The status of the operation; Success if the setpoints are now valid, ConstraintError if it was not possible to fix them
    */
    Protocols::InteractionModel::Status ChangeLimits(UserSetpointLimits & limits, Optional<temperature> min,
                                                     Optional<temperature> max, SetpointAttributes & changedAttributes);

    /*
    Attempt to fix the user setpoint limits to comply with the deadband
    @param heatLimit The heating setpoint limit to modify.
    @param coolLimit The cooling setpoint limit to modify.
    @param absoluteHeatLimit The absolute heat setpoint limit.
    @param absoluteCoolLimit The absolute cooling setpoint limit.
    @param changedAttributes The set of attributes changed before this operation.
    @param fixedAttributes The set of attributes fixed by this operation.
    */
    void FixUserLimitDeadband(OptionalSetpoint & heatLimit, OptionalSetpoint & coolLimit, temperature absoluteHeatLimit,
                              temperature absoluteCoolLimit, SetpointAttributes & changedAttributes,
                              SetpointAttributes & fixedAttributes);

    /*
    Attempt to fix a given user setpoint limits to comply with the absolute limits and the deadband
    @param absoluteLimits The absolute setpoint limits that apply to the user setpoint limits.
    @param userLimits The user setpoint limits to modify.
    @param changedAttributes The set of attributes changed before this operation.
    @param fixedAttributes The set of attributes fixed by this operation.
    */
    void FixUserLimits(AbsoluteSetpointLimits & absoluteLimits, UserSetpointLimits & userLimits,
                       SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes);

    /*
    Attempt to fix a given setpoint range to comply with all setpoint rules
    @param range The setpoint range to modify.
    @param changedAttributes The set of attributes changed before this operation.
    @param fixedAttributes The set of attributes fixed by this operation.
    */
    void FixRange(SetpointRange & range, SetpointAttributes & changedAttributes, SetpointAttributes & fixedAttributes);
};



} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
