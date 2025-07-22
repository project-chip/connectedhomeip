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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/brbinfo_3_2_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 3
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --app-pipe /tmp/brbinfo_3_2_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import chip.clusters as Clusters
import test_plan_support
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts


class TC_BRBINFO_3_2(MatterBaseTest):
    async def read_brbinfo_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BridgedDeviceBasicInformation
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BRBINFO_3_2(self) -> str:
        return "[TC-BRBINFO-3.2] Attributes with DUT as Server"

    def steps_TC_BRBINFO_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep(2, "Set up a subscription wildcard subscription, with MinIntervalFloor set to 0, MaxIntervalCeiling set to 30 and KeepSubscriptions set to false"),
            TestStep(3, "TH reads ConfigurationVersion from the DUT and stores the value as initialConfigurationVersion",
                     "Verify that the value is in the inclusive range of 1 to 4294967295"),
            TestStep(4, "Change the configuration version in a way which results in functionality to be added or removed (e.g. rewire thermostat to support a new mode)"),
            TestStep(5, "TH reads ConfigurationVersion from the DUT",
                     "Verify that the value is higher than the value of initialConfigurationVersion"),
            TestStep(6, "Verify that the DUT sends an attribute report for ConfigurationVersion",
                     "Only one report received"),
            TestStep(7, "Verify that the value in the attribute report is equal to the value read in step 5",
                     "Verify that the reported value is equal to the value read in step 5"),
        ]
        return steps

    def pics_TC_BRBINFO_3_2(self) -> list[str]:
        pics = [
            "BRBINFO.S",
            "BRBINFO.S.A.A0018",
            "BRBINFO.S.M.DeviceConfigurationChange",
        ]
        return pics

    @async_test_body
    async def test_TC_BRBINFO_3_2(self):

        self.step(1)
        endpoint = self.get_endpoint(default=3)
        cluster = Clusters.BridgedDeviceBasicInformation
        attributes = cluster.Attributes

        self.step(2)
        sub_handler = ClusterAttributeChangeAccumulator(cluster)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        initialConfigurationVersion = await self.read_brbinfo_attribute_expect_success(endpoint=endpoint, attribute=attributes.ConfigurationVersion)
        asserts.assert_greater_equal(initialConfigurationVersion, 1, "ConfigurationVersion attribute is out of range")

        self.step(4)
        if self.is_pics_sdk_ci_only:
            command_dict = {"Name": "SimulateConfigurationVersionChange"}
            self.write_to_app_pipe(command_dict)
        else:
            self.wait_for_user_input(
                prompt_msg="Change the configuration version in a way which results in functionality to be added or removed, then continue")

        self.step(5)
        newConfigurationVersion = await self.read_brbinfo_attribute_expect_success(endpoint=endpoint, attribute=attributes.ConfigurationVersion)
        asserts.assert_greater(newConfigurationVersion, initialConfigurationVersion,
                               "ConfigurationVersion attribute not grater than initialConfigurationVersion")

        self.step(6)
        count = sub_handler.attribute_report_counts[attributes.ConfigurationVersion]
        asserts.assert_equal(count, 1, "Unexpected number of ConfigurationVersion reports")

        self.step(7)
        reportedConfigurationVersionValuesList = sub_handler.attribute_reports[attributes.ConfigurationVersion]
        asserts.assert_equal(
            reportedConfigurationVersionValuesList[0].value, newConfigurationVersion, msg="Unexpected value in ConfigurationVersion report")


if __name__ == "__main__":
    default_matter_test_main()
