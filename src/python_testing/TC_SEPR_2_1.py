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
# pylint: disable=invalid-name

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ENERGY_GATEWAY_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_SEPR_2_1."""


import logging
import sys
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_SEPR_2_1(MatterBaseTest):
    """Implementation of test case TC_SEPR_2_1."""

    def desc_TC_SEPR_2_1(self) -> str:
        """Return a description of this test."""
        return "This test case verifies the non-global attributes of the Commodity Price cluster server"

    def pics_TC_SEPR_2_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "SEPR.S"
        ]
        return pics

    def steps_TC_SEPR_2_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)."),
            TestStep("2", "TH reads from the DUT the TariffUnit attribute.",
                     "Verify that the DUT response contains an enum8 value."),
            TestStep("3", "TH reads from the DUT the Currency attribute.",
                     "Verify that the DUT response contains either null or a CurrencyStruct value."),
            TestStep("4", "TH reads from the DUT the CurrentPrice attribute.",
                     "Verify that the DUT response contains either null or a CommodityPriceStruct value."),
            TestStep("5", "TH reads from the DUT the PriceForecast attribute.",
                     "Verify that the DUT response contains a list of CommodityPriceStruct entries"),
        ]

        return steps

    @async_test_body
    async def test_TC_SEPR_2_1(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")


if __name__ == "__main__":
    default_matter_test_main()
