#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import matter.clusters as Clusters
from matter.interaction_model import Status

cluster = Clusters.Thermostat


class ThermostatState:
    def __init__(self, occupiedHeatingSetpoint, occupiedCoolingSetpoint, unoccupiedHeatingSetpoint, unoccupiedCoolingSetpoint, minHeatSetpointLimit, maxHeatSetpointLimit, minCoolSetpointLimit, maxCoolSetpointLimit,
                 absMinHeatSetpointLimit, absMaxHeatSetpointLimit, absMinCoolSetpointLimit, absMaxCoolSetpointLimit, minSetpointDeadBand,
                 hasHeat, hasCool, hasAuto, hasOccupancy, occupancy=1):
        self.occupiedHeatingSetpoint = occupiedHeatingSetpoint
        self.occupiedCoolingSetpoint = occupiedCoolingSetpoint
        self.unoccupiedHeatingSetpoint = unoccupiedHeatingSetpoint
        self.unoccupiedCoolingSetpoint = unoccupiedCoolingSetpoint
        self.minHeatSetpointLimit = minHeatSetpointLimit
        self.maxHeatSetpointLimit = maxHeatSetpointLimit
        self.minCoolSetpointLimit = minCoolSetpointLimit
        self.maxCoolSetpointLimit = maxCoolSetpointLimit

        self.absMinHeatSetpointLimit = absMinHeatSetpointLimit
        self.absMaxHeatSetpointLimit = absMaxHeatSetpointLimit
        self.absMinCoolSetpointLimit = absMinCoolSetpointLimit
        self.absMaxCoolSetpointLimit = absMaxCoolSetpointLimit
        self.minSetpointDeadBand = minSetpointDeadBand  # in hundredths of a degree C

        self.hasHeat = hasHeat
        self.hasCool = hasCool
        self.hasAuto = hasAuto
        self.hasOccupancy = hasOccupancy
        self.occupancy = occupancy  # bitmap, Bit 0 (1) is kOccupied

    def copy(self) -> 'ThermostatState':
        return ThermostatState(
            self.occupiedHeatingSetpoint, self.occupiedCoolingSetpoint, self.unoccupiedHeatingSetpoint, self.unoccupiedCoolingSetpoint, self.minHeatSetpointLimit, self.maxHeatSetpointLimit, self.minCoolSetpointLimit, self.maxCoolSetpointLimit,
            self.absMinHeatSetpointLimit, self.absMaxHeatSetpointLimit, self.absMinCoolSetpointLimit, self.absMaxCoolSetpointLimit, self.minSetpointDeadBand,
            self.hasHeat, self.hasCool, self.hasAuto, self.hasOccupancy, self.occupancy
        )

    def valid(self) -> bool:
        if self.hasHeat:
            if self.minHeatSetpointLimit is not None and self.maxHeatSetpointLimit is not None and self.minHeatSetpointLimit > self.maxHeatSetpointLimit:
                return False
            if self.occupiedHeatingSetpoint is not None and (self.occupiedHeatingSetpoint < (self.minHeatSetpointLimit if self.minHeatSetpointLimit is not None else self.absMinHeatSetpointLimit) or
                                                             self.occupiedHeatingSetpoint > (self.maxHeatSetpointLimit if self.maxHeatSetpointLimit is not None else self.absMaxHeatSetpointLimit)):
                return False
            if self.hasOccupancy and self.unoccupiedHeatingSetpoint is not None:
                if self.unoccupiedHeatingSetpoint < (self.minHeatSetpointLimit if self.minHeatSetpointLimit is not None else self.absMinHeatSetpointLimit) or \
                   self.unoccupiedHeatingSetpoint > (self.maxHeatSetpointLimit if self.maxHeatSetpointLimit is not None else self.absMaxHeatSetpointLimit):
                    return False
        if self.hasCool:
            if self.minCoolSetpointLimit is not None and self.maxCoolSetpointLimit is not None and self.minCoolSetpointLimit > self.maxCoolSetpointLimit:
                return False
            if self.occupiedCoolingSetpoint is not None and (self.occupiedCoolingSetpoint < (self.minCoolSetpointLimit if self.minCoolSetpointLimit is not None else self.absMinCoolSetpointLimit) or
                                                             self.occupiedCoolingSetpoint > (self.maxCoolSetpointLimit if self.maxCoolSetpointLimit is not None else self.absMaxCoolSetpointLimit)):
                return False
            if self.hasOccupancy and self.unoccupiedCoolingSetpoint is not None:
                if self.unoccupiedCoolingSetpoint < (self.minCoolSetpointLimit if self.minCoolSetpointLimit is not None else self.absMinCoolSetpointLimit) or \
                   self.unoccupiedCoolingSetpoint > (self.maxCoolSetpointLimit if self.maxCoolSetpointLimit is not None else self.absMaxCoolSetpointLimit):
                    return False
        if self.hasAuto:
            heat_max = self.maxHeatSetpointLimit if self.maxHeatSetpointLimit is not None else self.absMaxHeatSetpointLimit
            cool_max = self.maxCoolSetpointLimit if self.maxCoolSetpointLimit is not None else self.absMaxCoolSetpointLimit
            if cool_max - heat_max < self.minSetpointDeadBand:
                return False

            heat_min = self.minHeatSetpointLimit if self.minHeatSetpointLimit is not None else self.absMinHeatSetpointLimit
            cool_min = self.minCoolSetpointLimit if self.minCoolSetpointLimit is not None else self.absMinCoolSetpointLimit
            if cool_min - heat_min < self.minSetpointDeadBand:
                return False

            if self.occupiedCoolingSetpoint is not None and self.occupiedHeatingSetpoint is not None and self.occupiedCoolingSetpoint - self.occupiedHeatingSetpoint < self.minSetpointDeadBand:
                return False
            if self.hasOccupancy and self.unoccupiedCoolingSetpoint is not None and self.unoccupiedHeatingSetpoint is not None and self.unoccupiedCoolingSetpoint - self.unoccupiedHeatingSetpoint < self.minSetpointDeadBand:
                return False

        return True


