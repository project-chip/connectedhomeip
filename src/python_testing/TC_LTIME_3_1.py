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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===""


import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


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
        cluster_supported_calendar_types = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.SupportedCalendarTypes)
        cluster_not_supported_calendar_types = []
        for ctv in calendar_type_values:
            if ctv not in cluster_supported_calendar_types:
                cluster_not_supported_calendar_types.append(ctv)
        calendar_type_values.append(255)

        # Commisioning (precondition)
        self.step(0)

        self.step(1)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        logging.info(f"HourFormat {type(hour_format)} with value {hour_format}")
        # Validate Enum8
        matter_asserts.assert_valid_uint8(hour_format, description="HourFormat")
        asserts.assert_is_instance(hour_format, self.cluster.Enums.HourFormatEnum, "HourFormat is not type of HourFormatEnum")
        # Verify the values are 0,1 and 255
        asserts.assert_in(hour_format, hour_format_values)

        self.step(2)
        await self.write_single_attribute(self.cluster.Attributes.HourFormat(0), self.endpoint)

        self.step(3)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 0)

        self.step(4)
        await self.write_single_attribute(self.cluster.Attributes.HourFormat(1), self.endpoint)

        self.step(5)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 1)

        self.step(6)
        await self.write_single_attribute(self.cluster.Attributes.HourFormat(255), self.endpoint)

        self.step(7)
        hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
        asserts.assert_equal(hour_format, 255)

        self.step(8)
        activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
        logging.info(f"Value for {activecalendartype_value}")
        asserts.assert_is_instance(activecalendartype_value, self.cluster.Enums.CalendarTypeEnum,
                                   "Activecalendartype  is not type of CalendarTypeEnum")
        # Validate Enum8 range
        matter_asserts.assert_valid_uint8(activecalendartype_value, "ActiveCalendarType")
        # Is in range of 0-11,255
        asserts.assert_in(activecalendartype_value, calendar_type_values)

        self.step(9)
        await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(0), self.endpoint)
        activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
        asserts.assert_equal(activecalendartype_value, 0)

        self.step(10)
        # Verify the supported calendar types are active (can read and write).
        for supported in cluster_supported_calendar_types:
            logging.info(f"Testing for SupportedCalendarType value {supported}")
            await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(supported), self.endpoint)
            activecalendartype_value = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
            asserts.assert_equal(activecalendartype_value, supported)

        # If is the case for not supported check they return a ConstraintError
        for unsupported in cluster_not_supported_calendar_types:
            status = await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(
                unsupported), self.endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError,
                                 f"ConstraintError, unable to write value {unsupported} into ActiveCalendarType")

        self.step(11)
        supportedcalendartype_values = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.SupportedCalendarTypes)
        # Validate Enum8
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
