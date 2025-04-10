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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

# Standard library imports
import asyncio

# Local/Matter imports
import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_infrastructure.chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                                                       default_matter_test_main)
# Third-party imports
from mobly import asserts

# Constants
BOOT_WAIT_TIME = 10  # seconds
MIN_STARTUP_COLOR_TEMP = 1
MAX_STARTUP_COLOR_TEMP = 0xfeff


class TC_CC_6_5(MatterBaseTest):
    """Test case for Color Control Cluster's StartUpColorTemperatureMireds functionality.

    This test verifies the Color Temperature StartUpColorTemperatureMireds functionality 
    of the Color Control Cluster server as specified in section 4.2.18.
    """

    def desc_TC_CC_6_5(self) -> str:
        """Return a description of this test case."""
        return (
            "4.2.18.[TC_CC_6_5] This test case verifies Color Temperature "
            "StartUpColorTemperatureMireds functionality of the Color Control "
            "Cluster server."
        )

    def steps_TC_CC_6_5(self) -> list[TestStep]:
        """Define the test steps for TC_CC_6_5."""
        return [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH writes 0x00 to the Options attribute"),
            TestStep("0b", "TH sends On command to DUT"),
            TestStep("0c", "TH reads ColorTemperatureMireds attribute from DUT."),
            TestStep("0d", "TH reads ColorTempPhysicalMinMireds attribute from DUT."),
            TestStep("0e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT."),
            TestStep(
                "1",
                "TH reads from the DUT the StartUpColorTemperatureMireds attribute"
            ),
            TestStep(
                "2a",
                "TH writes to StartUpColorTemperatureMireds attribute with value"
            ),
            TestStep(
                "2b",
                "TH reads from the DUT the StartUpColorTemperatureMireds attribute"
            ),
            TestStep("3a", "Power off DUT"),
            TestStep("3b", "Power on DUT"),
            TestStep(
                "4a",
                "TH reads from the DUT the StartUpColorTemperatureMireds attribute"
            ),
            TestStep(
                "4b",
                "TH reads from the DUT the ColorTemperatureMireds attribute"
            ),
            TestStep("5a", "TH reads ColorMode attribute from DUT."),
            TestStep("5b", "TH reads EnhancedColorMode attribute from DUT.")
        ]

    @async_test_body
    async def setup_test(self):
        """Set up the test environment and establish secure connection with DUT."""
        super().setup_test()
        self.TH1 = self.default_controller

        try:
            await self.TH1.ResolveNode(self.dut_node_id)
        except Exception as e:
            self.logger.warning(
                f"Resolving node {self.dut_node_id} failed, continuing test: {e}"
            )

        if not self.TH1.HasSecureSessionToNode(self.dut_node_id):
            raise Exception(
                f"Secure session to node {self.dut_node_id} could not be established."
            )

    @async_test_body
    async def test_TC_CC_6_5(self):
        """Execute the main test steps for TC_CC_6_5.

        Tests the Color Control Cluster's StartUpColorTemperatureMireds functionality
        through a series of read/write operations and power cycle verification.
        """
        cluster = Clusters.Objects.ColorControl
        attributes = cluster.Attributes

        # Step 0a: Write 0x00 to Options attribute
        self.step("0a")
        try:
            await self.TH1.WriteAttribute(
                self.dut_node_id,
                [(attributes.Options, attributes.Options.Type(0x00))]
            )
        except Exception as e:
            self.logger.error(f"Failed to write Options attribute: {e}")
            raise

        # Step 0b: Send On command and verify response
        self.step("0b")
        try:
            response = await self.TH1.WriteAttribute(
                self.dut_node_id,
                [(Clusters.Objects.OnOff.Attributes.OnOff, True)]
            )
            asserts.assert_equal(
                response.status,
                Status.Success,
                f"Expected status Success (0x00), got {response.status}"
            )
        except Exception as e:
            self.logger.error(f"Failed to send On command: {e}")
            raise

        # Read and verify color temperature attributes
        self.step("0c")
        try:
            current_color_temp = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.ColorTemperatureMireds)]
            )
            asserts.assert_true(
                isinstance(current_color_temp.value, int),
                "ColorTemperatureMireds must be an integer value"
            )
        except Exception as e:
            self.logger.error(f"Failed to read ColorTemperatureMireds: {e}")
            raise

        # Read physical limits
        self.step("0d")
        try:
            min_mireds = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.ColorTempPhysicalMinMireds)]
            )
        except Exception as e:
            self.logger.error(f"Failed to read ColorTempPhysicalMinMireds: {e}")
            raise

        self.step("0e")
        try:
            max_mireds = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.ColorTempPhysicalMaxMireds)]
            )
        except Exception as e:
            self.logger.error(f"Failed to read ColorTempPhysicalMaxMireds: {e}")
            raise

        # Read and verify StartUpColorTemperatureMireds
        self.step("1")
        try:
            startup_color_temp = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.StartUpColorTemperatureMireds)]
            )

            # Verify value constraints
            if startup_color_temp.value is not None:
                asserts.assert_true(
                    isinstance(startup_color_temp.value, int),
                    "StartUpColorTemperatureMireds must be an integer or null"
                )
                asserts.assert_in_range(
                    startup_color_temp.value,
                    MIN_STARTUP_COLOR_TEMP,
                    MAX_STARTUP_COLOR_TEMP,
                    "StartUpColorTemperatureMireds out of valid range"
                )
        except Exception as e:
            self.logger.error(f"Failed to read StartUpColorTemperatureMireds: {e}")
            raise

        # Calculate target color temperature (midpoint)
        target_color_temp = min_mireds.value + (
            (max_mireds.value - min_mireds.value) // 2
        )

        # Write and verify StartUpColorTemperatureMireds
        self.step("2a")
        try:
            response = await self.TH1.WriteAttribute(
                self.dut_node_id,
                [(attributes.StartUpColorTemperatureMireds, target_color_temp)]
            )
            asserts.assert_equal(
                response.status,
                Status.Success,
                f"Failed to write StartUpColorTemperatureMireds: {response.status}"
            )
        except Exception as e:
            self.logger.error(
                f"Failed to write StartUpColorTemperatureMireds: {e}"
            )
            raise

        # Verify written value
        self.step("2b")
        try:
            verify_startup = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.StartUpColorTemperatureMireds)]
            )
            asserts.assert_equal(
                verify_startup.value,
                target_color_temp,
                "StartUpColorTemperatureMireds verification failed"
            )
        except Exception as e:
            self.logger.error(
                f"Failed to verify StartUpColorTemperatureMireds: {e}"
            )
            raise

        # Power cycle test
        self.step("3a")
        await self.TH1.PowerCycle()

        # Allow sufficient time for DUT to complete boot sequence
        self.step("3b")
        await asyncio.sleep(BOOT_WAIT_TIME)

        # Verify attributes after power cycle
        self.step("4a")
        try:
            post_cycle_startup = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.StartUpColorTemperatureMireds)]
            )
            asserts.assert_equal(
                post_cycle_startup.value,
                target_color_temp,
                "StartUpColorTemperatureMireds mismatch after power cycle"
            )
        except Exception as e:
            self.logger.error(
                f"Failed to read StartUpColorTemperatureMireds after power cycle: {e}"
            )
            raise

        self.step("4b")
        try:
            post_cycle_color = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.ColorTemperatureMireds)]
            )
            asserts.assert_equal(
                post_cycle_color.value,
                target_color_temp,
                "ColorTemperatureMireds mismatch after power cycle"
            )
        except Exception as e:
            self.logger.error(
                f"Failed to read ColorTemperatureMireds after power cycle: {e}"
            )
            raise

        # Verify color modes
        self.step("5a")
        try:
            color_mode = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.ColorMode)]
            )
            asserts.assert_equal(
                color_mode.value,
                attributes.ColorMode.StartUpColorTemperatureMireds,
                "Incorrect ColorMode value"
            )
        except Exception as e:
            self.logger.error(f"Failed to read ColorMode: {e}")
            raise

        self.step("5b")
        try:
            enhanced_mode = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(attributes.EnhancedColorMode)]
            )
            asserts.assert_equal(
                enhanced_mode.value,
                attributes.EnhancedColorMode.ColorTemperatureMireds,
                "Incorrect EnhancedColorMode value"
            )
        except Exception as e:
            self.logger.error(f"Failed to read EnhancedColorMode: {e}")
            raise


if __name__ == "__main__":
    default_matter_test_main()