class ThermostatSimulator:
    def __init__(self):
        pass

    def _attr_name(self, attribute_id):
        return {
            cluster.Attributes.OccupiedHeatingSetpoint.attribute_id: 'occupiedHeatingSetpoint',
            cluster.Attributes.OccupiedCoolingSetpoint.attribute_id: 'occupiedCoolingSetpoint',
            cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id: 'unoccupiedHeatingSetpoint',
            cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id: 'unoccupiedCoolingSetpoint',
            cluster.Attributes.MinHeatSetpointLimit.attribute_id: 'minHeatSetpointLimit',
            cluster.Attributes.MaxHeatSetpointLimit.attribute_id: 'maxHeatSetpointLimit',
            cluster.Attributes.MinCoolSetpointLimit.attribute_id: 'minCoolSetpointLimit',
            cluster.Attributes.MaxCoolSetpointLimit.attribute_id: 'maxCoolSetpointLimit',
            cluster.Attributes.MinSetpointDeadBand.attribute_id: 'minSetpointDeadBand'
        }[attribute_id]

    def _get_dirty_attributes(self, state: ThermostatState, committed_state: ThermostatState) -> set:
        dirty = set()
        for attr_id in [
            cluster.Attributes.OccupiedHeatingSetpoint.attribute_id,
            cluster.Attributes.OccupiedCoolingSetpoint.attribute_id,
            cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id,
            cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id,
            cluster.Attributes.MinHeatSetpointLimit.attribute_id,
            cluster.Attributes.MaxHeatSetpointLimit.attribute_id,
            cluster.Attributes.MinCoolSetpointLimit.attribute_id,
            cluster.Attributes.MaxCoolSetpointLimit.attribute_id
        ]:
            name = self._attr_name(attr_id)
            if getattr(state, name) != getattr(committed_state, name):
                dirty.add(attr_id)
        return dirty

    def fix(self, state: ThermostatState, changed_ids: set) -> tuple[Status, set]:
        fixed_ids = set()

        # 1. Fix User Limits
        if state.hasHeat:
            if state.maxHeatSetpointLimit is not None and state.minHeatSetpointLimit is not None and state.maxHeatSetpointLimit < state.minHeatSetpointLimit:
                if cluster.Attributes.MinHeatSetpointLimit.attribute_id in changed_ids:
                    state.maxHeatSetpointLimit = state.minHeatSetpointLimit
                    fixed_ids.add(cluster.Attributes.MaxHeatSetpointLimit.attribute_id)
                elif cluster.Attributes.MaxHeatSetpointLimit.attribute_id in changed_ids:
                    state.minHeatSetpointLimit = state.maxHeatSetpointLimit
                    fixed_ids.add(cluster.Attributes.MinHeatSetpointLimit.attribute_id)

        if state.hasCool:
            if state.maxCoolSetpointLimit is not None and state.minCoolSetpointLimit is not None and state.maxCoolSetpointLimit < state.minCoolSetpointLimit:
                if cluster.Attributes.MinCoolSetpointLimit.attribute_id in changed_ids:
                    state.maxCoolSetpointLimit = state.minCoolSetpointLimit
                    fixed_ids.add(cluster.Attributes.MaxCoolSetpointLimit.attribute_id)
                elif cluster.Attributes.MaxCoolSetpointLimit.attribute_id in changed_ids:
                    state.minCoolSetpointLimit = state.maxCoolSetpointLimit
                    fixed_ids.add(cluster.Attributes.MinCoolSetpointLimit.attribute_id)

        # 2. Fix User Limit Deadbands
        if state.hasAuto:
            self.fix_user_limit_deadband(state, changed_ids, fixed_ids, is_min=False)
            self.fix_user_limit_deadband(state, changed_ids, fixed_ids, is_min=True)

        # 3. Fix occupied and unoccupied ranges
        self.fix_range(state, changed_ids, fixed_ids, is_occupied=True)
        if state.hasOccupancy:
            self.fix_range(state, changed_ids, fixed_ids, is_occupied=False)

        changed_ids.update(fixed_ids)
        return (Status.Success if state.valid() else Status.ConstraintError, changed_ids)

    def fix_user_limit_deadband(self, state, changed_ids, fixed_ids, is_min):
        if is_min:
            heat_limit_id = cluster.Attributes.MinHeatSetpointLimit.attribute_id
            cool_limit_id = cluster.Attributes.MinCoolSetpointLimit.attribute_id
            heat_val = state.minHeatSetpointLimit
            cool_val = state.minCoolSetpointLimit
            abs_heat = state.absMinHeatSetpointLimit
            abs_cool = state.absMinCoolSetpointLimit
        else:
            heat_limit_id = cluster.Attributes.MaxHeatSetpointLimit.attribute_id
            cool_limit_id = cluster.Attributes.MaxCoolSetpointLimit.attribute_id
            heat_val = state.maxHeatSetpointLimit
            cool_val = state.maxCoolSetpointLimit
            abs_heat = state.absMaxHeatSetpointLimit
            abs_cool = state.absMaxCoolSetpointLimit

        eff_heat = heat_val if heat_val is not None else abs_heat
        eff_cool = cool_val if cool_val is not None else abs_cool

        if eff_cool - eff_heat >= state.minSetpointDeadBand:
            return

        # Violation! Check if the change was heating or cooling
        is_heating_changed = changed_ids.intersection({
            cluster.Attributes.MinHeatSetpointLimit.attribute_id,
            cluster.Attributes.MaxHeatSetpointLimit.attribute_id
        })
        is_cooling_changed = changed_ids.intersection({
            cluster.Attributes.MinCoolSetpointLimit.attribute_id,
            cluster.Attributes.MaxCoolSetpointLimit.attribute_id
        })

        if is_heating_changed:
            new_cool = eff_heat + state.minSetpointDeadBand
            if state.absMinCoolSetpointLimit <= new_cool <= state.absMaxCoolSetpointLimit:
                if is_min:
                    state.minCoolSetpointLimit = new_cool
                else:
                    state.maxCoolSetpointLimit = new_cool
                fixed_ids.add(cool_limit_id)
            else:
                if is_min:
                    state.minCoolSetpointLimit = abs_cool
                    state.minHeatSetpointLimit = abs_cool - state.minSetpointDeadBand
                else:
                    state.maxCoolSetpointLimit = abs_cool
                    state.maxHeatSetpointLimit = abs_cool - state.minSetpointDeadBand
                fixed_ids.add(cool_limit_id)
                fixed_ids.add(heat_limit_id)
        elif is_cooling_changed:
            new_heat = eff_cool - state.minSetpointDeadBand
            if state.absMinHeatSetpointLimit <= new_heat <= state.absMaxHeatSetpointLimit:
                if is_min:
                    state.minHeatSetpointLimit = new_heat
                else:
                    state.maxHeatSetpointLimit = new_heat
                fixed_ids.add(heat_limit_id)
            else:
                if is_min:
                    state.minHeatSetpointLimit = abs_heat
                    state.minCoolSetpointLimit = abs_heat + state.minSetpointDeadBand
                else:
                    state.maxHeatSetpointLimit = abs_heat
                    state.maxCoolSetpointLimit = abs_heat + state.minSetpointDeadBand
                fixed_ids.add(heat_limit_id)
                fixed_ids.add(cool_limit_id)

    def fix_range(self, state, changed_ids, fixed_ids, is_occupied):
        if is_occupied:
            heat_id = cluster.Attributes.OccupiedHeatingSetpoint.attribute_id
            cool_id = cluster.Attributes.OccupiedCoolingSetpoint.attribute_id
            heat_val = state.occupiedHeatingSetpoint
            cool_val = state.occupiedCoolingSetpoint
        else:
            heat_id = cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id
            cool_id = cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id
            heat_val = state.unoccupiedHeatingSetpoint
            cool_val = state.unoccupiedCoolingSetpoint

        # 1. Clamp to user limits
        if state.hasHeat:
            min_heat = state.minHeatSetpointLimit if state.minHeatSetpointLimit is not None else state.absMinHeatSetpointLimit
            max_heat = state.maxHeatSetpointLimit if state.maxHeatSetpointLimit is not None else state.absMaxHeatSetpointLimit
            if heat_val < min_heat or heat_val > max_heat:
                heat_val = min(max(heat_val, min_heat), max_heat)
                fixed_ids.add(heat_id)

        if state.hasCool:
            min_cool = state.minCoolSetpointLimit if state.minCoolSetpointLimit is not None else state.absMinCoolSetpointLimit
            max_cool = state.maxCoolSetpointLimit if state.maxCoolSetpointLimit is not None else state.absMaxCoolSetpointLimit
            if cool_val < min_cool or cool_val > max_cool:
                cool_val = min(max(cool_val, min_cool), max_cool)
                fixed_ids.add(cool_id)

        if is_occupied:
            state.occupiedHeatingSetpoint = heat_val
            state.occupiedCoolingSetpoint = cool_val
        else:
            state.unoccupiedHeatingSetpoint = heat_val
            state.unoccupiedCoolingSetpoint = cool_val

        if not state.hasAuto:
            return

        if cool_val - heat_val >= state.minSetpointDeadBand:
            return

        # Deadband violation!
        if (heat_id in changed_ids or
            cluster.Attributes.MinHeatSetpointLimit.attribute_id in changed_ids or
                cluster.Attributes.MaxHeatSetpointLimit.attribute_id in changed_ids):
            new_cool = heat_val + state.minSetpointDeadBand
            if min_cool <= new_cool <= max_cool:
                cool_val = new_cool
                fixed_ids.add(cool_id)
            else:
                cool_val = max_cool
                heat_val = max_cool - state.minSetpointDeadBand
                fixed_ids.add(cool_id)
                fixed_ids.add(heat_id)
        elif (cool_id in changed_ids or
              cluster.Attributes.MinCoolSetpointLimit.attribute_id in changed_ids or
              cluster.Attributes.MaxCoolSetpointLimit.attribute_id in changed_ids):
            new_heat = cool_val - state.minSetpointDeadBand
            if min_heat <= new_heat <= max_heat:
                heat_val = new_heat
                fixed_ids.add(heat_id)
            else:
                heat_val = min_heat
                cool_val = min_heat + state.minSetpointDeadBand
                fixed_ids.add(cool_id)
                fixed_ids.add(heat_id)

        if is_occupied:
            state.occupiedHeatingSetpoint = heat_val
            state.occupiedCoolingSetpoint = cool_val
        else:
            state.unoccupiedHeatingSetpoint = heat_val
            state.unoccupiedCoolingSetpoint = cool_val

    def write(self, current_state: ThermostatState, attribute_id, new_value) -> tuple[Status, ThermostatState, set]:
        # Check identical write (NO-OP check)
        current_val = getattr(current_state, self._attr_name(attribute_id))
        if current_val == new_value:
            return (Status.Success, current_state, set())

        # Range Check
        match attribute_id:
            case (cluster.Attributes.OccupiedHeatingSetpoint.attribute_id |
                  cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id):
                min_heat = current_state.minHeatSetpointLimit if current_state.minHeatSetpointLimit is not None else current_state.absMinHeatSetpointLimit
                max_heat = current_state.maxHeatSetpointLimit if current_state.maxHeatSetpointLimit is not None else current_state.absMaxHeatSetpointLimit
                if new_value < min_heat or new_value > max_heat:
                    return (Status.ConstraintError, current_state, set())
            case (cluster.Attributes.MinHeatSetpointLimit.attribute_id |
                  cluster.Attributes.MaxHeatSetpointLimit.attribute_id):
                if new_value < current_state.absMinHeatSetpointLimit or new_value > current_state.absMaxHeatSetpointLimit:
                    return (Status.ConstraintError, current_state, set())
            case (cluster.Attributes.OccupiedCoolingSetpoint.attribute_id |
                  cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id):
                min_cool = current_state.minCoolSetpointLimit if current_state.minCoolSetpointLimit is not None else current_state.absMinCoolSetpointLimit
                max_cool = current_state.maxCoolSetpointLimit if current_state.maxCoolSetpointLimit is not None else current_state.absMaxCoolSetpointLimit
                if new_value < min_cool or new_value > max_cool:
                    return (Status.ConstraintError, current_state, set())
            case (cluster.Attributes.MinCoolSetpointLimit.attribute_id |
                  cluster.Attributes.MaxCoolSetpointLimit.attribute_id):
                if new_value < current_state.absMinCoolSetpointLimit or new_value > current_state.absMaxCoolSetpointLimit:
                    return (Status.ConstraintError, current_state, set())
            case cluster.Attributes.MinSetpointDeadBand.attribute_id:
                if new_value < 0 or new_value > 127:
                    return (Status.ConstraintError, current_state, set())
                # MinSetpointDeadBand writes don't actually succeed, even though they get a Success response
                return (Status.Success, current_state, {cluster.Attributes.MinSetpointDeadBand.attribute_id})

        # Pre-change check copy
        sim_state = current_state.copy()
        setattr(sim_state, self._attr_name(attribute_id), new_value)

        sim_status, _ = self.fix(sim_state, {attribute_id})
        if sim_status != Status.Success:
            return (Status.ConstraintError, current_state, set())

        # Succeeded! Simulate committed store and pure specs-compliant resolution cascade
        sim_state = current_state.copy()
        setattr(sim_state, self._attr_name(attribute_id), new_value)

        changed_ids = {attribute_id}
        status, final_changed = self.fix(sim_state, changed_ids)
        if status != Status.Success:
            return (Status.ConstraintError, current_state, set())

        return (Status.Success, sim_state, final_changed)

    def raise_lower(self, current_state: ThermostatState, mode, amount) -> tuple[Status, ThermostatState, set]:
        changed_ids = set()
        is_occupied = not current_state.hasOccupancy or (current_state.occupancy & 1) != 0

        if is_occupied:
            heat_val = current_state.occupiedHeatingSetpoint
            cool_val = current_state.occupiedCoolingSetpoint
            heat_attr = cluster.Attributes.OccupiedHeatingSetpoint.attribute_id
            cool_attr = cluster.Attributes.OccupiedCoolingSetpoint.attribute_id
        else:
            heat_val = current_state.unoccupiedHeatingSetpoint
            cool_val = current_state.unoccupiedCoolingSetpoint
            heat_attr = cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id
            cool_attr = cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id

        amount_hundredths = amount * 10

        def adjust_setpoint(val, min_limit, max_limit, abs_min, abs_max):
            raw = val + amount_hundredths
            min_val = min_limit if min_limit is not None else abs_min
            max_val = max_limit if max_limit is not None else abs_max
            return min(max(raw, min_val), max_val)

        match mode:
            case cluster.Enums.SetpointRaiseLowerModeEnum.kHeat:
                if current_state.hasHeat:
                    heat_val = adjust_setpoint(heat_val, current_state.minHeatSetpointLimit, current_state.maxHeatSetpointLimit,
                                               current_state.absMinHeatSetpointLimit, current_state.absMaxHeatSetpointLimit)
                    changed_ids.add(heat_attr)
            case cluster.Enums.SetpointRaiseLowerModeEnum.kCool:
                if current_state.hasCool:
                    cool_val = adjust_setpoint(cool_val, current_state.minCoolSetpointLimit, current_state.maxCoolSetpointLimit,
                                               current_state.absMinCoolSetpointLimit, current_state.absMaxCoolSetpointLimit)
                    changed_ids.add(cool_attr)
            case cluster.Enums.SetpointRaiseLowerModeEnum.kBoth:
                if current_state.hasHeat:
                    heat_val = adjust_setpoint(heat_val, current_state.minHeatSetpointLimit, current_state.maxHeatSetpointLimit,
                                               current_state.absMinHeatSetpointLimit, current_state.absMaxHeatSetpointLimit)
                    changed_ids.add(heat_attr)
                if current_state.hasCool:
                    cool_val = adjust_setpoint(cool_val, current_state.minCoolSetpointLimit, current_state.maxCoolSetpointLimit,
                                               current_state.absMinCoolSetpointLimit, current_state.absMaxCoolSetpointLimit)
                    changed_ids.add(cool_attr)

        new_state = current_state.copy()
        if is_occupied:
            new_state.occupiedHeatingSetpoint = heat_val
            new_state.occupiedCoolingSetpoint = cool_val
        else:
            new_state.unoccupiedHeatingSetpoint = heat_val
            new_state.unoccupiedCoolingSetpoint = cool_val

        status, final_changed = self.fix(new_state, changed_ids)
        if status != Status.Success:
            # Revert state transition by returning the unchanged state and empty changed set
            return (Status.Success, current_state, set())

        return (Status.Success, new_state, final_changed)

    def get_expected_events(self, old_state: ThermostatState, new_state: ThermostatState, changed_ids: set, written_attribute_id) -> list[dict]:
        events = []
        if new_state.hasHeat:
            if cluster.Attributes.OccupiedHeatingSetpoint.attribute_id in changed_ids and (new_state.occupiedHeatingSetpoint != old_state.occupiedHeatingSetpoint or written_attribute_id == cluster.Attributes.OccupiedHeatingSetpoint.attribute_id):
                events.append({
                    'systemMode': cluster.Enums.SystemModeEnum.kHeat,
                    'occupancy': cluster.Bitmaps.OccupancyBitmap.kOccupied,
                    'currentSetpoint': new_state.occupiedHeatingSetpoint
                })
        if new_state.hasCool:
            if cluster.Attributes.OccupiedCoolingSetpoint.attribute_id in changed_ids and (new_state.occupiedCoolingSetpoint != old_state.occupiedCoolingSetpoint or written_attribute_id == cluster.Attributes.OccupiedCoolingSetpoint.attribute_id):
                events.append({
                    'systemMode': cluster.Enums.SystemModeEnum.kCool,
                    'occupancy': cluster.Bitmaps.OccupancyBitmap.kOccupied,
                    'currentSetpoint': new_state.occupiedCoolingSetpoint
                })
        if new_state.hasHeat and new_state.hasOccupancy:
            if cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id in changed_ids and (new_state.unoccupiedHeatingSetpoint != old_state.unoccupiedHeatingSetpoint or written_attribute_id == cluster.Attributes.UnoccupiedHeatingSetpoint.attribute_id):
                events.append({
                    'systemMode': cluster.Enums.SystemModeEnum.kHeat,
                    'occupancy': 0,
                    'currentSetpoint': new_state.unoccupiedHeatingSetpoint
                })
        if new_state.hasCool and new_state.hasOccupancy:
            if cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id in changed_ids and (new_state.unoccupiedCoolingSetpoint != old_state.unoccupiedCoolingSetpoint or written_attribute_id == cluster.Attributes.UnoccupiedCoolingSetpoint.attribute_id):
                events.append({
                    'systemMode': cluster.Enums.SystemModeEnum.kCool,
                    'occupancy': 0,
                    'currentSetpoint': new_state.unoccupiedCoolingSetpoint
                })
        return events
