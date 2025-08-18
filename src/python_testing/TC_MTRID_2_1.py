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

from TC_MTRIDTestBase import MeterIdentificationTestBaseHelper

from matter.clusters import MeterIdentification
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)

cluster = MeterIdentification


class TC_MTRID_2_1(MatterBaseTest, MeterIdentificationTestBaseHelper):
    """Implementation of test case TC_MTRID_2_1."""

    def desc_TC_MTRID_2_1(self) -> str:
        """Returns a description of this test"""
        return "Attributes with Server as DUT"

    def pics_TC_MTRID_2_1(self) -> list[str]:
        """This function returns a list of PICS for this test case that must be True for the test to be run"""
        return ["MTRID.S"]

    def steps_TC_MTRID_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", "DUT is commissioned.", is_commissioning=True),
            TestStep("2", "TH reads MeterType attribute", """
                     - DUT replies a null or a MeterTypeEnum value;
                     - Verify that value in range 0 - 2."""),
            TestStep("3", "TH reads PointOfDelivery attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("4", "TH reads MeterSerialNumber attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("5", "TH reads ProtocolVersion attribute", """
                     - DUT replies a null or a value of string type;
                     - Verify that size is in range 0 - 64."""),
            TestStep("6", "TH reads PowerThreshold attribute", """
                     - DUT replies a null or a value of PowerThresholdStruct type;
                     - PowerThreshold field has type int64;
                     - ApparentPowerThreshold field has type int64;
                     - PowerThresholdSource field has type PowerThresholdSourceEnum and value in range 0 - 2."""),
        ]

        return steps

    @async_test_body
    async def test_TC_MTRID_2_1(self):
        """The main test procedure for TC_MTRID_2_1."""

        endpoint = self.get_endpoint()

        self.step("1")

        self.step("2")
        # TH reads MeterType attribute, expects a null or a MeterTypeEnum value
        self.check_meter_type_attribute(endpoint)

        self.step("3")
        # TH reads PointOfDelivery attribute, expects a null or a value of string type
        self.check_point_of_delivery_attribute(endpoint)

        self.step("4")
        # TH reads MeterSerialNumber attribute, expects a null or a value of string type
        self.check_meter_serial_number_attribute(endpoint)

        self.step("5")
        # Check that PICS MTRID.S.A0003 is True
        if not self.check_pics("MTRID.S.A0003"):
            logger.info("PICS MTRID.S.A0003 is not True")
            self.mark_current_step_skipped()

        # TH reads ProtocolVersion attribute, expects a null or a value of string type
        self.check_protocol_version_attribute(endpoint)

        self.step("6")
        # Check that PICS MTRID.S.F00 is True
        if not self.check_pics("MTRID.S.F00"):
            logger.info("PICS MTRID.S.F00 is not True")
            self.mark_current_step_skipped()

        # TH reads PowerThreshold attribute, expects a null or a value of PowerThresholdStruct type
        self.check_power_threshold_attribute(endpoint)


if __name__ == "__main__":
    default_matter_test_main()
