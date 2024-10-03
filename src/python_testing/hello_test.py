#
#    Copyright (c) 2022 Project CHIP Authors
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
#    run1:
#      app: ${TYPE_OF_APP}
#      app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#      script-args: >
#        --storage-path admin_storage.json
#        --commissioning-method on-network
#        --discriminator 1234
#        --passcode 20202021
#        --trace-to json:${TRACE_TEST_JSON}.json
#        --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#      factoryreset: true
#      quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class HelloTest(MatterBaseTest):
    # This example test does not include the step_ and desc_ markers
    # The MatterBaseTest will assume a single step and create a description
    # based on the test name
    # To work in the Test harness, all tests should be named as
    # TC_PICSCODE_#_#, (substituting the appropriate pics codes and numbers for the test)
    @async_test_body
    async def test_TC_NAMES_2_1(self):
        dev_ctrl = self.default_controller
        vendor_name = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            0,
            Clusters.BasicInformation.Attributes.VendorName
        )

        logging.info("Found VendorName: %s" % (vendor_name))
        asserts.assert_equal(vendor_name, "TEST_VENDOR", "VendorName must be TEST_VENDOR!")

    # To include individual steps and description for the TH, define a steps_ and desc_ function
    # for the test, then use self.step(#) to indicate how the test proceeds through the test plan.
    # Support for keeping the TH up to date is built into MatterBaseTest when you use the step()
    # function.
    # If the device needs to be commissioned before running the test, ensure the first step in
    # this function is marked is_commissioning=True.
    def steps_TC_ENDPOINT_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Read ProductName on endpoint 9999"),
                 ]
        return steps

    def desc_TC_ENDPOINT_2_1(self) -> str:
        return '#.#.#. [TC-HELLO-x.x] Test Failure On Wrong Endpoint'

    @async_test_body
    async def test_TC_ENDPOINT_2_1(self):
        self.step(1)  # Commissioning

        self.step(2)
        dev_ctrl = self.default_controller
        result = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            9999,
            Clusters.BasicInformation.Attributes.ProductName
        )
        asserts.assert_true(isinstance(result, Clusters.Attribute.ValueDecodeFailure), "Should fail to read on endpoint 9999")
        asserts.assert_equal(result.Reason.status, Status.UnsupportedEndpoint, "Failure reason should be UnsupportedEndpoint")

    def steps_TC_PICSTEST_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "Skip this step based on pics"),
                 TestStep(3, "Run this step"),
                 TestStep(4, "Always skip this step")
                 ]
        return steps

    def desc_TC_PICSTEST_2_1(self) -> str:
        return "#.#.#. [TC-HELLO-x.x] Test pics"

    @async_test_body
    async def test_TC_PICSTEST_2_1(self):
        self.step(1)  # commissioning
        print('This should be run')

        self.step(2)
        if self.pics_guard(self.check_pics('NON-EXISTANT_PICS')):
            asserts.fail('This should not be run')

        self.step(3)
        print('This should also be run')

        self.skip_step(4)


if __name__ == "__main__":
    default_matter_test_main()
