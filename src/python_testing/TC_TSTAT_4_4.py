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
                sent.enabledSensors, returned.enabledSensors,
                f"Transition {i} enabledSensors mismatch: expected {sent.enabledSensors}, got {returned.enabledSensors}")

    async def write_available_sensors(self,
                                      endpoint: int,
                                      available_sensors: list[bytes],
                                      dev_ctrl: ChipDeviceCtrl = None,
                                      expected_status: Status = Status.Success) -> Status:
        """Writes to AvailableSensors attribute and verifies the expected status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(endpoint, cluster.Attributes.AvailableSensors(available_sensors))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status,
                             f"AvailableSensors write returned {status.name}; expected {expected_status.name}")
        return status

    async def write_enabled_sensors(self,
                                    endpoint: int,
                                    enabled_sensors: list[bytes],
                                    dev_ctrl: ChipDeviceCtrl = None,
                                    expected_status: Status = Status.Success) -> Status:
        """Writes to EnabledSensors attribute and verifies the expected status."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        result = await dev_ctrl.WriteAttribute(
            self.dut_node_id, [(endpoint, cluster.Attributes.EnabledSensors(enabled_sensors))])
        status = result[0].Status
        asserts.assert_equal(status, expected_status,
                             f"EnabledSensors write returned {status.name}; expected {expected_status.name}")
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
        return ["TSTAT.S"]

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
            TestStep("2c", "TH reads the AvailableSensors attribute.",
                     "Verify that the read returns a list of sensor handles (list[octstr], up to 32 entries). "
                     "Save the list in an AvailableSensors variable. "
                     "Verify that each handle in AvailableSensors matches a SensorHandle in the Sensors attribute list."),
            TestStep("2d", "TH reads the EnabledSensors attribute.",
                     "Verify that the read returns a list of sensor handles (list[octstr], up to 32 entries). "
                     "Save the list in an EnabledSensors variable. "
                     "Verify that all handles in EnabledSensors are present in AvailableSensors."),
            TestStep("2e", "TH reads the NumberOfSensorScheduleTransitions attribute.",
                     "Verify that the read returns a uint8 value. "
                     "Save the value in a NumberOfSensorScheduleTransitions variable."),
            TestStep("2f", "TH reads the SensorSchedule attribute.",
                     "Verify that the read returns a list of SensorScheduleTransitionStruct entries. "
                     "Verify that the number of entries is <= NumberOfSensorScheduleTransitions. "
                     "Verify each transition has valid DayOfWeek (Away bit not set), TransitionTime (0 to 1439), "
                     "and EnabledSensors (list[octstr] with up to 32 handles). "
                     "Save the list in a SensorSchedule variable."),
            TestStep("3a", "TH writes to the AvailableSensors attribute with a valid list of sensor handles containing "
                     "only handles present in SupportedSensorHandles.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the AvailableSensors attribute and verifies that it equals the written list of sensor handles."),
            TestStep("3b", "If EnabledSensors contains at least one sensor handle, TH writes to AvailableSensors with "
                     "a list that omits one of the handles present in EnabledSensors.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the AvailableSensors and EnabledSensors attributes. "
                     "Verify that removing the sensor handle from AvailableSensors overrides and removes that sensor "
                     "handle from EnabledSensors."),
            TestStep("4a", "TH writes to the EnabledSensors attribute with a valid list of sensor handles where all "
                     "handles are present in AvailableSensors.",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the EnabledSensors attribute and verifies that it equals the written list of sensor handles."),
            TestStep("4b", "TH writes to the EnabledSensors attribute with a list containing a sensor handle that is "
                     "NOT present in AvailableSensors (or not present in Sensors).",
                     "Verify that the write request returns CONSTRAINT_ERROR (0x87)."),
            TestStep("5a", "TH writes to the SensorSchedule attribute with a valid list of SensorScheduleTransitionStruct "
                     "entries (valid DayOfWeek without Away bit, TransitionTime between 0 and 1439, and EnabledSensors "
                     "containing only handles present in AvailableSensors).",
                     "Verify that the write request returns SUCCESS. "
                     "TH reads the SensorSchedule attribute and verifies that the transitions match the written value."),
            TestStep("5b", "TH writes to the SensorSchedule attribute with a transition where EnabledSensors contains "
                     "a sensor handle that does NOT match any sensor in AvailableSensors (or Sensors).",
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
        asserts.assert_true(bool(feature_map & cluster.Bitmaps.Feature.kThermostatSensors),
                            "SENSORS bit (bit 11, 0x800) is not set in FeatureMap")

        self.step("2b")
        # TH reads the Sensors attribute.
        sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Sensors)
        log.info("Sensors: %s", sensors)
        supported_sensor_handles = self.check_sensors_attribute(sensors)

        self.step("2c")
        # TH reads the AvailableSensors attribute.
        available_sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensors)
        log.info("AvailableSensors: %s", available_sensors)
        asserts.assert_true(isinstance(available_sensors, list), "AvailableSensors attribute must be a list")
        asserts.assert_less_equal(len(available_sensors), 32, "AvailableSensors exceeds maximum of 32 entries")
        for handle in available_sensors:
            asserts.assert_true(isinstance(handle, bytes), "AvailableSensors handle must be bytes")
            asserts.assert_less_equal(len(handle), 16, "AvailableSensors handle exceeds 16 bytes")
            asserts.assert_in(handle, supported_sensor_handles,
                              f"Handle {handle} in AvailableSensors is not present in SupportedSensorHandles")

        self.step("2d")
        # TH reads the EnabledSensors attribute.
        enabled_sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensors)
        log.info("EnabledSensors: %s", enabled_sensors)
        asserts.assert_true(isinstance(enabled_sensors, list), "EnabledSensors attribute must be a list")
        asserts.assert_less_equal(len(enabled_sensors), 32, "EnabledSensors exceeds maximum of 32 entries")
        for handle in enabled_sensors:
            asserts.assert_true(isinstance(handle, bytes), "EnabledSensors handle must be bytes")
            asserts.assert_less_equal(len(handle), 16, "EnabledSensors handle exceeds 16 bytes")
            asserts.assert_in(handle, available_sensors,
                              f"Handle {handle} in EnabledSensors is not present in AvailableSensors")

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
            asserts.assert_true(isinstance(transition.enabledSensors, list),
                                f"Transition {idx} EnabledSensors must be a list")
            asserts.assert_less_equal(len(transition.enabledSensors), 32,
                                      f"Transition {idx} EnabledSensors exceeds 32 handles")
            for handle in transition.enabledSensors:
                asserts.assert_true(isinstance(handle, bytes), f"Transition {idx} handle must be bytes")
                asserts.assert_less_equal(len(handle), 16, f"Transition {idx} handle exceeds 16 bytes")

        self.step("3a")
        # TH writes to AvailableSensors with a valid list of sensor handles present in SupportedSensorHandles.
        available_sensors = copy.deepcopy(supported_sensor_handles)
        await self.write_available_sensors(endpoint=endpoint, available_sensors=available_sensors)
        read_available_sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensors)
        asserts.assert_equal(read_available_sensors, available_sensors, "AvailableSensors does not match written list")

        self.step("3b")
        # If EnabledSensors contains at least one sensor handle, TH writes to AvailableSensors with a list that omits one.
        # Ensure EnabledSensors contains the available_sensors handles
        if len(available_sensors) > 0:
            await self.write_enabled_sensors(endpoint=endpoint, enabled_sensors=available_sensors)
            enabled_sensors = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensors)
            log.info("EnabledSensors before omitting: %s", enabled_sensors)

            if len(enabled_sensors) > 0:
                omitted_handle = enabled_sensors[0]
                new_available_sensors = [h for h in available_sensors if h != omitted_handle]
                await self.write_available_sensors(endpoint=endpoint, available_sensors=new_available_sensors)

                available_sensors = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableSensors)
                enabled_sensors = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensors)
                log.info("AvailableSensors after omitting: %s", available_sensors)
                log.info("EnabledSensors after omitting: %s", enabled_sensors)

                asserts.assert_equal(available_sensors, new_available_sensors, "AvailableSensors does not match written list")
                # Verify that removing the sensor handle from AvailableSensors overrides and removes that handle from EnabledSensors
                asserts.assert_not_in(omitted_handle, enabled_sensors,
                                      f"Omitted handle {omitted_handle} was not removed from EnabledSensors as required by spec")

        # Restore AvailableSensors to all supported handles for subsequent steps
        await self.write_available_sensors(endpoint=endpoint, available_sensors=supported_sensor_handles)

        self.step("4a")
        # TH writes to EnabledSensors with a valid list of sensor handles where all handles are present in AvailableSensors.
        test_enabled = [supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
        await self.write_enabled_sensors(endpoint=endpoint, enabled_sensors=test_enabled)
        enabled_sensors = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.EnabledSensors)
        asserts.assert_equal(enabled_sensors, test_enabled, "EnabledSensors does not match written list")

        self.step("4b")
        # TH writes to EnabledSensors with a list containing a sensor handle that is NOT present in AvailableSensors.
        invalid_handle = b'\xff\xff\xff\xff'
        while invalid_handle in supported_sensor_handles:
            # Generate a random handle not in supported_sensor_handles; max 32 handles so this succeeds quickly
            invalid_handle = bytes([random.randint(0, 255) for _ in range(4)])
        # Verify that write returns CONSTRAINT_ERROR (0x87) per spec section 6.5.6.4
        await self.write_enabled_sensors(endpoint=endpoint, enabled_sensors=[invalid_handle],
                                         expected_status=Status.ConstraintError)

        self.step("5a")
        # TH writes to the SensorSchedule attribute with a valid list of SensorScheduleTransitionStruct entries.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        valid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=360,
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
        # TH writes to SensorSchedule with a transition where EnabledSensors contains a sensor handle not in AvailableSensors.
        await self.send_atomic_request_begin(
            {cluster.Attributes.SensorSchedule.attribute_id: Status.Success}, endpoint=endpoint)
        invalid_schedule = [
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kMonday,
                transitionTime=480,
                enabledSensors=[invalid_handle]
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
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
            ),
            cluster.Structs.SensorScheduleTransitionStruct(
                dayOfWeek=cluster.Bitmaps.ScheduleDayOfWeekBitmap.kTuesday | cluster.Bitmaps.ScheduleDayOfWeekBitmap.kWednesday,
                transitionTime=600,
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
                enabledSensors=[supported_sensor_handles[0]] if len(supported_sensor_handles) > 0 else []
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
