#
#    Copyright (c) 2023 Project CHIP Authors
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

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_DISHM_3_3(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.DishwasherMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_change_to_mode_cmd(self, newMode) -> Clusters.Objects.DishwasherMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.DishwasherMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.DishwasherMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToMode")
        return ret

    async def write_on_mode(self, newMode):
        ret = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, Clusters.DishwasherMode.Attributes.OnMode(newMode))])
        asserts.assert_equal(ret[0].Status, Status.Success, "Writing to OnMode failed")

    async def write_startup_onoff(self) -> Clusters.Objects.OnOff.Commands.Off:
        ret = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, Clusters.OnOff.Attributes.StartUpOnOff(1))])
        asserts.assert_equal(ret[0].Status, Status.Success, "Writing to OnOff failed")

    @async_test_body
    async def test_TC_DISHM_3_3(self):

        self.endpoint = self.user_params.get("endpoint", 1)

        asserts.assert_true(self.check_pics("DISHM.S.A0000"), "DISHM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.A0001"), "DISHM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C00.Rsp"), "DISHM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C01.Tx"), "DISHM.S.C01.Tx must be supported")

        if not self.check_pics("DISHM.S.A0002") and self.check_pics("DISHM.S.A0003"):
            logging.info("Test skipped because PICS DISHM.S.A0002 (StartupMode) and/or DISHM.S.A0003 (OnMode) is not set")
            return

        attributes = Clusters.DishwasherMode.Attributes

        from enum import Enum

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read StartUpMode attribute")

        startup_mode_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.StartUpMode)

        logging.info("StartUpMode: %s" % (startup_mode_dut))

        asserts.assert_false(startup_mode_dut == NullValue, "Startup mode value should be an integer value")

        self.print_step(3, "Read OnMode attribute")

        old_on_mode_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OnMode)

        logging.info("OnMode: %s" % (old_on_mode_dut))

        asserts.assert_false(old_on_mode_dut == NullValue, "On mode value should be an integer value")

        if old_on_mode_dut == startup_mode_dut:

            self.print_step(4, "Read SupportedModes attribute")
            supported_modes_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

            logging.info("SupportedModes: %s" % (supported_modes_dut))

            asserts.assert_greater_equal(len(supported_modes_dut), 2, "SupportedModes must have at least two entries!")

            new_mode_th = None

            for m in supported_modes_dut:
                if m.mode != startup_mode_dut:
                    new_mode_th = m.mode
                    break

            self.print_step(5, "Write to OnMode attribute with NewMode set to %d" % (new_mode_th))

            ret = await self.write_on_mode(newMode=new_mode_th)
            asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the Onmode should succeed")

        self.print_step(6, "Read OnMode attribute")

        new_on_mode_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OnMode)

        logging.info("OnMode: %s" % (new_on_mode_dut))

        asserts.assert_true(new_mode_th == new_on_mode_dut, "OnMode must match the mode written in previous step")

        self.print_step(7, "Write to the StartUpOnOff attribute of OnOff cluster with a value of 1")

        ret = await self.write_startup_onoff()
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value,
                            "Writing to the StartUpOnOff attribute of OnOff should succeed")

        self.default_controller.ExpireSessions(self.dut_node_id)

        self.print_step(8, "Physically power cycle the device")
        input("Press Enter when done.\n")

        self.print_step(9, "Read CurrentMode attribute")

        current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))

        asserts.assert_true(new_on_mode_dut == current_mode, "CurrentMode must match OnMode after a power cycle")


if __name__ == "__main__":
    default_matter_test_main()
