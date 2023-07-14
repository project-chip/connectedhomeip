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
from chip.interaction_model import Status
from chip.interaction_model import InteractionModelError
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# This test requires several additional command line arguments
# run with
# --int-arg PIXIT_ENDPOINT:<endpoint>


class TC_RVCRUNM_3_2(MatterBaseTest):

    async def read_mod_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcRunMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_change_to_mode_cmd(self, newMode) -> Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse:
        ret = await self.send_single_cmd(cmd=Clusters.Objects.RvcRunMode.Commands.ChangeToMode(newMode=newMode), endpoint=self.endpoint)
        asserts.assert_true(type_matches(ret, Clusters.Objects.RvcRunMode.Commands.ChangeToModeResponse),
                            "Unexpected return type for ChangeToMode")
        return ret

    @async_test_body
    async def test_TC_RVCRUNM_3_2(self):

        asserts.assert_true('PIXIT_ENDPOINT' in self.matter_test_config.global_test_params,
                            "PIXIT_ENDPOINT must be included on the command line in "
                            "the --int-arg flag as PIXIT_ENDPOINT:<endpoint>")

        self.endpoint = self.matter_test_config.global_test_params['PIXIT_ENDPOINT']

        asserts.assert_true(self.check_pics("RVCRUNM.S.A0000"), "RVCRUNM.S.A0000 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0001"), "RVCRUNM.S.A0001 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.A0002"), "RVCRUNM.S.A0002 must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.C00.Rsp"), "RVCRUNM.S.C00.Rsp must be supported")
        asserts.assert_true(self.check_pics("RVCRUNM.S.C01.Tx"), "RVCRUNM.S.C01.Tx must be supported")

        attributes = Clusters.RvcRunMode.Attributes

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read StartUpMode attribute")

        startup_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.StartUpMode)

        logging.info("StartUpMode: %s" % (startup_mode))

        self.print_step(3, "Read CurrentMode attribute")

        old_current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (old_current_mode))

        if old_current_mode == startup_mode:

            self.print_step(4, "Read SupportedModes attribute")
            supported_modes = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)

            logging.info("SupportedModes: %s" % (supported_modes))

            asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least two entries!")

            new_mode = None

            for m in supported_modes:
                if m.mode != startup_mode:
                    new_mode = m.mode
                    break

            self.print_step(5, "Send ChangeToMode command with NewMode set to %d" % (new_mode))

            ret = await self.send_change_to_mode_cmd(newMode=new_mode)
            asserts.assert_true(ret.status == CommonCodes.SUCCESS.value, "Changing the mode should succeed")

        self.print_step(6, "Physically power cycle the device")
        input("Press Enter when done.\n")

        self.print_step(7, "Read CurrentMode attribute")

        current_mode = await self.read_mod_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.CurrentMode)

        logging.info("CurrentMode: %s" % (current_mode))

        asserts.assert_true(startup_mode == current_mode, "CurrentMode must match StartUpMode after a power cycle")


if __name__ == "__main__":
    default_matter_test_main()