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
import random

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_DISHM_3_1(MatterBaseTest):

    async def read_mode_attribute_expect_success(self, endpoint, attribute):
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

    async def send_on_command(self) -> Clusters.Objects.OnOff.Commands.On:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OnOff.Commands.On, endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OnOff.Commands.On),
                            "Unexpected return type for OnOff")

    async def write_onoff_off_mode(self) -> Clusters.Objects.OnOff.Commands.Off:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.OnOff.Commands.Off, endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.OnOff.Commands.Off),
                            "Unexpected return type for OnOff")

    @async_test_body
    async def test_TC_DISHM_3_2(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        asserts.assert_true(self.check_pics("DISHM.S.A0000"), "DISHM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.A0001"), "DISHM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.A0002"), "DISHM.S.A0002 must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C00.Rsp"), "DISHM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("DISHM.S.C01.Tx"), "DISHM.S.C01.Tx must be supported")

        attributes = Clusters.DishwasherMode.Attributes

        from enum import Enum

        class CommonCodes(Enum):
            SUCCESS = 0x00
            UNSUPPORTED_MODE = 0x01
            GENERIC_FAILURE = 0x02

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read OnMode attribute")

        on_mode_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.OnMode)

        logging.info("OnMode: %s" % (on_mode_dut))

        asserts.assert_false(on_mode_dut == NullValue, "On mode value should be an integer value")

        self.print_step(3, "Read CurrentMode attribute")

        old_current_mode_dut = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (old_current_mode_dut))

        if old_current_mode_dut == on_mode_th:

            self.print_step(4, "Read SupportedModes attribute")
            supported_modes = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

            logging.info("SupportedModes: %s" % (supported_modes))

            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")

            new_mode_th = None

            for m in supported_modes:
                if m.mode != on_mode_th:
                    new_mode_th = m.mode
                    break

            self.print_step(5, "Send ChangeToMode command with NewMode set to %d" % (new_mode_th))

            ret = await self.send_change_to_mode_cmd(newMode=new_mode_th)
            asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode should succeed")

        self.print_step(6, "Send Off command")

        ret = await self.write_onoff_off_mode()
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode to off should succeed")

        self.print_step(7, "Send On command")

        ret = await self.write_onoff_on_mode()
        asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode to on should succeed")

        self.print_step(8, "Read CurrentMode attribute")

        current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))

        asserts.assert_true(on_mode_dut == current_mode, "CurrentMode must match OnMode after a power cycle")


if __name__ == "__main__":
    default_matter_test_main()
