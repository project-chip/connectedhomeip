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

from matter_testing_support import MatterBaseTest, default_matter_test_main, async_test_body
from chip.interaction_model import Status
import chip.clusters as Clusters
import logging
from mobly import asserts


class HelloTest(MatterBaseTest):
    @async_test_body
    async def test_names_as_expected(self):
        dev_ctrl = self.default_controller
        vendor_name = await self.read_single_attribute(dev_ctrl, self.dut_node_id, 0, Clusters.Basic.Attributes.VendorName)

        logging.info("Found VendorName: %s" % (vendor_name))
        asserts.assert_equal(vendor_name, "TEST_VENDOR", "VendorName must be TEST_VENDOR!")

    @async_test_body
    async def test_failure_on_wrong_endpoint(self):
        dev_ctrl = self.default_controller
        result = await self.read_single_attribute(dev_ctrl, self.dut_node_id, 9999, Clusters.Basic.Attributes.ProductName)
        asserts.assert_true(isinstance(result, Clusters.Attribute.ValueDecodeFailure), "Should fail to read on endpoint 9999")
        asserts.assert_equal(result.Reason.status, Status.UnsupportedEndpoint, "Failure reason should be UnsupportedEndpoint")


if __name__ == "__main__":
    default_matter_test_main()
