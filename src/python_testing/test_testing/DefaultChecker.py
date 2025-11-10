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

from typing import Optional

import matter.clusters as Clusters
from matter.testing.basic_composition import BasicCompositionTests
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.problem_notices import (AttributePathLocation, ClusterPathLocation, ProblemLocation, ProblemNotice,
                                            ProblemSeverity)

FLAG_PRODUCT_NAME = "pixit_allow_test_in_product_name"
FLAG_VENDOR_NAME = "pixit_allow_test_in_vendor_name"
FLAG_VENDOR_ID = "pixit_allow_default_vendor_id"
FLAG_DEFAULT_CALENDAR_FORMAT = "pixit_allow_default_calendar_format"
FLAG_UNIT_TESTING = "pixit_allow_unit_testing_cluster"
FLAG_FAULT_INJECTION = "pixit_allow_fault_injection_cluster"
FLAG_SAMPLE_MEI = "pixit_allow_sample_mei_cluster"
FLAG_FIXED_LABEL_EMPTY = "pixit_allow_empty_fixed_label_list"
FLAG_FIXED_LABEL_DEFAULT_VALUES = "pixit_allow_fixed_label_default_values"


DEFAULT_FIXED_LABEL_VALUES = [Clusters.FixedLabel.Structs.LabelStruct(label='room', value='bedroom 2'),
                              Clusters.FixedLabel.Structs.LabelStruct(label='orientation', value='North'),
                              Clusters.FixedLabel.Structs.LabelStruct(label='floor', value='2'),
                              Clusters.FixedLabel.Structs.LabelStruct(label='direction', value='up')]


def _problem(location: ProblemLocation, msg: str):
    return ProblemNotice("Default Value Checker", location, ProblemSeverity.ERROR, msg)


def warning_wrapper(override_flag: str):
    def warning_wrapper_internal(body):
        def runner(self: MatterBaseTest):
            skip = self.user_params.get(override_flag, False)
            if not skip:
                problem: ProblemNotice = body(self)
                if problem:
                    problem.problem += f"\nIf this is intended, you may disable this check using the {override_flag}"
                    self.problems.append(problem)
            else:
                self.mark_current_step_skipped()
        return runner
    return warning_wrapper_internal


class DefaultChecker():
    @warning_wrapper(FLAG_PRODUCT_NAME)
    def check_default_product_name(self):
        cluster = Clusters.BasicInformation
        attr = cluster.Attributes.ProductName
        val = self.endpoints[0][cluster][attr]
        if "test" in val.lower():
            return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), f"Product name contains test ({val})")

    @warning_wrapper(FLAG_VENDOR_NAME)
    def check_default_vendor_name(self):
        cluster = Clusters.BasicInformation
        attr = cluster.Attributes.VendorName
        val = self.endpoints[0][cluster][attr]
        if "test" in val.lower():
            return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), f"Vendor name contains test ({val})")

    @warning_wrapper(FLAG_VENDOR_ID)
    def check_default_vendor_id(self):
        cluster = Clusters.BasicInformation
        attr = cluster.Attributes.VendorID
        val = self.endpoints[0][cluster][attr]
        if val == 0x0000 or val > 0xFFF0:
            return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), f"Vendor is not in manufacturer code range ({val})")

    @warning_wrapper(FLAG_DEFAULT_CALENDAR_FORMAT)
    def check_default_calendar_format(self):
        cluster = Clusters.TimeFormatLocalization
        attr = cluster.Attributes.ActiveCalendarType
        if cluster in self.endpoints[0].keys() and attr in self.endpoints[0][cluster].keys():
            val = self.endpoints[0][cluster][attr]
            if val == cluster.Enums.CalendarTypeEnum.kBuddhist:
                return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), "Calendar format is set to default (Buddhist)")
        else:
            self.mark_current_step_skipped()

    def _check_testing_cluster_presence(self, cluster: Clusters.ClusterObjects.Cluster) -> Optional[ProblemNotice]:
        for endpoint_num, endpoint in self.endpoints.items():
            if cluster.id in endpoint[Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]:
                return _problem(ClusterPathLocation(endpoint_num, cluster.id), f"{cluster.__name__} cluster found on device.")

    @warning_wrapper(FLAG_UNIT_TESTING)
    def check_unit_testing_cluster_presence(self):
        return self._check_testing_cluster_presence(Clusters.UnitTesting)

    @warning_wrapper(FLAG_FAULT_INJECTION)
    def check_fault_injection_cluster_presence(self):
        return self._check_testing_cluster_presence(Clusters.FaultInjection)

    @warning_wrapper(FLAG_SAMPLE_MEI)
    def check_sample_mei_cluster_presence(self):
        return self._check_testing_cluster_presence(Clusters.SampleMei)

    @warning_wrapper(FLAG_FIXED_LABEL_EMPTY)
    def check_fixed_label_cluster_empty(self):
        cluster = Clusters.FixedLabel
        attr = cluster.Attributes.LabelList
        if cluster in self.endpoints[0].keys():
            val = self.endpoints[0][cluster][attr]
            if val == []:
                return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), "Fixed label list is empty")
        else:
            self.mark_current_step_skipped()

    @warning_wrapper(FLAG_FIXED_LABEL_DEFAULT_VALUES)
    def check_fixed_label_cluster_defaults(self):
        cluster = Clusters.FixedLabel
        attr = cluster.Attributes.LabelList
        if cluster in self.endpoints[0].keys():
            label_list = self.endpoints[0][cluster][attr]

            if any([label for label in label_list if label in DEFAULT_FIXED_LABEL_VALUES]):
                return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), f"Fixed label list contains default labels:\ndefaults {DEFAULT_FIXED_LABEL_VALUES}\nlist={label_list}")
        else:
            self.mark_current_step_skipped()
