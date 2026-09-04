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
#

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
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

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import is_valid_int_value
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_TSTATM_3_2(MatterBaseTest):

    def desc_TC_TSTATM_3_2(self) -> str:
        return "[TC-TSTATM-3.2] Startup Mode functionality with DUT as Server"

    def steps_TC_TSTATM_3_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads StartUpMode attribute from DUT",
                     "Save value as startup_mode_dut. If startup_mode_dut is null proceed to step 3, "
                     "else save startup_mode_dut as new_start_up_mode_th and proceed to step 5."),
            TestStep(3, "TH reads SupportedModes attribute from DUT",
                     "Verify list has two or more entries. Select a value from supported_modes_dut and save as new_start_up_mode_th."),
            TestStep(4, "TH writes StartUpMode attribute with the new_start_up_mode_th value",
                     "DUT responds with Success status."),
            TestStep(5, "TH reads CurrentMode attribute from DUT",
                     "Save value as old_current_mode_dut. If startup_mode_dut is equal to old_current_mode_dut "
                     "proceed to step 6, else proceed to step 8."),
            TestStep(6, "TH reads SupportedModes attribute from DUT",
                     "Select a value from supported_modes_dut different from startup_mode_dut and save as new_mode_th."),
            TestStep(7, "TH sends a ChangeToMode command to the DUT with NewMode set to new_mode_th",
                     "DUT responds with a ChangeToModeResponse command with Status set to Success(0x00)."),
            TestStep(8, "Physically power cycle the device"),
            TestStep(9, "TH reads StartUpMode attribute from DUT",
                     "DUT responds with an integer value equal to new_start_up_mode_th."),
            TestStep(10, "TH reads CurrentMode attribute from DUT",
                     "DUT responds with an integer value equal to new_start_up_mode_dut."),
        ]

    def pics_TC_TSTATM_3_2(self) -> list[str]:
        return [
            "TSTATM.S.A0002"
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_TSTATM_3_2(self):
        cluster = Clusters.Objects.ThermostatMode
        endpoint = self.get_endpoint()

        # Step 1: Commissioning, already done
        self.step(1)

        # Step 2: TH reads StartUpMode attribute
        self.step(2)
        startup_mode_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartUpMode
        )
        logger.info("startup_mode_dut: %s", startup_mode_dut)

        if startup_mode_dut is None or startup_mode_dut == NullValue:
            need_write_startup_mode = True
            new_start_up_mode_th = None
        else:
            is_valid_int_value(startup_mode_dut)
            need_write_startup_mode = False
            new_start_up_mode_th = startup_mode_dut

        # Step 3: TH reads SupportedModes attribute (if needed)
        self.step(3)
        if need_write_startup_mode:
            supported_modes = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedModes
            )
            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")
            supported_modes_dut = [m.mode for m in supported_modes]
            new_start_up_mode_th = supported_modes_dut[0]
            logger.info("Selected new_start_up_mode_th: %s", new_start_up_mode_th)
        else:
            self.mark_current_step_skipped()

        # Step 4: TH writes StartUpMode attribute (if needed)
        self.step(4)
        if need_write_startup_mode:
            await self.write_single_attribute(
                cluster.Attributes.StartUpMode(new_start_up_mode_th),
                endpoint_id=endpoint,
            )
        else:
            self.mark_current_step_skipped()

        # Step 5: TH reads CurrentMode attribute
        self.step(5)
        old_current_mode_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentMode
        )
        is_valid_int_value(old_current_mode_dut)
        logger.info("old_current_mode_dut: %s", old_current_mode_dut)

        effective_startup_mode = new_start_up_mode_th if need_write_startup_mode else startup_mode_dut
        need_change_mode = (effective_startup_mode == old_current_mode_dut)

        # Step 6: TH reads SupportedModes attribute (if needed)
        self.step(6)
        if need_change_mode:
            supported_modes = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedModes
            )
            supported_modes_dut = [m.mode for m in supported_modes]
            different_modes = [m for m in supported_modes_dut if m != effective_startup_mode]
            asserts.assert_greater_equal(len(different_modes), 1,
                                         "Must have at least one mode different from the effective startup mode")
            new_mode_th = different_modes[0]
            logger.info("Selected new_mode_th: %s", new_mode_th)
        else:
            self.mark_current_step_skipped()

        # Step 7: TH sends ChangeToMode command (if needed)
        self.step(7)
        if need_change_mode:
            cmd = cluster.Commands.ChangeToMode(newMode=new_mode_th)
            resp = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.assert_equal(resp.status, 0x00, f"ChangeToMode failed with status {resp.status}")
        else:
            self.mark_current_step_skipped()

        # Step 8: Physically power cycle the device
        self.step(8)
        await self.request_device_reboot()

        # Step 9: TH reads StartUpMode attribute
        self.step(9)
        new_start_up_mode_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.StartUpMode
        )
        is_valid_int_value(new_start_up_mode_dut)
        asserts.assert_equal(
            new_start_up_mode_dut, new_start_up_mode_th,
            f"StartUpMode ({new_start_up_mode_dut}) does not match expected ({new_start_up_mode_th}) after reboot",
        )

        # Step 10: TH reads CurrentMode attribute
        self.step(10)
        new_current_mode_dut = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.CurrentMode
        )
        is_valid_int_value(new_current_mode_dut)
        asserts.assert_equal(
            new_current_mode_dut, new_start_up_mode_dut,
            f"CurrentMode ({new_current_mode_dut}) does not match StartUpMode ({new_start_up_mode_dut}) after reboot",
        )


if __name__ == "__main__":
    default_matter_test_main()
