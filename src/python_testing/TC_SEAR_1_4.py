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

# TODO - this was copied/pasted from abother test, it needs to be reviewed and updated
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${CHIP_RVC_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS examples/rvc-app/rvc-common/pics/rvc-app-pics-values --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_SEAR_1_4(MatterBaseTest):
    def __init__(self, *args):
        super().__init__(*args)
        self.endpoint = None
        self.is_ci = False
        self.app_pipe = "/tmp/chip_rvc_fifo_"

    async def read_sear_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ServiceArea
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def TC_SEAR_1_4(self) -> list[str]:
        return ["SEAR.S"]

    @async_test_body
    async def test_TC_SEAR_1_4(self):
        self.endpoint = self.matter_test_config.endpoint
        asserts.assert_false(self.endpoint is None, "--endpoint <endpoint> must be included on the command line in.")
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
        if self.is_ci:
            app_pid = self.matter_test_config.app_pid
            if app_pid == 0:
                asserts.fail("The --app-pid flag must be set when PICS_SDK_CI_ONLY is set")
            self.app_pipe = self.app_pipe + str(app_pid)

        self.print_step(1, "Commissioning, already done")

        attribute_list = await self.read_sear_attribute_expect_success(
            endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.AttributeList)
        logging.info("AttributeList: %s" % (attribute_list))

        if Clusters.ServiceArea.Attributes.CurrentArea not in attribute_list \
                and Clusters.ServiceArea.Attributes.Progress not in attribute_list:

            cmd_list = await self.read_sear_attribute_expect_success(
                endpoint=self.endpoint, attribute=Clusters.ServiceArea.Attributes.AcceptedCommandList)
            logging.info("AcceptedCommandList: %s" % (cmd_list))
            asserts.assert_true(Clusters.ServiceArea.Commands.SkipArea not in cmd_list,
                                "SkipArea command should not be implemented if both CurrentArea and Progress are not")


if __name__ == "__main__":
    default_matter_test_main()
