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

import logging

import chip.clusters as Clusters
import test_plan_support
from chip.clusters import Globals
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_COMMTR_TestBase import CommodityMeteringTestBaseHelper

logger = logging.getLogger(__name__)

cluster = Clusters.CommodityMetering


class TC_COMMTR_2_1(MatterBaseTest, CommodityMeteringTestBaseHelper):
    """Implementation of test case TC_COMMTR_2_1."""

    def desc_TC_COMMTR_2_1(self) -> str:
        """Returns a description of this test"""

        return "Attributes with Server as DUT"

    def pics_TC_COMMTR_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["COMMTR.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_COMMTR_2_1(self) -> list[TestStep]:

        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster",
                     "TestEventTriggersEnabled must be True"),
            TestStep("3", "TH reads MaximumMeteredQuantities attribute", """
                     - DUT replies a null value or a uint16 value;
                     - Store value as MaxMeteredQuantities."""),
            TestStep("4", "TH reads MeteredQuantity attribute", """
                     - DUT replies Null or a list of MeteredQuantityStruct entries.
                     - Verify that the list length less or equal MaxMeteredQuantities from step 3;
                     - Verify that the TariffComponentIDs field is a list with length less or equal 128;
                     - Verify that the Quantity field has int64 type;"""),
            TestStep("5", "TH reads MeteredQuantityTimestamp attribute", "DUT replies a null value or epoch-s type."),
            TestStep("6", "TH reads MeasurementType attribute", "DUT replies a a null value or MeasurementTypeEnum type."),
            TestStep("7", "TH sends TestEventTrigger command Fake Value Update Test Event", "Status code must be SUCCESS."),
            TestStep("8", "TH reads MeteredQuantityTimestamp attribute", "DUT replies an epoch-s value."),
            TestStep("9", "TH reads MeasurementType attribute", "DUT replies a MeasurementTypeEnum value."),
            TestStep("10", "TH reads MaximumMeteredQuantities attribute", """
                     - DUT replies a uint16 value;
                     - Value saved as MaxMeteredQuantities."""),
            TestStep("11", "TH reads MeteredQuantity attribute", """
                     - DUT replies a list of MeteredQuantityStruct entries.
                     - Verify that the list length less or equal MaxMeteredQuantities from step 10;
                     - Verify that the TariffComponentIDs field is a list with length less or equal 128;
                     - Verify that the Quantity field has int64 type;"""),
            TestStep("12", "TH sends TestEventTrigger command for Test Event Clear", "Status code must be SUCCESS."),
        ]

        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.CommodityMetering))
    async def test_TC_COMMTR_2_1(self):
        """Implements test procedure for test case TC_COMMTR_2_1."""

        endpoint = self.get_endpoint()

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")
        # commissioning

        self.step("2")
        # TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster, expected to be True
        await self.check_test_event_triggers_enabled()

        self.step("3")
        # Read MaximumMeteredQuantities attribute, expected to be Null or uint16
        self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)
        if self.MaximumMeteredQuantities is not NullValue:
            matter_asserts.assert_valid_uint16(self.MaximumMeteredQuantities, 'MaximumMeteredQuantities must be uint16')

        self.step("4")
        # Read MeteredQuantity attribute, expected to be Null or list of MeteredQuantityStruct entries.
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity
        )
        if val is not NullValue:
            # Looks like MaximumMeteredQuantities can't be Null if MeteredQuantity is not Null due to it defines the size of the list
            asserts.assert_not_equal(self.MaximumMeteredQuantities, NullValue, "MaximumMeteredQuantities must not be NullValue")
            matter_asserts.assert_list(val, "MeteredQuantity attribute must return a list",
                                       max_length=self.MaximumMeteredQuantities)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.MeteredQuantityStruct, "MeteredQuantity attribute must contain MeteredQuantityStruct elements")

        self.step("5")
        # Read MeteredQuantityTimestamp attribute, expected to be Null or epoch-s
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp
        )
        if val is not NullValue:
            matter_asserts.assert_valid_uint32(val, 'MeteredQuantityTimestamp')

        self.step("6")
        # Read MeasurementType attribute, expected to be Null or MeasurementTypeEnum
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeasurementType)
        if val is not NullValue:
            asserts.assert_is_instance(
                val, Globals.Enums.MeasurementTypeEnum, "MeasurementType attribute must return a MeasurementTypeEnum")

        self.step("7")
        # TH sends TestEventTrigger command Fake Value Update Test Event, expected SUCCESS
        await self.send_test_event_trigger_fake_value_update()

        self.step("8")
        # Read MeteredQuantityTimestamp attribute, expected to be uint32
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantityTimestamp)
        asserts.assert_not_equal(val, NullValue, "MeteredQuantityTimestamp must not be NullValue")
        matter_asserts.assert_valid_uint32(val, 'MeteredQuantityTimestamp must be uint32')

        self.step("9")
        # Read MeasurementType attribute, expected to be MeasurementTypeEnum
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeasurementType)
        asserts.assert_not_equal(val, NullValue, "MeasurementType must not be NullValue")
        matter_asserts.assert_valid_enum(
            val, "MeasurementType attribute must return a MeasurementTypeEnum", cluster.Enums.MeasurementTypeEnum)

        self.step("10")
        # Read MaximumMeteredQuantities attribute, expected to be uint16
        self.MaximumMeteredQuantities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MaximumMeteredQuantities)
        asserts.assert_not_equal(self.MaximumMeteredQuantities, NullValue, "MaximumMeteredQuantities must not be NullValue")
        matter_asserts.assert_valid_uint16(self.MaximumMeteredQuantities, 'MaximumMeteredQuantities must be uint16')

        self.step("11")
        # Read MeteredQuantity attribute, expected to be list of MeteredQuantityStruct
        # Length of the list must be less than or equal to 128
        val = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeteredQuantity)

        if val is not NullValue:
            matter_asserts.assert_list(val, "MeteredQuantity attribute must return a list",
                                       max_length=self.MaximumMeteredQuantities)
            matter_asserts.assert_list_element_type(
                val, cluster.Structs.MeteredQuantityStruct, "MeteredQuantity attribute must contain MeteredQuantityStruct elements")
            for item in val:
                await self.checkMeteredQuantityStruct(struct=item)

        self.step("12")
        # TH sends TestEventTrigger command for Test Event Clear, expected SUCCESS
        await self.send_test_event_trigger_clear()


if __name__ == "__main__":
    default_matter_test_main()
