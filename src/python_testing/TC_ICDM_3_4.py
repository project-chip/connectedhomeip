
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIT_ICD_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
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
import time

import chip.clusters as Clusters
from chip.testing.matter_testing import (MatterBaseTest, MatterStackState, MatterTestConfig, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)

kRootEndpointId = 0
cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes


class TC_ICDM_3_4(MatterBaseTest):

    #
    # Class Helper functions
    #
    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    #
    # Test Harness Helpers
    #

    def desc_TC_ICDM_3_4(self) -> str:
        """Returns a description of this test"""
        return "[TC-ICDM-3.4] ICDCounter Persistence with DUT as Server"

    def steps_TC_ICDM_3_4(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads from the DUT the ICDCounter attribute."),
            TestStep("2a", "Power cycle DUT."),
            TestStep("2b", "TH waits for {PIXIT.WAITTIME.REBOOT}"),
            TestStep(3, "Verify that the DUT response contains value of ICDCounter and stores in IcdCounter2. \
                            IcdCounter2 is greater or equal to IcdCounter1. \
                            ICDCounter attribute can roll over. If the attribute rolls over, it will be greater or equal to 0.")
        ]
        return steps

    def pics_TC_ICDM_3_4(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
            "ICDM.S.F00"
        ]
        return pics

    #
    # ICDM 3.4 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_3_4(self):
        is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        if not is_ci:
            asserts.assert_true('PIXIT.WAITTIME.REBOOT' in self.matter_test_config.global_test_params,
                                "PIXIT.WAITTIME.REBOOT must be included on the command line in "
                                "the --int-arg flag as PIXIT.WAITTIME.REBOOT:<wait time>")

            wait_time_reboot = self.matter_test_config.global_test_params['PIXIT.WAITTIME.REBOOT']
            if wait_time_reboot == 0:
                asserts.fail("PIXIT.WAITTIME.REBOOT shall be higher than 0.")

        # Pre-Condition: Commissioning
        self.step(0)

        self.step(1)
        icdCounter1 = await self._read_icdm_attribute_expect_success(attribute=attributes.ICDCounter)

        self.step("2a")
        if not is_ci:
            self.wait_for_user_input(prompt_msg="Restart DUT. Press Enter when restart has been initiated.")

        self.step("2b")
        if not is_ci:
            time.sleep(wait_time_reboot)

        self.step(3)
        if not is_ci:
            # since device has rebooted, force establishing a new CASE session by closing it
            self.config = MatterTestConfig()
            self.stack = MatterStackState(self.config)
            devCtrl = self.stack.certificate_authorities[0].adminList[0].NewController(
                nodeId=self.config.controller_node_id,
                paaTrustStorePath=str(self.config.paa_trust_store_path),
                catTags=self.config.controller_cat_tags
            )
            devCtrl.MarkSessionDefunct(self.dut_node_id)
        icdCounter2 = await self._read_icdm_attribute_expect_success(attribute=attributes.ICDCounter)
        asserts.assert_greater_equal(icdCounter2, icdCounter1,
                                     "ICDCounter have reboot is not greater or equal to the ICDCounter read before the reboot.")


if __name__ == "__main__":
    default_matter_test_main()
