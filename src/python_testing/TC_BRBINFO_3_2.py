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
#     app: ${BRIDGE_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 3
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import json
import logging
from time import sleep
from typing import Any

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_BRBINFO_3_2(MatterBaseTest):
    async def read_brbinfo_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BridgedDeviceBasicInformation
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BRBINFO_3_2(self) -> str:
        return "[TC-BRBINFO-3.2] Attributes with DUT as Server"

    def steps_TC_BRBINFO_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH reads ConfigurationVersion from the DUT and stores the value as initialConfigurationVersion",
                     "Verify that the value is in the inclusive range of 1 to 4294967295"),
            TestStep(2, "Change the configuration version in a way which results in functionality to be added or removed (e.g. rewire thermostat to support a new mode)"),
            TestStep(3, "TH reads ConfigurationVersion from the DUT",
                     "Verify that the value is higher than the value of initialConfigurationVersion"),
        ]
        return steps

    def pics_TC_BRBINFO_3_2(self) -> list[str]:
        pics = [
            "BRBINFO.S",
            "BRBINFO.S.A.A0018",
            "BRBINFO.S.M.DeviceConfigurationChange",
        ]
        return pics

    def _send_named_pipe_command(self, command_dict: dict[str, Any]):
        app_pid = self.matter_test_config.app_pid
        if app_pid == 0:
            asserts.fail("The --app-pid flag must be set when usage of door state simulation named pipe is required (e.g. CI)")

        app_pipe = f"/tmp/chip_bridge_fifo_{app_pid}"
        command = json.dumps(command_dict)

        # Sends an out-of-band command to the sample app
        with open(app_pipe, "w") as outfile:
            logging.info(f"Sending named pipe command to {app_pipe}: '{command}'")
            outfile.write(command + "\n")
        # Delay for pipe command to be processed (otherwise tests may be flaky).
        sleep(0.1)

    @async_test_body
    async def test_TC_BRBINFO_3_2(self):

        endpoint = self.get_endpoint(default=3)

        attributes = Clusters.BridgedDeviceBasicInformation.Attributes

        self.step(1)
        initialConfigurationVersion = await self.read_brbinfo_attribute_expect_success(endpoint=endpoint, attribute=attributes.ConfigurationVersion)
        asserts.assert_greater_equal(initialConfigurationVersion, 1, "ConfigurationVersion attribute is out of range")

        self.step(2)
        if self.is_pics_sdk_ci_only:
            command_dict = {"Name": "SimulateConfigurationVersionChange"}
            self._send_named_pipe_command(command_dict)
        else:
            self.wait_for_user_input(
                prompt_msg="Change the configuration version in a way which results in functionality to be added or removed, then continue")

        self.step(3)
        newConfigurationVersion = await self.read_brbinfo_attribute_expect_success(endpoint=endpoint, attribute=attributes.ConfigurationVersion)
        asserts.assert_greater(newConfigurationVersion, initialConfigurationVersion,
                               "ConfigurationVersion attribute not grater than initialConfigurationVersion")


if __name__ == "__main__":
    default_matter_test_main()
