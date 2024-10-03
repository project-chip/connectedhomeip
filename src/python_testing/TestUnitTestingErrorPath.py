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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

""" Integration test for error path returns via the UnitTesting cluster.
"""


class TestUnitTestingErrorPath(MatterBaseTest):

    @async_test_body
    async def test_unit_test_error_read(self):
        endpoint_id = 1
        attributes = Clusters.UnitTesting.Attributes

        self.print_step(0, "Commissioning - already done")

        self.print_step(1, "Set error to 'Failure' for all subsequent reads of FailureInt32U.")
        await self.default_controller.WriteAttribute(
            nodeid=self.dut_node_id,
            attributes=[(endpoint_id, attributes.ReadFailureCode(int(Status.Failure)))],
        )

        self.print_step(2, "Expect that reading FailureInt32U returns the previously set 'Failure' code.")
        try:
            data = await self.default_controller.ReadAttribute(
                self.dut_node_id, [(endpoint_id, attributes.FailureInt32U)]
            )
            result = data[endpoint_id][Clusters.UnitTesting][attributes.FailureInt32U]

            asserts.assert_true(
                isinstance(result, Clusters.Attribute.ValueDecodeFailure),
                "Expect a decode error for reading the failure attribute"
            )
            asserts.assert_equal(result.Reason.status, Status.Failure, "Failure state is the default for the failure read.")
        except InteractionModelError:
            asserts.fail("Failure reading")

        self.print_step(3, "Set error to 'ResourceExhausted' for all subsequent reads of FailureInt32U.")
        await self.default_controller.WriteAttribute(
            nodeid=self.dut_node_id,
            attributes=[(endpoint_id, attributes.ReadFailureCode(int(Status.ResourceExhausted)))],
        )

        self.print_step(4, "Expect that reading FailureInt32U returns the previously set 'ResourceExhausted' code.")
        try:
            data = await self.default_controller.ReadAttribute(
                self.dut_node_id, [(endpoint_id, attributes.FailureInt32U)]
            )
            result = data[endpoint_id][Clusters.UnitTesting][attributes.FailureInt32U]

            asserts.assert_true(
                isinstance(result, Clusters.Attribute.ValueDecodeFailure),
                "Expect a decode error for reading the failure attribute"
            )
            asserts.assert_true(result.Reason.status, Status.ResourceExhausted, "Set failure is ResourceExhausted")
        except InteractionModelError:
            asserts.fail("Failure reading")

        self.print_step(5, "Reset ReadFailureCode error to default 'Failure' code.")
        await self.default_controller.WriteAttribute(
            nodeid=self.dut_node_id,
            attributes=[(1, attributes.ReadFailureCode(int(Status.Failure)))],
        )

        logging.info("Test completed")


if __name__ == "__main__":
    default_matter_test_main()
