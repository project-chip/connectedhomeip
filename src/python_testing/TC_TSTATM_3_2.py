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
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, matter_asserts
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Auto-generated from test specification: [TC-TSTATM-3.2] Startup Mode functionality with DUT as Server


class TC_TSTATM_3_2(MatterBaseTest):

    def pics_TC_TSTATM_3_2(self) -> list[str]:
        return [
            "TSTATM.S",
        ]

    def desc_TC_TSTATM_3_2(self) -> str:
        return "[TC-TSTATM-3.2] Startup Mode functionality with DUT as Server"

    def steps_TC_TSTATM_3_2(self):
        return [
            TestStep(1, "Commission DUT to TH (can be skipped if done in a preceding test).",
                     ""),
            TestStep(2, "TH reads from the DUT the StartUpMode attribute.",
                     "Verify that the DUT response contains an integer value or null Save the value as startup_mode_dut if startup_mode_dut is null proceed to step 3. Else save startup_mode_dut as new_start_up_mode_th and proceed to step 5."),
            TestStep(3, "TH reads from the DUT the SupportedModes attribute.",
                     "Verify that the DUT response contains a list of ModeOptionsStruct entries Verify that the list has two or more entries Save the Mode field values as supported_modes_dut Select a value from supported_modes_dut and save the value as new_start_up_mode_th."),
            TestStep(4, "TH writes to the DUT the StartUpMode attribute with the new_start_up_mode_th value",
                     "Verify DUT responds w/ status SUCCESS(0x00)"),
            TestStep(5, "TH reads from the DUT the CurrentMode attribute.",
                     "Verify that the DUT response contains an integer value Save the value as old_current_mode_dut If startup_mode_dut is equal to old_current_mode_dut proceed to step 6. Else proceed to step 8."),
            TestStep(6, "TH reads from the DUT the SupportedModes attribute.",
                     "Verify that the DUT response contains a list of ModeOptionsStruct entries Verify that the list has two or more entries Save the Mode field values as supported_modes_dut Select a value from supported_modes_dut different from startup_mode_dut. Save the value as new_mode_th."),
            TestStep(7, "TH sends a ChangeToMode command to the DUT with NewMode set to new_mode_th",
                     "Verify that the DUT response contains a ChangeToModeResponse command w/ data fields: * Status, set to Success(0x00)."),
            TestStep(8, "Physically power cycle the device",
                     ""),
            TestStep(9, "TH reads from the DUT the StartUpMode attribute.",
                     "Verify that the DUT response contains an integer value Save the value as new_start_up_mode_dut The value is equal to new_start_up_mode_th"),
            TestStep(10, "TH reads from the DUT the CurrentMode attribute.",
                     "Verify that the DUT response contains an integer value The value is equal to new_start_up_mode_dut"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_TSTATM_3_2(self):
        self.step(1)
        # Commission DUT to TH (can be skipped if done in a preceding test).
        #

        self.step(2)
        # TH reads from the DUT the StartUpMode attribute.
        # Verify that the DUT response contains an integer value or null
        # Save the value as startup_mode_dut
        endpoint = self.get_endpoint()
        cluster = Clusters.ThermostatMode
        attributes = cluster.Attributes

        startup_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.StartUpMode)
        matter_asserts.assert_true(isinstance(startup_mode_dut, int) or startup_mode_dut is Clusters.Types.NullValue,
                                   "StartUpMode must be an integer or null")

        # if startup_mode_dut is null proceed to step 3.
        # Else save startup_mode_dut as new_start_up_mode_th and proceed to step 5.
        if startup_mode_dut is Clusters.Types.NullValue:
            self.step(3)
            # TH reads from the DUT the SupportedModes attribute.
            supported_modes = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.SupportedModes)
            # Verify that the DUT response contains a list of ModeOptionsStruct entries Verify that the list has two or more entries
            matter_asserts.assert_is_list(supported_modes, "SupportedModes must be a list")
            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries")
            # Save the Mode field values as supported_modes_dut Select a value from supported_modes_dut and save the value as new_start_up_mode_th.
            supported_modes_dut = [m.mode for m in supported_modes]
            new_start_up_mode_th = supported_modes_dut[0]

            self.step(4)
            # TH writes to the DUT the StartUpMode attribute with the new_start_up_mode_th value
            # Verify DUT responds w/ status SUCCESS(0x00)
            await self.write_single_attribute(endpoint=endpoint, cluster=cluster,
                                              attribute=attributes.StartUpMode(new_start_up_mode_th))
        else:
            new_start_up_mode_th = startup_mode_dut
            self.skip_step(3)
            self.skip_step(4)

        self.step(5)
        # TH reads from the DUT the CurrentMode attribute.
        old_current_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.CurrentMode)
        # Verify that the DUT response contains an integer value
        matter_asserts.assert_valid_uint8(old_current_mode_dut, "CurrentMode should be a valid uint8")
        # Save the value as old_current_mode_dut
        # If startup_mode_dut is equal to old_current_mode_dut proceed to step 6.
        # Else proceed to step 8.
        if startup_mode_dut == old_current_mode_dut:
            self.step(6)
            # TH reads from the DUT the SupportedModes attribute.
            # Verify that the DUT response contains a list of ModeOptionsStruct entries
            # Verify that the list has two or more entries
            # Save the Mode field values as supported_modes_dut
            # Select a value from supported_modes_dut different from startup_mode_dut. Save the value as new_mode_th.
            supported_modes = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.SupportedModes)
            matter_asserts.assert_is_list(supported_modes, "SupportedModes must be a list")
            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries")
            supported_modes_dut = [m.mode for m in supported_modes]
            new_mode_th = [m for m in supported_modes_dut if m != startup_mode_dut][0]

            self.step(7)
            # TH sends a ChangeToMode command to the DUT with NewMode set to new_mode_th
            # Verify that the DUT response contains a ChangeToModeResponse command w/ data fields: * Status, set to Success(0x00).
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ThermostatMode.Commands.ChangeToMode(newMode=new_mode_th), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            self.skip_step(6)
            self.skip_step(7)

        self.step(8)
        # Physically power cycle the device
        await self.request_device_reboot()

        self.step(9)
        # TH reads from the DUT the StartUpMode attribute.
        # Verify that the DUT response contains an integer value
        # Save the value as new_start_up_mode_dut
        # The value is equal to new_start_up_mode_th
        new_start_up_mode_dut = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.StartUpMode)
        matter_asserts.assert_valid_uint8(new_start_up_mode_dut, "StartUpMode should not be a valid value")
        asserts.assert_equal(new_start_up_mode_dut, new_start_up_mode_th, "StartUpMode is not the expected value after reboot")

        self.step(10)
        # TH reads from the DUT the CurrentMode attribute.
        # Verify that the DUT response contains an integer value The value is equal to new_start_up_mode_dut
        current_mode_after_reboot = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attributes.CurrentMode)
        matter_asserts.assert_valid_uint8(current_mode_after_reboot, "CurrentMode should be an integer")
        asserts.assert_equal(current_mode_after_reboot, new_start_up_mode_dut,
                             "CurrentMode is not equal to the new StartUpMode after reboot")


if __name__ == '__main__':
    default_matter_test_main()
