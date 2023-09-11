#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class HelloTest(MatterBaseTest):
    @async_test_body
    async def test_names_as_expected(self):
        dev_ctrl = self.default_controller
        vendor_name = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            0,
            Clusters.BasicInformation.Attributes.VendorName
        )

        logging.info("Found VendorName: %s" % (vendor_name))
        asserts.assert_equal(vendor_name, "TEST_VENDOR", "VendorName must be TEST_VENDOR!")

    @async_test_body
    async def test_failure_on_wrong_endpoint(self):
        dev_ctrl = self.default_controller
        result = await self.read_single_attribute(
            dev_ctrl,
            self.dut_node_id,
            9999,
            Clusters.BasicInformation.Attributes.ProductName
        )
        asserts.assert_true(isinstance(result, Clusters.Attribute.ValueDecodeFailure), "Should fail to read on endpoint 9999")
        asserts.assert_equal(result.Reason.status, Status.UnsupportedEndpoint, "Failure reason should be UnsupportedEndpoint")


if __name__ == "__main__":
    default_matter_test_main()
