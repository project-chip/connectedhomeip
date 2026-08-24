#
#    Copyright (c) 2024 Project CHIP Authors
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

import queue

from mobly import asserts
from TC_TSTAT_Utils import ThermostatSimulator, ThermostatState

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

cluster = Clusters.Thermostat


class TC_TSTAT_2_2(MatterBaseTest):

    def desc_TC_TSTAT_2_2(self) -> str:
        """Returns a description of this test"""
        return "42.2.2. [TC-TSTAT-2.2] Setpoint Test Cases with server as DUT"

    def pics_TC_TSTAT_2_2(self):
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return [self.check_pics("TSTAT.S")]

    def steps_TC_TSTAT_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("1a", "Test Harness Client subscribes to events"),
            TestStep("2a", "Test Harness Client reads  attribute OccupiedCoolingSetpoint from the DUT"),
            TestStep("2b", "Test Harness Client then attempts Writes OccupiedCoolingSetpoint to a value below the MinCoolSetpointLimit"),
            TestStep("2c", "Test Harness Writes the limit of MaxCoolSetpointLimit to OccupiedCoolingSetpoint attribute"),
            TestStep("2d", "Test Harness checks for SetpointChange event"),
            TestStep("3a", "Test Harness Reads OccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("3b", "Test Harness Writes OccupiedHeatingSetpoint to value below the MinHeatSetpointLimit"),
            TestStep("3c", "Test Harness Writes the limit of MaxHeatSetpointLimit to OccupiedHeatingSetpoint attribute"),
            TestStep("3d", "Test Harness checks for SetpointChange event"),
            TestStep("4a", "Test Harness Reads UnoccupiedCoolingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("4b", "Test Harness Writes UnoccupiedCoolingSetpoint to value below the MinCoolSetpointLimit"),
            TestStep("4c", "Test Harness Writes the limit of MaxCoolSetpointLimit to UnoccupiedCoolingSetpoint attribute"),
            TestStep("4d", "Test Harness checks for SetpointChange event"),
            TestStep("5a", "Test Harness Reads UnoccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("5b", "Test Harness Writes UnoccupiedHeatingSetpoint to value below the MinHeatSetpointLimit"),
            TestStep("5c", "Test Harness Writes the limit of MaxHeatSetpointLimit to UnoccupiedHeatingSetpoint attribute"),
            TestStep("5d", "Test Harness checks for SetpointChange event"),
            TestStep("6a", "Test Harness Reads MinHeatSetpointLimit attribute from Server DUT and verifies that the value is within range"),
            TestStep("6b", "Test Harness Writes a value back that is different but violates the deadband"),
            TestStep("6c", "Test Harness Writes the limit of MaxHeatSetpointLimit to MinHeatSetpointLimit attribute"),
            TestStep("7a", "Test Harness Reads MaxHeatSetpointLimit attribute from Server DUT and verifies that the value is within range"),
            TestStep("7b", "Test Harness Writes the limit of AbsMinHeatSetpointLimit to MinHeatSetpointLimit attribute"),
            TestStep("7c", "Test Harness Writes the limit of AbsMaxHeatSetpointLimit to MaxHeatSetpointLimit attribute"),
            TestStep("8a", "Test Harness Reads MinCoolSetpointLimit attribute from Server DUT and verifies that the value is within range"),
            TestStep("8b", "Test Harness Writes MinCoolSetpointLimit to value below the AbsMinCoolSetpointLimit "),
            TestStep("8c", "Test Harness Writes the limit of MaxCoolSetpointLimit to MinCoolSetpointLimit attribute"),
            TestStep("9a", "Test Harness Reads MaxCoolSetpointLimit attribute from Server DUT and verifies that the value is within range"),
            TestStep("9b", "Test Harness Writes MaxCoolSetpointLimit to value below the AbsMinCoolSetpointLimit "),
            TestStep("9c", "Test Harness Writes the limit of AbsMaxCoolSetpointLimit to MaxCoolSetpointLimit attribute"),
            TestStep("10a", "Test Harness Writes (sets back) default value of MinHeatSetpointLimit"),
            TestStep("10b", "Test Harness Writes (sets back) default value of MinCoolSetpointLimit"),
            TestStep("11a", "Test Harness Reads MinSetpointDeadBand attribute from Server DUT and verifies that the value is within range"),
            TestStep("11b", "Test Harness Writes the value below MinSetpointDeadBand"),
            TestStep("11c", "Test Harness Writes the min limit of MinSetpointDeadBand"),
            TestStep("12", "Test Harness Reads ControlSequenceOfOperation from Server DUT, if TSTAT.S.F01 is true"),
            TestStep("13", "Sets OccupiedCoolingSetpoint to default value"),
            TestStep("14", "Sets OccupiedHeatingSetpoint to default value"),
            TestStep("15", "Test Harness Sends SetpointRaise Command Cool Only"),
            TestStep("16", "Sets OccupiedCoolingSetpoint to default value"),
            TestStep("17", "Sets OccupiedCoolingSetpoint to default value"),
            TestStep("18", "Sets OccupiedCoolingSetpoint to default value"),
        ]

    async def verify_events(self, expected_events: list[dict], events_callback: EventSubscriptionHandler) -> None:
        received_events = []
        while len(received_events) < len(expected_events):
            try:
                event_result = events_callback.get_event_from_queue(block=True, timeout=self.matter_test_config.timeout)
                if event_result.Header.EventId == cluster.Events.SetpointChange.event_id:
                    received_events.append(event_result.Data)
            except queue.Empty:
                break

        # Read any extra unexpected events non-blockingly
        while True:
            try:
                event_result = events_callback.get_event_from_queue(block=False, timeout=0)
                if event_result.Header.EventId == cluster.Events.SetpointChange.event_id:
                    received_events.append(event_result.Data)
            except queue.Empty:
                break

        # Verify that the number of received events matches the expected number
        asserts.assert_equal(len(received_events), len(expected_events),
                             f"Expected {len(expected_events)} events, but received {len(received_events)}: {received_events}")

        # Verify each expected event matches one received event
        for expected in expected_events:
            matched = False
            for idx, received in enumerate(received_events):
                if (received.systemMode == expected['systemMode'] and
                    received.occupancy == expected['occupancy'] and
                        received.currentSetpoint == expected['currentSetpoint']):
                    matched = True
                    received_events.pop(idx)
                    break
            asserts.assert_true(matched, f"Could not find expected event: {expected} in received events: {received_events}")

    async def read_and_verify_all_attributes(self, endpoint, cluster):
        for attr_class, current_val in [
            (cluster.Attributes.OccupiedHeatingSetpoint, self.state.occupiedHeatingSetpoint),
            (cluster.Attributes.OccupiedCoolingSetpoint, self.state.occupiedCoolingSetpoint),
            (cluster.Attributes.UnoccupiedHeatingSetpoint, self.state.unoccupiedHeatingSetpoint),
            (cluster.Attributes.UnoccupiedCoolingSetpoint, self.state.unoccupiedCoolingSetpoint),
            (cluster.Attributes.MinHeatSetpointLimit, self.state.minHeatSetpointLimit),
            (cluster.Attributes.MaxHeatSetpointLimit, self.state.maxHeatSetpointLimit),
            (cluster.Attributes.MinCoolSetpointLimit, self.state.minCoolSetpointLimit),
            (cluster.Attributes.MaxCoolSetpointLimit, self.state.maxCoolSetpointLimit)
        ]:
            has_attr = False
            match attr_class:
                case cluster.Attributes.OccupiedHeatingSetpoint:
                    has_attr = self.has_heating
                case cluster.Attributes.OccupiedCoolingSetpoint:
                    has_attr = self.has_cooling
                case cluster.Attributes.UnoccupiedHeatingSetpoint:
                    has_attr = self.has_heating and self.hasOccupancy
                case cluster.Attributes.UnoccupiedCoolingSetpoint:
                    has_attr = self.has_cooling and self.hasOccupancy
                case cluster.Attributes.MinHeatSetpointLimit:
                    has_attr = self.has_heating and self.has_min_heat_limit
                case cluster.Attributes.MaxHeatSetpointLimit:
                    has_attr = self.has_heating and self.has_max_heat_limit
                case cluster.Attributes.MinCoolSetpointLimit:
                    has_attr = self.has_cooling and self.has_min_cool_limit
                case cluster.Attributes.MaxCoolSetpointLimit:
                    has_attr = self.has_cooling and self.has_max_cool_limit

            if has_attr:
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr_class)
                asserts.assert_equal(val, current_val, f"Mismatch for {attr_class}")

    async def write_setpoint(self, attribute, value):
        endpoint = self.get_endpoint()
        attr_id = attribute.attribute_id

        old_state = self.state.copy()
        expected_status, new_state, changed_ids = self.simulator.write(self.state, attr_id, value)

        if expected_status != Status.Success:
            status = await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, expected_status)
            return

        if self.has_events:
            self.events_callback.flush_events()

        await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint)

        self.state = new_state

        if self.has_events:
            expected_events = self.simulator.get_expected_events(old_state, self.state, changed_ids, attr_id)
            await self.verify_events(expected_events, self.events_callback)

        await self.read_and_verify_all_attributes(endpoint, cluster)

    async def send_raise_lower_and_verify(self, mode, amount):
        endpoint = self.get_endpoint()

        old_state = self.state.copy()
        expected_status, new_state, changed_ids = self.simulator.raise_lower(self.state, mode, amount)
        asserts.assert_equal(expected_status, Status.Success)

        if self.has_events:
            self.events_callback.flush_events()

        await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=mode, amount=amount), endpoint=endpoint)

        self.state = new_state

        if self.has_events:
            expected_events = self.simulator.get_expected_events(old_state, self.state, changed_ids, None)
            if len(expected_events) > 0:
                await self.verify_events(expected_events, self.events_callback)

        await self.read_and_verify_all_attributes(endpoint, cluster)

    @async_test_body
    async def test_TC_TSTAT_2_2(self):
        endpoint = self.get_endpoint()

        # Default values for various optional attributes
        AbsMaxCoolSetpointLimitValue = 3200
        AbsMaxHeatSetpointLimitValue = 3000
        AbsMinCoolSetpointLimitValue = 1600
        AbsMinHeatSetpointLimitValue = 700
        MinSetpointDeadBandValue = 200

        # Supports a System Mode of Auto
        hasAutoModeFeature = self.check_pics("TSTAT.S.F05")
        # Thermostat is capable of managing a cooling device
        hasCoolingFeature = self.check_pics("TSTAT.S.F01")
        # Thermostat is capable of managing a heating device
        hasHeatingFeature = self.check_pics("TSTAT.S.F00")
        # Supports Occupied and Unoccupied setpoints
        hasOccupancyFeature = self.check_pics("TSTAT.S.F02")

        # Does the device implement the AbsMaxCoolSetpointLimit attribute?
        hasAbsMaxCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0006")
        # Does the device implement the AbsMaxHeatSetpointLimit attribute?
        hasAbsMaxHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0004")
        # Does the device implement the AbsMinCoolSetpointLimit attribute?
        hasAbsMinCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0005")
        # Does the device implement the AbsMinHeatSetpointLimit attribute?
        hasAbsMinHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0003")
        # Does the device implement the MaxCoolSetpointLimit attribute?
        hasMaxCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0018")
        # Does the device implement the MaxHeatSetpointLimit attribute?
        hasMaxHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0016")
        # Does the device implement the MinCoolSetpointLimit attribute?
        hasMinCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0017")
        # Does the device implement the MinHeatSetpointLimit attribute?
        hasMinHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0015")
        # Does the device implement the MinSetpointDeadBand attribute?
        hasMinSetpointDeadBandAttribute = self.check_pics("TSTAT.S.A0019")
        # Does the device implement the OccupiedCoolingSetpoint attribute?
        hasOccupiedCoolingSetpointAttribute = self.check_pics("TSTAT.S.A0011")
        # Does the device implement the OccupiedHeatingSetpoint attribute?
        hasOccupiedHeatingSetpointAttribute = self.check_pics("TSTAT.S.A0012")
        # Does the device implement the UnoccupiedCoolingSetpoint attribute?
        hasUnoccupiedCoolingSetpointAttribute = self.check_pics("TSTAT.S.A0013")
        # Does the device implement the UnoccupiedHeatingSetpoint attribute?
        hasUnoccupiedHeatingSetpointAttribute = self.check_pics("TSTAT.S.A0014")

        self.step("1")

        OccupiedHeatingSetpointValue = None
        OccupiedCoolingSetpointValue = None
        UnoccupiedHeatingSetpointValue = None
        UnoccupiedCoolingSetpointValue = None

        events_callback = None
        hasEventsFeature = False

        self.step("1a")
        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kEvents):
            hasEventsFeature = True

            events_callback = EventSubscriptionHandler(expected_cluster=Clusters.Thermostat)
            await events_callback.start(self.default_controller,
                                        self.dut_node_id,
                                        endpoint=endpoint)
            self.events_callback = events_callback

        ControlSequenceOfOperation = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

        if self.pics_guard(hasMinSetpointDeadBandAttribute):
            # Saving value for comparision in step 2c read attribute MinSetpointDeadBand
            MinSetpointDeadBandValue = (await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)) * 10

        if self.pics_guard(hasOccupiedHeatingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedHeatingSetpoint
            OccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

        if self.pics_guard(hasOccupiedCoolingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedCoolingSetpoint
            OccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

        if self.pics_guard(hasUnoccupiedHeatingSetpointAttribute):
            # Saving value for comparision in step3c read attribute UnoccupiedHeatingSetpoint
            UnoccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)

        if self.pics_guard(hasUnoccupiedCoolingSetpointAttribute):
            # Saving value for comparision in step3c read attribute UnoccupiedCoolingSetpoint
            UnoccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)

        if self.pics_guard(hasAbsMinHeatSetpointLimitAttribute):
            # Saving value for comparision in step 6a read attribute AbsMinHeatSetpointLimit
            AbsMinHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit)

        if self.pics_guard(hasAbsMaxHeatSetpointLimitAttribute):
            # Saving value for comparision in step 7a read attribute AbsMaxHeatSetpointLimit
            AbsMaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxHeatSetpointLimit)

        if self.pics_guard(hasAbsMinCoolSetpointLimitAttribute):
            # Saving value for comparision in step 8a read attribute AbsMinCoolSetpointLimit
            AbsMinCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinCoolSetpointLimit)

        if self.pics_guard(hasAbsMaxCoolSetpointLimitAttribute):
            # Saving value for comparision in step9a read attribute AbsMaxCoolSetpointLimit
            AbsMaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit)

        if self.pics_guard(hasMinHeatSetpointLimitAttribute):
            # Saving value for comparision in step 3a read MinHeatSetpointLimit
            MinHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
        else:
            MinHeatSetpointLimitValue = AbsMinHeatSetpointLimitValue

        if self.pics_guard(hasMaxHeatSetpointLimitAttribute):
            # Saving value for comparision in step 3a read MaxHeatSetpointLimit
            MaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
        else:
            MaxHeatSetpointLimitValue = AbsMaxHeatSetpointLimitValue

        if self.pics_guard(hasMinCoolSetpointLimitAttribute):
            # Saving value for comparision in step 2a read MinCoolSetpointLimit
            MinCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
        else:
            MinCoolSetpointLimitValue = AbsMinCoolSetpointLimitValue

        if self.pics_guard(hasMaxCoolSetpointLimitAttribute):
            # Saving value for comparision in step 2a read MaxCoolSetpointLimit
            MaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
        else:
            MaxCoolSetpointLimitValue = AbsMaxCoolSetpointLimitValue

        # Initialize support flags
        self.has_heating = hasHeatingFeature
        self.has_cooling = hasCoolingFeature
        self.hasAuto = hasAutoModeFeature
        self.hasOccupancy = hasOccupancyFeature
        self.has_events = hasEventsFeature

        self.has_min_heat_limit = hasMinHeatSetpointLimitAttribute
        self.has_max_heat_limit = hasMaxHeatSetpointLimitAttribute
        self.has_min_cool_limit = hasMinCoolSetpointLimitAttribute
        self.has_max_cool_limit = hasMaxCoolSetpointLimitAttribute

        # Initialize simulator and state
        self.simulator = ThermostatSimulator()
        self.state = ThermostatState(
            occupiedHeatingSetpoint=OccupiedHeatingSetpointValue,
            occupiedCoolingSetpoint=OccupiedCoolingSetpointValue,
            unoccupiedHeatingSetpoint=UnoccupiedHeatingSetpointValue,
            unoccupiedCoolingSetpoint=UnoccupiedCoolingSetpointValue,
            minHeatSetpointLimit=MinHeatSetpointLimitValue,
            maxHeatSetpointLimit=MaxHeatSetpointLimitValue,
            minCoolSetpointLimit=MinCoolSetpointLimitValue,
            maxCoolSetpointLimit=MaxCoolSetpointLimitValue,
            absMinHeatSetpointLimit=AbsMinHeatSetpointLimitValue,
            absMaxHeatSetpointLimit=AbsMaxHeatSetpointLimitValue,
            absMinCoolSetpointLimit=AbsMinCoolSetpointLimitValue,
            absMaxCoolSetpointLimit=AbsMaxCoolSetpointLimitValue,
            minSetpointDeadBand=MinSetpointDeadBandValue,
            hasHeat=self.has_heating,
            hasCool=self.has_cooling,
            hasAuto=self.hasAuto,
            hasOccupancy=self.hasOccupancy
        )

        if not self.state.valid():
            self.fail(f"Initial thermostat state is not valid: {self.state.__dict__}")

        self.step("2a")
        if self.pics_guard(hasCoolingFeature):
            target_val = self.state.minCoolSetpointLimit + \
                ((self.state.maxCoolSetpointLimit - self.state.minCoolSetpointLimit) // 2)
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, target_val)

        self.step("2b")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, self.state.minCoolSetpointLimit - 1)
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, self.state.maxCoolSetpointLimit + 1)

        self.step("2c")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, self.state.maxCoolSetpointLimit)
            if self.pics_guard(hasAutoModeFeature):
                target = max(self.state.minCoolSetpointLimit, self.state.occupiedHeatingSetpoint + self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, target)
            else:
                await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, self.state.minCoolSetpointLimit)

        self.step("2d")
        # Verified in 2c

        self.step("3a")
        if self.pics_guard(hasHeatingFeature):
            target_val = self.state.minHeatSetpointLimit + \
                ((self.state.maxHeatSetpointLimit - self.state.minHeatSetpointLimit) // 2)
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, target_val)

        self.step("3b")
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, self.state.minHeatSetpointLimit - 1)
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, self.state.maxHeatSetpointLimit + 1)

        self.step("3c")
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, self.state.minHeatSetpointLimit)
            if self.pics_guard(hasAutoModeFeature):
                target = min(self.state.maxHeatSetpointLimit, self.state.occupiedCoolingSetpoint - self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, target)
            else:
                await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, self.state.maxHeatSetpointLimit)

        self.step("3d")
        # Verified in 3c

        self.step("4a")
        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            target_val = self.state.minCoolSetpointLimit + \
                ((self.state.maxCoolSetpointLimit - self.state.minCoolSetpointLimit) // 2)
            await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, target_val)

        self.step("4b")
        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, self.state.minCoolSetpointLimit - 1)
            await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, self.state.maxCoolSetpointLimit + 1)

        self.step("4c")
        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            if self.pics_guard(hasAutoModeFeature):
                target = max(self.state.minCoolSetpointLimit, self.state.unoccupiedHeatingSetpoint + self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, target)
            else:
                await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, self.state.minCoolSetpointLimit)
            await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, self.state.maxCoolSetpointLimit)

        self.step("4d")
        # Verified in 4c

        self.step("5a")
        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            target_val = self.state.minHeatSetpointLimit + \
                ((self.state.maxHeatSetpointLimit - self.state.minHeatSetpointLimit) // 2)
            await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, target_val)

        self.step("5b")
        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, self.state.minHeatSetpointLimit - 1)
            await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, self.state.maxHeatSetpointLimit + 1)

        self.step("5c")
        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            if self.pics_guard(hasAutoModeFeature):
                target = min(self.state.maxHeatSetpointLimit, self.state.unoccupiedCoolingSetpoint - self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, target)
            else:
                await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, self.state.maxHeatSetpointLimit)
            await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, self.state.minHeatSetpointLimit)

        self.step("5d")
        # Verified in 5c

        self.step("6a")
        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMinHeatSetpointLimit + 1)

        self.step("6b")
        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                target = (self.state.minCoolSetpointLimit - self.state.minSetpointDeadBand) + 10
                await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, target)
            await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMinHeatSetpointLimit - 1)
            await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMaxHeatSetpointLimit + 1)

        self.step("6c")
        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                target = min(self.state.maxHeatSetpointLimit, self.state.minCoolSetpointLimit - self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, target)
            else:
                await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.maxHeatSetpointLimit)
            await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMinHeatSetpointLimit)

        self.step("7a")
        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.minHeatSetpointLimit + 10)

        self.step("7b")
        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMaxHeatSetpointLimit)
            else:
                await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, self.state.absMinHeatSetpointLimit)
                await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.absMaxHeatSetpointLimit - 10)

        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.absMinHeatSetpointLimit - 10)
            await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.minHeatSetpointLimit - 10)
            await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.absMaxHeatSetpointLimit + 10)

        self.step("7c")
        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                target = min(self.state.absMaxHeatSetpointLimit, self.state.maxCoolSetpointLimit - self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, target)
            else:
                await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.absMaxHeatSetpointLimit)
            await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, self.state.minHeatSetpointLimit)

        self.step("8a")
        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, self.state.absMinCoolSetpointLimit + 10)

        self.step("8b")
        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, self.state.absMinCoolSetpointLimit - 10)
            await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, self.state.maxCoolSetpointLimit + 10)

        self.step("8c")
        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, self.state.maxCoolSetpointLimit)
            if self.pics_guard(hasAutoModeFeature):
                target = max(self.state.absMinCoolSetpointLimit, self.state.minHeatSetpointLimit + self.state.minSetpointDeadBand)
                await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, target)
            else:
                await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, self.state.absMinCoolSetpointLimit)

        self.step("9a")
        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.absMaxCoolSetpointLimit - 10)

        self.step("9b")
        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.minCoolSetpointLimit - 10)
            await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.absMaxCoolSetpointLimit + 10)

        self.step("9c")
        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.absMaxCoolSetpointLimit)
            if self.pics_guard(hasAutoModeFeature):
                await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.minCoolSetpointLimit)
            else:
                await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, self.state.minCoolSetpointLimit)

        self.step("10a")
        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            await self.write_setpoint(cluster.Attributes.MinHeatSetpointLimit, MinHeatSetpointLimitValue)
            if self.pics_guard(hasAutoModeFeature):
                await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, MaxCoolSetpointLimitValue - MinSetpointDeadBandValue)
            else:
                await self.write_setpoint(cluster.Attributes.MaxHeatSetpointLimit, MaxHeatSetpointLimitValue)

        self.step("10b")
        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            if self.pics_guard(hasMinCoolSetpointLimitAttribute):
                await self.write_setpoint(cluster.Attributes.MinCoolSetpointLimit, AbsMinCoolSetpointLimitValue)
            if self.pics_guard(hasMaxCoolSetpointLimitAttribute):
                await self.write_setpoint(cluster.Attributes.MaxCoolSetpointLimit, AbsMaxCoolSetpointLimitValue)

        self.step("11a")
        if self.pics_guard(hasAutoModeFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)
            asserts.assert_less_equal(val, 127)

            if self.pics_guard(self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
                try:
                    # This won't actually change anything, as "writeable" deadbands are not actually writeable
                    # This is for backwards compatibility reasons
                    await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, 5)
                finally:
                    await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, MinSetpointDeadBandValue // 10)

        self.step("11b")
        if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
            # These are invalid deadbands, so despite it not really being "writeable", these calls should fail anyway
            await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, -1)
            await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, 128)

        self.step("11c")
        if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
            try:
                await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, 0)
                await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, 127)
            finally:
                await self.write_setpoint(cluster.Attributes.MinSetpointDeadBand, MinSetpointDeadBandValue // 10)

        self.step("12")
        if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [
                cluster.Enums.ControlSequenceOfOperationEnum.kCoolingOnly,
                cluster.Enums.ControlSequenceOfOperationEnum.kCoolingWithReheat
            ])

        if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [
                cluster.Enums.ControlSequenceOfOperationEnum.kHeatingOnly,
                cluster.Enums.ControlSequenceOfOperationEnum.kHeatingWithReheat
            ])

        if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [
                cluster.Enums.ControlSequenceOfOperationEnum.kCoolingAndHeating,
                cluster.Enums.ControlSequenceOfOperationEnum.kCoolingAndHeatingWithReheat
            ])

        if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
            await self.write_single_attribute(
                attribute_value=cluster.Attributes.ControlSequenceOfOperation(
                    cluster.Enums.ControlSequenceOfOperationEnum.kCoolingWithReheat),
                endpoint_id=endpoint
            )
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, cluster.Enums.ControlSequenceOfOperationEnum.kCoolingWithReheat)

        if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
            await self.write_single_attribute(
                attribute_value=cluster.Attributes.ControlSequenceOfOperation(
                    cluster.Enums.ControlSequenceOfOperationEnum.kHeatingWithReheat),
                endpoint_id=endpoint
            )
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, cluster.Enums.ControlSequenceOfOperationEnum.kHeatingWithReheat)

        if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
            await self.write_single_attribute(
                attribute_value=cluster.Attributes.ControlSequenceOfOperation(
                    cluster.Enums.ControlSequenceOfOperationEnum.kCoolingAndHeating),
                endpoint_id=endpoint
            )
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, cluster.Enums.ControlSequenceOfOperationEnum.kCoolingAndHeating)

        self.step("13")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, OccupiedCoolingSetpointValue)
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, OccupiedHeatingSetpointValue)
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=-30)

        self.step("14")
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, OccupiedHeatingSetpointValue)
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=30)

        self.step("15")
        if self.pics_guard(hasCoolingFeature):
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kCool, amount=-30)

        self.step("16")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, OccupiedCoolingSetpointValue)
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kCool, amount=30)

        self.step("17")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, OccupiedCoolingSetpointValue)
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, OccupiedHeatingSetpointValue)
        if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=-30)

        self.step("18")
        if self.pics_guard(hasCoolingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, OccupiedCoolingSetpointValue)
        if self.pics_guard(hasHeatingFeature):
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, OccupiedHeatingSetpointValue)
        if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
            await self.send_raise_lower_and_verify(mode=cluster.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=30)


if __name__ == "__main__":
    default_matter_test_main()
