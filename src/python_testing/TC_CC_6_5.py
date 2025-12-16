#
#    Copyright (c) 2025 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_attribute, has_command,
                                           run_if_endpoint_matches)

log = logging.getLogger(__name__)

MIN_STARTUP_COLOR_TEMP = 1
MAX_STARTUP_COLOR_TEMP = 0xFEFF


class TC_CC_6_5(MatterBaseTest):
    def desc_TC_CC_6_5(self) -> str:
        return (
            "4.2.18.[TC_CC_6_5] This test case verifies Color Temperature "
            "StartUpColorTemperatureMireds functionality of the Color Control "
            "Cluster server."
        )

    def steps_TC_CC_6_5(self) -> list[TestStep]:
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH writes 0x00 to the Options attribute"),
            TestStep("0b", "TH sends On command to DUT",
                     "Verify DUT responds with a successful (value 0x00) status response."),
            TestStep("0c", "TH reads ColorTemperatureMireds attribute from DUT.",
                     "Verify that the DUT response contains with ColorTemperatureMireds attribute value."),
            TestStep("0d", "TH reads ColorTempPhysicalMinMireds attribute from DUT.",
                     "Verify that the DUT response contains with ColorTempPhysicalMinMireds attribute value."),
            TestStep("0e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT.",
                     "Verify that the DUT response contains with ColorTempPhysicalMaxMireds attribute value."),
            TestStep("1", "TH reads from the DUT the StartUpColorTemperatureMireds attribute",
                     "Verify that the DUT response contains an uint16 [Min:1 Max:0xfeff or null]"),
            TestStep("2a", "TH writes to StartUpColorTemperatureMireds attribute with value",
                     "Verify DUT responds with a successful (value 0x00) status response."),
            TestStep("2b", "TH reads from the DUT the StartUpColorTemperatureMireds attribute",
                     "Verify that the DUT response contains StartUpColorTemperatureMireds that matches the StartUpColorTemperatureMireds set in Step 2a"),
            TestStep("3a", "Power off DUT"),
            TestStep("3b", "Power on DUT"),
            TestStep("4a", "TH reads from the DUT the StartUpColorTemperatureMireds attribute",
                     "Verify that the DUT response indicates that the StartUpColorTemperatureMireds attribute matches the StartUpColorTemperatureMireds set in Step 2a"),
            TestStep("4b", "TH reads from the DUT the ColorTemperatureMireds attribute",
                     "Verify that the DUT response indicates that the ColorTemperatureMireds attribute is StartUpColorTemperatureMireds"),
            TestStep("5a", "TH reads ColorMode attribute from DUT.",
                     "Value has to be between a range of 0x00 to 0x02; Verify that the DUT response indicates that the ColorMode attribute has the expected value 2 (ColorTemperatureMireds)."),
            TestStep("5b", "TH reads EnhancedColorMode attribute from DUT.",
                     "Value has to be between a range of 0x00 to 0x03; Verify that the DUT response indicates that the EnhancedColorMode attribute has the expected value 2 (ColorTemperatureMireds)."),
        ]

    @run_if_endpoint_matches(has_attribute(Clusters.ColorControl.Attributes.ColorTemperatureMireds) and has_command(Clusters.OnOff.Commands.On))
    async def test_TC_CC_6_5(self):
        cc_cluster = Clusters.Objects.ColorControl
        cc_attributes = cc_cluster.Attributes
        self.endpoint = self.get_endpoint()
        log.info(f"Starting test with endpoint {self.endpoint}")

        # commissioning - already done
        self.step("0")

        self.step("0a")
        self.TH1 = self.default_controller
        log.info(f"Writing Options attribute on endpoint {self.endpoint}")
        await self.write_single_attribute(cc_attributes.Options(0x00), self.endpoint, expect_success=True)

        self.step("0b")
        log.info(f"Sending On command to endpoint {self.endpoint}")

        await self.send_single_cmd(
            endpoint=self.endpoint,
            cmd=Clusters.OnOff.Commands.On(),
            node_id=self.dut_node_id)
        log.info("Lights On!")

        self.step("0c")
        color_temp_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTemperatureMireds, self.TH1, endpoint=self.endpoint)
        log.info(f"Current color temperature response: {color_temp_mireds}")
        asserts.assert_true(isinstance(color_temp_mireds, int), "Color temperature value should be an integer")

        self.step("0d")
        colortemp_physical_min_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTempPhysicalMinMireds, self.TH1, endpoint=self.endpoint)
        log.info(f"Extracted min mireds value: {colortemp_physical_min_mireds}")
        asserts.assert_true(isinstance(colortemp_physical_min_mireds, int), "Min mireds value should be an integer")

        self.step("0e")
        colortemp_physical_max_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTempPhysicalMaxMireds, self.TH1, endpoint=self.endpoint)
        log.info(f"Extracted max mireds value: {colortemp_physical_max_mireds}")
        asserts.assert_true(isinstance(colortemp_physical_max_mireds, int), "Max mireds value should be an integer")

        self.step("1")
        startup_color_temp_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.StartUpColorTemperatureMireds, self.TH1, endpoint=self.endpoint)
        log.info(f"Extracted startup color temperature value: {startup_color_temp_mireds}")
        asserts.assert_true(startup_color_temp_mireds is not NullValue, "StartUpColorTemperatureMireds is NullValue")
        asserts.assert_true(isinstance(startup_color_temp_mireds, int),
                            "Startup color temperature value should be an integer")
        asserts.assert_greater_equal(startup_color_temp_mireds, MIN_STARTUP_COLOR_TEMP,
                                     f"Startup color temperature {startup_color_temp_mireds} should be >= {MIN_STARTUP_COLOR_TEMP}")
        asserts.assert_less_equal(startup_color_temp_mireds, MAX_STARTUP_COLOR_TEMP,
                                  f"Startup color temperature {startup_color_temp_mireds} should be <= {MAX_STARTUP_COLOR_TEMP}")

        self.step("2a")
        # If (ColorTempPhysicalMaxMireds - ColorTempPhysicalMinMireds)/2 + ColorTempPhysicalMinMireds = ColorTemperatureMireds then
        #   StartUpColorTemperatureMireds = (ColorTempPhysicalMaxMireds - ColorTempPhysicalMinMireds)/4 + ColorTempPhysicalMinMireds;
        # else
        #   StartUpColorTemperatureMireds = (ColorTempPhysicalMaxMireds - ColorTempPhysicalMinMireds)/2 + ColorTempPhysicalMinMireds
        startup_color_temperature_mireds2a = None
        if ((colortemp_physical_max_mireds - colortemp_physical_min_mireds)/2 + colortemp_physical_min_mireds) == color_temp_mireds:
            startup_color_temperature_mireds2a = (colortemp_physical_max_mireds -
                                                  colortemp_physical_min_mireds)/4 + colortemp_physical_min_mireds
        else:
            startup_color_temperature_mireds2a = (colortemp_physical_max_mireds -
                                                  colortemp_physical_min_mireds)/2 + colortemp_physical_min_mireds

        log.info(f"Defined new value for StartUpColorTemperatureMireds with value {startup_color_temperature_mireds2a}")
        # First attempt: Write with minimal parameters
        await self.write_single_attribute(cc_attributes.StartUpColorTemperatureMireds(startup_color_temperature_mireds2a), self.endpoint, expect_success=True)

        self.step("2b")
        startup_color_temp_mireds = await self.read_single_attribute_check_success(cc_cluster, cc_attributes.StartUpColorTemperatureMireds, dev_ctrl=self.TH1, endpoint=self.endpoint)
        log.info(f"Verify startup response: {startup_color_temp_mireds}")
        asserts.assert_equal(startup_color_temp_mireds, startup_color_temperature_mireds2a,
                             "Startup color temperature should match target value")

        self.step("3a")
        restart_flag_file = self.get_restart_flag_file()
        if self.is_pics_sdk_ci_only and restart_flag_file is not None:
            with open(restart_flag_file, "w") as f:
                f.write("restart")
            log.info("Created restart flag file to signal app restart")
            await asyncio.sleep(1)
            self.TH1.ExpireSessions(self.dut_node_id)
        else:
            self.TH1.ExpireSessions(self.dut_node_id)
            self.wait_for_user_input("Power Off the device under test.")

        self.step("3b")
        # On CI Restart has the PowerOn
        if not (self.is_pics_sdk_ci_only and restart_flag_file is not None):
            self.wait_for_user_input("Power On the device under test.")

        self.step("4a")
        startup_color_temp_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.StartUpColorTemperatureMireds, dev_ctrl=self.TH1, endpoint=self.endpoint)
        asserts.assert_equal(startup_color_temp_mireds, startup_color_temperature_mireds2a,
                             "Post cycle startup color temperature should match target value")

        self.step("4b")
        # Verify ColorTemperatureMireds is valid as StartupColorTemperatureMireds
        color_temperature_mireds = await self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTemperatureMireds, dev_ctrl=self.TH1, endpoint=self.endpoint)
        asserts.assert_true(isinstance(color_temperature_mireds, int),
                            "Startup color temperature value should be an integer")
        asserts.assert_greater_equal(color_temperature_mireds, MIN_STARTUP_COLOR_TEMP,
                                     f"Color temperature {color_temperature_mireds} should be >= {MIN_STARTUP_COLOR_TEMP}")
        asserts.assert_less_equal(color_temperature_mireds, MAX_STARTUP_COLOR_TEMP,
                                  f"Color temperature {color_temperature_mireds} should be <= {MAX_STARTUP_COLOR_TEMP}")
        asserts.assert_equal(color_temperature_mireds, startup_color_temp_mireds,
                             "Post cycle color temperature should match target value")

        self.step("5a")
        color_mode = await self.read_single_attribute_check_success(cc_cluster, cc_attributes.ColorMode, dev_ctrl=self.TH1, endpoint=self.endpoint)
        log.info(f"Color mode response: {color_mode}")
        asserts.assert_in(color_mode, range(0, 3), "Value ColorMode in range of [0,2]")
        asserts.assert_equal(color_mode, 2, "Value ColorMode is not 2")

        self.step("5b")
        enhanced_color_mode = await self.read_single_attribute_check_success(cc_cluster, cc_attributes.EnhancedColorMode, dev_ctrl=self.TH1, endpoint=self.endpoint)
        log.info(f"Enhanced mode response: {enhanced_color_mode}")
        asserts.assert_in(enhanced_color_mode, range(0, 4), "Value EnhancedColorMode in range of [0,3]")
        asserts.assert_equal(enhanced_color_mode, 2, "Value EnhancedColorMode is not 2")


if __name__ == "__main__":
    default_matter_test_main()
