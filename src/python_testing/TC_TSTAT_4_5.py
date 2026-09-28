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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import copy
import logging
import random

from mobly import asserts
from TC_TSTAT_Utils import ThermostatBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Thermostat

ALL_DAYS_MASK = (
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kSunday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kWednesday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kThursday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kFriday
    | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kSaturday
)

INDIVIDUAL_DAYS = [
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kSunday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kWednesday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kThursday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kFriday,
    cluster.Bitmaps.ScheduleDayOfWeekBitmap.kSaturday,
]


class TC_TSTAT_4_5(ThermostatBaseTest):
    """Test case for Thermostat Schedules (MSCH) feature on Thermostat cluster."""

    def check_schedule_types_attribute(
        self,
        schedule_types: list,
        number_of_schedules: int,
        has_heat: bool,
        has_cool: bool,
        has_auto: bool,
        has_presets: bool,
    ) -> dict[int, cluster.Structs.ScheduleTypeStruct]:
        """Validates the ScheduleTypes attribute list and returns a mapping of SystemMode to ScheduleTypeStruct."""
        asserts.assert_true(isinstance(schedule_types, list), "ScheduleTypes attribute must be a list")
        asserts.assert_greater_equal(len(schedule_types), 1, "ScheduleTypes attribute must contain at least one entry")

        allowed_modes = set()
        if has_auto:
            allowed_modes.add(cluster.Enums.SystemModeEnum.kAuto)
        if has_cool:
            allowed_modes.add(cluster.Enums.SystemModeEnum.kCool)
        if has_heat:
            allowed_modes.add(cluster.Enums.SystemModeEnum.kHeat)

        features_mask = (
            cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets
            | cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints
            | cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsNames
            | cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsOff
        )
        presets_or_setpoints_mask = (
            cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets
            | cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints
        )

        schedule_type_by_mode: dict[int, cluster.Structs.ScheduleTypeStruct] = {}
        for idx, entry in enumerate(schedule_types):
            asserts.assert_in(
                entry.systemMode,
                (
                    cluster.Enums.SystemModeEnum.kAuto,
                    cluster.Enums.SystemModeEnum.kCool,
                    cluster.Enums.SystemModeEnum.kHeat,
                ),
                f"ScheduleTypeStruct {idx} has invalid SystemMode {entry.systemMode}",
            )
            asserts.assert_in(
                entry.systemMode,
                allowed_modes,
                f"ScheduleTypeStruct {idx} SystemMode {entry.systemMode} does not match supported DUT features",
            )
            asserts.assert_not_in(
                entry.systemMode,
                schedule_type_by_mode,
                f"ScheduleTypeStruct {idx} has duplicate SystemMode {entry.systemMode}",
            )
            asserts.assert_true(
                isinstance(entry.numberOfSchedules, int),
                f"ScheduleTypeStruct {idx} NumberOfSchedules must be an integer",
            )
            asserts.assert_greater_equal(
                entry.numberOfSchedules, 1, f"ScheduleTypeStruct {idx} NumberOfSchedules must be >= 1"
            )
            asserts.assert_less_equal(
                entry.numberOfSchedules,
                number_of_schedules,
                f"ScheduleTypeStruct {idx} NumberOfSchedules ({entry.numberOfSchedules}) exceeds NumberOfSchedules ({number_of_schedules})",
            )
            asserts.assert_equal(
                entry.scheduleTypeFeatures & ~features_mask,
                0,
                f"ScheduleTypeStruct {idx} ScheduleTypeFeatures has unknown bits set: 0x{entry.scheduleTypeFeatures:x}",
            )
            asserts.assert_greater(
                entry.scheduleTypeFeatures & presets_or_setpoints_mask,
                0,
                f"ScheduleTypeStruct {idx} ScheduleTypeFeatures must have SupportsPresets or SupportsSetpoints set",
            )
            if entry.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets:
                asserts.assert_true(
                    has_presets,
                    f"ScheduleTypeStruct {idx} has SupportsPresets set, but PRES feature is not supported",
                )
            schedule_type_by_mode[entry.systemMode] = entry

        return schedule_type_by_mode

    def check_schedules_attribute(
        self,
        schedules: list,
        number_of_schedules: int,
        number_of_schedule_transitions: int,
        number_of_schedule_transition_per_day: int | Nullable,
        schedule_type_by_mode: dict[int, cluster.Structs.ScheduleTypeStruct],
        preset_handles: set[bytes],
        min_heat_limit: int,
        max_heat_limit: int,
        min_cool_limit: int,
        max_cool_limit: int,
        min_deadband: int | None,
    ) -> set[bytes]:
        """Validates the Schedules attribute list and returns the set of ScheduleHandles."""
        asserts.assert_true(isinstance(schedules, list), "Schedules attribute must be a list")
        asserts.assert_less_equal(
            len(schedules),
            number_of_schedules,
            f"Schedules length ({len(schedules)}) exceeds NumberOfSchedules ({number_of_schedules})",
        )

        schedule_handles: set[bytes] = set()
        mode_counts: dict[int, int] = {}

        for idx, schedule in enumerate(schedules):
            asserts.assert_true(
                schedule.scheduleHandle is not NullValue and isinstance(schedule.scheduleHandle, bytes),
                f"Schedule {idx} ScheduleHandle must be non-null bytes",
            )
            asserts.assert_greater(len(schedule.scheduleHandle), 0, f"Schedule {idx} ScheduleHandle must not be empty")
            asserts.assert_less_equal(
                len(schedule.scheduleHandle), 16, f"Schedule {idx} ScheduleHandle exceeds 16 bytes"
            )
            asserts.assert_not_in(
                schedule.scheduleHandle, schedule_handles, f"Schedule {idx} ScheduleHandle is duplicate"
            )
            schedule_handles.add(schedule.scheduleHandle)

            asserts.assert_in(
                schedule.systemMode,
                schedule_type_by_mode,
                f"Schedule {idx} SystemMode {schedule.systemMode} not found in ScheduleTypes",
            )
            schedule_type = schedule_type_by_mode[schedule.systemMode]
            st_features = schedule_type.scheduleTypeFeatures
            mode_counts[schedule.systemMode] = mode_counts.get(schedule.systemMode, 0) + 1
            asserts.assert_less_equal(
                mode_counts[schedule.systemMode],
                schedule_type.numberOfSchedules,
                f"Schedule count for SystemMode {schedule.systemMode} exceeds ScheduleTypeStruct.NumberOfSchedules",
            )

            if schedule.name is not None and schedule.name is not NullValue:
                asserts.assert_true(isinstance(schedule.name, str), f"Schedule {idx} Name must be a string")
                asserts.assert_less_equal(
                    len(schedule.name.encode("utf-8")), 64, f"Schedule {idx} Name exceeds 64 bytes"
                )
                asserts.assert_true(
                    bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsNames),
                    f"Schedule {idx} has Name set, but ScheduleTypeStruct does not support names",
                )

            if schedule.presetHandle is not None and schedule.presetHandle is not NullValue:
                asserts.assert_true(
                    isinstance(schedule.presetHandle, bytes), f"Schedule {idx} PresetHandle must be bytes"
                )
                asserts.assert_less_equal(
                    len(schedule.presetHandle), 16, f"Schedule {idx} PresetHandle exceeds 16 bytes"
                )
                asserts.assert_true(
                    bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets),
                    f"Schedule {idx} has PresetHandle set, but ScheduleTypeStruct does not support presets",
                )
                asserts.assert_in(
                    schedule.presetHandle,
                    preset_handles,
                    f"Schedule {idx} PresetHandle {schedule.presetHandle} not found in Presets",
                )

            asserts.assert_true(
                schedule.builtIn is not NullValue and isinstance(schedule.builtIn, bool),
                f"Schedule {idx} BuiltIn must be a non-null boolean",
            )

            asserts.assert_true(
                isinstance(schedule.transitions, list), f"Schedule {idx} Transitions must be a list"
            )
            asserts.assert_greater_equal(
                len(schedule.transitions), 1, f"Schedule {idx} Transitions must have at least 1 entry"
            )
            asserts.assert_less_equal(
                len(schedule.transitions),
                number_of_schedule_transitions,
                f"Schedule {idx} Transitions length ({len(schedule.transitions)}) exceeds NumberOfScheduleTransitions ({number_of_schedule_transitions})",
            )

            for t_idx, transition in enumerate(schedule.transitions):
                asserts.assert_false(
                    bool(transition.dayOfWeek & cluster.Bitmaps.ScheduleDayOfWeekBitmap.kAway),
                    f"Schedule {idx} Transition {t_idx} DayOfWeek has Away bit set",
                )
                asserts.assert_greater(
                    transition.dayOfWeek, 0, f"Schedule {idx} Transition {t_idx} DayOfWeek must be non-zero"
                )
                asserts.assert_equal(
                    transition.dayOfWeek & ~ALL_DAYS_MASK,
                    0,
                    f"Schedule {idx} Transition {t_idx} DayOfWeek has invalid bits set",
                )
                asserts.assert_greater_equal(
                    transition.transitionTime, 0, f"Schedule {idx} Transition {t_idx} TransitionTime must be >= 0"
                )
                asserts.assert_less_equal(
                    transition.transitionTime, 1439, f"Schedule {idx} Transition {t_idx} TransitionTime exceeds 1439"
                )

                for prev_idx in range(t_idx):
                    prev_t = schedule.transitions[prev_idx]
                    if prev_t.transitionTime == transition.transitionTime:
                        asserts.assert_equal(
                            prev_t.dayOfWeek & transition.dayOfWeek,
                            0,
                            f"Schedule {idx} has duplicate transitions at time {transition.transitionTime} with overlapping DayOfWeek",
                        )

                has_t_preset = transition.presetHandle is not None and transition.presetHandle is not NullValue
                has_t_mode = transition.systemMode is not None and transition.systemMode is not NullValue
                has_t_cool = transition.coolingSetpoint is not None and transition.coolingSetpoint is not NullValue
                has_t_heat = transition.heatingSetpoint is not None and transition.heatingSetpoint is not NullValue

                if has_t_preset:
                    asserts.assert_true(
                        isinstance(transition.presetHandle, bytes),
                        f"Schedule {idx} Transition {t_idx} PresetHandle must be bytes",
                    )
                    asserts.assert_less_equal(
                        len(transition.presetHandle),
                        16,
                        f"Schedule {idx} Transition {t_idx} PresetHandle exceeds 16 bytes",
                    )
                    asserts.assert_true(
                        bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets),
                        f"Schedule {idx} Transition {t_idx} has PresetHandle, but ScheduleTypeStruct does not support presets",
                    )
                    asserts.assert_in(
                        transition.presetHandle,
                        preset_handles,
                        f"Schedule {idx} Transition {t_idx} PresetHandle not found in Presets",
                    )
                    asserts.assert_false(
                        has_t_mode or has_t_cool or has_t_heat,
                        f"Schedule {idx} Transition {t_idx} specifies both PresetHandle and SystemMode/Setpoints",
                    )
                else:
                    asserts.assert_true(
                        bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints),
                        f"Schedule {idx} Transition {t_idx} does not specify PresetHandle, so ScheduleTypeStruct must support setpoints",
                    )
                    if has_t_mode:
                        asserts.assert_not_equal(
                            transition.systemMode,
                            schedule.systemMode,
                            f"Schedule {idx} Transition {t_idx} SystemMode must not equal ScheduleStruct.SystemMode",
                        )
                        asserts.assert_in(
                            transition.systemMode,
                            (
                                cluster.Enums.SystemModeEnum.kOff,
                                cluster.Enums.SystemModeEnum.kAuto,
                                cluster.Enums.SystemModeEnum.kCool,
                                cluster.Enums.SystemModeEnum.kHeat,
                            ),
                            f"Schedule {idx} Transition {t_idx} SystemMode {transition.systemMode} is invalid",
                        )
                        if transition.systemMode == cluster.Enums.SystemModeEnum.kOff:
                            asserts.assert_true(
                                bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsOff),
                                f"Schedule {idx} Transition {t_idx} has SystemMode Off, but ScheduleTypeStruct does not support Off",
                            )
                        else:
                            asserts.assert_in(
                                transition.systemMode,
                                schedule_type_by_mode,
                                f"Schedule {idx} Transition {t_idx} SystemMode {transition.systemMode} is not supported by DUT",
                            )

                    effective_mode = transition.systemMode if has_t_mode else schedule.systemMode
                    if effective_mode == cluster.Enums.SystemModeEnum.kOff:
                        asserts.assert_false(
                            has_t_cool or has_t_heat,
                            f"Schedule {idx} Transition {t_idx} has SystemMode Off and must not specify setpoints",
                        )
                    elif effective_mode == cluster.Enums.SystemModeEnum.kHeat:
                        asserts.assert_true(
                            has_t_heat,
                            f"Schedule {idx} Transition {t_idx} effective mode is Heat and must specify HeatingSetpoint",
                        )
                        asserts.assert_false(
                            has_t_cool,
                            f"Schedule {idx} Transition {t_idx} effective mode is Heat and must not specify CoolingSetpoint",
                        )
                        asserts.assert_greater_equal(
                            transition.heatingSetpoint,
                            min_heat_limit,
                            f"Schedule {idx} Transition {t_idx} HeatingSetpoint below min limit",
                        )
                        asserts.assert_less_equal(
                            transition.heatingSetpoint,
                            max_heat_limit,
                            f"Schedule {idx} Transition {t_idx} HeatingSetpoint above max limit",
                        )
                    elif effective_mode == cluster.Enums.SystemModeEnum.kCool:
                        asserts.assert_true(
                            has_t_cool,
                            f"Schedule {idx} Transition {t_idx} effective mode is Cool and must specify CoolingSetpoint",
                        )
                        asserts.assert_false(
                            has_t_heat,
                            f"Schedule {idx} Transition {t_idx} effective mode is Cool and must not specify HeatingSetpoint",
                        )
                        asserts.assert_greater_equal(
                            transition.coolingSetpoint,
                            min_cool_limit,
                            f"Schedule {idx} Transition {t_idx} CoolingSetpoint below min limit",
                        )
                        asserts.assert_less_equal(
                            transition.coolingSetpoint,
                            max_cool_limit,
                            f"Schedule {idx} Transition {t_idx} CoolingSetpoint above max limit",
                        )
                    elif effective_mode == cluster.Enums.SystemModeEnum.kAuto:
                        asserts.assert_true(
                            has_t_heat and has_t_cool,
                            f"Schedule {idx} Transition {t_idx} effective mode is Auto and must specify both HeatingSetpoint and CoolingSetpoint",
                        )
                        asserts.assert_greater_equal(
                            transition.heatingSetpoint,
                            min_heat_limit,
                            f"Schedule {idx} Transition {t_idx} HeatingSetpoint below min limit",
                        )
                        asserts.assert_less_equal(
                            transition.heatingSetpoint,
                            max_heat_limit,
                            f"Schedule {idx} Transition {t_idx} HeatingSetpoint above max limit",
                        )
                        asserts.assert_greater_equal(
                            transition.coolingSetpoint,
                            min_cool_limit,
                            f"Schedule {idx} Transition {t_idx} CoolingSetpoint below min limit",
                        )
                        asserts.assert_less_equal(
                            transition.coolingSetpoint,
                            max_cool_limit,
                            f"Schedule {idx} Transition {t_idx} CoolingSetpoint above max limit",
                        )
                        if min_deadband is not None:
                            asserts.assert_greater_equal(
                                transition.coolingSetpoint - transition.heatingSetpoint,
                                min_deadband,
                                f"Schedule {idx} Transition {t_idx} violates MinSetpointDeadBand",
                            )

            if number_of_schedule_transition_per_day is not NullValue and number_of_schedule_transition_per_day is not None:
                for day_bit in INDIVIDUAL_DAYS:
                    day_count = sum(1 for t in schedule.transitions if t.dayOfWeek & day_bit)
                    asserts.assert_less_equal(
                        day_count,
                        number_of_schedule_transition_per_day,
                        f"Schedule {idx} has {day_count} transitions on day bit 0x{day_bit:x}, exceeding NumberOfScheduleTransitionPerDay ({number_of_schedule_transition_per_day})",
                    )

        return schedule_handles

    def make_valid_transition(
        self,
        schedule_type: cluster.Structs.ScheduleTypeStruct,
        heat_setpoint: int,
        cool_setpoint: int,
        day_of_week: int = ALL_DAYS_MASK,
        transition_time: int = 360,
        preset_handle: bytes | None = None,
    ) -> cluster.Structs.ScheduleTransitionStruct:
        """Creates a valid ScheduleTransitionStruct for the given ScheduleTypeStruct."""
        st_features = schedule_type.scheduleTypeFeatures
        supports_setpoints = bool(st_features & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints)

        if supports_setpoints:
            if schedule_type.systemMode == cluster.Enums.SystemModeEnum.kHeat:
                return cluster.Structs.ScheduleTransitionStruct(
                    dayOfWeek=day_of_week,
                    transitionTime=transition_time,
                    heatingSetpoint=heat_setpoint,
                )
            if schedule_type.systemMode == cluster.Enums.SystemModeEnum.kCool:
                return cluster.Structs.ScheduleTransitionStruct(
                    dayOfWeek=day_of_week,
                    transitionTime=transition_time,
                    coolingSetpoint=cool_setpoint,
                )
            return cluster.Structs.ScheduleTransitionStruct(
                dayOfWeek=day_of_week,
                transitionTime=transition_time,
                heatingSetpoint=heat_setpoint,
                coolingSetpoint=cool_setpoint,
            )

        asserts.assert_is_not_none(
            preset_handle,
            f"ScheduleTypeStruct for mode {schedule_type.systemMode} only supports presets, but no preset handle was available",
        )
        return cluster.Structs.ScheduleTransitionStruct(
            dayOfWeek=day_of_week,
            transitionTime=transition_time,
            presetHandle=preset_handle,
        )

    def make_valid_schedule(
        self,
        schedule_type: cluster.Structs.ScheduleTypeStruct,
        heat_setpoint: int,
        cool_setpoint: int,
        schedule_handle: bytes | Nullable = NullValue,
        built_in: bool | Nullable = False,
        name: str | None = None,
        preset_handle: bytes | None = None,
        transitions: list[cluster.Structs.ScheduleTransitionStruct] | None = None,
    ) -> cluster.Structs.ScheduleStruct:
        """Creates a valid ScheduleStruct for the given ScheduleTypeStruct."""
        if transitions is None:
            transitions = [
                self.make_valid_transition(
                    schedule_type=schedule_type,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    day_of_week=ALL_DAYS_MASK,
                    transition_time=360,
                    preset_handle=preset_handle,
                )
            ]
        return cluster.Structs.ScheduleStruct(
            scheduleHandle=schedule_handle,
            systemMode=schedule_type.systemMode,
            name=name,
            presetHandle=preset_handle if not (schedule_type.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints) else None,
            transitions=transitions,
            builtIn=built_in,
        )

    async def write_schedules(
        self,
        endpoint: int,
        schedules: list,
        dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None,
        expected_status: Status | list[Status] = Status.Success,
    ) -> Status:
        """Writes to the Schedules attribute and verifies the returned status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        try:
            result = await dev_ctrl.WriteAttribute(
                self.dut_node_id, [(endpoint, cluster.Attributes.Schedules(schedules))]
            )
            status = result[0].Status
        except InteractionModelError as e:
            status = e.status

        if isinstance(expected_status, list):
            asserts.assert_in(
                status,
                expected_status,
                f"Schedules write returned {status.name}; expected one of {[s.name for s in expected_status]}",
            )
        else:
            asserts.assert_equal(
                status,
                expected_status,
                f"Schedules write returned {status.name}; expected {expected_status.name}",
            )
        return status

    async def write_schedules_and_expect_error(
        self,
        endpoint: int,
        schedules: list,
        expected_status: Status,
    ) -> None:
        """Performs an atomic write of invalid schedules, verifying the expected error on WriteAttribute or CommitWrite, and rolls back if needed."""
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        try:
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id, [(endpoint, cluster.Attributes.Schedules(schedules))]
            )
            status = result[0].Status
        except InteractionModelError as e:
            status = e.status

        if status == Status.Success:
            # If the server deferred validation of the pending schedules to pre-commit, verify CommitWrite fails with expected_status.
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: expected_status},
                endpoint=endpoint,
                expected_atomic_status=Status.Failure,
            )
        else:
            asserts.assert_equal(
                status,
                expected_status,
                f"Schedules write returned {status.name}; expected {expected_status.name}",
            )
            await self.send_atomic_request_rollback(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )

    async def write_presets(
        self,
        endpoint: int,
        presets: list,
        dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None,
        expected_status: Status = Status.Success,
    ) -> Status:
        """Writes to the Presets attribute and verifies the returned status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        try:
            result = await dev_ctrl.WriteAttribute(
                self.dut_node_id, [(endpoint, cluster.Attributes.Presets(presets))]
            )
            status = result[0].Status
        except InteractionModelError as e:
            status = e.status
        asserts.assert_equal(
            status,
            expected_status,
            f"Presets write returned {status.name}; expected {expected_status.name}",
        )
        return status

    async def send_set_active_schedule_request(
        self,
        endpoint: int,
        schedule_handle: bytes,
        expected_status: Status = Status.Success,
    ) -> None:
        """Sends the SetActiveScheduleRequest command and verifies the expected status."""
        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.SetActiveScheduleRequest(scheduleHandle=schedule_handle),
                endpoint=endpoint,
            )
            asserts.assert_equal(
                expected_status,
                Status.Success,
                f"Expected SetActiveScheduleRequest to fail with {expected_status.name}, but it succeeded",
            )
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                expected_status,
                f"SetActiveScheduleRequest returned {e.status.name}; expected {expected_status.name}",
            )

    def generate_unused_handle(self, existing_handles: set[bytes]) -> bytes:
        """Generates a random 4-byte handle not present in existing_handles."""
        candidate = b"\xff\xff\xff\xff"
        while candidate in existing_handles:
            candidate = bytes([random.randint(0, 255) for _ in range(4)])
        return candidate

    def desc_TC_TSTAT_4_5(self) -> str:
        """Returns a description of this test."""
        return "[TC-TSTAT-4.5] Thermostat Schedules Test Cases with server as DUT"

    def pics_TC_TSTAT_4_5(self) -> list[str]:
        """Returns a list of PICS for this test case that must be True for the test to be run."""
        return ["TSTAT.S", "TSTAT.S.F07"]

    def steps_TC_TSTAT_4_5(self) -> list[TestStep]:
        """Returns the list of test steps for TC-TSTAT-4.5."""
        return [
            TestStep("1", "Commission DUT to TH", is_commissioning=True),
            TestStep(
                "2a",
                "TH reads the FeatureMap attribute.",
                "Verify that the MSCH bit is set in the FeatureMap value.",
            ),
            TestStep(
                "2b",
                "TH reads the NumberOfSchedules attribute.",
                "Verify that the read returns a uint8 value >= 1. Save the value in a NumberOfSchedules variable.",
            ),
            TestStep(
                "2c",
                "TH reads the NumberOfScheduleTransitions attribute.",
                "Verify that the read returns a uint8 value >= 1. Save the value in a NumberOfScheduleTransitions variable.",
            ),
            TestStep(
                "2d",
                "TH reads the NumberOfScheduleTransitionPerDay attribute.",
                "Verify that the read returns either null or a uint8 value >= 1. Save the value in a NumberOfScheduleTransitionPerDay variable.",
            ),
            TestStep(
                "2e",
                "TH reads the ScheduleTypes attribute.",
                "Verify that the read returns a list of ScheduleTypeStruct entries with at least one entry, with each entry having a unique SystemMode value that is supported by the DUT. "
                "Verify SystemMode, NumberOfSchedules, and ScheduleTypeFeatures constraints. Save the list in a SupportedScheduleTypes variable.",
            ),
            TestStep(
                "2f",
                "TH reads the Schedules attribute (and Presets if PRES is supported).",
                "Verify that the read returns a list of ScheduleStruct entries whose length is less than or equal to NumberOfSchedules, and that each ScheduleStruct and ScheduleTransitionStruct satisfies all specification constraints. "
                "Save the list in a CurrentSchedules variable.",
            ),
            TestStep(
                "2g",
                "TH reads the ActiveScheduleHandle attribute.",
                "Verify that the read returns either null or an octstr value (max 16 bytes) that matches the ScheduleHandle of an entry in CurrentSchedules. Save the value in a PreviousScheduleHandle variable.",
            ),
            TestStep(
                "3a",
                "TH selects a ScheduleHandle from CurrentSchedules (choosing one different from PreviousScheduleHandle if available) and sends the SetActiveScheduleRequest command with ScheduleHandle set to the chosen handle. "
                "TH reads the ActiveScheduleHandle attribute and stores the value in a CurrentScheduleHandle variable.",
                "Verify that the SetActiveScheduleRequest command returns SUCCESS and ActiveScheduleHandle is equal to the sent ScheduleHandle.",
            ),
            TestStep(
                "3b",
                "If the ScheduleHandle chosen in step 3a differed from PreviousScheduleHandle, TH reads the ActiveScheduleChange event from the DUT.",
                "Verify that ActiveScheduleChange event has a new event record with the CurrentScheduleHandle field matching CurrentScheduleHandle and the PreviousScheduleHandle field matching PreviousScheduleHandle (or omitted if PreviousScheduleHandle was unavailable).",
            ),
            TestStep(
                "3c",
                "TH sends the SetActiveScheduleRequest command with ScheduleHandle set to a random octstr value that does not match any entry in Schedules. "
                "TH reads the ActiveScheduleHandle attribute.",
                "Verify that the SetActiveScheduleRequest command returns INVALID_COMMAND (0x85) and ActiveScheduleHandle remains equal to CurrentScheduleHandle.",
            ),
            TestStep(
                "4a",
                "TH writes to the Schedules attribute without calling the AtomicRequest command.",
                "Verify that the write request returns INVALID_IN_STATE (0xcb) since the client did not start an atomic write by calling AtomicRequest with BeginWrite.",
            ),
            TestStep(
                "4b",
                "TH calls AtomicRequest with RequestType set to BeginWrite targeting the Schedules attribute. "
                "TH writes to the Schedules attribute modifying a transition on an existing schedule with valid values, but does not call AtomicRequest with CommitWrite; instead TH calls AtomicRequest with RequestType set to RollbackWrite targeting the Schedules attribute. "
                "TH reads the Schedules attribute.",
                "Verify that the AtomicRequest commands return SUCCESS, the edit request is rolled back, and the Schedules attribute matches the original unmodified schedules.",
            ),
            TestStep(
                "4c",
                "TH calls AtomicRequest with RequestType set to BeginWrite targeting the Schedules attribute. "
                "TH writes to the Schedules attribute modifying a transition on an existing schedule with different valid values (and setting the BuiltIn field on the modified schedule to null). "
                "TH calls AtomicRequest with RequestType set to CommitWrite targeting the Schedules attribute. "
                "TH reads the Schedules attribute.",
                "Verify that the AtomicRequest commands return SUCCESS, the Schedules attribute is updated with the modified transition, and the BuiltIn field on the modified schedule retains its previous value.",
            ),
            TestStep(
                "4d",
                "If the number of entries in Schedules is less than NumberOfSchedules and there is a ScheduleTypeStruct in SupportedScheduleTypes whose NumberOfSchedules is greater than the current number of schedules with that SystemMode: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules appending a new ScheduleStruct with ScheduleHandle set to null, BuiltIn set to null (or false), SystemMode set to the chosen ScheduleTypeStruct's SystemMode, and valid Transitions, calls AtomicRequest (CommitWrite), and reads Schedules.",
                "Verify that the AtomicRequest commands return SUCCESS, and the Schedules attribute contains the newly added schedule with a unique device-generated ScheduleHandle and BuiltIn set to false. Save the new handle as AddedScheduleHandle.",
            ),
            TestStep(
                "4e",
                "If step 4d added a schedule: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules removing the schedule with ScheduleHandle equal to AddedScheduleHandle, calls AtomicRequest (CommitWrite), and reads Schedules.",
                "Verify that the AtomicRequest commands return SUCCESS and the schedule with AddedScheduleHandle is removed from Schedules.",
            ),
            TestStep(
                "4f",
                "If step 4e removed a schedule: "
                "TH repeats the atomic write and commit from step 4d to add a new schedule with ScheduleHandle set to null, reads Schedules to inspect the newly assigned ScheduleHandle, and then removes the added schedule via an atomic write and commit.",
                "Verify that the device assigns a unique ScheduleHandle to the new schedule and does not reuse the deleted AddedScheduleHandle.",
            ),
            TestStep(
                "4g",
                "TH starts an atomic write on Schedules by calling AtomicRequest (BeginWrite), and a second client TH2 attempts to open an atomic write on Schedules before TH is complete.",
                "Verify that TH2's AtomicRequest is rejected.",
            ),
            TestStep(
                "4h",
                "While TH's atomic write on Schedules is open, TH2 attempts to write to the Schedules attribute. TH then calls AtomicRequest (RollbackWrite) to close its atomic write.",
                "Verify that TH2's write request is rejected with INVALID_IN_STATE (0xcb).",
            ),
            TestStep(
                "4i",
                "TH starts an atomic write on Schedules, and before it is complete, TH2 removes TH's fabric; TH2 then opens an atomic write on Schedules and rolls it back (re-commissioning TH afterwards if needed).",
                "Verify that TH2's AtomicRequest is successful.",
            ),
            TestStep(
                "5a",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a new schedule (ScheduleHandle set to null) having BuiltIn set to true, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "5b",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule having a non-null ScheduleHandle that does not exist in Schedules, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns NOT_FOUND (0x8b).",
            ),
            TestStep(
                "5c",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with duplicate schedules sharing the same ScheduleHandle, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "5d",
                "TH selects a schedule in Schedules with BuiltIn set to false (adding one first if none exists), calls AtomicRequest (BeginWrite), writes to Schedules modifying that non-built-in schedule to have BuiltIn set to true, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "5e",
                "TH selects a schedule in Schedules with BuiltIn set to true, calls AtomicRequest (BeginWrite), writes to Schedules modifying that built-in schedule to have BuiltIn set to false, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "5f",
                "TH selects a schedule in Schedules with BuiltIn set to true, calls AtomicRequest (BeginWrite), writes to Schedules with that built-in schedule removed, and calls AtomicRequest (CommitWrite).",
                "Verify that AtomicRequest (CommitWrite) returns CONSTRAINT_ERROR (0x87) and the built-in schedule is not removed from Schedules.",
            ),
            TestStep(
                "5g",
                "TH ensures ActiveScheduleHandle is set to a non-null schedule handle (creating a non-built-in schedule and activating it if needed), calls AtomicRequest (BeginWrite), writes to Schedules removing the schedule whose ScheduleHandle matches ActiveScheduleHandle, and calls AtomicRequest (CommitWrite).",
                "Verify that AtomicRequest (CommitWrite) returns INVALID_IN_STATE (0xcb) and the active schedule is not removed from Schedules.",
            ),
            TestStep(
                "5h",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule whose SystemMode is not present in ScheduleTypes (such as Off or an unsupported SystemMode), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "6a",
                "If any ScheduleTypeStruct in SupportedScheduleTypes has the SupportsNames bit set in ScheduleTypeFeatures: "
                "TH writes a valid Name (<= 64 chars) and commits, then writes an invalid Name (> 64 chars) and rolls back.",
                "Verify that setting a valid Name succeeds and Schedules reflects the updated Name, and setting a Name longer than 64 characters returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "6b",
                "If any ScheduleTypeStruct in SupportedScheduleTypes does not have the SupportsNames bit set in ScheduleTypeFeatures: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule of that SystemMode having the Name field set, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "6c",
                "If any ScheduleTypeStruct in SupportedScheduleTypes does not have the SupportsPresets bit set in ScheduleTypeFeatures: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule of that SystemMode having PresetHandle set on the ScheduleStruct or on a ScheduleTransitionStruct, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "6d",
                "If any ScheduleTypeStruct in SupportedScheduleTypes has the SupportsPresets bit set in ScheduleTypeFeatures: "
                "TH tests valid PresetHandle references, non-existent ScheduleStruct.PresetHandle, non-existent ScheduleTransitionStruct.PresetHandle, and removing a Preset referenced by a ScheduleTransitionStruct.",
                "Verify that writing a valid PresetHandle succeeds, non-existent PresetHandles return CONSTRAINT_ERROR (0x87), and committing the removal of a Preset referenced by a ScheduleTransitionStruct returns INVALID_IN_STATE (0xcb).",
            ),
            TestStep(
                "6e",
                "If any ScheduleTypeStruct in SupportedScheduleTypes does not have the SupportsSetpoints bit set in ScheduleTypeFeatures: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule of that SystemMode containing a transition that specifies SystemMode, HeatingSetpoint, or CoolingSetpoint, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "6f",
                "For a ScheduleTypeStruct in SupportedScheduleTypes that has the SupportsSetpoints bit set in ScheduleTypeFeatures: "
                "TH tests writing a transition with SystemMode set to Off.",
                "If SupportsOff is not set, verify that the write request returns CONSTRAINT_ERROR (0x87). If SupportsOff is set, verify that the AtomicRequest commands return SUCCESS and Schedules reflects the transition with SystemMode set to Off.",
            ),
            TestStep(
                "7a",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule whose Transitions list is empty (0 entries), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7b",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a transition where DayOfWeek has the Away/Vacation bit (bit 7) set, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7c",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a transition where TransitionTime is greater than 1439 (e.g. 1440), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7d",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule containing duplicate transitions (multiple transitions with the exact same TransitionTime and overlapping DayOfWeek bits), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7e",
                "If any ScheduleTypeStruct in SupportedScheduleTypes has the SupportsPresets bit set: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a transition on a schedule of that SystemMode that specifies both PresetHandle and at least one of SystemMode, CoolingSetpoint, or HeatingSetpoint, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7f",
                "If any ScheduleTypeStruct in SupportedScheduleTypes has the SupportsSetpoints bit set: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a transition where ScheduleTransitionStruct.SystemMode is set to the same value as the encompassing ScheduleStruct.SystemMode, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "7g",
                "If any ScheduleTypeStruct in SupportedScheduleTypes has the SupportsSetpoints bit set: "
                "TH tests omitting required setpoint and preset fields for the transition's effective SystemMode, and writing a transition setpoint outside the configured setpoint limits.",
                "Verify that omitting required setpoint and preset fields returns CONSTRAINT_ERROR (0x87), and writing a transition setpoint outside configured setpoint limits returns CONSTRAINT_ERROR (0x87).",
            ),
            TestStep(
                "8a",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule containing NumberOfScheduleTransitions + 1 transitions, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns RESOURCE_EXHAUSTED (0x89).",
            ),
            TestStep(
                "8b",
                "If NumberOfScheduleTransitionPerDay is not null and NumberOfScheduleTransitionPerDay < NumberOfScheduleTransitions: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules with a schedule containing NumberOfScheduleTransitionPerDay + 1 transitions that have the same day-of-week bit set in DayOfWeek, and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns RESOURCE_EXHAUSTED (0x89).",
            ),
            TestStep(
                "8c",
                "If there is a ScheduleTypeStruct in SupportedScheduleTypes whose NumberOfSchedules field is less than the NumberOfSchedules attribute: "
                "TH calls AtomicRequest (BeginWrite), writes to Schedules adding valid schedules with that SystemMode such that the number of schedules with that SystemMode exceeds ScheduleTypeStruct.NumberOfSchedules (while total schedules <= NumberOfSchedules), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns RESOURCE_EXHAUSTED (0x89).",
            ),
            TestStep(
                "8d",
                "TH calls AtomicRequest (BeginWrite), writes to Schedules such that the total number of schedules exceeds the NumberOfSchedules attribute (NumberOfSchedules + 1), and calls AtomicRequest (RollbackWrite).",
                "Verify that the write request returns RESOURCE_EXHAUSTED (0x89).",
            ),
        ]

    @async_test_body
    async def test_TC_TSTAT_4_5(self) -> None:
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2a")
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.FeatureMap
        )
        log.info("FeatureMap: 0x%08x", feature_map)
        has_msch = bool(feature_map & cluster.Bitmaps.Feature.kMatterScheduleConfiguration)
        if not has_msch:
            log.warning(
                "MSCH bit (bit 7, 0x80) is not set in FeatureMap on endpoint %d. Skipping steps 2b-8d.",
                endpoint,
            )
            self.mark_step_range_skipped("2b", "8d")
            return

        has_heat = bool(feature_map & cluster.Bitmaps.Feature.kHeating)
        has_cool = bool(feature_map & cluster.Bitmaps.Feature.kCooling)
        has_auto = bool(feature_map & cluster.Bitmaps.Feature.kAutoMode)
        has_presets = bool(feature_map & cluster.Bitmaps.Feature.kPresets)
        has_events = bool(feature_map & cluster.Bitmaps.Feature.kEvents)

        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AttributeList
        )

        min_heat_limit = 700
        max_heat_limit = 3000
        min_cool_limit = 1600
        max_cool_limit = 3200
        min_deadband: int | None = None

        if has_heat:
            if (
                cluster.Attributes.MinHeatSetpointLimit.attribute_id in attribute_list
                and cluster.Attributes.MaxHeatSetpointLimit.attribute_id in attribute_list
            ):
                min_heat_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit
                )
                max_heat_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit
                )
            elif (
                cluster.Attributes.AbsMinHeatSetpointLimit.attribute_id in attribute_list
                and cluster.Attributes.AbsMaxHeatSetpointLimit.attribute_id in attribute_list
            ):
                min_heat_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit
                )
                max_heat_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxHeatSetpointLimit
                )

        if has_cool:
            if (
                cluster.Attributes.MinCoolSetpointLimit.attribute_id in attribute_list
                and cluster.Attributes.MaxCoolSetpointLimit.attribute_id in attribute_list
            ):
                min_cool_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit
                )
                max_cool_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit
                )
            elif (
                cluster.Attributes.AbsMinCoolSetpointLimit.attribute_id in attribute_list
                and cluster.Attributes.AbsMaxCoolSetpointLimit.attribute_id in attribute_list
            ):
                min_cool_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinCoolSetpointLimit
                )
                max_cool_limit = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit
                )

        if has_auto and cluster.Attributes.MinSetpointDeadBand.attribute_id in attribute_list:
            deadband_attr = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand
            )
            min_deadband = deadband_attr * 10

        effective_deadband = min_deadband if min_deadband is not None else 200
        heat_setpoint = min_heat_limit + ((max_heat_limit - min_heat_limit) // 2)
        cool_setpoint = min_cool_limit + ((max_cool_limit - min_cool_limit) // 2)
        if has_auto and (cool_setpoint - heat_setpoint) < effective_deadband:
            heat_setpoint = max(min_heat_limit, cool_setpoint - effective_deadband)
            if (cool_setpoint - heat_setpoint) < effective_deadband:
                cool_setpoint = min(max_cool_limit, heat_setpoint + effective_deadband)

        self.step("2b")
        number_of_schedules = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfSchedules
        )
        log.info("NumberOfSchedules: %s", number_of_schedules)
        asserts.assert_true(isinstance(number_of_schedules, int), "NumberOfSchedules must be an integer")
        asserts.assert_greater_equal(number_of_schedules, 1, "NumberOfSchedules must be >= 1")
        asserts.assert_less_equal(number_of_schedules, 255, "NumberOfSchedules must fit in uint8")

        self.step("2c")
        number_of_schedule_transitions = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfScheduleTransitions
        )
        log.info("NumberOfScheduleTransitions: %s", number_of_schedule_transitions)
        asserts.assert_true(
            isinstance(number_of_schedule_transitions, int),
            "NumberOfScheduleTransitions must be an integer",
        )
        asserts.assert_greater_equal(
            number_of_schedule_transitions, 1, "NumberOfScheduleTransitions must be >= 1"
        )
        asserts.assert_less_equal(
            number_of_schedule_transitions, 255, "NumberOfScheduleTransitions must fit in uint8"
        )

        self.step("2d")
        number_of_schedule_transition_per_day = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfScheduleTransitionPerDay
        )
        log.info("NumberOfScheduleTransitionPerDay: %s", number_of_schedule_transition_per_day)
        if number_of_schedule_transition_per_day is not NullValue:
            asserts.assert_true(
                isinstance(number_of_schedule_transition_per_day, int),
                "NumberOfScheduleTransitionPerDay must be null or an integer",
            )
            asserts.assert_greater_equal(
                number_of_schedule_transition_per_day, 1, "NumberOfScheduleTransitionPerDay must be >= 1"
            )
            asserts.assert_less_equal(
                number_of_schedule_transition_per_day,
                255,
                "NumberOfScheduleTransitionPerDay must fit in uint8",
            )

        self.step("2e")
        supported_schedule_types = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ScheduleTypes
        )
        log.info("ScheduleTypes: %s", supported_schedule_types)
        schedule_type_by_mode = self.check_schedule_types_attribute(
            schedule_types=supported_schedule_types,
            number_of_schedules=number_of_schedules,
            has_heat=has_heat,
            has_cool=has_cool,
            has_auto=has_auto,
            has_presets=has_presets,
        )

        self.step("2f")
        current_presets: list[cluster.Structs.PresetStruct] = []
        preset_handles: set[bytes] = set()
        if has_presets:
            current_presets = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets
            )
            log.info("Presets: %s", current_presets)
            preset_handles = {
                p.presetHandle
                for p in current_presets
                if p.presetHandle is not NullValue and isinstance(p.presetHandle, bytes)
            }

        current_schedules = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
        )
        log.info("Schedules: %s", current_schedules)
        schedule_handles = self.check_schedules_attribute(
            schedules=current_schedules,
            number_of_schedules=number_of_schedules,
            number_of_schedule_transitions=number_of_schedule_transitions,
            number_of_schedule_transition_per_day=number_of_schedule_transition_per_day,
            schedule_type_by_mode=schedule_type_by_mode,
            preset_handles=preset_handles,
            min_heat_limit=min_heat_limit,
            max_heat_limit=max_heat_limit,
            min_cool_limit=min_cool_limit,
            max_cool_limit=max_cool_limit,
            min_deadband=min_deadband,
        )

        default_preset_handle = next(iter(preset_handles), None)

        self.step("2g")
        previous_schedule_handle = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActiveScheduleHandle
        )
        log.info("ActiveScheduleHandle: %s", previous_schedule_handle)
        if previous_schedule_handle is not NullValue:
            asserts.assert_true(
                isinstance(previous_schedule_handle, bytes),
                "ActiveScheduleHandle must be null or bytes",
            )
            asserts.assert_less_equal(
                len(previous_schedule_handle), 16, "ActiveScheduleHandle exceeds 16 bytes"
            )
            asserts.assert_in(
                previous_schedule_handle,
                schedule_handles,
                "ActiveScheduleHandle does not match any ScheduleHandle in Schedules",
            )

        # Ensure there is at least one schedule in CurrentSchedules for step 3a
        if len(current_schedules) == 0:
            first_st = supported_schedule_types[0]
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            new_sched = self.make_valid_schedule(
                schedule_type=first_st,
                heat_setpoint=heat_setpoint,
                cool_setpoint=cool_setpoint,
                preset_handle=default_preset_handle,
            )
            await self.write_schedules(endpoint=endpoint, schedules=[new_sched])
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            current_schedules = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            schedule_handles = {s.scheduleHandle for s in current_schedules}

        self.step("3a")
        existing_events = []
        if has_events:
            event_path = [(endpoint, cluster.Events.ActiveScheduleChange, 1)]
            existing_events = await self.default_controller.ReadEvent(
                nodeId=self.dut_node_id, events=event_path
            )

        candidate_handles = [
            s.scheduleHandle
            for s in current_schedules
            if s.scheduleHandle != previous_schedule_handle
        ]
        chosen_handle = (
            candidate_handles[0]
            if len(candidate_handles) > 0
            else current_schedules[0].scheduleHandle
        )
        await self.send_set_active_schedule_request(
            endpoint=endpoint, schedule_handle=chosen_handle, expected_status=Status.Success
        )
        current_schedule_handle = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActiveScheduleHandle
        )
        asserts.assert_equal(
            current_schedule_handle,
            chosen_handle,
            "ActiveScheduleHandle was not updated to the chosen ScheduleHandle",
        )

        self.step("3b")
        if has_events and chosen_handle != previous_schedule_handle:
            event_path = [(endpoint, cluster.Events.ActiveScheduleChange, 1)]
            if existing_events:
                max_event_num = max(e.Header.EventNumber for e in existing_events)
                new_events = await self.default_controller.ReadEvent(
                    nodeId=self.dut_node_id,
                    events=event_path,
                    eventNumberFilter=max_event_num + 1,
                )
            else:
                new_events = await self.default_controller.ReadEvent(
                    nodeId=self.dut_node_id, events=event_path
                )
            asserts.assert_greater_equal(
                len(new_events),
                1,
                "Expected at least one new ActiveScheduleChange event record",
            )
            latest_event = new_events[-1].Data
            asserts.assert_equal(
                latest_event.currentScheduleHandle,
                current_schedule_handle,
                "ActiveScheduleChange event CurrentScheduleHandle mismatch",
            )
            if previous_schedule_handle is NullValue:
                asserts.assert_in(
                    latest_event.previousScheduleHandle,
                    (NullValue, None),
                    "ActiveScheduleChange event PreviousScheduleHandle should be null or omitted when previously null",
                )
            else:
                asserts.assert_equal(
                    latest_event.previousScheduleHandle,
                    previous_schedule_handle,
                    "ActiveScheduleChange event PreviousScheduleHandle mismatch",
                )
        else:
            log.info(
                "Skipping ActiveScheduleChange event verification in step 3b (has_events=%s, handle_changed=%s)",
                has_events,
                chosen_handle != previous_schedule_handle,
            )

        self.step("3c")
        invalid_schedule_handle = self.generate_unused_handle(schedule_handles)
        await self.send_set_active_schedule_request(
            endpoint=endpoint,
            schedule_handle=invalid_schedule_handle,
            expected_status=Status.InvalidCommand,
        )
        active_after_invalid = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActiveScheduleHandle
        )
        asserts.assert_equal(
            active_after_invalid,
            current_schedule_handle,
            "ActiveScheduleHandle changed after failed SetActiveScheduleRequest",
        )

        self.step("4a")
        await self.write_schedules(
            endpoint=endpoint,
            schedules=current_schedules,
            expected_status=Status.InvalidInState,
        )

        self.step("4b")
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        rollback_schedules = copy.deepcopy(current_schedules)
        existing_times_0 = {t.transitionTime for t in rollback_schedules[0].transitions}
        new_time_4b = (rollback_schedules[0].transitions[0].transitionTime + 15) % 1440
        while new_time_4b in existing_times_0:
            new_time_4b = (new_time_4b + 15) % 1440
        rollback_schedules[0].transitions[0].transitionTime = new_time_4b
        await self.write_schedules(endpoint=endpoint, schedules=rollback_schedules)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        schedules_after_rollback = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
        )
        asserts.assert_equal(
            schedules_after_rollback,
            current_schedules,
            "Schedules attribute was modified despite RollbackWrite",
        )

        self.step("4c")
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        commit_schedules = copy.deepcopy(current_schedules)
        original_built_in = commit_schedules[0].builtIn
        existing_times_0 = {t.transitionTime for t in commit_schedules[0].transitions}
        new_time_4c = (commit_schedules[0].transitions[0].transitionTime + 30) % 1440
        while new_time_4c in existing_times_0:
            new_time_4c = (new_time_4c + 15) % 1440
        commit_schedules[0].transitions[0].transitionTime = new_time_4c
        commit_schedules[0].builtIn = NullValue
        await self.write_schedules(endpoint=endpoint, schedules=commit_schedules)
        await self.send_atomic_request_commit(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        current_schedules = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
        )
        asserts.assert_equal(
            current_schedules[0].transitions[0].transitionTime,
            new_time_4c,
            "Committed transitionTime modification was not persisted in Schedules",
        )
        asserts.assert_equal(
            current_schedules[0].builtIn,
            original_built_in,
            "BuiltIn field did not retain its previous value when written as null",
        )
        schedule_handles = {s.scheduleHandle for s in current_schedules}

        self.step("4d")
        added_schedule_handle: bytes | None = None
        added_schedule_type: cluster.Structs.ScheduleTypeStruct | None = None
        mode_counts = {}
        for s in current_schedules:
            mode_counts[s.systemMode] = mode_counts.get(s.systemMode, 0) + 1
        available_types = [
            st
            for st in supported_schedule_types
            if mode_counts.get(st.systemMode, 0) < st.numberOfSchedules
        ]

        if len(current_schedules) < number_of_schedules and len(available_types) > 0:
            added_schedule_type = available_types[0]
            new_schedule = self.make_valid_schedule(
                schedule_type=added_schedule_type,
                heat_setpoint=heat_setpoint,
                cool_setpoint=cool_setpoint,
                schedule_handle=NullValue,
                built_in=NullValue,
                preset_handle=default_preset_handle,
            )
            schedules_with_added = copy.deepcopy(current_schedules)
            schedules_with_added.append(new_schedule)

            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=schedules_with_added)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            schedules_after_add = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            asserts.assert_equal(
                len(schedules_after_add),
                len(current_schedules) + 1,
                "Schedules length did not increase by 1 after adding a schedule",
            )
            added_entries = [
                s for s in schedules_after_add if s.scheduleHandle not in schedule_handles
            ]
            asserts.assert_equal(
                len(added_entries),
                1,
                "Expected exactly one newly generated unique ScheduleHandle",
            )
            added_entry = added_entries[0]
            asserts.assert_true(
                isinstance(added_entry.scheduleHandle, bytes)
                and 1 <= len(added_entry.scheduleHandle) <= 16,
                "Newly assigned ScheduleHandle must be bytes of length 1..16",
            )
            asserts.assert_equal(
                added_entry.builtIn,
                False,
                "Newly added schedule must have BuiltIn set to False",
            )
            added_schedule_handle = added_entry.scheduleHandle
            current_schedules = schedules_after_add
            schedule_handles = {s.scheduleHandle for s in current_schedules}
        else:
            log.info("Skipping schedule addition in step 4d because Schedules is at capacity")

        self.step("4e")
        if added_schedule_handle is not None:
            schedules_without_added = [
                s for s in current_schedules if s.scheduleHandle != added_schedule_handle
            ]
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=schedules_without_added)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            current_schedules = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            schedule_handles = {s.scheduleHandle for s in current_schedules}
            asserts.assert_not_in(
                added_schedule_handle,
                schedule_handles,
                "Removed schedule handle is still present in Schedules",
            )

        self.step("4f")
        if added_schedule_handle is not None and added_schedule_type is not None:
            new_schedule_2 = self.make_valid_schedule(
                schedule_type=added_schedule_type,
                heat_setpoint=heat_setpoint,
                cool_setpoint=cool_setpoint,
                schedule_handle=NullValue,
                built_in=NullValue,
                preset_handle=default_preset_handle,
            )
            schedules_readd = copy.deepcopy(current_schedules)
            schedules_readd.append(new_schedule_2)

            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=schedules_readd)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            schedules_after_readd = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            new_entries_2 = [
                s for s in schedules_after_readd if s.scheduleHandle not in schedule_handles
            ]
            asserts.assert_equal(
                len(new_entries_2), 1, "Expected one newly added schedule in step 4f"
            )
            readded_handle = new_entries_2[0].scheduleHandle
            asserts.assert_not_equal(
                readded_handle,
                added_schedule_handle,
                "Device reused the deleted AddedScheduleHandle instead of assigning a unique handle",
            )

            # Remove the added schedule to restore state
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=current_schedules)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            current_schedules = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            schedule_handles = {s.scheduleHandle for s in current_schedules}

        self.step("4g")
        log.info("Commissioning under second controller (TH2)")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=600,
            iteration=10000,
            discriminator=1234,
            option=1,
        )
        secondary_authority = self.certificate_authority_manager.NewCertificateAuthority()
        secondary_fabric_admin = secondary_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        secondary_controller = secondary_fabric_admin.NewController(nodeId=112233)
        await secondary_controller.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=1234,
        )
        secondary_fabric_index = await self.read_single_attribute_check_success(
            dev_ctrl=secondary_controller,
            endpoint=0,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
        )

        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Busy},
            dev_ctrl=secondary_controller,
            endpoint=endpoint,
            expected_atomic_status=Status.Failure,
        )

        self.step("4h")
        await self.write_schedules(
            endpoint=endpoint,
            schedules=current_schedules,
            dev_ctrl=secondary_controller,
            expected_status=[Status.InvalidInState, Status.Busy],
        )
        await self.send_atomic_request_rollback(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )

        self.step("4i")
        # Swap TH and TH2 roles here so self.default_controller remains commissioned for subsequent test steps.
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success},
            dev_ctrl=secondary_controller,
            endpoint=endpoint,
        )
        await self.send_single_cmd(
            Clusters.OperationalCredentials.Commands.RemoveFabric(
                fabricIndex=secondary_fabric_index
            ),
            endpoint=0,
        )
        await self.send_atomic_request_begin(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )
        await self.send_atomic_request_rollback(
            {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
        )

        self.step("5a")
        first_st = supported_schedule_types[0]
        invalid_new_builtin = self.make_valid_schedule(
            schedule_type=first_st,
            heat_setpoint=heat_setpoint,
            cool_setpoint=cool_setpoint,
            schedule_handle=NullValue,
            built_in=True,
            preset_handle=default_preset_handle,
        )
        test_schedules_5a = copy.deepcopy(current_schedules)
        if len(test_schedules_5a) < number_of_schedules:
            test_schedules_5a.append(invalid_new_builtin)
        else:
            test_schedules_5a[-1] = invalid_new_builtin
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_5a,
            expected_status=Status.ConstraintError,
        )

        self.step("5b")
        non_existent_handle = self.generate_unused_handle(schedule_handles)
        invalid_handle_schedule = self.make_valid_schedule(
            schedule_type=first_st,
            heat_setpoint=heat_setpoint,
            cool_setpoint=cool_setpoint,
            schedule_handle=non_existent_handle,
            built_in=False,
            preset_handle=default_preset_handle,
        )
        test_schedules_5b = copy.deepcopy(current_schedules)
        if len(test_schedules_5b) < number_of_schedules:
            test_schedules_5b.append(invalid_handle_schedule)
        else:
            test_schedules_5b[-1] = invalid_handle_schedule
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_5b,
            expected_status=Status.NotFound,
        )

        self.step("5c")
        if len(current_schedules) > 0 and number_of_schedules >= 2:
            dup_schedule = copy.deepcopy(current_schedules[0])
            test_schedules_5c = copy.deepcopy(current_schedules)
            if len(test_schedules_5c) < number_of_schedules:
                test_schedules_5c.append(dup_schedule)
            else:
                test_schedules_5c[-1] = dup_schedule
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_5c,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 5c because NumberOfSchedules < 2")

        self.step("5d")
        temp_non_builtin_handle: bytes | None = None
        non_builtin_indices = [
            i for i, s in enumerate(current_schedules) if s.builtIn is False
        ]
        if len(non_builtin_indices) == 0 and added_schedule_type is not None and len(current_schedules) < number_of_schedules:
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            new_non_builtin = self.make_valid_schedule(
                schedule_type=added_schedule_type,
                heat_setpoint=heat_setpoint,
                cool_setpoint=cool_setpoint,
                schedule_handle=NullValue,
                built_in=False,
                preset_handle=default_preset_handle,
            )
            await self.write_schedules(
                endpoint=endpoint, schedules=[*current_schedules, new_non_builtin]
            )
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            current_schedules = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            schedule_handles = {s.scheduleHandle for s in current_schedules}
            non_builtin_indices = [
                i for i, s in enumerate(current_schedules) if s.builtIn is False
            ]
            if len(non_builtin_indices) > 0:
                temp_non_builtin_handle = current_schedules[non_builtin_indices[0]].scheduleHandle

        if len(non_builtin_indices) > 0:
            test_schedules_5d = copy.deepcopy(current_schedules)
            test_schedules_5d[non_builtin_indices[0]].builtIn = True
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_5d,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 5d because no non-built-in schedule exists or could be added")

        self.step("5e")
        builtin_indices = [i for i, s in enumerate(current_schedules) if s.builtIn is True]
        if len(builtin_indices) > 0:
            test_schedules_5e = copy.deepcopy(current_schedules)
            test_schedules_5e[builtin_indices[0]].builtIn = False
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_5e,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 5e because no built-in schedule is present in Schedules")

        self.step("5f")
        if len(builtin_indices) > 0:
            builtin_handle = current_schedules[builtin_indices[0]].scheduleHandle
            test_schedules_5f = [
                s for s in current_schedules if s.scheduleHandle != builtin_handle
            ]
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=test_schedules_5f)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.ConstraintError},
                endpoint=endpoint,
                expected_atomic_status=Status.Failure,
            )
            schedules_after_5f = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            asserts.assert_in(
                builtin_handle,
                {s.scheduleHandle for s in schedules_after_5f},
                "Built-in schedule was removed despite failed CommitWrite",
            )
        else:
            log.info("Skipping step 5f because no built-in schedule is present in Schedules")

        self.step("5g")
        if len(non_builtin_indices) > 0:
            active_non_builtin_handle = current_schedules[non_builtin_indices[0]].scheduleHandle
            await self.send_set_active_schedule_request(
                endpoint=endpoint,
                schedule_handle=active_non_builtin_handle,
                expected_status=Status.Success,
            )
            test_schedules_5g = [
                s for s in current_schedules if s.scheduleHandle != active_non_builtin_handle
            ]
            await self.send_atomic_request_begin(
                {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
            )
            await self.write_schedules(endpoint=endpoint, schedules=test_schedules_5g)
            await self.send_atomic_request_commit(
                {cluster.Attributes.Schedules.attribute_id: Status.InvalidInState},
                endpoint=endpoint,
                expected_atomic_status=Status.Failure,
            )
            schedules_after_5g = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
            )
            asserts.assert_in(
                active_non_builtin_handle,
                {s.scheduleHandle for s in schedules_after_5g},
                "Active schedule was removed despite failed CommitWrite",
            )

            # If we added a temporary non-built-in schedule for 5d/5g, switch ActiveScheduleHandle back and remove the temp schedule
            if temp_non_builtin_handle is not None:
                other_schedules = [
                    s for s in current_schedules if s.scheduleHandle != temp_non_builtin_handle
                ]
                if len(other_schedules) > 0:
                    await self.send_set_active_schedule_request(
                        endpoint=endpoint,
                        schedule_handle=other_schedules[0].scheduleHandle,
                        expected_status=Status.Success,
                    )
                    await self.send_atomic_request_begin(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    await self.write_schedules(endpoint=endpoint, schedules=other_schedules)
                    await self.send_atomic_request_commit(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    current_schedules = await self.read_single_attribute_check_success(
                        endpoint=endpoint,
                        cluster=cluster,
                        attribute=cluster.Attributes.Schedules,
                    )
                    schedule_handles = {s.scheduleHandle for s in current_schedules}
        else:
            log.info("Skipping step 5g because no non-built-in schedule could be activated and removed")

        self.step("5h")
        unsupported_modes = [
            mode
            for mode in (
                cluster.Enums.SystemModeEnum.kOff,
                cluster.Enums.SystemModeEnum.kAuto,
                cluster.Enums.SystemModeEnum.kCool,
                cluster.Enums.SystemModeEnum.kHeat,
                cluster.Enums.SystemModeEnum.kEmergencyHeat,
                cluster.Enums.SystemModeEnum.kPrecooling,
                cluster.Enums.SystemModeEnum.kFanOnly,
                cluster.Enums.SystemModeEnum.kDry,
                cluster.Enums.SystemModeEnum.kSleep,
            )
            if mode not in schedule_type_by_mode
        ]
        if len(unsupported_modes) > 0:
            test_schedules_5h = copy.deepcopy(current_schedules)
            test_schedules_5h[0].systemMode = unsupported_modes[0]
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_5h,
                expected_status=Status.ConstraintError,
            )

        self.step("6a")
        types_with_names = [
            st
            for st in supported_schedule_types
            if st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsNames
        ]
        if len(types_with_names) > 0:
            st_names = types_with_names[0]
            matching_idx = next(
                (i for i, s in enumerate(current_schedules) if s.systemMode == st_names.systemMode),
                None,
            )
            temp_added_for_6a = False
            if matching_idx is None and len(current_schedules) < number_of_schedules:
                await self.send_atomic_request_begin(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                new_s = self.make_valid_schedule(
                    schedule_type=st_names,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    preset_handle=default_preset_handle,
                )
                await self.write_schedules(endpoint=endpoint, schedules=[*current_schedules, new_s])
                await self.send_atomic_request_commit(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                current_schedules = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                )
                matching_idx = len(current_schedules) - 1
                temp_added_for_6a = True

            if matching_idx is not None:
                original_name = current_schedules[matching_idx].name
                valid_name = "Weekday Schedule"
                test_schedules_6a_valid = copy.deepcopy(current_schedules)
                test_schedules_6a_valid[matching_idx].name = valid_name

                await self.send_atomic_request_begin(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                await self.write_schedules(endpoint=endpoint, schedules=test_schedules_6a_valid)
                await self.send_atomic_request_commit(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                current_schedules = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                )
                asserts.assert_equal(
                    current_schedules[matching_idx].name,
                    valid_name,
                    "Schedules did not reflect updated valid Name",
                )

                # Test Name > 64 chars
                test_schedules_6a_invalid = copy.deepcopy(current_schedules)
                test_schedules_6a_invalid[matching_idx].name = "A" * 65
                await self.write_schedules_and_expect_error(
                    endpoint=endpoint,
                    schedules=test_schedules_6a_invalid,
                    expected_status=Status.ConstraintError,
                )

                # Restore original schedule state
                await self.send_atomic_request_begin(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                if temp_added_for_6a:
                    restored_schedules = current_schedules[:-1]
                else:
                    restored_schedules = copy.deepcopy(current_schedules)
                    restored_schedules[matching_idx].name = original_name
                await self.write_schedules(endpoint=endpoint, schedules=restored_schedules)
                await self.send_atomic_request_commit(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                current_schedules = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                )
        else:
            log.info("Skipping step 6a because no ScheduleTypeStruct has SupportsNames set")

        self.step("6b")
        types_without_names = [
            st
            for st in supported_schedule_types
            if not (st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsNames)
        ]
        if len(types_without_names) > 0:
            st_no_names = types_without_names[0]
            test_schedules_6b = copy.deepcopy(current_schedules)
            matching_idx = next(
                (i for i, s in enumerate(test_schedules_6b) if s.systemMode == st_no_names.systemMode),
                None,
            )
            if matching_idx is not None:
                test_schedules_6b[matching_idx].name = "UnsupportedName"
            else:
                new_s = self.make_valid_schedule(
                    schedule_type=st_no_names,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    name="UnsupportedName",
                    preset_handle=default_preset_handle,
                )
                if len(test_schedules_6b) < number_of_schedules:
                    test_schedules_6b.append(new_s)
                else:
                    test_schedules_6b[-1] = new_s
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_6b,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 6b because all ScheduleTypeStructs support names")

        self.step("6c")
        types_without_presets = [
            st
            for st in supported_schedule_types
            if not (st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets)
        ]
        if len(types_without_presets) > 0:
            st_no_presets = types_without_presets[0]
            test_schedules_6c = copy.deepcopy(current_schedules)
            matching_idx = next(
                (i for i, s in enumerate(test_schedules_6c) if s.systemMode == st_no_presets.systemMode),
                None,
            )
            dummy_preset_handle = default_preset_handle if default_preset_handle is not None else b"\x01"
            if matching_idx is not None:
                test_schedules_6c[matching_idx].presetHandle = dummy_preset_handle
            else:
                new_s = self.make_valid_schedule(
                    schedule_type=st_no_presets,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                )
                new_s.presetHandle = dummy_preset_handle
                if len(test_schedules_6c) < number_of_schedules:
                    test_schedules_6c.append(new_s)
                else:
                    test_schedules_6c[-1] = new_s
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_6c,
                expected_status=Status.ConstraintError,
            )

            # Also verify setting PresetHandle on a ScheduleTransitionStruct fails with CONSTRAINT_ERROR
            test_schedules_6c_trans = copy.deepcopy(current_schedules)
            if matching_idx is not None:
                test_schedules_6c_trans[matching_idx].transitions = [
                    cluster.Structs.ScheduleTransitionStruct(
                        dayOfWeek=ALL_DAYS_MASK,
                        transitionTime=360,
                        presetHandle=dummy_preset_handle,
                    )
                ]
            else:
                new_s_trans = self.make_valid_schedule(
                    schedule_type=st_no_presets,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    transitions=[
                        cluster.Structs.ScheduleTransitionStruct(
                            dayOfWeek=ALL_DAYS_MASK,
                            transitionTime=360,
                            presetHandle=dummy_preset_handle,
                        )
                    ],
                )
                if len(test_schedules_6c_trans) < number_of_schedules:
                    test_schedules_6c_trans.append(new_s_trans)
                else:
                    test_schedules_6c_trans[-1] = new_s_trans
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_6c_trans,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 6c because all ScheduleTypeStructs support presets")

        self.step("6d")
        types_with_presets = [
            st
            for st in supported_schedule_types
            if st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsPresets
        ]
        if has_presets and len(types_with_presets) > 0:
            st_presets = types_with_presets[0]
            current_presets = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets
            )
            active_preset_handle = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ActivePresetHandle
            )
            preset_types = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PresetTypes
            )
            number_of_presets = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfPresets
            )

            # Ensure we have a non-built-in, non-active preset if possible
            temp_added_preset_handle: bytes | None = None
            removable_presets = [
                p
                for p in current_presets
                if p.builtIn is False and p.presetHandle != active_preset_handle
            ]
            if len(removable_presets) == 0 and len(current_presets) < number_of_presets and len(preset_types) > 0:
                scenario_counts: dict[int, int] = {}
                for p in current_presets:
                    scenario_counts[p.presetScenario] = scenario_counts.get(p.presetScenario, 0) + 1
                avail_pt = [
                    pt
                    for pt in preset_types
                    if scenario_counts.get(pt.presetScenario, 0) < pt.numberOfPresets
                ]
                if len(avail_pt) > 0:
                    new_preset = cluster.Structs.PresetStruct(
                        presetHandle=NullValue,
                        presetScenario=avail_pt[0].presetScenario,
                        coolingSetpoint=cool_setpoint if has_cool else None,
                        heatingSetpoint=heat_setpoint if has_heat else None,
                        builtIn=False,
                    )
                    old_handles = {p.presetHandle for p in current_presets}
                    await self.send_atomic_request_begin(
                        {cluster.Attributes.Presets.attribute_id: Status.Success}, endpoint=endpoint
                    )
                    await self.write_presets(
                        endpoint=endpoint, presets=[*current_presets, new_preset]
                    )
                    await self.send_atomic_request_commit(
                        {cluster.Attributes.Presets.attribute_id: Status.Success}, endpoint=endpoint
                    )
                    current_presets = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets
                    )
                    preset_handles = {p.presetHandle for p in current_presets}
                    added_p = [p for p in current_presets if p.presetHandle not in old_handles]
                    if len(added_p) > 0:
                        temp_added_preset_handle = added_p[0].presetHandle
                        removable_presets = added_p

            target_preset_handle = (
                removable_presets[0].presetHandle
                if len(removable_presets) > 0
                else (current_presets[0].presetHandle if len(current_presets) > 0 else None)
            )

            if target_preset_handle is not None:
                schedules_before_6d = copy.deepcopy(current_schedules)
                matching_idx = next(
                    (i for i, s in enumerate(current_schedules) if s.systemMode == st_presets.systemMode),
                    None,
                )
                preset_transition = cluster.Structs.ScheduleTransitionStruct(
                    dayOfWeek=ALL_DAYS_MASK,
                    transitionTime=420,
                    presetHandle=target_preset_handle,
                )
                test_schedules_6d = copy.deepcopy(current_schedules)
                if matching_idx is not None:
                    test_schedules_6d[matching_idx].presetHandle = target_preset_handle
                    test_schedules_6d[matching_idx].transitions = [preset_transition]
                elif len(test_schedules_6d) < number_of_schedules:
                    new_s = cluster.Structs.ScheduleStruct(
                        scheduleHandle=NullValue,
                        systemMode=st_presets.systemMode,
                        presetHandle=target_preset_handle,
                        transitions=[preset_transition],
                        builtIn=False,
                    )
                    test_schedules_6d.append(new_s)
                    matching_idx = len(test_schedules_6d) - 1

                if matching_idx is not None:
                    # 1. Valid PresetHandle write & commit
                    await self.send_atomic_request_begin(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    await self.write_schedules(endpoint=endpoint, schedules=test_schedules_6d)
                    await self.send_atomic_request_commit(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    schedules_with_preset = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                    )
                    asserts.assert_equal(
                        schedules_with_preset[matching_idx].transitions[0].presetHandle,
                        target_preset_handle,
                        "Schedule transition PresetHandle was not updated as expected",
                    )

                    # 2. Non-existent ScheduleStruct.PresetHandle
                    unused_preset_handle = self.generate_unused_handle(preset_handles)
                    invalid_sched_preset = copy.deepcopy(schedules_with_preset)
                    invalid_sched_preset[matching_idx].presetHandle = unused_preset_handle
                    await self.write_schedules_and_expect_error(
                        endpoint=endpoint,
                        schedules=invalid_sched_preset,
                        expected_status=Status.ConstraintError,
                    )

                    # 3. Non-existent ScheduleTransitionStruct.PresetHandle
                    invalid_trans_preset = copy.deepcopy(schedules_with_preset)
                    invalid_trans_preset[matching_idx].transitions[0].presetHandle = unused_preset_handle
                    await self.write_schedules_and_expect_error(
                        endpoint=endpoint,
                        schedules=invalid_trans_preset,
                        expected_status=Status.ConstraintError,
                    )

                    # 4. Attempt to remove a non-built-in, non-active preset referenced by ScheduleTransitionStruct
                    if len(removable_presets) > 0:
                        presets_without_target = [
                            p for p in current_presets if p.presetHandle != target_preset_handle
                        ]
                        await self.send_atomic_request_begin(
                            {cluster.Attributes.Presets.attribute_id: Status.Success},
                            endpoint=endpoint,
                        )
                        await self.write_presets(endpoint=endpoint, presets=presets_without_target)
                        await self.send_atomic_request_commit(
                            {cluster.Attributes.Presets.attribute_id: Status.InvalidInState},
                            endpoint=endpoint,
                            expected_atomic_status=Status.Failure,
                        )

                    # Restore Schedules to schedules_before_6d
                    await self.send_atomic_request_begin(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    await self.write_schedules(endpoint=endpoint, schedules=schedules_before_6d)
                    await self.send_atomic_request_commit(
                        {cluster.Attributes.Schedules.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    current_schedules = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                    )

                if temp_added_preset_handle is not None:
                    presets_restored = [
                        p for p in current_presets if p.presetHandle != temp_added_preset_handle
                    ]
                    await self.send_atomic_request_begin(
                        {cluster.Attributes.Presets.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    await self.write_presets(endpoint=endpoint, presets=presets_restored)
                    await self.send_atomic_request_commit(
                        {cluster.Attributes.Presets.attribute_id: Status.Success},
                        endpoint=endpoint,
                    )
                    current_presets = await self.read_single_attribute_check_success(
                        endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets
                    )
                    preset_handles = {p.presetHandle for p in current_presets}
        else:
            log.info("Skipping step 6d because no ScheduleTypeStruct supports presets")

        self.step("6e")
        types_without_setpoints = [
            st
            for st in supported_schedule_types
            if not (st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints)
        ]
        if len(types_without_setpoints) > 0:
            st_no_setpoints = types_without_setpoints[0]
            test_schedules_6e = copy.deepcopy(current_schedules)
            invalid_trans = cluster.Structs.ScheduleTransitionStruct(
                dayOfWeek=ALL_DAYS_MASK,
                transitionTime=360,
                heatingSetpoint=heat_setpoint if has_heat else None,
                coolingSetpoint=cool_setpoint if not has_heat else None,
            )
            matching_idx = next(
                (i for i, s in enumerate(test_schedules_6e) if s.systemMode == st_no_setpoints.systemMode),
                None,
            )
            if matching_idx is not None:
                test_schedules_6e[matching_idx].transitions = [invalid_trans]
            else:
                new_s = cluster.Structs.ScheduleStruct(
                    scheduleHandle=NullValue,
                    systemMode=st_no_setpoints.systemMode,
                    transitions=[invalid_trans],
                    builtIn=False,
                )
                if len(test_schedules_6e) < number_of_schedules:
                    test_schedules_6e.append(new_s)
                else:
                    test_schedules_6e[-1] = new_s
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_6e,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 6e because all ScheduleTypeStructs support setpoints")

        self.step("6f")
        types_with_setpoints = [
            st
            for st in supported_schedule_types
            if st.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsSetpoints
        ]
        if len(types_with_setpoints) > 0:
            st_setpoints = types_with_setpoints[0]
            supports_off = bool(
                st_setpoints.scheduleTypeFeatures & cluster.Bitmaps.ScheduleTypeFeaturesBitmap.kSupportsOff
            )
            matching_idx = next(
                (i for i, s in enumerate(current_schedules) if s.systemMode == st_setpoints.systemMode),
                0,
            )
            off_transition = cluster.Structs.ScheduleTransitionStruct(
                dayOfWeek=ALL_DAYS_MASK,
                transitionTime=540,
                systemMode=cluster.Enums.SystemModeEnum.kOff,
            )
            test_schedules_6f = copy.deepcopy(current_schedules)
            test_schedules_6f[matching_idx].transitions = [off_transition]

            if not supports_off:
                await self.write_schedules_and_expect_error(
                    endpoint=endpoint,
                    schedules=test_schedules_6f,
                    expected_status=Status.ConstraintError,
                )
            else:
                schedules_before_6f = copy.deepcopy(current_schedules)
                await self.send_atomic_request_begin(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                await self.write_schedules(endpoint=endpoint, schedules=test_schedules_6f)
                await self.send_atomic_request_commit(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                schedules_after_6f = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                )
                asserts.assert_equal(
                    schedules_after_6f[matching_idx].transitions[0].systemMode,
                    cluster.Enums.SystemModeEnum.kOff,
                    "Schedule transition SystemMode was not updated to Off",
                )
                # Restore original transitions
                await self.send_atomic_request_begin(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                await self.write_schedules(endpoint=endpoint, schedules=schedules_before_6f)
                await self.send_atomic_request_commit(
                    {cluster.Attributes.Schedules.attribute_id: Status.Success}, endpoint=endpoint
                )
                current_schedules = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Schedules
                )
        else:
            log.info("Skipping step 6f because no ScheduleTypeStruct supports setpoints")

        self.step("7a")
        test_schedules_7a = copy.deepcopy(current_schedules)
        test_schedules_7a[0].transitions = []
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_7a,
            expected_status=Status.ConstraintError,
        )

        self.step("7b")
        test_schedules_7b = copy.deepcopy(current_schedules)
        test_schedules_7b[0].transitions[0].dayOfWeek = (
            cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday
            | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kAway
        )
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_7b,
            expected_status=Status.ConstraintError,
        )

        self.step("7c")
        test_schedules_7c = copy.deepcopy(current_schedules)
        test_schedules_7c[0].transitions[0].transitionTime = 1440
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_7c,
            expected_status=Status.ConstraintError,
        )

        self.step("7d")
        if number_of_schedule_transitions >= 2:
            test_schedules_7d = copy.deepcopy(current_schedules)
            t1 = copy.deepcopy(test_schedules_7d[0].transitions[0])
            t1.dayOfWeek = (
                cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday
                | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday
            )
            t1.transitionTime = 480
            t2 = copy.deepcopy(t1)
            t2.dayOfWeek = (
                cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday
                | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kWednesday
            )
            t2.transitionTime = 480
            test_schedules_7d[0].transitions = [t1, t2]
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_7d,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 7d because NumberOfScheduleTransitions < 2")

        self.step("7e")
        if has_presets and len(types_with_presets) > 0 and default_preset_handle is not None:
            st_presets = types_with_presets[0]
            test_schedules_7e = copy.deepcopy(current_schedules)
            invalid_both_trans = cluster.Structs.ScheduleTransitionStruct(
                dayOfWeek=ALL_DAYS_MASK,
                transitionTime=360,
                presetHandle=default_preset_handle,
                heatingSetpoint=heat_setpoint if has_heat else None,
                coolingSetpoint=cool_setpoint if not has_heat else None,
            )
            matching_idx = next(
                (i for i, s in enumerate(test_schedules_7e) if s.systemMode == st_presets.systemMode),
                None,
            )
            if matching_idx is not None:
                test_schedules_7e[matching_idx].transitions = [invalid_both_trans]
            else:
                new_s = cluster.Structs.ScheduleStruct(
                    scheduleHandle=NullValue,
                    systemMode=st_presets.systemMode,
                    transitions=[invalid_both_trans],
                    builtIn=False,
                )
                if len(test_schedules_7e) < number_of_schedules:
                    test_schedules_7e.append(new_s)
                else:
                    test_schedules_7e[-1] = new_s
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_7e,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 7e because no ScheduleTypeStruct supports presets")

        self.step("7f")
        if len(types_with_setpoints) > 0:
            st_setpoints = types_with_setpoints[0]
            test_schedules_7f = copy.deepcopy(current_schedules)
            matching_idx = next(
                (i for i, s in enumerate(test_schedules_7f) if s.systemMode == st_setpoints.systemMode),
                0,
            )
            test_schedules_7f[matching_idx].transitions[0].systemMode = test_schedules_7f[
                matching_idx
            ].systemMode
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_7f,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 7f because no ScheduleTypeStruct supports setpoints")

        self.step("7g")
        if len(types_with_setpoints) > 0:
            st_setpoints = types_with_setpoints[0]
            matching_idx = next(
                (i for i, s in enumerate(current_schedules) if s.systemMode == st_setpoints.systemMode),
                0,
            )
            # 1. Omit required setpoint and preset fields for the transition's effective SystemMode
            test_schedules_7g_omit = copy.deepcopy(current_schedules)
            test_schedules_7g_omit[matching_idx].presetHandle = None
            empty_trans = cluster.Structs.ScheduleTransitionStruct(
                dayOfWeek=ALL_DAYS_MASK,
                transitionTime=360,
                presetHandle=None,
                systemMode=None,
                coolingSetpoint=None,
                heatingSetpoint=None,
            )
            test_schedules_7g_omit[matching_idx].transitions = [empty_trans]
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_7g_omit,
                expected_status=Status.ConstraintError,
            )

            # 2. Setpoint outside configured setpoint limits
            test_schedules_7g_range = copy.deepcopy(current_schedules)
            effective_mode = test_schedules_7g_range[matching_idx].systemMode
            if effective_mode in (
                cluster.Enums.SystemModeEnum.kHeat,
                cluster.Enums.SystemModeEnum.kAuto,
            ):
                test_schedules_7g_range[matching_idx].transitions[0].heatingSetpoint = (
                    max_heat_limit + 100
                )
            else:
                test_schedules_7g_range[matching_idx].transitions[0].coolingSetpoint = (
                    max_cool_limit + 100
                )
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_7g_range,
                expected_status=Status.ConstraintError,
            )
        else:
            log.info("Skipping step 7g because no ScheduleTypeStruct supports setpoints")

        self.step("8a")
        test_schedules_8a = copy.deepcopy(current_schedules)
        target_st_8a = schedule_type_by_mode[test_schedules_8a[0].systemMode]
        excess_transitions = []
        for i in range(number_of_schedule_transitions + 1):
            day_bit = INDIVIDUAL_DAYS[i % len(INDIVIDUAL_DAYS)]
            t_time = ((i // len(INDIVIDUAL_DAYS)) * 10 + (i % len(INDIVIDUAL_DAYS))) % 1440
            excess_transitions.append(
                self.make_valid_transition(
                    schedule_type=target_st_8a,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    day_of_week=day_bit,
                    transition_time=t_time,
                    preset_handle=default_preset_handle,
                )
            )
        test_schedules_8a[0].transitions = excess_transitions
        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_8a,
            expected_status=Status.ResourceExhausted,
        )

        self.step("8b")
        if (
            number_of_schedule_transition_per_day is not NullValue
            and number_of_schedule_transition_per_day < number_of_schedule_transitions
        ):
            test_schedules_8b = copy.deepcopy(current_schedules)
            target_st_8b = schedule_type_by_mode[test_schedules_8b[0].systemMode]
            excess_day_transitions = []
            for i in range(number_of_schedule_transition_per_day + 1):
                excess_day_transitions.append(
                    self.make_valid_transition(
                        schedule_type=target_st_8b,
                        heat_setpoint=heat_setpoint,
                        cool_setpoint=cool_setpoint,
                        day_of_week=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                        transition_time=(i * 10) % 1440,
                        preset_handle=default_preset_handle,
                    )
                )
            test_schedules_8b[0].transitions = excess_day_transitions
            await self.write_schedules_and_expect_error(
                endpoint=endpoint,
                schedules=test_schedules_8b,
                expected_status=Status.ResourceExhausted,
            )
        else:
            log.info(
                "Skipping step 8b because NumberOfScheduleTransitionPerDay is null or >= NumberOfScheduleTransitions"
            )

        self.step("8c")
        constrained_types = [
            st for st in supported_schedule_types if st.numberOfSchedules < number_of_schedules
        ]
        if len(constrained_types) > 0:
            st_constrained = constrained_types[0]
            test_schedules_8c = [
                copy.deepcopy(s)
                for s in current_schedules
                if s.systemMode == st_constrained.systemMode or s.builtIn is True
            ]
            current_mode_count = sum(
                1 for s in test_schedules_8c if s.systemMode == st_constrained.systemMode
            )
            while current_mode_count <= st_constrained.numberOfSchedules:
                test_schedules_8c.append(
                    self.make_valid_schedule(
                        schedule_type=st_constrained,
                        heat_setpoint=heat_setpoint,
                        cool_setpoint=cool_setpoint,
                        schedule_handle=NullValue,
                        built_in=False,
                        preset_handle=default_preset_handle,
                    )
                )
                current_mode_count += 1

            if len(test_schedules_8c) <= number_of_schedules:
                await self.write_schedules_and_expect_error(
                    endpoint=endpoint,
                    schedules=test_schedules_8c,
                    expected_status=Status.ResourceExhausted,
                )
            else:
                log.info(
                    "Skipping step 8c because exceeding ScheduleTypeStruct.NumberOfSchedules would also exceed total NumberOfSchedules"
                )
        else:
            log.info(
                "Skipping step 8c because no ScheduleTypeStruct has NumberOfSchedules < NumberOfSchedules attribute"
            )

        self.step("8d")
        test_schedules_8d = copy.deepcopy(current_schedules)
        mode_counts_8d: dict[int, int] = {}
        for s in test_schedules_8d:
            mode_counts_8d[s.systemMode] = mode_counts_8d.get(s.systemMode, 0) + 1

        while len(test_schedules_8d) < number_of_schedules + 1:
            chosen_st = next(
                (
                    st
                    for st in supported_schedule_types
                    if mode_counts_8d.get(st.systemMode, 0) < st.numberOfSchedules
                ),
                supported_schedule_types[0],
            )
            test_schedules_8d.append(
                self.make_valid_schedule(
                    schedule_type=chosen_st,
                    heat_setpoint=heat_setpoint,
                    cool_setpoint=cool_setpoint,
                    schedule_handle=NullValue,
                    built_in=False,
                    preset_handle=default_preset_handle,
                )
            )
            mode_counts_8d[chosen_st.systemMode] = (
                mode_counts_8d.get(chosen_st.systemMode, 0) + 1
            )

        await self.write_schedules_and_expect_error(
            endpoint=endpoint,
            schedules=test_schedules_8d,
            expected_status=Status.ResourceExhausted,
        )


if __name__ == "__main__":
    default_matter_test_main()
