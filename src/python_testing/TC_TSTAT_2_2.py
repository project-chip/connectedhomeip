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

import copy
import logging
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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
			TestStep("1", "Commission DUT to TH"),
			TestStep("2a", "Test Harness Client reads  attribute OccupiedCoolingSetpoint from the DUT"),
			TestStep("2b", "Test Harness Client then attempts  Writes OccupiedCoolingSetpoint to value below the MinCoolSetpointLimit"),
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
		endpoint = self.matter_test_config.endpoint if self.matter_test_config.endpoint is not None else 1

		AbsMaxCoolSetpointLimitValue = None
		AbsMaxHeatSetpointLimitValue = None
		AbsMinCoolSetpointLimitValue = None
		AbsMinHeatSetpointLimitValue = None
		MaxCoolSetpointLimitValue = None
		MaxHeatSetpointLimitValue = None
		MinCoolSetpointLimitValue = None
		MinHeatSetpointLimitValue = None
		MinSetpointDeadBandValue = None
		OccupiedCoolingSetpointValue = None
		OccupiedHeatingSetpointValue = None
		UnoccupiedCoolingSetpointValue = None

		hasAutoModeFeature = self.check_pics("TSTAT.S.F05") # Supports a System Mode of Auto
		hasCoolingFeature = self.check_pics("TSTAT.S.F01") # Thermostat is capable of managing a cooling device
		hasHeatingFeature = self.check_pics("TSTAT.S.F00") # Thermostat is capable of managing a heating device
		hasOccupancyFeature = self.check_pics("TSTAT.S.F02") # Supports Occupied and Unoccupied setpoints

		hasAbsMaxCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0006") # Does the device implement the AbsMaxCoolSetpointLimit attribute?
		hasAbsMaxHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0004") # Does the device implement the AbsMaxHeatSetpointLimit attribute?
		hasAbsMinCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0005") # Does the device implement the AbsMinCoolSetpointLimit attribute?
		hasAbsMinHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0003") # Does the device implement the AbsMinHeatSetpointLimit attribute?
		hasMaxCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0018") # Does the device implement the MaxCoolSetpointLimit attribute?
		hasMaxHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0016") # Does the device implement the MaxHeatSetpointLimit attribute?
		hasMinCoolSetpointLimitAttribute = self.check_pics("TSTAT.S.A0017") # Does the device implement the MinCoolSetpointLimit attribute?
		hasMinHeatSetpointLimitAttribute = self.check_pics("TSTAT.S.A0015") # Does the device implement the MinHeatSetpointLimit attribute?
		hasMinSetpointDeadBandAttribute = self.check_pics("TSTAT.S.A0019") # Does the device implement the MinSetpointDeadBand attribute?
		hasOccupiedCoolingSetpointAttribute = self.check_pics("TSTAT.S.A0011") # Does the device implement the OccupiedCoolingSetpoint attribute?
		hasOccupiedHeatingSetpointAttribute = self.check_pics("TSTAT.S.A0012") # Does the device implement the OccupiedHeatingSetpoint attribute?
		hasUnoccupiedCoolingSetpointAttribute = self.check_pics("TSTAT.S.A0013") # Does the device implement the UnoccupiedCoolingSetpoint attribute?

		self.step("1")

		if self.pics_guard(hasMinCoolSetpointLimitAttribute):
			MinCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)

		if self.pics_guard(hasMaxCoolSetpointLimitAttribute):
			MaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)

		if self.pics_guard(hasMinSetpointDeadBandAttribute):
			MinSetpointDeadBandValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)

		if self.pics_guard(hasMinHeatSetpointLimitAttribute):
			MinHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)

		if self.pics_guard(hasUnoccupiedCoolingSetpointAttribute):
			UnoccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)

		if self.pics_guard(hasMaxHeatSetpointLimitAttribute):
			MaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)

		if self.pics_guard(hasOccupiedHeatingSetpointAttribute):
			OccupiedHeatingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

		if self.pics_guard(hasOccupiedCoolingSetpointAttribute):
			OccupiedCoolingSetpointValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		if self.pics_guard(hasAbsMinHeatSetpointLimitAttribute):
			AbsMinHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinHeatSetpointLimit)

		if self.pics_guard(hasAbsMaxHeatSetpointLimitAttribute):
			AbsMaxHeatSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxHeatSetpointLimit)

		if self.pics_guard(hasAbsMinCoolSetpointLimitAttribute):
			AbsMinCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMinCoolSetpointLimit)

		if self.pics_guard(hasAbsMaxCoolSetpointLimitAttribute):
			AbsMaxCoolSetpointLimitValue = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AbsMaxCoolSetpointLimit)

		self.step("2a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
			asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
			asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasCoolingFeature and not hasMinCoolSetpointLimitAttribute and not hasMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)
			asserts.assert_greater_equal(val, 1600)
			asserts.assert_less_equal(val, 3200)

		if self.pics_guard(hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(2500), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		self.step("2b")

		if self.pics_guard(hasCoolingFeature and not hasMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(30), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# MinCoolSetPointLimit might be negative if not checked before decrement
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #2 Test Harness Client  attempts to write OccupiedCoolingSetpoint below the MinCoolSetpointLimit and confirms that the device does not accept the value.
			# ./chip-tool thermostat write occupied-cooling-setpoint 1000 1 1
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87) for the value which is below of the  MinCoolSetpointLimitValue
			# 
			# [1676028984.901635][19375:19377] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676028984.901706][19375:19377] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676028984.901895][19375:19377] CHIP:EM: <<< [E:18153i M:141749864 (Ack:137408936)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# [1676028984.901980][19375:19377] CHIP:IN: (S) Sending msg 141749864 on secure session with LSID: 58847
			# 

		if self.pics_guard(hasCoolingFeature and not hasMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(4000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MaxCoolSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("2c")

		if self.pics_guard(hasCoolingFeature and not hasMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(3200), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and not hasAutoModeFeature and not hasMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(1600), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and not hasAutoModeFeature and hasMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(MinCoolSetpointLimitValue), endpoint_id=endpoint)

		# LowerLimit = Max(MinCoolSetpointLimit,(OccupiedHeatingSetpoint + MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasAutoModeFeature and hasOccupiedCoolingSetpointAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# Test Harness Client then attempts to write OccupiedCoolingSetpoint to both of the limits of LowerLimit(1600) & MaxCoolSetpointLimit(3200) and confirms that the device does accept the value. See also Note 5.
			# 
			# ./chip-tool thermostat write occupied-cooling-setpoint 1600 1 1
			# On TH(chip-tool) verify that  DUT sends a success response
			# [1676029009.026653][19381:19383] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029009.026688][19381:19383] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029009.026776][19381:19383] CHIP:EM: <<< [E:29492i M:142792974 (Ack:241003612)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10
			# 

		self.step("3a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
			asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
			asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasHeatingFeature and not hasMinHeatSetpointLimitAttribute and not hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)
			asserts.assert_greater_equal(val, 700)
			asserts.assert_less_equal(val, 3000)

		if self.pics_guard(hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(2100), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

		self.step("3b")

		if self.pics_guard(hasHeatingFeature and not hasMinHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(100), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# MinHeatSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client attempts to write OccupiedHeatingSetpoint below the MinHeatSetpointLimit and confirms that the device does not accept the value
			# 
			# ./chip-tool thermostat write  occupied-heating-setpoint 500 1 1
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676029113.651245][19392:19394] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029113.651285][19392:19394] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029113.651397][19392:19394] CHIP:EM: <<< [E:43315i M:59735604 (Ack:107613834)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10
			# 

		if self.pics_guard(hasHeatingFeature and not hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(4010), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MaxHeatSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("3c")

		if self.pics_guard(hasHeatingFeature and not hasAutoModeFeature and not hasMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(3000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and not hasAutoModeFeature and hasMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasAutoModeFeature and not hasMinHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(3000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# UpperLimit = Min(MaxHeatSetpointLimit,(OccupiedCoolingSetpoint - MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasAutoModeFeature and hasOccupiedCoolingSetpointAttribute and hasOccupiedHeatingSetpointAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# Test Harness Client then attempts to write OccupiedHeatingSetpoint to both of the limits of MinHeatSetpointLimit(700) & UpperLimit(3000) and confirms that the device does accept the value.
			# 
			# ./chip-tool thermostat write occupied-heating-setpoint 3000 1 1
			# 
			# On TH(chip-tool) verify that  DUT sends a success response
			# 1676029222.303920][19403:19405] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029222.303955][19403:19405] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029222.304055][19403:19405] CHIP:EM: <<< [E:236i M:251820549 (Ack:138263425)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10
			# 
			# ./chip-tool thermostat write occupied-heating-setpoint 700 1 1
			# On TH(chip-tool) verify that  DUT sends a success response
			# 
			# 
			# [1678943387.942204][770598:770600] CHIP:DMG:                         StatusIB =
			# [1678943387.942212][770598:770600] CHIP:DMG:                         {
			# [1678943387.942220][770598:770600] CHIP:DMG:                                 status = 0x00 (SUCCESS),
			# [1678943387.942228][770598:770600] CHIP:DMG:                         },
			# [1678943387.942236][770598:770600] CHIP:DMG:
			# [1678943387.942243][770598:770600] CHIP:DMG:                 },
			# 

		if self.pics_guard(hasHeatingFeature and not hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(700), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(MinHeatSetpointLimitValue), endpoint_id=endpoint)

		self.step("4a")

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
			asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
			asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMinCoolSetpointLimitAttribute and not hasMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)
			asserts.assert_greater_equal(val, 1600)
			asserts.assert_less_equal(val, 3200)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(2500), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedCoolingSetpoint)

		self.step("4b")

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(500), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# MinCoolSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasMinCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client attempts to write OccupiedHeatingSetpoint below the MinHeatSetpointLimit and confirms that the device does not accept the value
			# 
			# ./chip-tool thermostat write  unoccupied-cooling-setpoint 500 1 1
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676029263.842915][19409:19411] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029263.842943][19409:19411] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029263.843052][19409:19411] CHIP:EM: <<< [E:47350i M:72320003 (Ack:188090912)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# 

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(4010), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MaxCoolSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("4c")

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMaxCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(3200), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

		# LowerLimit = Max(MinCoolSetpointLimit,(UnoccupiedCoolingSetpoint + MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasUnoccupiedCoolingSetpointAttribute and hasAutoModeFeature and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# 
			# below mentioned the example command to verify
			# 
			# ./chip-tool thermostat write unoccupied-cooling-setpoint 2000  1 1
			# On TH(chip-tool) verify that  DUT sends a success response.  As its an optional attribute, we are not getting expected result
			# [1676029365.250210][19420:19422] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029365.250263][19420:19422] CHIP:TOO: Response Failure: IM Error 0x00000586: General error: 0x86 (UNSUPPORTED_ATTRIBUTE)
			# [1676029365.250403][19420:19422] CHIP:EM: <<< [E:16230i M:267641165 (Ack:117850882)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# [1676029365.250467][19420:19422] CHIP:IN: (S) Sending msg 267641165 on secure session with LSID: 45606
			# 

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(1600), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and not hasMinCoolSetpointLimitAttribute and hasAutoModeFeature):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(1600), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasOccupancyFeature and hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedCoolingSetpoint(MinCoolSetpointLimitValue), endpoint_id=endpoint)

		self.step("5a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)
			asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
			asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

		# Using hardcoded values when optional attributes are not available
		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMinHeatSetpointLimitAttribute and not hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)
			asserts.assert_greater_equal(val, 700)
			asserts.assert_less_equal(val, 3000)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(2500), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.UnoccupiedHeatingSetpoint)

		self.step("5b")

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMinHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(100), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# MinHeatSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasMinHeatSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client  attempts to write UnoccupiedHeatingSetpoint below the MinHeatSetpointLimit and confirms that the device does not accept the value
			# 
			# ./chip-tool thermostat write  unoccupied-heating-setpoint 500 1 1
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87). As its an optional attribute, we are not getting expected result
			# 
			# [1676029557.268065][19431:19433] CHIP:DMG: }
			# [1676029557.268220][19431:19433] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029557.268290][19431:19433] CHIP:TOO: Response Failure: IM Error 0x00000586: General error: 0x86 (UNSUPPORTED_ATTRIBUTE)
			# [1676029557.268470][19431:19433] CHIP:EM: <<< [E:8458i M:31631638 (Ack:115187982)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# 

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(4010), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MaxHeatSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("5c")

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(3000), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMaxHeatSetpointLimitAttribute and hasAutoModeFeature):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(3000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# UpperLimit = Min(MaxHeatSetpointLimit,(UnoccupiedCoolingSetpoint - MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasUnoccupiedCoolingSetpointAttribute and hasAutoModeFeature and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# 
			# below mentioned the example command to verify
			# 
			# ./chip-tool thermostat write unoccupied-heating-setpoint 2000  1 1
			# On TH(chip-tool) verify that  DUT sends a success response.  As its an optional attribute, we are not getting expected result
			# 
			# [1676029629.242182][19437:19439] CHIP:DMG: }
			# [1676029629.242250][19437:19439] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029629.242280][19437:19439] CHIP:TOO: Response Failure: IM Error 0x00000586: General error: 0x86 (UNSUPPORTED_ATTRIBUTE)
			# [1676029629.242366][19437:19439] CHIP:EM: <<< [E:51650i M:149591010 (Ack:214538556)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# [1676029629.242402][19437:19439] CHIP:IN: (S) Sending msg 149591010 on secure session with LSID: 6132
			# 

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and not hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(700), endpoint_id=endpoint)

		if self.pics_guard(hasOccupancyFeature and hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.UnoccupiedHeatingSetpoint(MinHeatSetpointLimitValue), endpoint_id=endpoint)

		self.step("6a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute and hasAbsMinHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
			asserts.assert_greater_equal(val, AbsMinHeatSetpointLimitValue)
			asserts.assert_less_equal(val, MaxHeatSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasMaxHeatSetpointLimitAttribute and not hasAbsMinHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)
			asserts.assert_greater_equal(val, 700)
			asserts.assert_less_equal(val, 3000)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(800), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinHeatSetpointLimit)

		self.step("6b")

		if self.pics_guard(hasMinHeatSetpointLimitAttribute and hasAutoModeFeature):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(2000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasAbsMinHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(100), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# AbsMinHeatSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasAbsMinHeatSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# #1. Test Harness Client  attempts to write MinHeatSetpointLimit below the AbsMinHeatSetpointLimit and confirms that the device does not accept the value.
			# ./chip-tool thermostat write min-heat-setpoint-limit 300  1 1
			# On TH  (chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676029705.839179][19450:19452] CHIP:DMG: }
			# [1676029705.839253][19450:19452] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029705.839287][19450:19452] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029705.839411][19450:19452] CHIP:EM: <<< [E:34538i M:185292291 (Ack:127655640)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# [1676029705.839456][19450:19452] CHIP:IN: (S) Sending msg 185292291 on secure session with LSID: 61694
			# 

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(4050), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(MaxHeatSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("6c")

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasAutoModeFeature and not hasMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(3000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasAutoModeFeature and hasMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(MaxHeatSetpointLimitValue), endpoint_id=endpoint)

		# UpperLimit = Min(MaxHeatSetpointLimit,(MinCoolSetpointLimit - MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasAutoModeFeature and hasAbsMinCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Test Harness Client then attempts to write MinHeatSetpointLimit to both of the limits of AbsMinHeatSetpointLimit & UpperLimit and confirms that the device does accept the value.
			# below is an exaple command.
			# 
			# ./chip-tool thermostat write min-heat-setpoint-limit 1200  1 1
			# 
			# On TH(chip-tool) verify that  DUT sends a success response
			# 
			# [1676029742.329801][19458:19460] CHIP:DMG:                         }
			# [1676029742.329846][19458:19460] CHIP:DMG:
			# [1676029742.329883][19458:19460] CHIP:DMG:                         StatusIB =
			# [1676029742.329922][19458:19460] CHIP:DMG:                         {
			# [1676029742.329962][19458:19460] CHIP:DMG:                                 status = 0x00 (SUCCESS),
			# [1676029742.330001][19458:19460] CHIP:DMG:                         },
			# [1676029742.330040][19458:19460] CHIP:DMG:
			# 

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasAbsMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(700), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasAbsMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue), endpoint_id=endpoint)

		self.step("7a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and hasAbsMaxHeatSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
			asserts.assert_greater_equal(val, MinHeatSetpointLimitValue)
			asserts.assert_less_equal(val, AbsMaxHeatSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and not hasAbsMaxHeatSetpointLimitAttribute and not hasMaxHeatSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)
			asserts.assert_greater_equal(val, 700)
			asserts.assert_less_equal(val, 3000)

		self.step("7b")

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(700), endpoint_id=endpoint)

		if self.pics_guard(hasMinHeatSetpointLimitAttribute and hasAutoModeFeature):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(3000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(2900), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxHeatSetpointLimit)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasMinHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(100), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# MinHeatSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and hasMinHeatSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client  attempts to write MaxHeatSetpointLimit below the MinHeatSetpointLimit and confirms that the device does not accept the value.
			# ./chip-tool thermostat write  max-heat-setpoint-limit 100 1 1
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676029976.380552][19475:19477] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676029976.380598][19475:19477] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676029976.380705][19475:19477] CHIP:EM: <<< [E:10254i M:97110781 (Ack:264765613)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# 

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAbsMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(4000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and hasAbsMaxHeatSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("7c")

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and not hasAbsMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(3000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and hasAbsMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue), endpoint_id=endpoint)

		# UpperLimit = Min(AbsMaxHeatSetpointLimit,(MaxCoolSetpointLimit - MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and hasAutoModeFeature and hasMaxCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation
			# 
			# Test Harness Client then attempts to write MaxHeatSetpointLimit to both of the limits of MinHeatSetpointLimit & UpperLimit and confirms that the device does accept the value.
			# 
			# below is an examble command
			# 
			# ./chip-tool thermostat write  max-heat-setpoint-limit 2000( Consider the different value but valid4) 1 1
			# On TH(chip-tool) verify that  DUT sends a success response
			# 
			# [1676030018.852009][19481:19483] CHIP:DMG:                                 Cluster = 0x201,
			# [1676030018.852054][19481:19483] CHIP:DMG:                                 Attribute = 0x0000_0016,
			# [1676030018.852094][19481:19483] CHIP:DMG:                         }
			# [1676030018.852138][19481:19483] CHIP:DMG:
			# [1676030018.852178][19481:19483] CHIP:DMG:                         StatusIB =
			# [1676030018.852218][19481:19483] CHIP:DMG:                         {
			# [1676030018.852258][19481:19483] CHIP:DMG:                                 status = 0x00 (SUCCESS),
			# [1676030018.852297][19481:19483] CHIP:DMG:                         },
			# [1676030018.852337][19481:19483] CHIP:DMG:
			# [1676030018.852370][19481:19483] CHIP:DMG:                 },
			# 

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(700), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and hasMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(MinHeatSetpointLimitValue), endpoint_id=endpoint)

		self.step("8a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMaxCoolSetpointLimitAttribute and hasAbsMinCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
			asserts.assert_greater_equal(val, AbsMinCoolSetpointLimitValue)
			asserts.assert_less_equal(val, MaxCoolSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasMaxCoolSetpointLimitAttribute and not hasAbsMinCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)
			asserts.assert_greater_equal(val, 1600)
			asserts.assert_less_equal(val, 3200)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(2000), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinCoolSetpointLimit)

		self.step("8b")

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasAbsMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(500), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# AbsMinCoolSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasAbsMinCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client then attempts to write MinCoolSetpointLimit below the AbsMinCoolSetpointLimit  and confirms that the device does not accept the value.
			# 
			# ./chip-tool thermostat write min-cool-setpoint-limit 400  1 1
			# 
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676030095.274876][19495:19497] CHIP:DMG: }
			# [1676030095.274936][19495:19497] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676030095.274965][19495:19497] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676030095.275097][19495:19497] CHIP:EM: <<< [E:51146i M:60043723 (Ack:28649429)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10
			# 

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(4000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(MaxCoolSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("8c")

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(3200), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(MaxCoolSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasAbsMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(1600), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasAbsMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue), endpoint_id=endpoint)

		# LowerLimit = Max(AbsMinCoolSetpointLimit,(MinHeatSetpointLimit + MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasMinHeatSetpointLimitAttribute and hasAutoModeFeature and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# Test Harness Client then attempts to write MinCoolSetpointLimit to both of the limits of LowerLimit & MaxCoolSetpointLimit and confirms that the device does accept the value.
			# 
			# ./chip-tool thermostat write min-cool-setpoint-limit 1600 1 1
			# 
			# On TH(chip-tool) verify that  DUT sends a success response
			# 
			# [1678947233.814534][773656:773658] CHIP:DMG:
			# [1678947233.814541][773656:773658] CHIP:DMG:                         StatusIB =
			# [1678947233.814548][773656:773658] CHIP:DMG:                         {
			# [1678947233.814555][773656:773658] CHIP:DMG:                                 status = 0x00 (SUCCESS),
			# [1678947233.814562][773656:773658] CHIP:DMG:                         },
			# [1678947233.814570][773656:773658] CHIP:DMG:
			# [1678947233.814576][773656:773658] CHIP:DMG:                 },
			# 

		self.step("9a")

		# Using saved values when optional attributes are available
		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasMinCoolSetpointLimitAttribute and hasAbsMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
			asserts.assert_greater_equal(val, MinCoolSetpointLimitValue)
			asserts.assert_less_equal(val, AbsMaxCoolSetpointLimitValue)

		# Using hard coded values when optional attributes are not available
		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasMinCoolSetpointLimitAttribute and not hasAbsMaxCoolSetpointLimitAttribute):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)
			asserts.assert_greater_equal(val, 1600)
			asserts.assert_less_equal(val, 3200)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(2000), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaxCoolSetpointLimit)

		self.step("9b")

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasMinCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(500), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		# AbsMinCoolSetpointLimit might be negative if not checked before decrement
		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasMinCoolSetpointLimitAttribute and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# #1. Test Harness Client attempts to write MaxCoolSetpointLimit below the MinCoolSetpointLimit and confirms that the device does not accept the value.
			# 
			# ./chip-tool thermostat write max-cool-setpoint-limit 100 1 1
			# 
			# On TH(chip-tool) verify that  DUT sends a CONSTRAINT_ERROR (0x87)
			# [1676030395.441963][19525:19527] CHIP:DMG: }
			# [1676030395.442028][19525:19527] CHIP:DMG: WriteClient moving to [AwaitingDe]
			# [1676030395.442059][19525:19527] CHIP:TOO: Response Failure: IM Error 0x00000587: General error: 0x87 (CONSTRAINT_ERROR)
			# [1676030395.442147][19525:19527] CHIP:EM: <<< [E:35527i M:176995637 (Ack:93643096)] (S) Msg TX to 1:0000000000000001 [E8D5] --- Type 0000:10 (SecureChannel:StandaloneAck)
			# 

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAbsMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(4000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasAbsMaxCoolSetpointLimitAttribute):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue + 1000), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("9c")

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAbsMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(3200), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasAbsMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(1600), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasMinCoolSetpointLimitAttribute and hasAutoModeFeature):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(1600), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasMinCoolSetpointLimitAttribute and not hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(MinCoolSetpointLimitValue), endpoint_id=endpoint)

		# LowerLimit = Max(MinCoolSetpointLimit,(MaxHeatSetpointLimit + MinSetpointDeadBand)) not possible in YAML
		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasMaxHeatSetpointLimitAttribute and hasAutoModeFeature and self.check_pics("PICS_SKIP_SAMPLE_APP")):
			pass
			# TODO: Rewrite this user prompt test
			#
			# Optional Attribute - If it is supported, then in TH log it will results in displaying the value, else it will display UNSUPPORTED_ATTRIBUTE. Below is the log of RPI the result may be vary on the basis of dut implementation.
			# 
			# Test Harness Client then attempts to write MaxCoolSetpointLimit to both of the limits of LowerLimit & AbsMaxCoolSetpointLimit and confirms that the device does accept the value.
			# below is an example command
			# 
			# ./chip-tool thermostat write max-cool-setpoint-limit 2000 1 1
			# 
			# On TH verify that  DUT sends a success response
			# 
			# [1678947558.840324][773890:773892] CHIP:DMG:
			# [1678947558.840331][773890:773892] CHIP:DMG:                         StatusIB =
			# [1678947558.840339][773890:773892] CHIP:DMG:                         {
			# [1678947558.840347][773890:773892] CHIP:DMG:                                 status = 0x00 (SUCCESS),
			# [1678947558.840354][773890:773892] CHIP:DMG:                         },
			# [1678947558.840362][773890:773892] CHIP:DMG:
			# [1678947558.840369][773890:773892] CHIP:DMG:                 },
			# 

		self.step("10a")

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasAbsMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(AbsMinHeatSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and not hasAbsMinHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinHeatSetpointLimit(700), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and hasAbsMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(AbsMaxHeatSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and not hasAutoModeFeature and not hasAbsMaxHeatSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(3000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasMaxHeatSetpointLimitAttribute and hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(2950), endpoint_id=endpoint)

		self.step("10b")

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and hasAbsMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(AbsMinCoolSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMinCoolSetpointLimitAttribute and not hasAbsMinCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinCoolSetpointLimit(1600), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and hasAbsMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(AbsMaxCoolSetpointLimitValue), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and hasMaxCoolSetpointLimitAttribute and not hasAbsMaxCoolSetpointLimitAttribute):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxCoolSetpointLimit(3200), endpoint_id=endpoint)

		self.step("11a")

		if self.pics_guard(hasAutoModeFeature):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)
			asserts.assert_greater_equal(val, 0)
			asserts.assert_less_equal(val, 25)

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(5), endpoint_id=endpoint)

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MinSetpointDeadBand)

		self.step("11b")

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(-1), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			status = await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(30), endpoint_id=endpoint, expect_success = False)
			asserts.assert_equal(status, Status.ConstraintError)

		self.step("11c")

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(0), endpoint_id=endpoint)

		if self.pics_guard(hasAutoModeFeature and self.check_pics("TSTAT.S.M.MinSetpointDeadBandWritable")):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MinSetpointDeadBand(25), endpoint_id=endpoint)

		self.step("12")

		if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
			val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(1), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature and not hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(3), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and not hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.ControlSequenceOfOperation(4), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature and hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ControlSequenceOfOperation)

		if self.pics_guard(hasHeatingFeature and hasMinHeatSetpointLimitAttribute and hasAutoModeFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.MaxHeatSetpointLimit(2950), endpoint_id=endpoint)

		self.step("13")

		if self.pics_guard(hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(2600), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(2000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=-30))

		if self.pics_guard(hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

		self.step("14")

		if self.pics_guard(hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(2000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kHeat, amount=30))

		if self.pics_guard(hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

		self.step("15")

		if self.pics_guard(hasCoolingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kCool, amount=-30))

		if self.pics_guard(hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		self.step("16")

		if self.pics_guard(hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(2600), endpoint_id=endpoint)

		if self.pics_guard(hasCoolingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kCool, amount=30))

		if self.pics_guard(hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		self.step("17")

		if self.pics_guard(hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(2600), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(2000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=-30))

		if self.pics_guard(hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

		self.step("18")

		if self.pics_guard(hasCoolingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedCoolingSetpoint(2600), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.write_single_attribute(attribute_value=cluster.Attributes.OccupiedHeatingSetpoint(2000), endpoint_id=endpoint)

		if self.pics_guard(hasHeatingFeature or hasCoolingFeature):
			await self.send_single_cmd(cmd=Clusters.Objects.Thermostat.Commands.SetpointRaiseLower(mode=Clusters.Objects.Thermostat.Enums.SetpointRaiseLowerModeEnum.kBoth, amount=30))

		if self.pics_guard(hasCoolingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedCoolingSetpoint)

		if self.pics_guard(hasHeatingFeature):
			await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.OccupiedHeatingSetpoint)

if __name__ == "__main__":
	default_matter_test_main()
