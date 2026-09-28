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

from mobly import asserts
from TC_TSTAT_Utils import ThermostatBaseTest, ThermostatSimulator, ThermostatState

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body, pics
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import default_matter_test_main

cluster = Clusters.Thermostat


class TC_TSTAT_2_3(MatterTestCommissionedDevice, ThermostatBaseTest):
    """[TC-TSTAT-2.3] Setpoint Deadband Test Cases with Server as DUT"""

    @pics("TSTAT.S", "TSTAT.S.F05")
    @async_test_body
    async def test_TC_TSTAT_2_3(self) -> None:
        """[TC-TSTAT-2.3] Setpoint Deadband Test Cases with Server as DUT"""
        endpoint = self.get_endpoint()

        # Default values for various optional attributes
        AbsMaxCoolSetpointLimitValue = 3200
        AbsMaxHeatSetpointLimitValue = 3000
        AbsMinCoolSetpointLimitValue = 1600
        AbsMinHeatSetpointLimitValue = 700
        MinSetpointDeadBandValue = 200

        # Thermostat is capable of managing a cooling device
        self.has_cooling = self.check_pics("TSTAT.S.F01")
        # Thermostat is capable of managing a heating device
        self.has_heating = self.check_pics("TSTAT.S.F00")
        # Supports Occupied and Unoccupied setpoints
        self.hasOccupancy = self.check_pics("TSTAT.S.F02")

        # Does the device implement the AbsMaxCoolSetpointLimit attribute?
        hasAbsMaxCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0006")
        # Does the device implement the AbsMaxHeatSetpointLimit attribute?
        hasAbsMaxHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0004")
        # Does the device implement the AbsMinCoolSetpointLimit attribute?
        hasAbsMinCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0005")
        # Does the device implement the AbsMinHeatSetpointLimit attribute?
        hasAbsMinHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0003")
        # Does the device implement the MaxCoolSetpointLimit attribute?
        self.has_max_cool_limit = self.check_pics("TSTAT.S.A0018")
        # Does the device implement the MaxHeatSetpointLimit attribute?
        self.has_max_heat_limit = self.check_pics("TSTAT.S.A0016")
        # Does the device implement the MinCoolSetpointLimit attribute?
        self.has_min_cool_limit = self.check_pics("TSTAT.S.A0017")
        # Does the device implement the MinHeatSetpointLimit attribute?
        self.has_min_heat_limit = self.check_pics("TSTAT.S.A0015")
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

        self.step("1", "Commission DUT to TH", is_commissioning=True)

        OccupiedHeatingSetpointValue = None
        OccupiedCoolingSetpointValue = None
        UnoccupiedHeatingSetpointValue = None
        UnoccupiedCoolingSetpointValue = None

        self.events_callback = None
        self.has_events = False

        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.FeatureMap
        )
        if feature_map & cluster.Bitmaps.Feature.kEvents:
            self.has_events = True
            self.events_callback = EventSubscriptionHandler(expected_cluster=Clusters.Thermostat)
            await self.events_callback.start(self.default_controller, self.dut_node_id, endpoint=endpoint)

        self.step(
            "2",
            "Test Harness Client reads AbsMinHeatSetpointLimit, MinHeatSetpointLimit, AbsMaxCoolSetpointLimit, "
            "MaxCoolSetpointLimit, DeadBand, OccupiedCoolingSetpoint, OccupiedHeatingSetpoint attributes from Server DUT",
        )
        if hasMinSetpointDeadBandAttribute:
            MinSetpointDeadBandValue = (
                await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand
                )
            ) * 10

        if hasOccupiedHeatingSetpointAttribute:
            OccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint
            )

        if hasOccupiedCoolingSetpointAttribute:
            OccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint
            )

        if hasAbsMinHeatSetpointLimitAttribute:
            AbsMinHeatSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit
            )

        if hasAbsMaxHeatSetpointLimitAttribute:
            AbsMaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxHeatSetpointLimit
            )

        if hasAbsMinCoolSetpointLimitAttribute:
            AbsMinCoolSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinCoolSetpointLimit
            )

        if hasAbsMaxCoolSetpointLimitAttribute:
            AbsMaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit
            )

        if self.has_min_heat_limit:
            MinHeatSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit
            )
        else:
            MinHeatSetpointLimitValue = AbsMinHeatSetpointLimitValue

        if self.has_max_heat_limit:
            MaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit
            )
        else:
            MaxHeatSetpointLimitValue = AbsMaxHeatSetpointLimitValue

        if self.has_min_cool_limit:
            MinCoolSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit
            )
        else:
            MinCoolSetpointLimitValue = AbsMinCoolSetpointLimitValue

        if self.has_max_cool_limit:
            MaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit
            )
        else:
            MaxCoolSetpointLimitValue = AbsMaxCoolSetpointLimitValue

        self.step(
            "3",
            "Test Harness Client reads UnoccupiedCoolingSetpoint, UnoccupiedHeatingSetpoint attributes from Server DUT",
        )
        if self.pics_guard(self.hasOccupancy):
            if hasUnoccupiedHeatingSetpointAttribute:
                UnoccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint
                )

            if hasUnoccupiedCoolingSetpointAttribute:
                UnoccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint
                )

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
            hasAuto=True,
            hasOccupancy=self.hasOccupancy,
        )

        if not self.state.valid():
            self.fail(f"Initial thermostat state is not valid: {self.state.__dict__}")

        self.step(
            "4a",
            "If the OccupiedCoolingSetpoint is at least 0.01C less than the minimum of MaxCoolSetpointLimit and "
            "AbsMaxCoolSetpointLimit, the Test Harness Client sets the OccupiedHeatingSetpoint to "
            "(OccupiedCoolingSetpoint - Deadband) + 0.01C",
        )
        max_cool = min(self.state.maxCoolSetpointLimit, self.state.absMaxCoolSetpointLimit)
        if self.state.occupiedCoolingSetpoint <= max_cool - 1:
            old_cool = self.state.occupiedCoolingSetpoint
            target_heat = (self.state.occupiedCoolingSetpoint - self.state.minSetpointDeadBand) + 1
            await self.write_setpoint(cluster.Attributes.OccupiedHeatingSetpoint, target_heat)
            asserts.assert_equal(
                self.state.occupiedCoolingSetpoint,
                old_cool + 1,
                "OccupiedCoolingSetpoint attribute should increase in value by 0.01C",
            )

        self.step("4b", "Test Harness Client reads SetpointChange event from Server DUT")
        # Verified in 4a

        self.step(
            "5a",
            "If the OccupiedHeatingSetpoint is at least 0.01C more than the maximum of MinHeatSetpointLimit and "
            "AbsMinHeatSetpointLimit, the Test Harness Client sets the OccupiedCoolingSetpoint to "
            "(OccupiedHeatingSetpoint + Deadband) - 0.01C",
        )
        min_heat = max(self.state.minHeatSetpointLimit, self.state.absMinHeatSetpointLimit)
        if self.state.occupiedHeatingSetpoint >= min_heat + 1:
            old_heat = self.state.occupiedHeatingSetpoint
            target_cool = (self.state.occupiedHeatingSetpoint + self.state.minSetpointDeadBand) - 1
            await self.write_setpoint(cluster.Attributes.OccupiedCoolingSetpoint, target_cool)
            asserts.assert_equal(
                self.state.occupiedHeatingSetpoint,
                old_heat - 1,
                "OccupiedHeatingSetpoint attribute should decrease in value by 0.01C",
            )

        self.step("5b", "Test Harness Client reads SetpointChange event from Server DUT")
        # Verified in 5a

        self.step(
            "6a",
            "If the UnoccupiedCoolingSetpoint is at least 0.01C less than the minimum of MaxCoolSetpointLimit and "
            "AbsMaxCoolSetpointLimit, the Test Harness Client sets the UnoccupiedHeatingSetpoint to "
            "(UnoccupiedCoolingSetpoint - Deadband) + 0.01C",
        )
        if self.pics_guard(self.hasOccupancy):
            max_cool = min(self.state.maxCoolSetpointLimit, self.state.absMaxCoolSetpointLimit)
            if self.state.unoccupiedCoolingSetpoint <= max_cool - 1:
                old_unocc_cool = self.state.unoccupiedCoolingSetpoint
                target_unocc_heat = (self.state.unoccupiedCoolingSetpoint - self.state.minSetpointDeadBand) + 1
                await self.write_setpoint(cluster.Attributes.UnoccupiedHeatingSetpoint, target_unocc_heat)
                asserts.assert_equal(
                    self.state.unoccupiedCoolingSetpoint,
                    old_unocc_cool + 1,
                    "UnoccupiedCoolingSetpoint attribute should increase in value by 0.01C",
                )

        self.step("6b", "Test Harness Client reads SetpointChange event from Server DUT")
        # Verified in 6a

        self.step(
            "7a",
            "If the UnoccupiedHeatingSetpoint is at least 0.01C more than the maximum of MinHeatSetpointLimit and "
            "AbsMinHeatSetpointLimit, the Test Harness Client sets the UnoccupiedCoolingSetpoint to "
            "(UnoccupiedHeatingSetpoint + Deadband) - 0.01C",
        )
        if self.pics_guard(self.hasOccupancy):
            min_heat = max(self.state.minHeatSetpointLimit, self.state.absMinHeatSetpointLimit)
            if self.state.unoccupiedHeatingSetpoint >= min_heat + 1:
                old_unocc_heat = self.state.unoccupiedHeatingSetpoint
                target_unocc_cool = (self.state.unoccupiedHeatingSetpoint + self.state.minSetpointDeadBand) - 1
                await self.write_setpoint(cluster.Attributes.UnoccupiedCoolingSetpoint, target_unocc_cool)
                asserts.assert_equal(
                    self.state.unoccupiedHeatingSetpoint,
                    old_unocc_heat - 1,
                    "UnoccupiedHeatingSetpoint attribute should decrease in value by 0.01C",
                )

        self.step("7b", "Test Harness Client reads SetpointChange event from Server DUT")
        # Verified in 7a


if __name__ == "__main__":
    default_matter_test_main()
