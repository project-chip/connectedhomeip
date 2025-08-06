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

from chip.clusters import Globals, MeterIdentification
from chip.clusters.Types import NullValue
from chip.testing import matter_asserts
from chip.testing.event_attribute_reporting import AttributeSubscriptionHandler
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_MTRIDTestBase import MeterIdentificationTestBaseHelper

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class TC_MTRID_3_1(MatterBaseTest, MeterIdentificationTestBaseHelper):
    """Implementation of test case TC_MTRID_2_1."""

    def desc_TC_MTRID_3_1(self) -> str:
        """Returns a description of this test"""
        return "Subscription Report Verification with Server as DUT"

    def pics_TC_MTRID_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MTRID.S",
                "DGGEN.S",
                "DGGEN.S.A0008",
                "DGGEN.S.C00.Rsp"]

    def steps_TC_MTRID_3_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", "DUT is commissioned.", is_commissioning=True),
            TestStep("2", """Set up a subscription to the Meter Identification cluster:
                     - MinIntervalFloor: 0
                     - MaxIntervalCeiling: 10""", "Subscription successfully established."),
            TestStep("3", "TH read MeterType attribute.", """
                     - DUT replies a null or a MeterTypeEnum value;
                     - Store value as meter_type."""),
            TestStep("4", "TH read PointOfDelivery attribute.", """
                     - DUT replies a null or a value of string type;
                     - Store value as point_of_delivery."""),
            TestStep("5", "TH read MeterSerialNumber attribute.", """
                     - DUT replies a null or a value of string type;
                     - Store value as meter_serial_number."""),
            TestStep("6", "TH read ProtocolVersion attribute.", """
                     - DUT replies a null or a value of string type;
                     - Store value as protocol_version."""),
            TestStep("7", "TH read PowerThreshold attribute.", """
                     - DUT replies a null or a MeterTypeEnum value;
                     - Store value as power_threshold."""),
            TestStep("8", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "TestEventTriggersEnabled is True."),
            TestStep("9", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.MTRID.TEST_EVENT_TRIGGER_KEY 
                     and EventTrigger field set to PIXIT.MTRID.TEST_EVENT_TRIGGER for Attributes Value Set Test Event.""",
                     "DUT returns SUCCESS."),
            TestStep("10", "TH awaits a MeterType attribute with 10s timeout.",
                     "Verify the report is received and the value does not match the meter_type."),
            TestStep("11", "TH awaits a PointOfDelivery attribute with 10s timeout.",
                     "Verify the report is received and the value does not match the point_of_delivery."),
            TestStep("12", "TH awaits a MeterSerialNumber attribute with 10s timeout.",
                     "Verify the report is received and the value does not match the meter_serial_number."),
            TestStep("13", "TH awaits a ProtocolVersion attribute with 10s timeout.",
                     "Verify the report is received and the value does not match the protocol_version."),
            TestStep("14", "TH awaits a PowerThreshold attribute with 10s timeout.",
                     "Verify the report is received and the value does not match the power_threshold."),
            TestStep("15", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.MTRID.TEST_EVENT_TRIGGER_KEY 
                     and EventTrigger field set to PIXIT.MTRID.TEST_EVENT_TRIGGER for Test Event Clear.""",
                     "DUT returns SUCCESS."),
            TestStep("16", "TH removes the subscriptions to the Meter Identification cluster.", "Subscription successfully removed."),
        ]

        return steps

    @async_test_body
    async def test_TC_MTRID_3_1(self):

        endpoint = self.get_endpoint()
        attributes = cluster.Attributes

        # If TestEventTriggers is not enabled this TC can't be checked properly.
        if not self.check_pics('DGGEN.S') or not self.check_pics('DGGEN.S.A0008') or not self.check_pics('DGGEN.S.C00.Rsp'):
            asserts.skip("PICS DGGEN.S or DGGEN.S.A0008 or DGGEN.S.C00.Rsp is not True")

        self.step("1")

        self.step("2")
        subscription_handler = AttributeSubscriptionHandler(cluster)
        await subscription_handler.start(self.default_controller, self.dut_node_id,
                                         endpoint,
                                         min_interval_sec=0,
                                         max_interval_sec=10, keepSubscriptions=True)

        self.step("3")
        meter_type = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterType
        )
        if meter_type is not NullValue:
            matter_asserts.assert_valid_enum(
                meter_type,
                "MeterType attribute must return a Clusters.MeterIdentification.Enums.MeterTypeEnum",
                MeterIdentification.Enums.MeterTypeEnum,
            )

        self.step("4")
        point_of_delivery = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        if point_of_delivery is not NullValue:
            matter_asserts.assert_is_string(point_of_delivery, "PointOfDelivery must be a string")

        self.step("5")
        meter_serial_number = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        if meter_serial_number is not NullValue:
            matter_asserts.assert_is_string(meter_serial_number, "MeterSerialNumber must be a string")

        self.step("6")
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0003 is not True")
            self.mark_current_step_skipped()

        if await self.attribute_guard(endpoint=endpoint, attribute=attributes.ProtocolVersion):
            protocol_version = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
            )
            if protocol_version is not NullValue and protocol_version is not None:
                matter_asserts.assert_is_string(protocol_version, "ProtocolVersion must be a string")

        self.step("7")
        if not self.check_pics("MTRID.S.F00"):
            logger.info("PICS MTRID.S.F00 is not True")
            self.mark_current_step_skipped()

        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):
            power_threshold = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
            )
            if power_threshold is not NullValue:
                asserts.assert_true(
                    isinstance(power_threshold, Globals.Structs.PowerThresholdStruct),
                    "val must be of type Globals.Structs.PowerThresholdStruct",
                )

        self.step("8")
        await self.check_test_event_triggers_enabled()

        self.step("9")
        await self.send_test_event_trigger_fake_data()

        self.step("10")
        self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeterType, "MeterType", meter_type)

        self.step("11")
        self.verify_reporting(subscription_handler.attribute_reports,
                              cluster.Attributes.PointOfDelivery, "PointOfDelivery", point_of_delivery)

        self.step("12")
        self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeterSerialNumber,
                              "MeterSerialNumber", meter_serial_number)

        if self.check_pics("MTRID.S.A0003"):
            self.step("13")
            self.verify_reporting(subscription_handler.attribute_reports,
                                  cluster.Attributes.ProtocolVersion, "ProtocolVersion", protocol_version)
        else:
            self.skip_step("13")

        if self.check_pics("MTRID.S.F00"):
            self.step("14")
            self.verify_reporting(subscription_handler.attribute_reports,
                                  cluster.Attributes.PowerThreshold, "PowerThreshold", power_threshold)
        else:
            self.skip_step("14")

        self.step("15")
        await self.send_test_event_clear()

        self.step("16")
        await subscription_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
