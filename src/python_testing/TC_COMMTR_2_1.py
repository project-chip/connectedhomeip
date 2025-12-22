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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_COMMTR_2_1."""

from TC_COMMTR_TestBase import CommodityMeteringTestBaseHelper

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import TestStep, default_matter_test_main

cluster = Clusters.CommodityMetering


class TC_COMMTR_2_1(CommodityMeteringTestBaseHelper):
    """Implementation of test case TC_COMMTR_2_1."""

    def desc_TC_COMMTR_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with Server as DUT"

    def pics_TC_COMMTR_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["COMMTR.S"]

    def steps_TC_COMMTR_2_1(self) -> list[TestStep]:

        return [
            TestStep("1", "Commissioning, already done", "DUT is commissioned", is_commissioning=True),
            TestStep("2", "TH reads MaximumMeteredQuantities attribute", """
                     - DUT replies a null value or a uint16 value;
                     - Store value as MaxMeteredQuantities."""),
            TestStep("3", "TH reads MeteredQuantity attribute", """
                     - DUT replies Null or a list of MeteredQuantityStruct entries.
                     - Verify that the list length less or equal MaxMeteredQuantities from step 2;
                     - Verify that the TariffComponentIDs field is a list of uint32 values with length less or equal 128;
                     - Verify that the Quantity field has int64 type;"""),
            TestStep("4", "TH reads MeteredQuantityTimestamp attribute", "DUT replies a null value or epoch-s type."),
            TestStep("5", "TH reads TariffUnit attribute", "DUT replies a null value or TariffUnitEnum value in range 0-1."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_COMMTR_2_1(self):
        """Implements test procedure for test case TC_COMMTR_2_1."""

        endpoint = self.get_endpoint()

        self.step("1")
        # commissioning

        self.step("2")
        # Read MaximumMeteredQuantities attribute, expected to be Null or uint16
        await self.check_maximum_metered_quantities_attribute(endpoint)

        self.step("3")
        # Read MeteredQuantity attribute, expected to be Null or list of MeteredQuantityStruct entries.
        await self.check_metered_quantity_attribute(endpoint)

        self.step("4")
        # Read MeteredQuantityTimestamp attribute, expected to be Null or epoch-s
        await self.check_metered_quantity_timestamp_attribute(endpoint)

        self.step("5")
        # Read TariffUnit attribute, expected to be Null or TariffUnitEnum
        await self.check_tariff_unit_attribute(endpoint)


if __name__ == "__main__":
    default_matter_test_main()
