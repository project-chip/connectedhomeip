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
#       --endpoint 0
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
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
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
            TestStep(2, "TH writes 0(12hr) to HourFormat attribute"),
            TestStep(3, "If the prior write was successful, TH reads HourFormat attribute", "Verify that the value is 0 (12hr)."),
            TestStep(4, "TH writes 1(24hr) to HourFormat attribute"),
            TestStep(5, "If the prior write was successful, TH reads HourFormat attribute", "Verify that the value is 1 (24hr)."),
            TestStep(6, "TH writes 255 (UseActiveLocale) to HourFormat attribute"),
            TestStep(7, "If the prior write was successful, TH reads HourFormat attribute",
                     "Verify that the value is 255 (UseActiveLocale)."),
            TestStep(8, "TH writes 5 to HourFormat attribute", "Verify that the write request shows 0x87 (Constraint Error)."),
            TestStep(9, "TH reads the feature map attribute. If the CALFMT feature is not supported, skip all remaining steps"),
            TestStep(10, "TH reads SupportedCalendarTypes attribute from DUT and saves as `cluster_supported_calendar_types`",
                     "Verify that the SupportedCalendarTypes attribute is of Enum8 datatype and only contains values from 0-11 (specific calendar types) and 255 (UseActiveLocale), as defined in the CalendarTypeEnum in the specification."),
            TestStep(11, "TH reads ActiveCalendarType attribute from DUT",
                     "Verify that the ActiveCalendarType is in `cluster_supported_calendar_types`"),
            TestStep(12, "For each entry in `cluster_supported_calendar_types`, TH writes that value to ActiveCalendarType, verifies that the write was successful and confirms the value via a read",
                     "Write is successful and read value matches for every entry"),
            TestStep(13, "TH finds the set of CalendarTypesEnum value that do not appear in `cluster_supported_calendar_types` and saves as `cluster_not_supported_calendar_types`"),
            TestStep(14, "For each value in `cluster_unsupported_calendar_types`, TH writes the value to the ActiveCalendarType attributes",
                     "DUT returns CONSTRAINT_ERROR"),
            TestStep(15, "TH writes 50 to ActiveCalendarType attribute",
                     "Verify that the write request shows 0x87 (Constraint Error)."),
        ]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.TimeFormatLocalization))
    async def test_TC_LTIME_3_1(self):
        self.endpoint = self.get_endpoint(default=0)
        self.cluster = Clusters.TimeFormatLocalization
        hour_format_values = [0, 1, 255]

        # Commissioning (precondition)
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
        resp = await self.write_single_attribute(self.cluster.Attributes.HourFormat(0), self.endpoint, expect_success=False)

        self.step(3)
        if resp == Status.Success:
            hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
            asserts.assert_equal(hour_format, 0)
        else:
            self.mark_current_step_skipped()

        self.step(4)
        resp = await self.write_single_attribute(self.cluster.Attributes.HourFormat(1), self.endpoint, expect_success=False)

        self.step(5)
        if resp == Status.Success:
            hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
            asserts.assert_equal(hour_format, 1)
        else:
            self.mark_current_step_skipped()

        self.step(6)
        resp = await self.write_single_attribute(self.cluster.Attributes.HourFormat(255), self.endpoint, expect_success=False)

        self.step(7)
        if resp == Status.Success:
            hour_format = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.HourFormat)
            asserts.assert_equal(hour_format, 255)
        else:
            self.mark_current_step_skipped()

        self.step(8)
        hourformat_status = await self.write_single_attribute(self.cluster.Attributes.HourFormat(5), self.endpoint, expect_success=False)
        asserts.assert_equal(hourformat_status, Status.ConstraintError)

        self.step(9)
        feature_map = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.FeatureMap)
        if (feature_map & self.cluster.Bitmaps.Feature.kCalendarFormat) == 0:
            self.skip_all_remaining_steps(10)
            return

        self.step(10)
        calendar_type_values = set([i for i in range(0, 12)])
        calendar_type_values.add(255)
        cluster_supported_calendar_types = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.SupportedCalendarTypes)
        asserts.assert_true(set(cluster_supported_calendar_types).issubset(
            set(calendar_type_values)), "SupportedCalendarTypes contains unknown values")

        self.step(11)
        active_calendar_type = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
        logging.info(f"Value for {active_calendar_type}")
        matter_asserts.assert_valid_uint8(active_calendar_type, "ActiveCalendarType")
        asserts.assert_is_instance(active_calendar_type, self.cluster.Enums.CalendarTypeEnum,
                                   "ActiveCalendarType  is not type of CalendarTypeEnum")
        asserts.assert_in(active_calendar_type, cluster_supported_calendar_types,
                          "ActiveCalendarType is not listed in SupportedCalendarTypes")

        self.step(12)
        # Verify the supported calendar types are active (can read and write).
        for supported in cluster_supported_calendar_types:
            logging.info(f"Testing for SupportedCalendarType value {supported}")
            await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(supported), self.endpoint)
            active_calendar_type = await self.read_single_attribute_check_success(self.cluster, self.cluster.Attributes.ActiveCalendarType)
            asserts.assert_equal(active_calendar_type, supported)

        self.step(13)
        cluster_not_supported_calendar_types = calendar_type_values - set(cluster_supported_calendar_types)

        self.step(14)
        # If is the case for not supported check they return a ConstraintError
        for unsupported in cluster_not_supported_calendar_types:
            status = await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(
                unsupported), self.endpoint, expect_success=False)
            asserts.assert_equal(status, Status.ConstraintError,
                                 f"ConstraintError, unable to write value {unsupported} into ActiveCalendarType")

        self.step(15)
        activecalendartype_status = await self.write_single_attribute(self.cluster.Attributes.ActiveCalendarType(50), self.endpoint, expect_success=False)
        asserts.assert_equal(activecalendartype_status, Status.ConstraintError)


if __name__ == "__main__":
    default_matter_test_main()
