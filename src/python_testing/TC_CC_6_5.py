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

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                           default_matter_test_main, has_attribute, run_if_endpoint_matches)
from mobly import asserts

BOOT_WAIT_TIME = 10  # seconds
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
        steps = [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH writes 0x00 to the Options attribute"),
            TestStep("0b", "TH sends MoveToColorTemperature command to DUT"),
            TestStep("0c", "TH reads ColorTemperatureMireds attribute from DUT."),
            TestStep("0d", "TH reads ColorTempPhysicalMinMireds attribute from DUT."),
            TestStep("0e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT."),
            TestStep("1", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("2a", "TH writes to StartUpColorTemperatureMireds attribute with value"),
            TestStep("2b", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("3a", "Power off DUT"),
            TestStep("3b", "Power on DUT"),
            TestStep("4a", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("4b", "TH reads from the DUT the ColorTemperatureMireds attribute"),
            TestStep("5a", "TH reads ColorMode attribute from DUT."),
            TestStep("5b", "TH reads EnhancedColorMode attribute from DUT."),
        ]
        return steps

    @async_test_body
    @run_if_endpoint_matches(has_attribute(Clusters.ColorControl.Attributes.ColorTemperatureMireds))
    async def test_TC_CC_6_5(self):
        cc_cluster = Clusters.Objects.ColorControl
        cc_attributes = cc_cluster.Attributes
        self.endpoint = self.get_endpoint(0)
        self.logger.info(f"Starting test with endpoint {self.endpoint}")

        # commissioning - already done
        self.step("0")

        self.step("0a")
        self.TH1 = self.default_controller
        self.logger.info(f"Writing Options attribute on endpoint {self.endpoint}")
        await self.write_single_attribute(cc_attributes.Options(0x00), self.endpoint, expect_success=True)

        self.step("0b")
        self.logger.info(f"Sending MoveToColorTemperature command to endpoint {self.endpoint}")

        ret = await self.send_single_cmd(
            endpoint=self.endpoint,
            cmd=Clusters.ColorControl.Commands.MoveToColorTemperature(
                colorTemperatureMireds=0x7FFF,
                transitionTime=0,
                optionsMask=0,
                optionsOverride=0
            ),
            node_id=self.dut_node_id)

        self.logger.info("MoveToColorTemperature command sent successfully")
        asserts.assert_true(ret.status == Status.Success,
                            "Error sending MoveToColorTemperature command, status={}".format(str(ret.status)))

        self.step("0c")
        current_color_temp = self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTemperatureMireds, self.TH1, endpoint=self.endpoint)
        self.logger.info(f"Current color temperature response: {current_color_temp}")
        asserts.assert_true(isinstance(current_color_temp, int), "Color temperature value should be an integer")

        self.step("0d")
        min_mireds = self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTempPhysicalMinMireds, self.TH1, endpoint=self.endpoint)
        self.logger.info(f"Extracted min mireds value: {min_mireds}")
        asserts.assert_true(isinstance(min_mireds, int), "Min mireds value should be an integer")

        self.step("0e")
        max_mireds = self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.ColorTempPhysicalMaxMireds, self.TH1, endpoint=self.endpoint)
        self.logger.info(f"Extracted max mireds value: {max_mireds}")
        asserts.assert_true(isinstance(max_mireds, int), "Max mireds value should be an integer")

        self.step("1")
        startup_color_temp_mireds = self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.StartUpColorTemperatureMireds, self.TH1, endpoint=self.endpoint)
        self.logger.info(f"Extracted startup color temperature value: {startup_color_temp_mireds}")
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
        #   StartUpColorTemperatureMireds = (ColorTempPhysicalMaxMireds - ColorTempPhysicalMinMireds)/2 + ColorTempPhysicalMinMired

        tmp_color_temp_mireds = (max_mireds - min_mireds)/2 + min_mireds
        new_color_temp_mireds = None
        if tmp_color_temp_mireds == startup_color_temp_mireds:
            new_color_temp_mireds = (max_mireds - min_mireds)/4 + min_mireds
        else:
            new_color_temp_mireds = tmp_color_temp_mireds

        self.logger.info(f"Defined new value for StartUpColorTemperatureMireds with value {new_color_temp_mireds}")

        # First attempt: Write with minimal parameters
        res = await self.write_single_attribute(cc_attributes.StartUpColorTemperatureMireds(new_color_temp_mireds), self.endpoint, expect_success=True)

        self.step("2b")
        startup_color_temp_mireds = await self.read_single_attribute_check_success(cc_cluster, cc_attributes.StartUpColorTemperatureMireds, self.TH1)
        self.logger.info(f"Verify startup response: {startup_color_temp_mireds}")
        asserts.assert_equal(startup_color_temp_mireds, new_color_temp_mireds,
                             "Startup color temperature should match target value")

        self.step("3a")
        restart_flag_file = self.get_restart_flag_file()
        if self.is_pics_sdk_ci_only() and restart_flag_file:
            with open(restart_flag_file, "w") as f:
                f.write("restart")
            logging.info("Created restart flag file to signal app restart")
            await asyncio.sleep(1)
            self.TH1.ExpireSessions(self.dut_node_id)

        else:
            self.wait_for_user_input("Power Off the device under test.")

        self.step("3b")
        if self.is_pics_sdk_ci_only() and restart_flag_file:
            self.skip_step("3b")
        else:
            self.wait_for_user_input("Power On the device under test.")

        self.step("4a")
        startup_color_temp_mireds = self.read_single_attribute_check_success(
            cc_cluster, cc_attributes.StartUpColorTemperatureMireds, self.TH1, self.endpoint)
        asserts.assert_equal(startup_color_temp_mireds, new_color_temp_mireds,
                             "Post cycle startup color temperature should match target value")

        self.step("4b")
        color_temperature_mireds = self.read_single_attribute(
            cc_cluster, cc_attributes.ColorTemperatureMireds, self.endpoint, self.TH1)
        asserts.assert_equal(color_temperature_mireds, startup_color_temp_mireds,
                             "Post cycle color temperature should match target value")

        self.step("5a")
        color_mode = self.read_single_attribute(cc_cluster, cc_attributes.ColorMode, self.endpoint, self.TH1)
        self.logger.info(f"Color mode response: {color_mode}")
        asserts.assert_in(color_mode, range(0, 3), "Value i in range of 1-2")
        asserts.assert_count_equal(color_mode, 2, "ColorMode value is not 2")

        self.step("5b")
        enhanced_color_mode = self.read_single_attribute(cc_cluster, cc_attributes.EnhancedColorMode, self.endpoint, self.TH1)
        self.logger.info(f"Enhanced mode response: {enhanced_color_mode}")
        asserts.assert_in(color_mode, range(0, 4), "Value i in range of 1-3")
        asserts.assert_count_equal(enhanced_color_mode, 2, "Enhanced color is not 2")


if __name__ == "__main__":
    default_matter_test_main()
