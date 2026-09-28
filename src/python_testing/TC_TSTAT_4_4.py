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
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster = Clusters.Thermostat


class TC_TSTAT_4_4(ThermostatBaseTest):
    """Test case for Thermostat Sensors (SENS) feature on Thermostat cluster."""

    def check_sensors_attribute(self, sensors: list) -> list[bytes]:
        """Validates the Sensors attribute list per Matter spec section 6.5.6.3 and returns the list of handles."""
        asserts.assert_true(isinstance(sensors, list), "Sensors attribute must be a list")
        asserts.assert_greater(len(sensors), 0, "Sensors attribute must contain at least one sensor")
        asserts.assert_less_equal(len(sensors), 32, "Sensors attribute exceeds maximum of 32 entries")

        sensor_handles = []
        for idx, sensor in enumerate(sensors):
            asserts.assert_true(isinstance(sensor.name, str), f"Sensor {idx} Name must be a string")
            asserts.assert_less_equal(len(sensor.name.encode("utf-8")), 64, f"Sensor {idx} Name exceeds 64 bytes")
            asserts.assert_true(isinstance(sensor.sensorHandle, bytes), f"Sensor {idx} SensorHandle must be bytes")
            asserts.assert_greater(len(sensor.sensorHandle), 0, f"Sensor {idx} SensorHandle must not be empty")
            asserts.assert_less_equal(len(sensor.sensorHandle), 16, f"Sensor {idx} SensorHandle exceeds 16 bytes")
            asserts.assert_true(isinstance(sensor.cluster, int), f"Sensor {idx} Cluster must be an integer")
            if sensor.endpoint is not None and sensor.endpoint is not NullValue:
                asserts.assert_true(isinstance(sensor.endpoint, int), f"Sensor {idx} Endpoint must be an integer")
            if sensor.node is not None and sensor.node is not NullValue:
                asserts.assert_true(isinstance(sensor.node, int), f"Sensor {idx} Node must be an integer")
            if sensor.fabricIndex is not None and sensor.fabricIndex is not NullValue:
                asserts.assert_true(isinstance(sensor.fabricIndex, int), f"Sensor {idx} FabricIndex must be an integer")
            asserts.assert_not_in(sensor.sensorHandle, sensor_handles, f"Sensor {idx} SensorHandle is duplicate")
            sensor_handles.append(sensor.sensorHandle)

        return sensor_handles

    def check_returned_sensor_schedule(self, sent_schedule: list, returned_schedule: list) -> None:
        """Validates that the returned SensorSchedule matches the sent schedule."""
        asserts.assert_equal(
            len(sent_schedule), len(returned_schedule),
            f"Returned schedule length ({len(returned_schedule)}) differs from sent ({len(sent_schedule)})")
        for i, sent in enumerate(sent_schedule):
            returned = returned_schedule[i]
            asserts.assert_equal(
                sent.dayOfWeek, returned.dayOfWeek,
                f"Transition {i} dayOfWeek mismatch: expected {sent.dayOfWeek}, got {returned.dayOfWeek}")
            asserts.assert_equal(
                sent.transitionTime, returned.transitionTime,
                f"Transition {i} transitionTime mismatch: expected {sent.transitionTime}, got {returned.transitionTime}")
            asserts.assert_equal(
                sent.enabledSensorHandles, returned.enabledSensorHandles,
                f"Transition {i} enabledSensorHandles mismatch: expected {sent.enabledSensorHandles}, got {returned.enabledSensorHandles}")

    async def write_available_sensor_handles(self,
                                             endpoint: int,
                                             available_sensor_handles: list[bytes],
                                             dev_ctrl: ChipDeviceCtrl = None,
                                             expected_status: Status = Status.Success) -> Status:
        """Writes to AvailableSensorHandles attribute and verifies the expected status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(endpoint, cluster.Attributes.AvailableSensorHandles(available_sensor_handles))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status,
                             f"AvailableSensorHandles write returned {status.name}; expected {expected_status.name}")
        return status

    async def write_enabled_sensor_handles(self,
                                           endpoint: int,
                                           enabled_sensor_handles: list[bytes],
                                           dev_ctrl: ChipDeviceCtrl = None,
                                           expected_status: Status = Status.Success) -> Status:
        """Writes to EnabledSensorHandles attribute and verifies the expected status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(endpoint, cluster.Attributes.EnabledSensorHandles(enabled_sensor_handles))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status,
                             f"EnabledSensorHandles write returned {status.name}; expected {expected_status.name}")
        return status

    async def write_sensor_schedule(self,
                                    endpoint: int,
                                    sensor_schedule: list,
                                    dev_ctrl: ChipDeviceCtrl = None,
                                    expected_status: Status = Status.Success) -> Status:
        """Writes to SensorSchedule attribute and verifies the expected status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(endpoint, cluster.Attributes.SensorSchedule(sensor_schedule))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status,
                             f"SensorSchedule write returned {status.name}; expected {expected_status.name}")
        return status

    def desc_TC_TSTAT_4_4(self) -> str:
        """Returns a description of this test."""
        return "[TC-TSTAT-4.4] Thermostat Sensors Test Cases with server as DUT"

    def pics_TC_TSTAT_4_4(self) -> list[str]:
        """Returns a list of PICS for this test case that must be True for the test to be run."""
        return ["TSTAT.S", "TSTAT.S.F0b"]

    def steps_TC_TSTAT_4_4(self) -> list[TestStep]:
        """Returns the list of test steps for TC-TSTAT-4.4."""
        return [
            TestStep("1", "Commission DUT to TH",
                     is_commissioning=True),
            TestStep("2a", "TH reads the FeatureMap attribute.",
                     "Verify that the SENSORS bit (bit 11) is set in the FeatureMap value."),
            TestStep("2b", "TH reads the Sensors attribute.",
                     "Verify that the read returns a list of ThermostatSensorStruct entries (up to 32 entries). "
                     "Verify each entry has valid Name (max 64 chars), SensorHandle (octstr, max 16 bytes), "
                     "Cluster, and optional Endpoint, Node, and FabricIndex fields. "
                     "Save the list of sensor handles in a SupportedSensorHandles variable."),
            TestStep("2c", "TH reads the AvailableSensorHandles attribute.",
                     "Verify that the read returns a list of sensor handles (list[octstr], up to 32 entries). "
                     "Save the list in an AvailableSensorHandles variable. "
                     "Verify that each handle in AvailableSensorHandles matches a SensorHandle in the Sensors attribute list."),
            TestStep("2d", "TH reads the EnabledSensorHandles attribute.",
                     "Verify that the read returns a list of sensor handles (list[octstr], up to 32 entries). "
                     "Save the list in an EnabledSensorHandles variable. "
                     "Verify that all handles in EnabledSensorHandles are present in AvailableSensorHandles."),
            TestStep("2e", "TH reads the NumberOfSensorScheduleTransitions attribute.",
                     "Verify that the read returns a uint8 value. "
                     "Save the value in a NumberOfSensorScheduleTransitions variable."),
            TestStep("2f", "TH reads the SensorSchedule attribute.",
                     "Verify that the read returns a list of SensorScheduleTransitionStruct entries. "
                     "Verify that the number of entries is <= NumberOfSensorScheduleTransitions. "
                     "Verify each transition has valid DayOfWeek (Away bit not set), TransitionTime (0 to 1439), "
                     "and EnabledSensorHandles (list[octstr] with up to 32 handles). "
                     "Save the list in a SensorSchedule variable."),
            TestStep("3a", "TH writes to the AvailableSensorHandles attribute with a valid list of sensor handles containing "
                     "only handles present in SupportedSensorHandles.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the AvailableSensorHandles attribute and verifies that it equals the written list of sensor handles."),
            TestStep("3b", "If EnabledSensorHandles contains at least one sensor handle, TH writes to AvailableSensorHandles with "
                     "a list that omits one of the handles present in EnabledSensorHandles.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the AvailableSensorHandles and EnabledSensorHandles attributes. "
                     "Verify that removing the sensor handle from AvailableSensorHandles overrides and removes that sensor "
                     "handle from EnabledSensorHandles."),
            TestStep("4a", "TH writes to the EnabledSensorHandles attribute with a valid list of sensor handles where all "
                     "handles are present in AvailableSensorHandles.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the EnabledSensorHandles attribute and verifies that it equals the written list of sensor handles."),
            TestStep("4b", "TH writes to the EnabledSensorHandles attribute with a list containing a sensor handle that is "
                     "NOT present in AvailableSensorHandles (or not present in Sensors).",
                     "Verify that the write request returns CONSTRAINT_ERROR (0x87)."),
            TestStep("5a", "TH writes to the SensorSchedule attribute with a valid list of SensorScheduleTransitionStruct "
                     "entries (valid DayOfWeek without Away bit, TransitionTime between 0 and 1439, and EnabledSensorHandles "
                     "containing only handles present in AvailableSensorHandles).",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the SensorSchedule attribute and verifies that the transitions match the written value."),
            TestStep("5b", "TH writes to the SensorSchedule attribute with a transition where EnabledSensorHandles contains "
                     "a sensor handle that does NOT match any sensor in AvailableSensorHandles (or Sensors).",
                     "Verify that the write request returns CONSTRAINT_ERROR (0x87)."),
            TestStep("5c", "TH writes to the SensorSchedule attribute with a transition where DayOfWeek has the Away "
                     "bit (bit 7) set.",
                     "Verify that the write request returns CONSTRAINT_ERROR (0x87)."),
            TestStep("5d", "TH writes to the SensorSchedule attribute containing duplicate transitions (multiple "
                     "transitions with the exact same TransitionTime and overlapping DayOfWeek fields).",
                     "Verify that the write request returns CONSTRAINT_ERROR (0x87)."),
            TestStep("5e", "TH writes to the SensorSchedule attribute with the number of transitions exceeding "
                     "NumberOfSensorScheduleTransitions.",
                     "Verify that the write request returns RESOURCE_EXHAUSTED (0x89)."),
            TestStep("6a", "TH calls the AtomicRequest command with RequestType set to BeginWrite. "
                     "TH writes to the SensorSchedule attribute with a valid set of transitions. "
                     "TH calls the AtomicRequest command with RequestType set to CommitWrite.",
                     "Verify that the AtomicRequest commands return SUCCESS and the SensorSchedule attribute is "
                     "updated with the new transitions."),
            TestStep("6b", "TH calls the AtomicRequest command with RequestType set to BeginWrite. "
                     "TH writes to the SensorSchedule attribute with a valid set of transitions. "
                     "TH calls the AtomicRequest command with RequestType set to RollbackWrite.",
                     "Verify that the edit request is rolled back and the SensorSchedule attribute remains unchanged.")
        ]

    @async_test_body
    async def test_TC_TSTAT_4_4(self) -> None:
        endpoint = self.get_endpoint()

        self.step("1")
        # Commission DUT - already done

        self.step("2a")
        # TH reads the FeatureMap attribute.
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.FeatureMap)
        log.info("FeatureMap: 0x%08x", feature_map)
        # Verify that the SENSORS bit (bit 11) is set in the FeatureMap value.
        has_sensors = bool(feature_map & cluster.Bitmaps.Feature.kThermostatSensors)
        if not has_sensors:
            log.warning("SENSORS bit (bit 11, 0x800) is not set in FeatureMap on endpoint %d. Skipping steps 2b-6b.", endpoint)
            self.mark_step_range_skipped("2b", "6b")
            return

        self.step("2b")
        # TH reads the Sensors attribute.
        sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Sensors)
        log.info("Sensors: %s", sensors)
        supported_sensor_handles = self.check_sensors_attribute(sensors)

        self.step("2c")
        # TH reads the AvailableSensorHandles attribute.
        available_sensor_handles = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensorHandles)
        log.info("AvailableSensorHandles: %s", available_sensor_handles)
        asserts.assert_true(isinstance(available_sensor_handles, list), "AvailableSensorHandles attribute must be a list")
        asserts.assert_less_equal(len(available_sensor_handles), 32, "AvailableSensorHandles exceeds maximum of 32 entries")
        for handle in available_sensor_handles:
            asserts.assert_true(isinstance(handle, bytes), "AvailableSensorHandles handle must be bytes")
            asserts.assert_less_equal(len(handle), 16, "AvailableSensorHandles handle exceeds 16 bytes")
            asserts.assert_in(handle, supported_sensor_handles,
                              f"Handle {handle} in AvailableSensorHandles is not present in SupportedSensorHandles")

        self.step("2d")
        # TH reads the EnabledSensorHandles attribute.
        enabled_sensor_handles = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensorHandles)
        log.info("EnabledSensorHandles: %s", enabled_sensor_handles)
        asserts.assert_true(isinstance(enabled_sensor_handles, list), "EnabledSensorHandles attribute must be a list")
        asserts.assert_less_equal(len(enabled_sensor_handles), 32, "EnabledSensorHandles exceeds maximum of 32 entries")
        for handle in enabled_sensor_handles:
            asserts.assert_true(isinstance(handle, bytes), "EnabledSensorHandles handle must be bytes")
            asserts.assert_less_equal(len(handle), 16, "EnabledSensorHandles handle exceeds 16 bytes")
            asserts.assert_in(handle, available_sensor_handles,
                              f"Handle {handle} in EnabledSensorHandles is not present in AvailableSensorHandles")

        self.step("2e")
        # TH reads the NumberOfSensorScheduleTransitions attribute.
        number_of_sensor_schedule_transitions = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.NumberOfSensorScheduleTransitions)
        log.info("NumberOfSensorScheduleTransitions: %s", number_of_sensor_schedule_transitions)
        asserts.assert_true(isinstance(number_of_sensor_schedule_transitions, int),
                            "NumberOfSensorScheduleTransitions must be an integer")
        asserts.assert_greater_equal(number_of_sensor_schedule_transitions, 0,
                                     "NumberOfSensorScheduleTransitions must be non-negative")
        asserts.assert_less_equal(number_of_sensor_schedule_transitions, 255,
                                  "NumberOfSensorScheduleTransitions must fit in uint8")

        self.step("2f")
        # TH reads the SensorSchedule attribute.
        current_schedule = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SensorSchedule)
        log.info("SensorSchedule: %s", current_schedule)
        asserts.assert_true(isinstance(current_schedule, list), "SensorSchedule attribute must be a list")
        asserts.assert_less_equal(len(current_schedule), number_of_sensor_schedule_transitions,
                                  "SensorSchedule entry count exceeds NumberOfSensorScheduleTransitions")
        for idx, transition in enumerate(current_schedule):
            asserts.assert_false(bool(transition.dayOfWeek & cluster.Bitmaps.ScheduleDayOfWeekBitmap.kAway),
                                 f"Transition {idx} DayOfWeek has Away bit set, which is forbidden per spec section 6.5.6.2")
            asserts.assert_greater(transition.dayOfWeek, 0, f"Transition {idx} DayOfWeek must be non-zero")
            asserts.assert_equal(transition.dayOfWeek & ~0x7F, 0, f"Transition {idx} DayOfWeek contains invalid bits")
            asserts.assert_greater_equal(transition.transitionTime, 0, f"Transition {idx} transitionTime must be >= 0")
            asserts.assert_less_equal(transition.transitionTime, 1439, f"Transition {idx} transitionTime exceeds 1439")
            asserts.assert_true(isinstance(transition.enabledSensorHandles, list),
                                f"Transition {idx} EnabledSensorHandles must be a list")
            asserts.assert_less_equal(len(transition.enabledSensorHandles), 32,
                                      f"Transition {idx} EnabledSensorHandles exceeds 32 handles")
            for handle in transition.enabledSensorHandles:
                asserts.assert_true(isinstance(handle, bytes), f"Transition {idx} handle must be bytes")
                asserts.assert_less_equal(len(handle), 16, f"Transition {idx} handle exceeds 16 bytes")

        self.step("3a")
        # TH writes to AvailableSensorHandles with a valid list of sensor handles present in SupportedSensorHandles.
        available_sensor_handles = copy.deepcopy(supported_sensor_handles)
        await self.write_available_sensor_handles(endpoint=endpoint, available_sensor_handles=available_sensor_handles)
        read_available_sensor_handles = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensorHandles)
        asserts.assert_equal(read_available_sensor_handles, available_sensor_handles,
                             "AvailableSensorHandles does not match written list")

        self.step("3b")
        # If EnabledSensorHandles contains at least one sensor handle, TH writes to AvailableSensorHandles with a list that omits one.
        # Ensure EnabledSensorHandles contains the available_sensor_handles handles
        if len(available_sensor_handles) > 0:
            await self.write_enabled_sensor_handles(endpoint=endpoint, enabled_sensor_handles=available_sensor_handles)
            enabled_sensor_handles = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensorHandles)
            log.info("EnabledSensorHandles before omitting: %s", enabled_sensor_handles)

            if len(enabled_sensor_handles) > 0:
                omitted_handle = enabled_sensor_handles[0]
                new_available_sensor_handles = [h for h in available_sensor_handles if h != omitted_handle]
                await self.write_available_sensor_handles(endpoint=endpoint, available_sensor_handles=new_available_sensor_handles)

                available_sensor_handles = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensorHandles)
                enabled_sensor_handles = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensorHandles)
                log.info("AvailableSensorHandles after omitting: %s", available_sensor_handles)
                log.info("EnabledSensorHandles after omitting: %s", enabled_sensor_handles)

                asserts.assert_equal(available_sensor_handles, new_available_sensor_handles,
                                     "AvailableSensorHandles does not match written list")
                # Verify that removing the sensor handle from AvailableSensorHandles overrides and removes that handle from EnabledSensorHandles
                asserts.assert_not_in(omitted_handle, enabled_sensor_handles,
                                      f"Omitted handle {omitted_handle} was not removed from EnabledSensorHandles as required by spec")

        # Restore AvailableSensorHandles to all supported handles for subsequent steps
        await self.write_available_sensor_handles(endpoint=endpoint, available_sensor_handles=supported_sensor_handles)

        self.step("4a")
        # TH writes to EnabledSensorHandles with a valid list of sensor handles where all handles are present in AvailableSensorHandles.
        test_enabled = [supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
        await self.write_enabled_sensor_handles(endpoint=endpoint, enabled_sensor_handles=test_enabled)
        enabled_sensor_handles = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensorHandles)
        asserts.assert_equal(enabled_sensor_handles, test_enabled, "EnabledSensorHandles does not match written list")

        self.step("4b")
        # TH writes to EnabledSensorHandles with a list containing a sensor handle that is NOT present in AvailableSensorHandles.
        invalid_handle = b'\xff\xff\xff\xff'
        while invalid_handle in supported_sensor_handles:
            # Generate a random handle not in supported_sensor_handles; max 32 handles so this succeeds quickly
            invalid_handle = bytes([random.randint(0, 255) for _ in range(4)])
        # Verify that write returns CONSTRAINT_ERROR (0x87) per spec section 6.5.6.4
        await self.write_enabled_sensor_handles(endpoint=endpoint, enabled_sensor_handles=[invalid_handle],
                                                expected_status=Status.ConstraintError)

        self.step("5a")
        # TH writes to the SensorSchedule attribute with a valid list of SensorScheduleTransitionStruct entries.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        valid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=360,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            )
        ]
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=valid_schedule)
        current_schedule = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SensorSchedule,
            verify_wildcard_subscription=False)
        self.check_returned_sensor_schedule(valid_schedule, current_schedule)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        self.step("5b")
        # TH writes to SensorSchedule with a transition where EnabledSensorHandles contains a sensor handle not in AvailableSensorHandles.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        invalid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=480,
                enabledSensorHandles=[invalid_handle]
            )
        ]
        # Verify that write returns CONSTRAINT_ERROR
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=invalid_schedule,
                                         expected_status=Status.ConstraintError)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        self.step("5c")
        # TH writes to the SensorSchedule attribute with a transition where DayOfWeek has the Away bit set.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        invalid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kAway | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=480,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            )
        ]
        # Verify that write returns CONSTRAINT_ERROR
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=invalid_schedule,
                                         expected_status=Status.ConstraintError)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        self.step("5d")
        # TH writes to SensorSchedule containing duplicate transitions (same TransitionTime and overlapping DayOfWeek).
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        schedule_with_duplicates = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday,
                transitionTime=600,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            ),
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kWednesday,
                transitionTime=600,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            )
        ]
        try:
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id, [(endpoint, cluster.Attributes.SensorSchedule(schedule_with_duplicates))])
            status = result[0].Status
            if status == Status.Success:
                # Precommit validation on CommitWrite detects duplicate transitions per spec section 6.5.6.6
                await self.send_atomic_request_commit(
                    {cluster.Attributes.SensorSchedule.attribute_id: Status.ConstraintError},
                    endpoint=endpoint, expected_atomic_status=Status.Failure)
            else:
                asserts.assert_equal(status, Status.ConstraintError,
                                     f"Expected CONSTRAINT_ERROR for duplicate transitions, got {status.name}")
                await self.send_atomic_request_rollback(
                    {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR for duplicate transitions, got {e.status.name}")
            await self.send_atomic_request_rollback(
                {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        self.step("5e")
        # TH writes to the SensorSchedule attribute with transitions exceeding NumberOfSensorScheduleTransitions.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        invalid_schedule = []
        for i in range(number_of_sensor_schedule_transitions + 1):
            invalid_schedule.append(cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=i * 10,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            ))
        # Verify that write returns RESOURCE_EXHAUSTED
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=invalid_schedule,
                                         expected_status=Status.ResourceExhausted)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        self.step("6a")
        # TH calls AtomicRequest (BeginWrite), writes valid SensorSchedule transitions, and calls AtomicRequest (CommitWrite).
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        valid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=480,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            )
        ]
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=valid_schedule)
        await self.send_atomic_request_commit(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        current_schedule = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SensorSchedule)
        self.check_returned_sensor_schedule(valid_schedule, current_schedule)

        self.step("6b")
        # TH calls AtomicRequest (BeginWrite), writes valid SensorSchedule transitions, and calls AtomicRequest (RollbackWrite).
        existing_schedule_before_rollback = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SensorSchedule)

        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        schedule_to_be_rolled_back = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday,
                transitionTime=720,
                enabledSensorHandles=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            )
        ]
        await self.write_sensor_schedule(endpoint=endpoint, sensor_schedule=schedule_to_be_rolled_back)
        await self.send_atomic_request_rollback(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)

        read_schedule_after_6b = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SensorSchedule)
        self.check_returned_sensor_schedule(existing_schedule_before_rollback, read_schedule_after_6b)


if __name__ == "__main__":
    default_matter_test_main()
