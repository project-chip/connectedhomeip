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


import logging

import test_plan_support
from chip.clusters import Globals, MeterIdentification
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_MTRIDTestBase import MeterIdentificationTestBaseHelper

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class TC_MTRID_2_1(MatterBaseTest, MeterIdentificationTestBaseHelper):
    """Implementation of test case TC_MTRID_2_1."""

    def desc_TC_MTRID_2_1(self) -> str:
        """Returns a description of this test"""
        return "Attributes with Server as DUT"

    def pics_TC_MTRID_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MTRID.S", "DGGEN.S", "DGGEN.S.A0008", "DGGEN.S.C00.Rsp"]

    def steps_TC_MTRID_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", test_plan_support.commission_if_required(), is_commissioning=True),
            TestStep("2", "Read MeterType attribute", """
                     - DUT replies a null or a MeterTypeEnum value;
                     - Verify that value in range 0 - 2."""),
            TestStep("3", "Read PointOfDelivery attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("4", "Read MeterSerialNumber attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("5", "Read ProtocolVersion attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("6", "Read PowerThreshold attribute", """
                     - DUT replies a null or a value of PowerThresholdStruct type;
                     - PowerThreshold field has type int64;
                     - ApparentPowerThreshold field has type int64;
                     - PowerThresholdSource field has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("7", "Read TestEventTriggersEnabled attribute", "TestEventTriggersEnabled is True."),
            TestStep("8", "Send TestEventTrigger", "DUT returns SUCCESS."),
            TestStep("9", "Read MeterType attribute", """
                     - DUT replies a MeterTypeEnum value;
                     - Verify that value in range 0 - 2."""),
            TestStep("10", "Read PointOfDelivery attribute", """
                     - DUT replies a value of string type;
                     - Verify that size is in range 0 - 64.""",
                     ),
            TestStep("11", "Read MeterSerialNumber attribute", """
                     - DUT replies a value of string type;
                     - Verify that size is in range 0 - 64.""",
                     ),
            TestStep("12", "Read ProtocolVersion attribute", """
                     - DUT replies a value of string type;
                     - Verify that size is in range 0 - 64.""",
                     ),
            TestStep("13", "Read PowerThreshold attribute", """
                     - DUT replies a value of PowerThresholdStruct type;
                     - PowerThreshold field has type int64;
                     - ApparentPowerThreshold field has type int64;
                     - PowerThresholdSource field has type PowerThresholdSourceEnum and value in range 0 - 2."""),
            TestStep("14", "Send TestEventTrigger Clear", "DUT returns SUCCESS."),
        ]

        return steps

    @async_test_body
    async def test_TC_MTRID_2_1(self):
        """The main test procedure for TC_MTRID_2_1."""

        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics("DGGEN.S") or not self.check_pics("DGGEN.S.A0008") or not self.check_pics("DGGEN.S.C00.Rsp"):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterType
        )
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val,
                "MeterType attribute must return a Clusters.MeterIdentification.Enums.MeterTypeEnum",
                MeterIdentification.Enums.MeterTypeEnum,
            )

        self.step("3")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        if val is not NullValue:
            matter_asserts.assert_is_string(val, "PointOfDelivery must be a string")
            asserts.assert_less_equal(len(val), 64, "PointOfDelivery must have length at most 64!")

        self.step("4")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        if val is not NullValue:
            matter_asserts.assert_is_string(val, "MeterSerialNumber must be a string")
            asserts.assert_less_equal(len(val), 64, "MeterSerialNumber must have length at most 64!")

        self.step("5")
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0003 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ProtocolVersion):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
            )
            if val is not NullValue and val is not None:
                matter_asserts.assert_is_string(val, "ProtocolVersion must be a string")
                asserts.assert_less_equal(len(val), 64, "ProtocolVersion must have length at most 64!")

        self.step("6")
        if not self.check_pics("MTRID.S.F00"):
            logger.info("PICS MTRID.S.F00 is not True")
            self.mark_current_step_skipped()

        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
            )
            if val is not NullValue:
                asserts.assert_true(
                    isinstance(val, Globals.Structs.PowerThresholdStruct),
                    "val must be of type Globals.Structs.PowerThresholdStruct",
                )
                await self.checkPowerThresholdStruct(struct=val)

        self.step("7")
        await self.check_test_event_triggers_enabled()

        self.step("8")
        await self.send_test_event_trigger_fake_data()

        self.step("9")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterType
        )
        asserts.assert_not_equal(val, NullValue, "MeterType attribute must return a value")
        matter_asserts.assert_valid_enum(
            val,
            "MeterType attribute must return a Clusters.MeterIdentification.Enums.MeterTypeEnum",
            MeterIdentification.Enums.MeterTypeEnum,
        )

        self.step("10")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        asserts.assert_not_equal(val, NullValue, "PointOfDelivery attribute must return a value")
        matter_asserts.assert_is_string(val, "PointOfDelivery must be a string")
        asserts.assert_less_equal(len(val), 64, "PointOfDelivery must have length at most 64!")

        self.step("11")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        asserts.assert_not_equal(val, NullValue, "MeterSerialNumber attribute must return a value")
        matter_asserts.assert_is_string(val, "MeterSerialNumber must be a string")
        asserts.assert_less_equal(len(val), 64, "MeterSerialNumber must have length at most 64!")

        self.step("12")
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0003 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ProtocolVersion):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
            )
            if self.check_pics("MTRID.S.A0003"):
                asserts.assert_is_not_none(val, "ProtocolVersion must be not None as it is enabled.")
            asserts.assert_not_equal(val, NullValue, "ProtocolVersion attribute must return a value")
            matter_asserts.assert_is_string(val, "ProtocolVersion must be a string")
            asserts.assert_less_equal(len(val), 64, "ProtocolVersion must have length at most 64!")

        self.step("13")
        if not self.check_pics("MTRID.S.F00"):
            logger.info("PICS MTRID.S.F00 is not True")
            self.mark_current_step_skipped()

        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
            )
            if self.check_pics("MTRID.S.F00"):
                asserts.assert_is_not_none(val, "PowerThreshold must be not None as it is enabled.")
            asserts.assert_not_equal(val, NullValue, "PowerThreshold attribute must return a value")
            asserts.assert_true(
                isinstance(val, Globals.Structs.PowerThresholdStruct),
                "val must be of type Globals.Structs.PowerThresholdStruct",
            )
            await self.checkPowerThresholdStruct(struct=val)

        self.step("14")
        await self.send_test_event_clear()


if __name__ == "__main__":
    default_matter_test_main()
