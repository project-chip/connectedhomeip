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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md"#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:TRACE_APP.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:TRACE_TEST_JSON.json
#       --trace-to perfetto:TRACE_TEST_PERFETTO.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===""import typing

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from chip.clusters.Types import NullValue

logger = logging.getLogger(__name__)


class MOD_1_2(MatterBaseTest):
    """Proposal test for Mode Select Cluster attributes as a server."""

    def desc_MOD_1_2(self) -> str:
        return "80.2.1. [TC-MOD-1.2] Cluster attributes with DUT as Server"

    def pics_MOD_1_2(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "MOD.S"
        ]
        return pics

    def steps_MOD_1_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).", is_commissioning=True),
            TestStep(2, "TH reads the SupportedModes attribute from DUT"),
            TestStep(3, "TH reads the CurrentMode attribute from the DUT"),
            TestStep(4, "TH reads the OnMode attribute from the DUT"),
            TestStep(5, "TH reads the StartUpMode attribute from the DUT"),
            TestStep(6, "TH reads the Description attribute from the DUT"),
            TestStep(7, "TH reads the StandardNamespace attribute from the DUT")
        ]
        return steps

    def _verify_supported_mode(self, supported_mode):
        """Verify supported mode.
        Each mode should be a struct containing a label (user understandable string describing the mode), mode number (integer that identifies the mode and is unique within this SupportedMode list) and  list of semantic tags
        MfgCode and Value fields present in the SemanticTags structs should be no larger than 16 bits on the TH (Chip-tool)."""
        if not hasattr(supported_mode, 'label') and not isinstance(supported_mode.label, str):
            asserts.fail("Supported mode struct does not have attribute label or does not contain string ")

        if not hasattr(supported_mode, 'mode') and not isinstance(supported_mode.mode, int):
            asserts.fail("Supported modes struct does not have attribute mode or does not contain int")

        if not hasattr(supported_mode, 'semanticTags'):
            asserts.fail("Supported mode struct does not have attribute semanticTags")

        # veirfy if we have entries for semanticTags if there are any  verfy values no larger that 16 bits
        # entries must have the value and manufacturer code.
        self._log_attribute("Semantic tags", supported_mode.semanticTags)

        if isinstance(supported_mode.semanticTags, list) and len(supported_mode.semanticTags) > 0:
            logger.info(
                "SupportedMode.semanticTags contains values, verifying attributes for manufacturedcode and value are not longer than 16bits int")
            for semantictag in supported_mode.semanticTags:
                asserts.assert_true(semantictag.mfgCode >= 0 and semantictag.mfgCode <= self._16bitshex,
                                    "Element semantictag.Mfgcode is greater than 16 bits")
                asserts.assert_true(semantictag.value >= 0 and semantictag.value <= self._16bitshex,
                                    "Element semantictag.Value is greater than 16 bits")

    def _log_attribute(self, name, value):
        logger.info(f"{name} attribute with value: {value} with type: {type(value)}")

    @async_test_body
    async def test_MOD_1_2(self):
        self.cluster = Clusters.ModeSelect
        self.endpoint = self.get_endpoint(1)
        self._16bitshex = 0xFFFF

        # Commision device
        # In the test plan step 1 is defined as a precondition.
        self.step(1)

        # Veirfy the Supported modes
        # Verify contains attributes label and mode
        # Verify if semantic tags has elements in list , and if there are values assert the values
        self.step(2)
        supported_modes = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.SupportedModes)
        logger.info(f"Supported modes {supported_modes}")
        # List must not be empty
        asserts.assert_true(len(supported_modes) > 0, "Supported modes can not be empty.")
        supported_modes_values = []
        for supported_mode in supported_modes:
            logger.info(
                f"Label {supported_mode} with type {type(supported_mode)} and {supported_mode.label} and {supported_mode.mode} and {supported_mode.semanticTags}")
            # Verify the struct values
            self._verify_supported_mode(supported_mode=supported_mode)
            # After verifying the struct is correct append the mode value.
            supported_modes_values.append(supported_mode.mode)
        logger.info(f"Supported modes values {supported_modes_values}")

        # Currentmode attribute check must be int and must be in the supported modes values.
        self.step(3)
        current_mode = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.CurrentMode)
        self._log_attribute('CurrentMode', current_mode)
        asserts.assert_true(isinstance(current_mode, int), "Current mode is not int")
        asserts.assert_in(current_mode, supported_modes_values, f"Current mode {current_mode} is not in {supported_modes_values}")

        self.step(4)
        on_mode = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.OnMode)
        # On mode can be Nullvalue
        self._log_attribute("OnMode", on_mode)
        asserts.assert_true((isinstance(on_mode, int) or on_mode is NullValue),
                            "Onmode is not int or is not Nullvalue")
        # Verify that OnMode is in the list of Supported Modes, but if null, cant be verified.
        if on_mode is not NullValue:
            asserts.assert_in(on_mode, supported_modes_values, f"Onmode {on_mode} is not in {supported_modes_values}")

        # Validate startup mode ( attribute Startup is optional)
        self.step(5)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=self.cluster.Attributes.StartUpMode):
            startup_mode = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.StartUpMode)
            self._log_attribute("StartupMode", startup_mode)
            asserts.assert_true(isinstance(startup_mode, int), "Startupmode is not int")
            asserts.assert_in(startup_mode, supported_modes_values,
                              f"Startupmode {startup_mode} is not in {supported_modes_values}")

        # Verify the string  is str and larger that 1 char.
        # If is non ci ask the user if can read and understand the string.
        self.step(6)
        description = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.Description)
        self._log_attribute("Description", description)
        asserts.assert_true(isinstance(description, str), "Description attribute is not str")
        asserts.assert_true(len(description) >= 1, "Description is lower that 1 char.")
        if not self.is_pics_sdk_ci_only:
            user_response = self.wait_for_user_input(prompt_msg=f"Is the value \"{description}\" for attribute Description a readable and understandable string? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_true(user_response.lower() == 'y', "The description is not understandable to the user.")

        # Verify the StandardNamespace can be 16 bits enum or null
        self.step(7)
        standard_namepace = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster, attribute=self.cluster.Attributes.StandardNamespace)
        self._log_attribute("StandardNamespace", standard_namepace)
        asserts.assert_true((standard_namepace is NullValue or (isinstance(standard_namepace, int) and (standard_namepace >= 0 and standard_namepace <= self._16bitshex))),
                            "Standard namespace is not 16bit enum or not Nullvalue")


if __name__ == "__main__":
    default_matter_test_main()
