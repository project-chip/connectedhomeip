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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.Thermostat


class TC_TSTAT_2_2(MatterBaseTest):
    def desc_TC_TSTAT_2_2(self) -> str:
        """Returns a description of this test"""
        return "42.2.2. [TC-TSTAT-2.2] Setpoint Test Cases with server as DUT"

    def pics_TC_TSTAT_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["TSTAT.S"]

    def steps_TC_TSTAT_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done",
                     is_commissioning=True),
            TestStep("2", "Test Harness Client reads AbsMinHeatSetpointLimit, MinHeatSetpointLimit, AbsMaxCoolSetpointLimit, MaxCoolSetpointLimit, DeadBand, OccupiedCoolingSetpoint, OccupiedHeatingSetpoint attributes from Server DUT"),
            TestStep("3", "Test Harness Client reads UnoccupiedCoolingSetpoint, UnoccupiedHeatingSetpoint attributes from Server DUT"),
            TestStep("4", "If the OccupiedCoolingSetpoint is at least 0.01C less than the minimum of MaxCoolSetpointLimit and AbsMaxCoolSetpointLimit, the Test Harness Client sets the OccupiedHeatingSetpoint to (OccupiedCoolingSetpoint - Deadband) + 0.01C",
                     "The OccupiedCoolingSetpoint attribute should increase in value by 0.01C"),
            TestStep("5", "If the OccupiedHeatingSetpoint is at least 0.01C more than the maximum of MinHeatSetpointLimit and AbsMinHeatSetpointLimit, the Test Harness Client sets the OccupiedCoolingSetpoint to (OccupiedHeatingSetpoint + Deadband) - 0.01C",
                     "The OccupiedHeatingSetpoint attribute should decrease in value by 0.01C"),
            TestStep("6", "If the UnoccupiedCoolingSetpoint is at least 0.01C less than the minimum of MaxCoolSetpointLimit and AbsMaxCoolSetpointLimit, the Test Harness Client sets the UnoccupiedHeatingSetpoint to (UnoccupiedCoolingSetpoint - Deadband) + 0.01C",
                     "The UnoccupiedCoolingSetpoint attribute should increase in value by 0.01C"),
            TestStep("7", "If the UnoccupiedHeatingSetpoint is at least 0.01C more than the maximum of MinHeatSetpointLimit and AbsMinHeatSetpointLimit, the Test Harness Client sets the UnoccupiedCoolingSetpoint to (UnoccupiedHeatingSetpoint + Deadband) - 0.01C",
                     "The UnoccupiedHeatingSetpoint attribute should decrease in value by 0.01C"),
        ]
        return steps

    @async_test_body
    async def test_TC_TSTAT_2_2(self):
        endpoint = self.user_params.get("endpoint", 1)

        self.step("1")
        # Commission DUT - already done

        if not self.pics_guard(self.check_pics("TSTAT.S.F05")):
            # Thermostats that do not support the Auto system mode do not have to maintain a deadband
            self.skip_all_remaining_steps("2")
            return

        self.step("2")
        absMinHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit)
        minHeatSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
        absMaxCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit)
        maxCoolSetpointLimit = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
        occupiedCoolingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
        occupiedHeatingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
        deadband = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)
        deadband *= 10  # Unlike all the setpoints it affects, deadband is in 0.1C increments, because reasons
        unoccupiedCoolingSetpoint = None
        unoccupiedHeatingSetpoint = None

        self.step("3")
        if self.pics_guard(self.check_pics("TSTAT.S.F02")):
            unoccupiedCoolingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
            unoccupiedHeatingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)

        maxCool = min(maxCoolSetpointLimit, absMaxCoolSetpointLimit)
        minHeat = max(minHeatSetpointLimit, absMinHeatSetpointLimit)

        self.step("4")
        if occupiedCoolingSetpoint < maxCool:
            targetHeatingSetpoint = (occupiedCoolingSetpoint - deadband) + 1
            targetCoolingSetpoint = targetHeatingSetpoint + deadband
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(targetHeatingSetpoint), endpoint_id=endpoint)
            occupiedCoolingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
            asserts.assert_equal(occupiedCoolingSetpoint, targetCoolingSetpoint, "OccupiedCoolingSetpoint was not adjusted")

        self.step("5")
        if occupiedHeatingSetpoint > minHeat:
            targetCoolingSetpoint = (occupiedHeatingSetpoint + deadband) - 1
            targetHeatingSetpoint = targetCoolingSetpoint - deadband
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(targetCoolingSetpoint), endpoint_id=endpoint)
            occupiedHeatingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
            asserts.assert_equal(occupiedHeatingSetpoint, targetHeatingSetpoint, "OccupiedHeatingSetpoint was not adjusted")

        self.step("6")
        if self.pics_guard(self.check_pics("TSTAT.S.F02")) and unoccupiedCoolingSetpoint < maxCool:
            targetHeatingSetpoint = (unoccupiedCoolingSetpoint - deadband) + 1
            targetCoolingSetpoint = targetHeatingSetpoint + deadband
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(targetHeatingSetpoint), endpoint_id=endpoint)
            unoccupiedCoolingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
            asserts.assert_equal(unoccupiedCoolingSetpoint, targetCoolingSetpoint, "UnoccupiedCoolingSetpoint was not adjusted")

        self.step("7")
        if self.pics_guard(self.check_pics("TSTAT.S.F02")) and unoccupiedHeatingSetpoint > minHeat:
            targetCoolingSetpoint = (unoccupiedHeatingSetpoint + deadband) - 1
            targetHeatingSetpoint = targetCoolingSetpoint - deadband
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(targetCoolingSetpoint), endpoint_id=endpoint)
            unoccupiedHeatingSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)
            asserts.assert_equal(unoccupiedHeatingSetpoint, targetHeatingSetpoint, "UnoccupiedHeatingSetpoint was not adjusted")


if __name__ == "__main__":
    default_matter_test_main()
