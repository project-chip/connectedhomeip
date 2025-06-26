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

from typing import Callable

import chip.clusters as Clusters
import TC_DefaultWarnings
from chip.testing.matter_testing import MatterBaseTest, default_matter_test_main
from mobly import asserts


class TestDefaultChecker(MatterBaseTest):
    ''' This is a unit-style test of the defaults test. It runs the individual test functions against mocks.'''

    def setup_test(self):
        self.test = TC_DefaultWarnings.DefaultChecker()
        self.test.mark_current_step_skipped = self.mark_current_step_skipped
        self.skipped = 0
        super().setup_test()

    def mark_current_step_skipped(self):
        ''' Override of the base mark_current_step_skipped so the tests here can assert that these get called correctly.'''
        self.skipped += 1

    def test_product_name_is_not_default_check(self):
        def run_check(product_name: str, set_override: bool, expect_problem: bool):
            self.test.user_params = {TC_DefaultWarnings.FLAG_PRODUCT_NAME: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.BasicInformation: {Clusters.BasicInformation.Attributes.ProductName: product_name}}}
            self.test.check_default_product_name()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with product name {product_name} (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with product name {product_name} (override = {set_override})")

        asserts.assert_equal(self.skipped, 0, "Internal error: skip not reset properly")
        run_check('TEST_PRODUCT', set_override=False, expect_problem=True)
        run_check('TestBad', set_override=False, expect_problem=True)
        run_check('BadTest', set_override=False, expect_problem=True)
        run_check('TEST_PRODUCT', set_override=True, expect_problem=False)
        run_check('TestBad', set_override=True, expect_problem=False)
        run_check('BadTest', set_override=True, expect_problem=False)
        run_check('OK name', set_override=False, expect_problem=False)
        run_check('OK name', set_override=True, expect_problem=False)
        asserts.assert_equal(self.skipped, 4, "Some override tests did not mark steps skipped")

    def test_vendor_name_is_not_default_check(self):
        def run_check(vendor_name: str, set_override: bool, expect_problem: bool):
            self.test.user_params = {TC_DefaultWarnings.FLAG_VENDOR_NAME: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.BasicInformation: {Clusters.BasicInformation.Attributes.VendorName: vendor_name}}}
            self.test.check_default_vendor_name()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with vendor name {vendor_name} (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with vendor name {vendor_name} (override = {set_override})")

        asserts.assert_equal(self.skipped, 0, "Skip not reset properly")
        run_check('TEST_VENDOR', set_override=False, expect_problem=True)
        run_check('TestBad', set_override=False, expect_problem=True)
        run_check('BadTest', set_override=False, expect_problem=True)
        run_check('TEST_VENDOR', set_override=True, expect_problem=False)
        run_check('TestBad', set_override=True, expect_problem=False)
        run_check('BadTest', set_override=True, expect_problem=False)
        run_check('OK name', set_override=False, expect_problem=False)
        run_check('OK name', set_override=True, expect_problem=False)
        asserts.assert_equal(self.skipped, 4, "Some override tests did not mark steps skipped")

    def test_vendor_id_is_not_default_check(self):
        def run_check(vendor_id: int, set_override: bool, expect_problem: bool):
            self.test.user_params = {TC_DefaultWarnings.FLAG_VENDOR_ID: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.BasicInformation: {Clusters.BasicInformation.Attributes.VendorID: vendor_id}}}
            self.test.check_default_vendor_id()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with vendor id {vendor_id} (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with vendor id {vendor_id} (override = {set_override})")

        asserts.assert_equal(self.skipped, 0, "Skip not reset properly")
        run_check(0xFFF1, set_override=False, expect_problem=True)
        run_check(0xFFF2, set_override=False, expect_problem=True)
        run_check(0, set_override=False, expect_problem=True)
        run_check(0xFFF1, set_override=True, expect_problem=False)
        run_check(0xFFF2, set_override=True, expect_problem=False)
        run_check(0, set_override=True, expect_problem=False)
        run_check(0x6006, set_override=False, expect_problem=False)
        run_check(0x6006, set_override=True, expect_problem=False)
        asserts.assert_equal(self.skipped, 4, "Some override tests did not mark steps skipped")

    def test_calendar_is_not_default_check(self):
        enum = Clusters.TimeFormatLocalization.Enums.CalendarTypeEnum

        def run_check(calendar_type: Clusters.TimeFormatLocalization.Enums.CalendarTypeEnum, set_override: bool, expect_problem: bool):
            self.test.user_params = {TC_DefaultWarnings.FLAG_DEFAULT_CALENDAR_FORMAT: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.TimeFormatLocalization: {
                Clusters.TimeFormatLocalization.Attributes.ActiveCalendarType: calendar_type}}}
            self.test.check_default_calendar_format()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with calendar type {calendar_type} (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with calendar {calendar_type} (override = {set_override})")

        asserts.assert_equal(self.skipped, 0, "Skip not reset properly")
        run_check(enum.kBuddhist, set_override=False, expect_problem=True)
        run_check(enum.kChinese, set_override=False, expect_problem=False)
        run_check(enum.kBuddhist, set_override=True, expect_problem=False)
        run_check(enum.kChinese, set_override=True, expect_problem=False)
        asserts.assert_equal(self.skipped, 2, "Some override tests did not mark steps skipped")

        # Cluster not present
        self.skipped = 0
        self.test.problems = []
        self.test.user_params = {}
        self.test.endpoints = {0: {}}
        self.test.check_default_calendar_format()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no TimeFormatLocalization cluster")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")

        # Flag should also work here
        self.skipped = 0
        self.test.user_params = {TC_DefaultWarnings.FLAG_DEFAULT_CALENDAR_FORMAT: True}
        self.test.check_default_calendar_format()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no TimeFormatLocalization cluster and override")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")

        # Attribute not present
        self.skipped = 0
        self.test.problems = []
        self.test.user_params = {}
        self.test.endpoints = {0: {Clusters.TimeFormatLocalization: {}}}
        self.test.check_default_calendar_format()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no ActiveCalendarType attribute")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Override test did not mark steps skipped")

        # Flag should also work here
        self.skipped = 0
        self.test.user_params = {TC_DefaultWarnings.FLAG_DEFAULT_CALENDAR_FORMAT: True}
        self.test.check_default_calendar_format()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no ActiveCalendarType cluster and override")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Override test did not mark steps skipped")

    def _cluster_presence_test(self, cluster: Clusters.ClusterObjects.Cluster, fn: Callable, flag: str):

        def test_endpoint(endpoint: int):
            # no problem expected when cluster is not present
            self.test.problems = []
            self.test.user_params = {}

            self.test.endpoints = {endpoint: {Clusters.Descriptor: {Clusters.Descriptor.Attributes.ServerList: []}}}

            fn()
            asserts.assert_equal(len(self.test.problems), 0,
                                 f"Unexpected problem when testing for cluster presence of cluster {cluster} on endpoint {endpoint} (override = False)")

            # No problem when testing with override flag, but should get skip marked
            self.skipped = 0
            self.test.user_params = {flag: True}
            fn()
            asserts.assert_equal(len(self.test.problems), 0,
                                 f"Unexpected problem when testing for cluster presence of cluster {cluster} on endpoint {endpoint} (override = True)")
            asserts.assert_equal(self.skipped, 1, f"Test not marked skipped as expected (cluster = {cluster} flag = {flag})")

            # Should get a problem when the cluster is present
            self.test.user_params = {flag: False}
            self.test.endpoints = {endpoint: {cluster: {}, Clusters.Descriptor: {
                Clusters.Descriptor.Attributes.ServerList: [cluster.id]}}}
            fn()
            asserts.assert_equal(len(self.test.problems), 1,
                                 f"Did not get expected problem notification when testing with {cluster} on endpoint {endpoint}")

            self.test.user_params = {flag: True}
            self.skipped = 0
            self.test.problems = []
            fn()
            asserts.assert_equal(len(
                self.test.problems), 0, f"Unexpected problem notification when testing with {cluster} on endpoint {endpoint} and override {flag} set")
            asserts.assert_equal(self.skipped, 1, f"Test with override flag {flag} did not mark skipped")

        test_endpoint(0)
        test_endpoint(1)

    def test_unit_testing_is_not_present_check(self):
        self._cluster_presence_test(Clusters.UnitTesting, self.test.check_unit_testing_cluster_presence,
                                    TC_DefaultWarnings.FLAG_UNIT_TESTING)

    def test_fault_injection_is_not_present_check(self):
        self._cluster_presence_test(Clusters.FaultInjection, self.test.check_fault_injection_cluster_presence,
                                    TC_DefaultWarnings.FLAG_FAULT_INJECTION)

    def test_sample_mei_is_not_present_check(self):
        self._cluster_presence_test(Clusters.SampleMei, self.test.check_sample_mei_cluster_presence,
                                    TC_DefaultWarnings.FLAG_SAMPLE_MEI)

    def test_fixed_label_is_not_empty_check(self):
        def run_check(label_list: list[Clusters.FixedLabel.Structs.LabelStruct], set_override: bool, expect_problem: bool):
            self.test.user_params = {TC_DefaultWarnings.FLAG_FIXED_LABEL_EMPTY: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.FixedLabel: {Clusters.FixedLabel.Attributes.LabelList: label_list}}}
            self.test.check_fixed_label_cluster_empty()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with empty fixed label list (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with non-empty fixed label list (override = {set_override})")

        asserts.assert_equal(self.skipped, 0, "Skip not reset properly")
        run_check([], set_override=False, expect_problem=True)
        run_check([Clusters.FixedLabel.Structs.LabelStruct("test", "val")], set_override=False, expect_problem=False)
        run_check([], set_override=True, expect_problem=False)
        run_check([Clusters.FixedLabel.Structs.LabelStruct("test", "val")], set_override=True, expect_problem=False)
        asserts.assert_equal(self.skipped, 2, "Some override tests did not mark steps skipped")

        # Cluster not present
        self.skipped = 0
        self.test.problems = []
        self.test.user_params = {}
        self.test.endpoints = {0: {}}
        self.test.check_fixed_label_cluster_empty()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no FixedLabel cluster")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")

        # Flag should also work here
        self.skipped = 0
        self.test.user_params = {TC_DefaultWarnings.FLAG_FIXED_LABEL_EMPTY: True}
        self.test.check_fixed_label_cluster_empty()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no TimeFormatLocalization cluster and override")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")

    def test_fixed_label_is_not_default_check(self):
        def run_check(label_list: list[Clusters.FixedLabel.Structs.LabelStruct], set_override: bool, expect_problem: bool):
            self.skipped = 0
            self.test.user_params = {TC_DefaultWarnings.FLAG_FIXED_LABEL_DEFAULT_VALUES: set_override}
            self.test.problems = []
            self.test.endpoints = {0: {Clusters.FixedLabel: {Clusters.FixedLabel.Attributes.LabelList: label_list}}}
            self.test.check_fixed_label_cluster_defaults()
            if expect_problem:
                asserts.assert_equal(len(self.test.problems), 1,
                                     f"did not generate expected problem when testing with empty fixed label list (override = {set_override})")
            else:
                asserts.assert_equal(len(self.test.problems), 0,
                                     f"Unexpected problem when testing with non-empty fixed label list (override = {set_override})")
            if set_override:
                asserts.assert_equal(self.skipped, 1, "Test with override did not mark steps skipped")

        asserts.assert_equal(self.skipped, 0, "Skip not reset properly")
        for default_label in TC_DefaultWarnings.DEFAULT_FIXED_LABEL_VALUES:
            run_check([default_label], set_override=False, expect_problem=True)
            run_check([default_label], set_override=True, expect_problem=False)

        run_check(TC_DefaultWarnings.DEFAULT_FIXED_LABEL_VALUES, set_override=False, expect_problem=True)
        run_check(TC_DefaultWarnings.DEFAULT_FIXED_LABEL_VALUES, set_override=True, expect_problem=False)

        run_check([Clusters.FixedLabel.Structs.LabelStruct("test", "val")], set_override=False, expect_problem=False)
        run_check([Clusters.FixedLabel.Structs.LabelStruct("test", "val")], set_override=True, expect_problem=False)

        # Cluster not present
        self.skipped = 0
        self.test.problems = []
        self.test.user_params = {}
        self.test.endpoints = {0: {}}
        self.test.check_fixed_label_cluster_defaults()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no FixedLabel cluster")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")

        # Flag should also work here
        self.skipped = 0
        self.test.user_params = {TC_DefaultWarnings.FLAG_FIXED_LABEL_DEFAULT_VALUES: True}
        self.test.check_fixed_label_cluster_defaults()
        asserts.assert_equal(len(self.test.problems), 0,
                             "Unexpected problem when testing device with no FixedLabel cluster and override")
        # Should have marked this as skipped
        asserts.assert_equal(self.skipped, 1, "Some override tests did not mark steps skipped")


if __name__ == "__main__":
    default_matter_test_main()
