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

"""Define Matter test case TC_SEPR_2_2."""


import logging
import sys
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_SEPR_2_2(MatterBaseTest):
    """Implementation of test case TC_SEPR_2_2."""

    def desc_TC_SEPR_2_2(self) -> str:
        """Return a description of this test."""
        return "This test case verifies the primary functionality of the Commodity Price cluster server"

    def pics_TC_SEPR_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "SEPR.S"
        ]
        return pics

    def steps_TC_SEPR_2_2(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)."),
            TestStep("2", "Set up a subscription to all CommodityPrice cluster events"),
            TestStep("3", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "Value has to be 1 (True)"),
            TestStep("4", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to True, and Components set to False.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice structure.
                     - verify that the PeriodStart is in the past.
                     - verify that the PeriodEnd is in the future or is null.
                     - verify that the Price is a PriceStruct with a valid Amount as a signed integer and Currency contains a CurrencyStruct with a valid Currency.Currency (unsigned integer max 999) and Currency.DecimalPoints.
                     - verify that the Description is a string with max length of 32.
                     - verify that the Components list is not included."""),
            TestStep("5", "TH sends command GetDetailedPriceRequest with Details=CommodityPriceDetailBitmap.Description set to False and Components set to True.",
                     """Verify that the DUT response contains GetDetailedPriceResponse with CurrentPrice structure.
                     - verify that the PeriodStart is in the past.
                     - verify that the PeriodEnd is in the future or is null.
                     - verify that the Price is a PriceStruct with a valid Amount as a signed integer and Currency which contains a CurrencyStruct with a valid Currency.Currency (unsigned integer max 999) and Currency.DecimalPoints.
                     - verify that the Description field is not included.
                     - verify that the Components field is included. It may be an empty list but shall have no more than 10 entries. Each entry shall have a valid value of Price, Source (a valid TariffPriceTypeEnum), it may include an optional Description (a string of max length 32) and may include an optional TariffComponentID (unsigned integer value)."""),
            TestStep("6", "TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.SEPR.TESTEVENT_TRIGGERKEY and EventTrigger field set to PIXIT.SEPR.TESTEVENTTRIGGER for Price Update Test Event",
                     """Verify DUT responds w / status SUCCESS(0x00) and event SEPR.S.E0000(PriceChange) sent.
                        Store the event's CurrentPrice field as NewCurrentPrice.
                        Verify that CurrentPrice includes a PeriodStart that is within 10s of the event timestamp."""),
            TestStep("6a", "TH reads CurrenPrice attribute.",
                     "Verify that the DUT response contains a CommodityPriceStruct value. Verify that the value matches the NewCurrentPrice from step 6."),

        ]

        return steps

    @async_test_body
    async def test_TC_SEPR_2_2(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")


if __name__ == "__main__":
    default_matter_test_main()
