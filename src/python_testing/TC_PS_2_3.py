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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from matter_testing_support import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                    default_matter_test_main)
from mobly import asserts


class TC_PS_2_3(MatterBaseTest):

    def pics_TC_PS_2_3(self) -> list[str]:
        return ["PWRTL.S"]

    def steps_TC_PS_2_3(self):
        return [TestStep(1, "Commission DUT to TH", "", is_commissioning=True),
                TestStep(2, "Subscribe to all attributes of the PowerSource Cluster"),
                TestStep(3, "Accumulate all attribute reports on the endpoint under test for 30 seconds",
                            "For each of the attributes in the set of BatTimeToFullCharge, BatPercentRemaining and BatTimeRemaining, verify that there are not more than 4 reports per attribute where the value is non-null over the period of accumulation.")]

    @async_test_body
    async def test_TC_PS_2_3(self):
        # Commissioning, already done.
        self.step(1)

        self.step(2)
        ps = Clusters.PowerSource
        sub_handler = ClusterAttributeChangeAccumulator(ps)
        await sub_handler.start(self.default_controller, self.dut_node_id, self.matter_test_config.endpoint)

        self.step(3)
        logging.info("This test will now wait for 30 seconds.")
        time.sleep(30)

        counts = sub_handler.attribute_report_counts
        asserts.assert_less_equal(counts[ps.Attributes.BatTimeToFullCharge], 4, "Too many reports for BatTimeToFullCharge")
        asserts.assert_less_equal(counts[ps.Attributes.BatPercentRemaining], 4, "Too many reports for BatPercentRemaining")
        asserts.assert_less_equal(counts[ps.Attributes.BatTimeRemaining], 4, "Too many reports for BatTimeRemaining")


if __name__ == "__main__":
    default_matter_test_main()
