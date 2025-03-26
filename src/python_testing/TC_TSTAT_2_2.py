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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = Clusters.Thermostat


class TC_TSTAT_2_2(MatterBaseTest):

    def desc_TC_TSTAT_2_2(self) -> str:
        """Returns a description of this test"""
        return "42.2.2. [TC-TSTAT-2.2] Setpoint Test Cases with server as DUT"

    def pics_TC_TSTAT_2_2(self):
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return [self.check_pics("TSTAT.S")]

    def steps_TC_TSTAT_2_2(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Test Harness Client reads  attribute OccupiedCoolingSetpoint from the DUT"),
            TestStep("2b", "Test Harness Client then attempts Writes OccupiedCoolingSetpoint to a value below the MinCoolSetpointLimit"),
            TestStep("2c", "Test Harness Writes the limit of MaxCoolSetpointLimit to OccupiedCoolingSetpoint attribute"),
            TestStep("3a", "Test Harness Reads OccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("3b", "Test Harness Writes OccupiedHeatingSetpoint to value below the MinHeatSetpointLimit"),
            TestStep("3c", "Test Harness Writes the limit of MaxHeatSetpointLimit to OccupiedHeatingSetpoint attribute"),
            TestStep("4a", "Test Harness Reads UnoccupiedCoolingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("4b", "Test Harness Writes UnoccupiedCoolingSetpoint to value below the MinCoolSetpointLimit"),
            TestStep("4c", "Test Harness Writes the limit of MaxCoolSetpointLimit to UnoccupiedCoolingSetpoint attribute"),
            TestStep("5a", "Test Harness Reads UnoccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range"),
            TestStep("5b", "Test Harness Writes UnoccupiedHeatingSetpoint to value below the MinHeatSetpointLimit"),
            TestStep("5c", "Test Harness Writes the limit of MaxHeatSetpointLimit to UnoccupiedHeatingSetpoint attribute"),
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

        return steps

    @ async_test_body
    async def test_TC_TSTAT_2_2(self):
        endpoint = self.get_endpoint()

        # Default values for various optional attributes
        AbsMaxCoolSetpointLimitValue = 3200
        AbsMaxHeatSetpointLimitValue = 3000
        AbsMinCoolSetpointLimitValue = 1600
        AbsMinHeatSetpointLimitValue = 700
        MinSetpointDeadBandValue = 250
        MaxCoolSetpointLimitValue = AbsMaxCoolSetpointLimitValue
        MaxHeatSetpointLimitValue = AbsMaxHeatSetpointLimitValue
        MinCoolSetpointLimitValue = AbsMinCoolSetpointLimitValue
        MinHeatSetpointLimitValue = AbsMinHeatSetpointLimitValue

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

        ControlSequenceOfOperation = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

        if self.pics_guard(hasMinCoolSetpointLimitAttribute):
            # Saving value for comparision in step 2a read MinCoolSetpointLimit
            MinCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)

        if self.pics_guard(hasMaxCoolSetpointLimitAttribute):
            # Saving value for comparision in step 2a read MaxCoolSetpointLimit
            MaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)

        if self.pics_guard(hasMinSetpointDeadBandAttribute):
            # Saving value for comparision in step 2c read attribute MinSetpointDeadBand
            MinSetpointDeadBandValue = (await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)) * 10

        if self.pics_guard(hasMinHeatSetpointLimitAttribute):
            # Saving value for comparision in step 3a read MinHeatSetpointLimit
            MinHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)

        if self.pics_guard(hasMaxHeatSetpointLimitAttribute):
            # Saving value for comparision in step 3a read MaxHeatSetpointLimit
            MaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)

        if self.pics_guard(hasOccupiedHeatingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedHeatingSetpoint
            OccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

        if self.pics_guard(hasOccupiedCoolingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedCoolingSetpoint
            OccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

        if self.pics_guard(hasUnoccupiedHeatingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedHeatingSetpoint
            UnoccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)

        if self.pics_guard(hasUnoccupiedCoolingSetpointAttribute):
            # Saving value for comparision in step3c read attribute OccupiedCoolingSetpoint
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

        self.step("2a")

        if self.pics_guard(hasCoolingFeature):
            # Test Harness Client reads  attribute OccupiedCoolingSetpoint from the DUT
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
            asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
            asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

            # Test Harness Client then attempts Writes a value back that is different but valid for OccupiedCoolingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MinCoolSetpointLimitValue + ((MaxCoolSetpointLimitValue - MinCoolSetpointLimitValue) // 2)), endpoint_id=endpoint)

            # Test Harness Client reads it back again to confirm the successful write of OccupiedCoolingSetpoint attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
            asserts.assert_equal(val, MinCoolSetpointLimitValue + ((MaxCoolSetpointLimitValue - MinCoolSetpointLimitValue) // 2))

        self.step("2b")

        if self.pics_guard(hasCoolingFeature):
            # Test Harness Client then attempts  Writes OccupiedCoolingSetpoint to value below the MinCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MinCoolSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes OccupiedCoolingSetpoint to value above the MaxCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MaxCoolSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("2c")

        if self.pics_guard(hasCoolingFeature):
            # Test Harness Writes the limit of MaxCoolSetpointLimit to OccupiedCoolingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes If TSTAT.S.F05(AUTO) LowerLimit = Max(MinCoolSetpointLimit, (OccupiedHeatingSetpoint + MinSetpointDeadBand)) to OccupiedCoolingSetpoint attribute when Auto is enabled
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(max(MinCoolSetpointLimitValue, (OccupiedHeatingSetpointValue + (MinSetpointDeadBandValue)))), endpoint_id=endpoint)

            else:
                # Test Harness Writes the limit of MinCoolSetpointLimit to OccupiedCoolingSetpoint attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MinCoolSetpointLimitValue), endpoint_id=endpoint)

        self.step("3a")

        if self.pics_guard(hasHeatingFeature):
            # Test Harness Reads OccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range
            # Using saved values when optional attributes are available
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
            asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
            asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for OccupiedHeatingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MinHeatSetpointLimitValue + ((MaxHeatSetpointLimitValue - MinHeatSetpointLimitValue) // 2)), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of OccupiedHeatingSetpoint attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
            asserts.assert_equal(val, MinHeatSetpointLimitValue + ((MaxHeatSetpointLimitValue - MinHeatSetpointLimitValue) // 2))

        self.step("3b")

        if self.pics_guard(hasHeatingFeature):
            # Test Harness Writes OccupiedHeatingSetpoint to value below the MinHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MinHeatSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes OccupiedHeatingSetpoint to value above the MaxHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MaxHeatSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("3c")

        if self.pics_guard(hasHeatingFeature):
            # Test Harness Writes the limit of MinHeatSetpointLimit to OccupiedHeatingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MinHeatSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes the limit of MaxHeatSetpointLimit to OccupiedHeatingSetpoint attribute
                upper_limit = min(MaxHeatSetpointLimitValue, (OccupiedCoolingSetpointValue - MinSetpointDeadBandValue))
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(upper_limit), endpoint_id=endpoint)

            else:
                # Test Harness Writes the limit of MaxHeatSetpointLimit to OccupiedHeatingSetpoint attribute If TSTAT.S.F05 is true
                await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

        self.step("4a")

        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            # Test Harness Reads UnoccupiedCoolingSetpoint attribute from Server DUT and verifies that the value is within range
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
            asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
            asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for UnoccupiedCoolingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MinCoolSetpointLimitValue + ((MaxCoolSetpointLimitValue - MinCoolSetpointLimitValue) // 2)), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of UnoccupiedCoolingSetpoint attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
            asserts.assert_equal(val, MinCoolSetpointLimitValue + ((MaxCoolSetpointLimitValue - MinCoolSetpointLimitValue) // 2))

        self.step("4b")

        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            # Test Harness Writes UnoccupiedCoolingSetpoint to value below the MinCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MinCoolSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes UnoccupiedCoolingSetpoint to value above the MaxCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MaxCoolSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("4c")

        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes If TSTAT.S.F05(AUTO) LowerLimit = Max(MinCoolSetpointLimit, (UnoccupiedCoolingSetpoint + MinSetpointDeadBand)) to UnoccupiedCoolingSetpoint attribute
                LowerLimit = max(MinCoolSetpointLimitValue, (UnoccupiedCoolingSetpointValue + MinSetpointDeadBandValue))
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(LowerLimit), endpoint_id=endpoint)

            else:
                # Test Harness Writes the limit of MinCoolSetpointLimit to UnoccupiedCoolingSetpoint attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MinCoolSetpointLimitValue), endpoint_id=endpoint)
            # Test Harness Writes the limit of MaxCoolSetpointLimit to UnoccupiedCoolingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

        self.step("5a")

        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            # Test Harness Reads UnoccupiedHeatingSetpoint attribute from Server DUT and verifies that the value is within range
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)
            asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
            asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for UnoccupiedHeatingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MinHeatSetpointLimitValue + ((MaxHeatSetpointLimitValue - MinHeatSetpointLimitValue) // 2)), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of UnoccupiedHeatingSetpoint attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)
            asserts.assert_equal(val, MinHeatSetpointLimitValue + ((MaxHeatSetpointLimitValue - MinHeatSetpointLimitValue) // 2))

        self.step("5b")

        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            # Test Harness Writes UnoccupiedHeatingSetpoint to value below the MinHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MinHeatSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes UnoccupiedHeatingSetpoint to value above the MaxHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MaxHeatSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("5c")

        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes the limit of MaxHeatSetpointLimit to UnoccupiedHeatingSetpoint attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(max(MaxHeatSetpointLimitValue, (UnoccupiedCoolingSetpointValue + MinSetpointDeadBandValue))), endpoint_id=endpoint)

            else:
                # Test Harness Writes the limit of MaxHeatSetpointLimit to UnoccupiedHeatingSetpoint attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

            # Test Harness Writes the limit of MinHeatSetpointLimit to UnoccupiedHeatingSetpoint attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MinHeatSetpointLimitValue), endpoint_id=endpoint)

        self.step("6a")

        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            # Test Harness Reads MinHeatSetpointLimit attribute from Server DUT and verifies that the value is within range
            # Using saved values when optional attributes are available
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
            asserts.assert_greater_equal(val, AbsMinHeatSetpointLimitValue)
            asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for MinHeatSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue + 1), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of MinHeatSetpointLimit attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
            asserts.assert_equal(val, AbsMinHeatSetpointLimitValue + 1)

        self.step("6b")

        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes a value back that is different but violates the deadband
                existingCoolMinSetpoint = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
                status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit((existingCoolMinSetpoint - MinSetpointDeadBandValue) + 1), endpoint_id=endpoint, expect_success=False)
                asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MinHeatSetpointLimit to value below the AbsMinHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MinHeatSetpointLimit to value above the AbsMaxHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMaxHeatSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("6c")

        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes If TSTAT.S.F05(AUTO) UpperLimit = Min(MaxHeatSetpointLimit, (MinCoolSetpointLimit - MinSetpointDeadBand)) to MinHeatSetpointLimit attribute when Auto is enabled
                # UpperLimit = Min(MaxHeatSetpointLimit,(MinCoolSetpointLimit - MinSetpointDeadBand)) not possible in YAML
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(min(MaxHeatSetpointLimitValue, (MinCoolSetpointLimitValue - MinSetpointDeadBandValue))), endpoint_id=endpoint)
            else:
                # Test Harness Writes the limit of MaxHeatSetpointLimit to MinHeatSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

            # Test Harness Writes the limit of AbsMinHeatSetpointLimit to MinHeatSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue), endpoint_id=endpoint)

        self.step("7a")

        # Test Harness Reads MaxHeatSetpointLimit attribute from Server DUT and verifies that the value is within range
        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
            asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
            asserts.assert_less_equal(val, AbsMaxHeatSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for MaxHeatSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MinHeatSetpointLimitValue + 1), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of MaxHeatSetpointLimit attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
            asserts.assert_equal(val, MinHeatSetpointLimitValue + 1)

        self.step("7b")

        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes the limit of AbsMaxHeatSetpointLimit to MinHeatSetpointLimit attribute
                status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMaxHeatSetpointLimitValue), endpoint_id=endpoint, expect_success=False)
                asserts.assert_equal(status, Status.ConstraintError)
            else:
                # Test Harness Writes the limit of AbsMinHeatSetpointLimit to MinHeatSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue), endpoint_id=endpoint)

                # Test Harness Writes a value back that is different but valid for MaxHeatSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue-1), endpoint_id=endpoint)

                # Test Harness Reads it back again to confirm the successful write of MaxHeatSetpointLimit attribute
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
                asserts.assert_equal(val, AbsMaxHeatSetpointLimitValue-1)

        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            # Test Harness Writes MaxHeatSetpointLimit to value below the AbsMinHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMinHeatSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MaxHeatSetpointLimit to value below the MinHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MinHeatSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MaxHeatSetpointLimit to value above the AbsMaxHeatSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("7c")

        if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes If TSTAT.S.F05(AUTO) UpperLimit = Min(AbsMaxHeatSetpointLimit, (MaxCoolSetpointLimit - MinSetpointDeadBand)) to MaxHeatSetpointLimit attribute
                # UpperLimit = Min(AbsMaxHeatSetpointLimit,(MaxCoolSetpointLimit - MinSetpointDeadBand))
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(min(AbsMaxHeatSetpointLimitValue, MaxCoolSetpointLimitValue - MinSetpointDeadBandValue)), endpoint_id=endpoint)
            else:
                # Test Harness Writes the limit of AbsMaxHeatSetpointLimit to MaxHeatSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue), endpoint_id=endpoint)

            # Test Harness Writes the limit of MinHeatSetpointLimit to MaxHeatSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MinHeatSetpointLimitValue), endpoint_id=endpoint)

        self.step("8a")

        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            # Test Harness Reads MinCoolSetpointLimit attribute from Server DUT and verifies that the value is within range
            # Using saved values when optional attributes are available
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
            asserts.assert_greater_equal(val, AbsMinCoolSetpointLimitValue)
            asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for MinCoolSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue + 1), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of MinCoolSetpointLimit attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
            asserts.assert_equal(val, AbsMinCoolSetpointLimitValue + 1)

        self.step("8b")

        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            # Test Harness Writes MinCoolSetpointLimit to value below the AbsMinCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MinCoolSetpointLimit to value above the MaxCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(MaxCoolSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("8c")

        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            # Test Harness Writes the limit of MaxCoolSetpointLimit to MinCoolSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes If TSTAT.S.F05(AUTO) LowerLimit = Max(AbsMinCoolSetpointLimit, (MinHeatSetpointLimit + MinSetpointDeadBand)) to MinCoolSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(max(AbsMinCoolSetpointLimitValue, MinHeatSetpointLimitValue + MinSetpointDeadBandValue)), endpoint_id=endpoint)
            else:
                # Test Harness Writes the limit of AbsMinCoolSetpointLimit to MinCoolSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue), endpoint_id=endpoint)

        self.step("9a")

        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            # Test Harness Reads MaxCoolSetpointLimit attribute from Server DUT and verifies that the value is within range
            # Using saved values when optional attributes are available
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
            asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
            asserts.assert_less_equal(val, AbsMaxCoolSetpointLimitValue)

            # Test Harness Writes a value back that is different but valid for MaxCoolSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue - 1), endpoint_id=endpoint)

            # Test Harness Reads it back again to confirm the successful write of MaxCoolSetpointLimit attribute
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
            asserts.assert_equal(val, AbsMaxCoolSetpointLimitValue - 1)

        self.step("9b")

        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            # Test Harness Writes MaxCoolSetpointLimit to value below the AbsMinCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(MinCoolSetpointLimitValue - 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes MaxCoolSetpointLimit to value above the AbsMaxCoolSetpointLimit
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue + 1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("9c")

        if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
            # Test Harness Writes the limit of AbsMaxCoolSetpointLimit to MaxCoolSetpointLimit attribute
            await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes the limit of MinCoolSetpointLimit to MaxCoolSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(MinCoolSetpointLimitValue), endpoint_id=endpoint, expect_success=False)

                # Test Harness Writes If TSTAT.S.F05(AUTO) LowerLimit = Max(MinCoolSetpointLimit, (MaxHeatSetpointLimit + MinSetpointDeadBand)) to MaxCoolSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(max(MinCoolSetpointLimitValue, min(AbsMaxCoolSetpointLimitValue, (MaxHeatSetpointLimitValue + MinSetpointDeadBandValue)))), endpoint_id=endpoint)
            else:
                # Test Harness Writes the limit of MinCoolSetpointLimit to MaxCoolSetpointLimit attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(MinCoolSetpointLimitValue), endpoint_id=endpoint)

        self.step("10a")

        if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
            # Test Harness Writes (sets back) default value of MinHeatSetpointLimit
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(MinHeatSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasAutoModeFeature):
                # Test Harness Writes MaxHeatSetpointLimit That meets the deadband of 2.5C
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MaxCoolSetpointLimitValue - MinSetpointDeadBandValue), endpoint_id=endpoint)
            else:
                # Test Harness Writes (sets back)default value of MaxHeatSetpointLimit
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

        self.step("10b")

        if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
            if self.pics_guard(hasMinCoolSetpointLimitAttribute):
                # Test Harness Writes (sets back) default value of MinCoolSetpointLimit
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue), endpoint_id=endpoint)

            if self.pics_guard(hasMaxCoolSetpointLimitAttribute):
                # Test Harness Writes (sets back) default value of MaxCoolSetpointLimit
                await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue), endpoint_id=endpoint)

        self.step("11a")

        # Test Harness Reads MinSetpointDeadBand attribute from Server DUT and verifies that the value is within range
        if self.pics_guard(hasAutoModeFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)
            asserts.assert_less_equal(val, 25)

            if self.pics_guard(self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
                # Test Harness Writes a value back that is different but valid for MinSetpointDeadBand attribute
                await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(5), endpoint_id=endpoint)

                # Test Harness Reads it back again to confirm the successful write of MinSetpointDeadBand attribute
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)
                asserts.assert_equal(val, 5)

        self.step("11b")

        if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
            # Test Harness Writes the value below MinSetpointDeadBand
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(-1), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

            # Test Harness Writes the value above MinSetpointDeadBand
            status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(30), endpoint_id=endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError)

        self.step("11c")

        if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
            # Test Harness Writes the min limit of MinSetpointDeadBand
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(0), endpoint_id=endpoint)

            # Test Harness Writes the max limit of MinSetpointDeadBand
            await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(25), endpoint_id=endpoint)

        self.step("12")

        # Test Harness Reads ControlSequenceOfOperation from Server DUT, if TSTAT.S.F01 is true
        if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [0, 1])

        # Test Harness Reads ControlSequenceOfOperation from Server DUT, if TSTAT.S.F00 is true
        if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [2, 3])

        # Test Harness Reads ControlSequenceOfOperation from Server DUT, if both TSTAT.S.F01 and TSTAT.S.F01 are true
        if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)
            asserts.assert_in(val, [4, 5])

        if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
            # Test Harness  writes value 1 for attribute ControlSequenceOfOperation. If TSTAT.S.F01 is true & TSTAT.S.F00 is false
            await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(1), endpoint_id=endpoint)

            # Test Harness Read it back again to confirm the successful write
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

            # It's acceptable for the server to ignore any attempts to write to the ControlSequenceOfOperation attribute
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, 1)

        # Test Harness writes value 3 for attribute ControlSequenceOfOperation. If TSTAT.S.F00 is true & TSTAT.S.F01 is false
        if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
            await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(3), endpoint_id=endpoint)

        # Test Harness Read it back again to confirm the successful write
        if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

            # It's acceptable for the server to ignore any attempts to write to the ControlSequenceOfOperation attribute
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, 3)

        # Test Harness writes value 4 for attribute ControlSequenceOfOperation. If TSTAT.S.F01 & TSTAT.S.F00 are true
        if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
            await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(4), endpoint_id=endpoint)

        # Test Harness Read it back again to confirm the successful write
        if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

            # It's acceptable for the server to ignore any attempts to write to the ControlSequenceOfOperation attribute
            if val != ControlSequenceOfOperation:
                asserts.assert_equal(val, 4)

        self.step("13")

        if self.pics_guard(hasCoolingFeature):
            # Sets OccupiedCoolingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(OccupiedCoolingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature):
            # Sets OccupiedHeatingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(OccupiedHeatingSetpointValue), endpoint_id=endpoint)

            # Sends SetpointRaise Command Heat Only
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=-30), endpoint=endpoint)

           # Test Harness Reads back OccupiedHeatingSetpoint to confirm the success of the write
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
            asserts.assert_equal(val, OccupiedHeatingSetpointValue - 30 * 10)

        self.step("14")

        if self.pics_guard(hasHeatingFeature):
            # Sets OccupiedHeatingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(OccupiedHeatingSetpointValue), endpoint_id=endpoint)

            # Test Harness Sends SetpointRaise Command Heat Only
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=30), endpoint=endpoint)

            # Test Harness Reads back OccupiedHeatingSetpoint to confirm the success of the write
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
            asserts.assert_equal(val, OccupiedHeatingSetpointValue + 30 * 10)

        self.step("15")

        if self.pics_guard(hasCoolingFeature):
            # Test Harness Sends SetpointRaise Command Cool Only
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kCool, amount=-30), endpoint=endpoint)

            # Test Harness Reads back OccupiedCoolingSetpoint to confirm the success of the write
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
            asserts.assert_equal(val, OccupiedCoolingSetpointValue - 30 * 10)

        self.step("16")

        if self.pics_guard(hasCoolingFeature):
            # Sets OccupiedCoolingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(OccupiedCoolingSetpointValue), endpoint_id=endpoint)

            # Test Harness Sends SetpointRaise Command Cool Only
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kCool, amount=30), endpoint=endpoint)

            # Test Harness Reads back OccupiedCoolingSetpoint to confirm the success of the write
            val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
            asserts.assert_equal(val, OccupiedCoolingSetpointValue + 30 * 10)

        self.step("17")

        if self.pics_guard(hasCoolingFeature):
            # Sets OccupiedCoolingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(OccupiedCoolingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature):
            # Sets OccupiedHeatingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(OccupiedHeatingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
            # Test Harness Sends SetpointRaise Command Heat & Cool
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=-30), endpoint=endpoint)

            if self.pics_guard(hasCoolingFeature):
                # Test Harness Reads back OccupiedCoolingSetpoint to confirm the success of the write
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
                asserts.assert_equal(val, OccupiedCoolingSetpointValue - 30 * 10)

            if self.pics_guard(hasHeatingFeature):
                # Test Harness Reads back OccupiedHeatingSetpoint to confirm the success of the write
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
                asserts.assert_equal(val, OccupiedHeatingSetpointValue - 30 * 10)

        self.step("18")

        if self.pics_guard(hasCoolingFeature):
            # Sets OccupiedCoolingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(OccupiedCoolingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature):
            # Sets OccupiedHeatingSetpoint to default value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(OccupiedHeatingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
            # Test Harness Sends SetpointRaise Command Heat & Cool
            await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=30), endpoint=endpoint)

            if self.pics_guard(hasCoolingFeature):
                # Test Harness Reads back OccupiedCoolingSetpoint to confirm the success of the write
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
                asserts.assert_equal(val, OccupiedCoolingSetpointValue + 30 * 10)

            if self.pics_guard(hasHeatingFeature):
                # Test Harness Reads back OccupiedHeatingSetpoint to confirm the success of the write
                val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
                asserts.assert_equal(val, OccupiedHeatingSetpointValue + 30 * 10)

        if self.pics_guard(hasCoolingFeature):
            # Restores OccupiedCoolingSetpoint to original value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(OccupiedCoolingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasHeatingFeature):
            # Restores OccupiedHeatingSetpoint to original value
            await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(OccupiedHeatingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
            # Restores UnoccupiedCoolingSetpoint to original value
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(UnoccupiedCoolingSetpointValue), endpoint_id=endpoint)

        if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
            # Restores UnoccupiedHeatingSetpoint to original value
            await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(UnoccupiedHeatingSetpointValue), endpoint_id=endpoint)


if __name__ == "__main__":
    default_matter_test_main()
