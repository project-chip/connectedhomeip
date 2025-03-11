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
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md"#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --str-arg PIXIT.LTIME.SCT:0,1,2,3,4,5,6,7,8,9,10,11
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===""


import chip.clusters as Clusters

from chip.interaction_model import Status
from chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body, default_matter_test_main)
from mobly import asserts
import logging


class TC_LTIME_3_1(MatterBaseTest):

    def desc_TC_LTIME_3_1(self) -> str:
        return "4.1.1. [TC-LTIME-3.1] Read and Write Time Format Localization Cluster Attributes [DUT as Server]"

    def pics_TC_LTIME_3_1(self):
        """Return PICS definitions asscociated with this test."""
        pics = [
            "LTIME.S"
        ]
        return pics

    def steps_TC_LTIME_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(0, "TH is commissioned with DUT", is_commissioning=True),
            TestStep(1, "TH reads HourFormat attribute from DUT",
                     "Verify that the HourFormat attribute is of Enum8 datatype and that the values are 0 (12hr), 1 (24hr), and 255 (UseActiveLocale) as per the HourFormatEnum in the specification."),
            TestStep(2, "If (LTIME.S.M.12Hr) TH writes 0(12hr) to HourFormat attribute",
                     "Verify that the write request is successful."),
            TestStep(3, "TH reads HourFormat attribute", "Verify that the value is 0 (12hr)."),
            TestStep(4, "If (LTIME.S.M.24Hr) TH writes 1(24hr) to HourFormat attribute",
                     "Verify that the write request is successful."),
            TestStep(5, "TH reads HourFormat attribute", "Verify that the value is 1 (24hr)."),
            TestStep(6, "TH writes 255 (UseActiveLocale) to HourFormat attribute", "Verify that the write request is successful."),
            TestStep(7, "TH reads HourFormat attribute", "Verify that the value is 255 (UseActiveLocale)."),
            TestStep(8, "TH reads ActiveCalendarType attribute from DUT",
                     "Verify that the ActiveCalendarType attribute is of Enum8 datatype and only accepts values 0-11 (specific calendar types) and 255 (UseActiveLocale), as defined in the CalendarTypeEnum in the specification."),
            TestStep(9, "TH writes value in PIXIT.LTIME.SCT to ActiveCalendarType attribute, followed by reading the ActiveCalendarType attribute value",
                     "Verify that the write request is successful and that the read value matches the written value in previous step. as the value that was set before."),
            TestStep(10, "Repeat step 9 for all the values in PIXIT.LTIME.SCT",
                     "Verify that the write request is successful and that the read value matches the written value."),
            TestStep(11, "TH reads SupportedCalendarTypes attribute from DUT",
                     "Verify that the SupportedCalendarTypes attribute is of Enum8 datatype and only accepts values 0-11 (specific calendar types) and 255 (UseActiveLocale), as defined in the CalendarTypeEnum in the specification."),
            TestStep(12, "TH writes 50 to ActiveCalendarType attribute",
                     "Verify that the write request shows 0x87 (Constraint Error)."),
            TestStep(13, "TH writes 5 to HourFormat attribute", "Verify that the write request shows 0x87 (Constraint Error).")
        ]
        return steps

    @async_test_body
    async def test_TC_LTIME_3_1(self):
        self.endpoint = self.get_endpoint(0)
        self.cluster = Clusters.TimeFormatLocalization
        hour_format_values = [0, 1, 255]

        calendar_type_values = [i for i in range(0, 12)]
        calendar_type_values.append(255)

        # Read PIXIT values

        pixit_SCT = self.user_params['PIXIT.LTIME.SCT']
        logging.info(f"PIXIT SCT: {pixit_SCT}")
        if pixit_SCT is not None:
            pixit_SCT = pixit_SCT.split(",")
            logging.info(f"List of PIXIT.SCT {pixit_SCT}")

        self.step(0)

        self.step(1)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        logging.info(f"Hourformat {type(hour_format)} with value {hour_format}")
        asserts.assert_true(isinstance(hour_format, self.cluster.Enums.HourFormatEnum), "Hourformat is not type of HourFormatEnum")
        # Is in range of 0-11,255
        asserts.assert_in(hour_format, hour_format_values)

        self.step(2)
        if self.pics_guard("LTIME.S.M.12Hr"):
            status = await self.write_single_attribute(self.cluster.Attributes.HourFormat(self.cluster.Enums.HourFormatEnum.k12hr), self.endpoint, expect_success=True)
            logging.info(f"Write status {status}")

        self.step(3)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 0)

        await self.write_single_attribute(Clusters.NetworkCommissioning.Attributes.MaxNetworks(2), endpoint_id=0, expect_success=True)

        self.step(4)
        if self.pics_guard("LTIME.S.M.24Hr"):
            status = await self.write_single_attribute(self.cluster.Attributes.HourFormat(self.cluster.Enums.HourFormatEnum.k24hr), self.endpoint, expect_success=True)

        self.step(5)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 1)

        self.step(6)
        status = await self.write_single_attribute(self.cluster.Attributes.HourFormat(self.cluster.Enums.HourFormatEnum.kUseActiveLocale), self.endpoint, expect_success=True)

        self.step(7)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 255)

        self.step(8)
        activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
        asserts.assert_true(isinstance(activecalendartype_value, self.cluster.Enums.CalendarTypeEnum),
                            "Activecalendartype  is not type of CalendarTypeEnum")
        # Is in range of 0-11,255
        asserts.assert_in(activecalendartype_value, calendar_type_values)
        # check values in the range

        self.step(9)
        await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(0), self.endpoint, expect_success=True)
        activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
        asserts.assert_equal(activecalendartype_value, 0)

        self.step(10)
        for i in range(1, 12):
            await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(i), self.endpoint, expect_success=True)
            activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
            asserts.assert_equal(activecalendartype_value, i)

        self.step(11)
        supportedcalendartype_values = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.SupportedCalendarTypes)
        logging.info(f"Supported calendar type value {supportedcalendartype_values}")
        for calendartype in supportedcalendartype_values:
            asserts.assert_in(calendartype, calendar_type_values)

        self.step(12)
        activecalendartype_status = await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(50), self.endpoint, expect_success=False)
        asserts.assert_equal(activecalendartype_status, Status.ConstraintError)

        self.step(13)
        hourformat_status = await self.write_single_attribute(self.cluster.Attributes.HourFormat(5), self.endpoint, expect_success=False)
        asserts.assert_equal(hourformat_status, Status.ConstraintError)


if __name__ == "__main__":
    default_matter_test_main()
