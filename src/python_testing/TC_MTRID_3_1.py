#!/usr/bin/env python3
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
#     app: ${energy-gateway}
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

from mobly import asserts
from TC_MTRIDTestBase import MeterIdentificationTestBaseHelper

from matter.clusters import MeterIdentification
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class TC_MTRID_3_1(MeterIdentificationTestBaseHelper):
    """Implementation of test case TC_MTRID_3_1."""

    def desc_TC_MTRID_3_1(self) -> str:
        """Returns a description of this test"""

        return "Subscription Report Verification with Server as DUT"

    def pics_TC_MTRID_3_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""

        return ["MTRID.S"]

    def steps_TC_MTRID_3_1(self) -> list[TestStep]:

        return [
            TestStep("1", "Commissioning, already done", "DUT is commissioned.", is_commissioning=True),
            TestStep("2", """Set up a subscription to the Meter Identification cluster:
                     - MinIntervalFloor: 0
                     - MaxIntervalCeiling: 10""", "Subscription successfully established."),
            TestStep("3", "TH reads MeterType attribute.", """
                     - DUT replies a null or a MeterTypeEnum value;
                     - Verify that value in range 0 - 2;
                     - Store value as meter_type."""),
            TestStep("4", "TH reads PointOfDelivery attribute.", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64;
                     - Store value as point_of_delivery."""),
            TestStep("5", "TH reads MeterSerialNumber attribute.", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64;
                     - Store value as meter_serial_number."""),
            TestStep("6", "TH reads ProtocolVersion attribute.", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64;
                     - Store value as protocol_version."""),
            TestStep("7", "TH reads PowerThreshold attribute.", """
                     - DUT replies a null or a value of PowerThresholdStruct type;
                     - PowerThreshold field has type int64;
                     - ApparentPowerThreshold field has type int64;
                     - PowerThresholdSource field has type PowerThresholdSourceEnum and value in range 0 - 2;
                     - Store value as power_threshold."""),
            TestStep("8", "TH reads TestEventTriggersEnabled attribute from General Diagnostics Cluster.",
                     "TestEventTriggersEnabled is True."),
            TestStep("9", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.MTRID.TEST_EVENT_TRIGGER_KEY
                     and EventTrigger field set to PIXIT.MTRID.TEST_EVENT_TRIGGER for Attributes Value Set Test Event.""",
                     "DUT returns SUCCESS."),
            TestStep("10", "TH awaits a MeterType attribute with 10s timeout.", """
                     - Verify the report is received and contains a null or a MeterTypeEnum value;
                     - Value in range 0 - 2;
                     - The value does not match the meter_type."""),
            TestStep("11", "TH awaits a PointOfDelivery attribute with 10s timeout.", """
                     - Verify the report is received and contains a null or a value of string type;
                     - Size in range 0 - 64;
                     - The value does not match the point_of_delivery."""),
            TestStep("12", "TH awaits a MeterSerialNumber attribute with 10s timeout.", """
                     - Verify the report is received and contains a null or a value of string type;
                     - Size in range 0 - 64;
                     - The value does not match the meter_serial_number."""),
            TestStep("13", "TH awaits a ProtocolVersion attribute with 10s timeout.", """
                     - Verify the report is received and contains a null or a value of string type;
                     - Size in range 0 - 64;
                     - The value does not match the protocol_version."""),
            TestStep("14", "TH awaits a PowerThreshold attribute with 10s timeout.", """
                     - Verify the report is received and contains a null or a value of PowerThresholdStruct type;
                     - PowerThreshold field has type int64;
                     - ApparentPowerThreshold field has type int64;
                     - PowerThresholdSource field has type PowerThresholdSourceEnum and value in range 0 - 2;
                     - The value does not match the power_threshold."""),
            TestStep("15", """TH sends TestEventTrigger command to General Diagnostics Cluster on Endpoint 0 with EnableKey field set to PIXIT.MTRID.TEST_EVENT_TRIGGER_KEY
                     and EventTrigger field set to PIXIT.MTRID.TEST_EVENT_TRIGGER for Test Event Clear.""",
                     "DUT returns SUCCESS."),
            TestStep("16", "TH removes the subscriptions to the Meter Identification cluster.", "Subscription successfully removed."),
        ]

    @async_test_body
    async def test_TC_MTRID_3_1(self):

        endpoint = self.get_endpoint()

        matcher_list = self.get_mandatory_matchers()

        self.step("1")
        # Commissioning, already done

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
        await self.check_meter_type_attribute(endpoint, meter_type)

        self.step("4")
        point_of_delivery = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PointOfDelivery
        )
        await self.check_point_of_delivery_attribute(endpoint, point_of_delivery)

        self.step("5")
        meter_serial_number = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.MeterSerialNumber
        )
        await self.check_meter_serial_number_attribute(endpoint, meter_serial_number)

        # Checks if ProtocolVersion attribute is supported
        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.ProtocolVersion):

            self.step("6")

            if not self.check_pics("MTRID.S.A0003"):  # for cases when it is supported by DUT, but disabled in PICS
                logger.warning("ProtocolVersion attribute is actually supported by DUT, but PICS MTRID.S.A0003 is False")

            # TH reads ProtocolVersion attribute, expects a null or a value of string type
            protocol_version = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ProtocolVersion
            )
            await self.check_protocol_version_attribute(endpoint, protocol_version)
            matcher_list.append(self._protocol_version_matcher())
        else:

            if self.check_pics("MTRID.S.A0003"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("6")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. ProtocolVersion is not actually supported, but MTRID.S.A0003 is True.")
            else:  # attribute is not supported at all
                self.skip_step("6")

        # Checks if PowerThreshold feature is supported
        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):

            self.step("7")

            if not self.check_pics("MTRID.S.A0004"):
                logger.warning("PowerThreshold feature is actually supported by DUT, but PICS MTRID.S.A0004 is False")

            # TH reads PowerThreshold attribute, expects a null or a value of PowerThresholdStruct type
            power_threshold = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.PowerThreshold
            )
            await self.check_power_threshold_attribute(endpoint, power_threshold)
            matcher_list.append(self._power_threshold_matcher())
        else:

            if self.check_pics("MTRID.S.A0004"):
                self.step("7")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. PowerThreshold feature is not actually supported, but MTRID.S.A0004 is True.")
            else:
                self.skip_step("7")

        self.step("8")
        await self.check_test_event_triggers_enabled()

        self.step("9")
        await self.send_test_event_trigger_attributes_value_set()
        subscription_handler.await_all_expected_report_matches(matcher_list, timeout_sec=10)

        self.step("10")
        await self.check_meter_type_attribute(endpoint, subscription_handler.attribute_reports[cluster.Attributes.MeterType][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeterType, "MeterType", meter_type)

        self.step("11")
        await self.check_point_of_delivery_attribute(endpoint, subscription_handler.attribute_reports[cluster.Attributes.PointOfDelivery][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports,
                                    cluster.Attributes.PointOfDelivery, "PointOfDelivery", point_of_delivery)

        self.step("12")
        await self.check_meter_serial_number_attribute(
            endpoint, subscription_handler.attribute_reports[cluster.Attributes.MeterSerialNumber][0].value)
        await self.verify_reporting(subscription_handler.attribute_reports, cluster.Attributes.MeterSerialNumber,
                                    "MeterSerialNumber", meter_serial_number)

        if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.ProtocolVersion):

            self.step("13")

            if not self.check_pics("MTRID.S.A0003"):  # for cases when it is supported by DUT, but disabled in PICS
                logger.warning("ProtocolVersion attribute is actually supported by DUT, but PICS MTRID.S.A0003 is False")

            # TH reads ProtocolVersion attribute, expects a null or a value of string type
            await self.check_protocol_version_attribute(
                endpoint, subscription_handler.attribute_reports[cluster.Attributes.ProtocolVersion][0].value)
            await self.verify_reporting(subscription_handler.attribute_reports,
                                        cluster.Attributes.ProtocolVersion, "ProtocolVersion", protocol_version)
        else:

            if self.check_pics("MTRID.S.A0003"):  # for cases when it is not supported by DUT, but enabled in PICS
                self.step("13")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. ProtocolVersion is not actually supported, but MTRID.S.A0003 is True.")
            else:  # attribute is not supported at all
                self.skip_step("13")

        if await self.feature_guard(endpoint=endpoint, cluster=cluster, feature_int=cluster.Bitmaps.Feature.kPowerThreshold):

            self.step("14")

            if not self.check_pics("MTRID.S.A0004"):
                logger.warning("PowerThreshold feature is actually supported by DUT, but PICS MTRID.S.A0004 is False")

            # TH reads PowerThreshold attribute, expects a null or a value of PowerThresholdStruct type
            await self.check_power_threshold_attribute(
                endpoint, subscription_handler.attribute_reports[cluster.Attributes.PowerThreshold][0].value)
            await self.verify_reporting(subscription_handler.attribute_reports,
                                        cluster.Attributes.PowerThreshold, "PowerThreshold", power_threshold)
        else:

            if self.check_pics("MTRID.S.A0004"):
                self.step("14")
                asserts.fail(
                    "PICS file does not correspond to real DUT functionality. PowerThreshold feature is not actually supported, but MTRID.S.A0004 is True.")
            else:
                self.skip_step("14")

        self.step("15")
        await self.send_test_event_clear()

        self.step("16")
        await subscription_handler.cancel()


if __name__ == "__main__":
    default_matter_test_main()
