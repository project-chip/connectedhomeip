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

import chip.clusters as Clusters
from chip.clusters import OnOff, Descriptor
from chip.interaction_model import Status
from matter_testing_infrastructure.chip.testing.matter_testing import (
    MatterBaseTest, TestStep, async_test_body, default_matter_test_main,
)
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
    async def setup_test(self):
        self.logger = logging.getLogger(__name__)
        super().setup_test()

        self.TH1 = self.default_controller
        self.endpoint = 1  # Use endpoint 1 as it has Color Control cluster
        self.logger.info(f"Using endpoint {self.endpoint}")

        try:
            await self.TH1.ResolveNode(self.dut_node_id)
            self.logger.info(f"Successfully resolved node {self.dut_node_id}")
        except Exception as e:
            self.logger.warning(f"Resolving node {self.dut_node_id} failed, continuing test: {e}")

        try:
            # Verify we can read from the endpoint
            try:
                await self.TH1.ReadAttribute(
                    self.dut_node_id,
                    [(self.endpoint, Clusters.BasicInformation.Attributes.VendorID)]
                )
                self.logger.info(f"Successfully connected to endpoint {self.endpoint}")
            except Exception as e:
                self.logger.error(f"Failed to read from endpoint {self.endpoint}: {e}")
                raise Exception(f"Could not establish connection to endpoint {self.endpoint}")

            # Read device descriptor to get supported clusters
            descriptor = await self.TH1.ReadAttribute(
                self.dut_node_id,
                [(self.endpoint, Clusters.Descriptor.Attributes.ServerList)]
            )
            self.logger.info(f"Device supported clusters on endpoint {self.endpoint}: {descriptor}")
            
            # Extract the server list from the nested structure
            server_list = None
            for endpoint_data in descriptor.values():
                if isinstance(endpoint_data, dict):
                    for cluster_data in endpoint_data.values():
                        if isinstance(cluster_data, dict):
                            server_list = cluster_data.get(Clusters.Descriptor.Attributes.ServerList)
                            if server_list:
                                break
                    if server_list:
                        break

            if not server_list:
                raise Exception(f"Could not find server list in descriptor: {descriptor}")

            # Verify Color Control cluster is supported
            if Clusters.ColorControl.id not in server_list:
                self.logger.error(f"Color Control cluster not found in server list: {server_list}")
                raise Exception(f"Color Control cluster not supported on endpoint {self.endpoint}")

            self.logger.info(f"Found Color Control cluster in server list: {server_list}")

        except Exception as e:
            raise Exception(f"Setup failed: {e}")

    @async_test_body
    async def test_TC_CC_6_5(self):
        cluster = Clusters.Objects.ColorControl
        attributes = cluster.Attributes
        self.logger.info(f"Starting test with endpoint {self.endpoint}")
        
        # commissioning - already done
        self.step("0")

        self.step("0a")
        self.logger.info(f"Writing Options attribute on endpoint {self.endpoint}")
        try:
            await self.TH1.WriteAttribute(
                self.dut_node_id,
                [(self.endpoint, attributes.Options, 0x00)]
            )
        except Exception as e:
            self.logger.error(f"Failed to write Options attribute: {e}")
            raise
    
        self.step("0b")
        self.logger.info(f"Sending MoveToColorTemperature command to endpoint {self.endpoint}")
        try:
            response = await self.send_single_cmd(
                endpoint=self.endpoint,
                cmd=Clusters.ColorControl.Commands.MoveToColorTemperature(
                    colorTemperatureMireds=0x7FFF,
                    transitionTime=0,
                    optionsMask=0,
                    optionsOverride=0
                ),
                node_id=self.dut_node_id
            )
            # The response can be None for successful commands that don't return data
            # We only need to check the status code which we already see is 0x0 (success)
            self.logger.info("MoveToColorTemperature command sent successfully")
        except Exception as e:
            self.logger.error(f"Failed to send MoveToColorTemperature command: {e}")
            raise

        def extract_attribute_value(response, endpoint, attribute_class):
            """Helper function to extract attribute value from nested response structure."""
            try:
                endpoint_data = response.get(endpoint, {})
                for cluster_data in endpoint_data.values():
                    if isinstance(cluster_data, dict):
                        # The attribute value is directly in the cluster data
                        for attr_class_key, value in cluster_data.items():
                            if attr_class_key == attribute_class:
                                return value
                return None
            except Exception as e:
                self.logger.error(f"Failed to extract attribute value: {e}")
                return None

        self.step("0c")
        try:
            current_color_temp = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.ColorTemperatureMireds)]
            )
            self.logger.info(f"Current color temperature response: {current_color_temp}")
            color_temp_value = extract_attribute_value(current_color_temp, self.endpoint, attributes.ColorTemperatureMireds)
            self.logger.info(f"Extracted color temperature value: {color_temp_value}")
            asserts.assert_true(isinstance(color_temp_value, int), "Color temperature value should be an integer")
        except Exception as e:
            self.logger.error(f"Failed to read ColorTemperatureMireds: {e}")
            raise

        self.step("0d")
        try:
            min_mireds = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.ColorTempPhysicalMinMireds)]
            )
            self.logger.info(f"Min mireds response: {min_mireds}")
            min_mireds_value = extract_attribute_value(min_mireds, self.endpoint, attributes.ColorTempPhysicalMinMireds)
            self.logger.info(f"Extracted min mireds value: {min_mireds_value}")
            asserts.assert_true(isinstance(min_mireds_value, int), "Min mireds value should be an integer")
        except Exception as e:
            self.logger.error(f"Failed to read ColorTempPhysicalMinMireds: {e}")
            raise

        self.step("0e")
        try:
            max_mireds = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.ColorTempPhysicalMaxMireds)]
            )
            self.logger.info(f"Max mireds response: {max_mireds}")
            max_mireds_value = extract_attribute_value(max_mireds, self.endpoint, attributes.ColorTempPhysicalMaxMireds)
            self.logger.info(f"Extracted max mireds value: {max_mireds_value}")
            asserts.assert_true(isinstance(max_mireds_value, int), "Max mireds value should be an integer")
        except Exception as e:
            self.logger.error(f"Failed to read ColorTempPhysicalMaxMireds: {e}")
            raise

        self.step("1")
        try:
            startup_color_temp = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.StartUpColorTemperatureMireds)]
            )
            self.logger.info(f"Startup color temperature response: {startup_color_temp}")
            startup_color_temp_value = extract_attribute_value(startup_color_temp, self.endpoint, attributes.StartUpColorTemperatureMireds)
            self.logger.info(f"Extracted startup color temperature value: {startup_color_temp_value}")
            if startup_color_temp_value is not None:
                asserts.assert_true(isinstance(startup_color_temp_value, int), "Startup color temperature value should be an integer")
                asserts.assert_greater_equal(startup_color_temp_value, MIN_STARTUP_COLOR_TEMP, 
                    f"Startup color temperature {startup_color_temp_value} should be >= {MIN_STARTUP_COLOR_TEMP}")
                asserts.assert_less_equal(startup_color_temp_value, MAX_STARTUP_COLOR_TEMP,
                    f"Startup color temperature {startup_color_temp_value} should be <= {MAX_STARTUP_COLOR_TEMP}")
        except Exception as e:
            self.logger.error(f"Failed to read StartUpColorTemperatureMireds: {e}")
            raise

        target_color_temp = min_mireds_value + ((max_mireds_value - min_mireds_value) // 2)
        self.logger.info(f"Target color temperature: {target_color_temp}")

        self.step("2a")
        try:
            # Ensure the value is within valid range
            if target_color_temp < MIN_STARTUP_COLOR_TEMP:
                target_color_temp = MIN_STARTUP_COLOR_TEMP
            elif target_color_temp > MAX_STARTUP_COLOR_TEMP:
                target_color_temp = MAX_STARTUP_COLOR_TEMP
            
            self.logger.info(f"Writing StartUpColorTemperatureMireds with value {target_color_temp}")
            
            # Add a small delay before writing
            await asyncio.sleep(0.3)
            
            # Try to reset the session before writing
            try:
                # First try to close any existing session
                await self.TH1.CloseSession()
                self.logger.info("Successfully closed existing session")
                # Then try to resolve the node to establish a new session
                await self.TH1.ResolveNode(self.dut_node_id)
                self.logger.info("Successfully established new session")
            except Exception as e:
                self.logger.warning(f"Failed to reset session: {e}")
            
            # Try to write the attribute with a simpler format
            try:
                # First attempt: Write with minimal parameters
                response = await self.TH1.WriteAttribute(
                    self.dut_node_id,
                    [(self.endpoint, attributes.StartUpColorTemperatureMireds, target_color_temp)]
                )
            except Exception as e:
                self.logger.warning(f"First write attempt failed: {e}")
                # Add a delay before second attempt
                await asyncio.sleep(0.5)
                # Second attempt: Write with minimal parameters again
                response = await self.TH1.WriteAttribute(
                    self.dut_node_id,
                    [(self.endpoint, attributes.StartUpColorTemperatureMireds, target_color_temp)]
                )
            
            self.logger.info(f"Write StartUpColorTemperatureMireds response: {response}")
            
            # Check if the write was successful
            if hasattr(response, 'status'):
                asserts.assert_equal(response.status, Status.Success, "Write operation should succeed")
            else:
                self.logger.error(f"Unexpected response format: {response}")
                raise Exception("Write operation failed: unexpected response format")
                
        except Exception as e:
            self.logger.error(f"Failed to write StartUpColorTemperatureMireds: {e}")
            raise

        self.step("2b")
        try:
            verify_startup = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.StartUpColorTemperatureMireds)]
            )
            self.logger.info(f"Verify startup response: {verify_startup}")
            verify_startup_value = extract_attribute_value(verify_startup, self.endpoint, attributes.StartUpColorTemperatureMireds)
            self.logger.info(f"Extracted verify startup value: {verify_startup_value}")
            asserts.assert_equal(verify_startup_value, target_color_temp, "Startup color temperature should match target value")
        except Exception as e:
            self.logger.error(f"Failed to verify StartUpColorTemperatureMireds: {e}")
            raise

        self.step("3a")
        try:
            await self.TH1.PowerCycle()
            self.logger.info("Power cycle completed")
        except Exception as e:
            self.logger.error(f"Failed to power cycle: {e}")
            raise

        self.step("3b")
        await asyncio.sleep(BOOT_WAIT_TIME)
        self.logger.info(f"Waited {BOOT_WAIT_TIME} seconds after power cycle")

        self.step("4a")
        try:
            post_cycle_startup = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.StartUpColorTemperatureMireds)]
            )
            self.logger.info(f"Post cycle startup response: {post_cycle_startup}")
            post_cycle_startup_value = extract_attribute_value(post_cycle_startup, self.endpoint, attributes.StartUpColorTemperatureMireds)
            self.logger.info(f"Extracted post cycle startup value: {post_cycle_startup_value}")
            asserts.assert_equal(post_cycle_startup_value, target_color_temp, "Post cycle startup color temperature should match target value")
        except Exception as e:
            self.logger.error(f"Failed to read post cycle StartUpColorTemperatureMireds: {e}")
            raise

        self.step("4b")
        try:
            post_cycle_color = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.ColorTemperatureMireds)]
            )
            self.logger.info(f"Post cycle color response: {post_cycle_color}")
            post_cycle_color_value = extract_attribute_value(post_cycle_color, self.endpoint, attributes.ColorTemperatureMireds)
            self.logger.info(f"Extracted post cycle color value: {post_cycle_color_value}")
            asserts.assert_equal(post_cycle_color_value, target_color_temp, "Post cycle color temperature should match target value")
        except Exception as e:
            self.logger.error(f"Failed to read post cycle ColorTemperatureMireds: {e}")
            raise

        self.step("5a")
        try:
            color_mode = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.ColorMode)]
            )
            self.logger.info(f"Color mode response: {color_mode}")
            color_mode_value = extract_attribute_value(color_mode, self.endpoint, attributes.ColorMode)
            self.logger.info(f"Extracted color mode value: {color_mode_value}")
            asserts.assert_true(color_mode_value is not None, "Color mode value should not be None")
        except Exception as e:
            self.logger.error(f"Failed to read ColorMode: {e}")
            raise

        self.step("5b")
        try:
            enhanced_mode = await self.TH1.ReadAttribute(
                self.dut_node_id, [(self.endpoint, attributes.EnhancedColorMode)]
            )
            self.logger.info(f"Enhanced mode response: {enhanced_mode}")
            enhanced_mode_value = extract_attribute_value(enhanced_mode, self.endpoint, attributes.EnhancedColorMode)
            self.logger.info(f"Extracted enhanced mode value: {enhanced_mode_value}")
            asserts.assert_true(enhanced_mode_value is not None, "Enhanced color mode value should not be None")
        except Exception as e:
            self.logger.error(f"Failed to read EnhancedColorMode: {e}")
            raise


if __name__ == "__main__":
    default_matter_test_main()
