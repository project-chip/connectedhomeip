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
#       --enable-key 00112233445566778899aabbccddeeff
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --hex-arg enableKey:00112233445566778899aabbccddeeff
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_MTRID_2_1."""

import logging

import test_plan_support
from chip.clusters import Globals, MeterIdentification
from chip.clusters.Types import Nullable, NullValue
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class TC_MTRID_2_1(MatterBaseTest):
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
            TestStep("2", "Read MeterType attribute",
                          "- DUT reply a null value of Nullable type."),
            TestStep("3", "Read PointOfDelivery attribute",
                          "- DUT reply a null value of Nullable type."),
            TestStep("4", "Read MeterSerialNumber attribute",
                          "- DUT reply a null value of Nullable type."),
            TestStep("5", "Read ProtocolVersion attribute",
                          "- DUT reply a null value of Nullable type."),
            TestStep("6", "Read PowerThreshold attribute",
                          "- DUT reply a null value of Nullable type."),
            TestStep("7", "Read TestEventTriggersEnabled attribute",
                          "- TestEventTriggersEnabled is True."),
            TestStep("8", "Send TestEventTrigger",
                          "- DUT returns SUCCESS."),
            TestStep("9", "Read MeterType attribute",
                          """- DUT reply a enum8 (MeterTypeEnum) value"""),
            TestStep("10", "Read PointOfDelivery attribute",
                           """- DUTreply a value of string type.
                              - Verify that size is in range 0 - 64""",
                     ),
            TestStep("11", "Read MeterSerialNumber attribute",
                           """- DUT reply a value of string type.
                              - Verify that size is in range 0 - 64""",
                     ),
            TestStep("12", "Read ProtocolVersion attribute",
                           """- DUT reply a value of string type.
                              - Verify that size is in range 0 - 64""",
                     ),
            TestStep("13", "Read PowerThreshold attribute",
                           """- DUT reply a struct (PowerThresholdStruct) value."""),
            TestStep("14", "Send TestEventTrigger Clear",
                           """- DUT returns SUCCESS."""),
        ]

        return steps

    async def checkPowerThresholdStruct(self, struct: Globals.Structs.PowerThresholdStruct = None):
        if struct.powerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.powerThreshold, "PowerThreshold")
        if struct.apparentPowerThreshold is not None:
            matter_asserts.assert_valid_int64(struct.apparentPowerThreshold, "ApparentPowerThreshold")
        if struct.powerThresholdSource is not NullValue:
            matter_asserts.assert_valid_enum(
                struct.powerThresholdSource,
                "PowerThresholdSource attribute must return a Globals.Enums.PowerThresholdSourceEnum",
                Globals.Enums.PowerThresholdSourceEnum,
            )

    @async_test_body
    async def test_TC_MTRID_2_1(self):
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
        asserts.assert_true(val is NullValue, "MeterType attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "MeterType attribute must be a Nullable")

        self.step("3")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        asserts.assert_true(val is NullValue, "PointOfDelivery attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "PointOfDelivery attribute must be a Nullable")

        self.step("4")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        asserts.assert_true(val is NullValue, "MeterSerialNumber attribute must return a Null")
        asserts.assert_is_instance(val, Nullable, "MeterSerialNumber attribute must be a Nullable")

        self.step("5")
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0003 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ProtocolVersion):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
            )
            asserts.assert_true(val is NullValue, "ProtocolVersion attribute must return a Null")
            asserts.assert_is_instance(val, Nullable, "ProtocolVersion attribute must be a Nullable")

        self.step("6")
        if not self.check_pics("MTRID.S.A0004"):
            logger.info("PICS MTRID.S.A0004 is not True")
            self.mark_current_step_skipped()

        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            val = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
            )
            asserts.assert_true(val is NullValue, "PowerThreshold attribute must return a Null")
            asserts.assert_is_instance(val, Nullable, "PowerThreshold attribute must be a Nullable")

        self.step("7")
        await self.check_test_event_triggers_enabled()

        self.step("8")
        await self.send_test_event_triggers(eventTrigger=0x0B06000000000000)

        self.step("9")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterType
        )
        if val is not NullValue:
            matter_asserts.assert_valid_enum(
                val,
                "MeterType attribute must return a Clusters.MeterIdentification.Enums.MeterTypeEnum",
                MeterIdentification.Enums.MeterTypeEnum,
            )

        self.step("10")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        if val is not NullValue:
            matter_asserts.assert_is_string(val, "PointOfDelivery must be a string")
            asserts.assert_less_equal(len(val), 64, "PointOfDelivery must have length at most 64!")

        self.step("11")
        val = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        if val is not NullValue:
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
            if val is not NullValue and val is not None:
                matter_asserts.assert_is_string(val, "ProtocolVersion must be a string")
                asserts.assert_less_equal(len(val), 64, "ProtocolVersion must have length at most 64!")

        self.step("13")
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0004 is not True")
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

        self.step("14")
        await self.send_test_event_triggers(eventTrigger=0x0B06000000000001)


if __name__ == "__main__":
    default_matter_test_main()
