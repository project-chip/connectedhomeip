# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LOCK_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --bool-arg PIXIT.AllowTestInProductName:True PIXIT.AllowTestInVendorName:True PIXIT.AllowDefaultVendorId:True
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

from typing import Optional

import chip.clusters as Clusters
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.matter_testing import (AttributePathLocation, ClusterPathLocation, MatterBaseTest, ProblemLocation, ProblemNotice, ProblemSeverity, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts


FLAG_PRODUCT_NAME = "PIXIT.AllowTestInProductName"
FLAG_VENDOR_NAME = "PIXIT.AllowTestInVendorName"
FLAG_VENDOR_ID = "PIXIT.AllowDefaultVendorId"
FLAG_DEFAULT_CALENDAR_FORMAT = "PIXIT.AllowDefaultCalendarFormat"
FLAG_UNIT_TESTING = "PIXIT.AllowUnitTestingCluster"
FLAG_FAULT_INJECTION = "PIXIT.AllowFaultInjectionCluster"
FLAG_SAMPLE_MEI = "PIXIT.AllowSampleMeiCluster"


def _problem(location: ProblemLocation, msg: str):
    return ProblemNotice("Default Value Checker", location, ProblemSeverity.ERROR, msg)


def warning_wrapper(override_flag: str):
    def warning_wrapper_internal(body):
        def runner(self: MatterBaseTest):
            skip = self.user_params.get(override_flag.lower(), False)
            if not skip:
                problem: ProblemNotice = body(self)
                if problem:
                    problem.problem += f"\nIf this is intended, you may disable this check using the {override_flag}"
                    self.problems.append(problem)
            else:
                self.mark_current_step_skipped()
        return runner
    return warning_wrapper_internal


class DefaultChecker(BasicCompositionTests):
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
                return _problem(AttributePathLocation(0, cluster.id, attr.attribute_id), f"Calendar format is set to default (Buddhist)")
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


class TC_DefaultChecker(MatterBaseTest, DefaultChecker):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        await self.setup_class_helper()

    @async_test_body
    async def setup_test(self):
        super().setup_test()
        self.success = True

    def steps_TC_SOMETHING_1_1(self):
        steps = [TestStep(1, "TH performs a wildcard read of all attributes"),
                 TestStep(2, f"If the {FLAG_PRODUCT_NAME} flag is not set, check for \"Test\" in the product name",
                          "\"Test\" does not appear in the product name"),
                 TestStep(3, f"If the {FLAG_VENDOR_NAME} flag is not set, check for \"Test\" in the vendor name",
                          "\"Test\" does not appear in the vendor name"),
                 TestStep(4, f"If the {FLAG_VENDOR_ID} flag is not set, check for test vendor IDs",
                          "Product does not use a test vendor ID"),
                 TestStep(5, f"If the {FLAG_DEFAULT_CALENDAR_FORMAT} flag is not set, and the TimeFormatLocalization cluster is present and has the ActiveCalendarType attribute, check for the default calendar format", "Calendar format is not the default"),
                 TestStep(6, f"If the {FLAG_UNIT_TESTING} flag is not set, check for the presence of a unit testing cluster on any endpoint",
                          "Unit testing cluster does not appear on any endpoint"),
                 TestStep(7, f"If the {FLAG_FAULT_INJECTION} flag is not set, check for the presence of a fault injection cluster on any endpoint",
                          "Fault injection cluster does not appear on any endpoint"),
                 TestStep(8, f"If the {FLAG_SAMPLE_MEI} flag is not set, check for the presence of a sample mei cluster on any endpoint",
                          "Sample MEI cluster does not appear on any endpoint"),
                 TestStep(9, "Fail on any problems")
                 ]
        return steps

    def desc_TC_SOMETHING_1_1(self):
        return "[TC-SOMETHING-1.1] Check for accidental defaults - [DUT as Server]"

    def test_TC_SOMETHING_1_1(self):
        print(self.user_params)
        self.step(1)
        self.step(2)
        self.check_default_product_name()
        self.step(3)
        self.check_default_vendor_name()
        self.step(4)
        self.check_default_vendor_id()
        self.step(5)
        self.check_default_calendar_format()
        self.step(6)
        self.check_unit_testing_cluster_presence()
        self.step(7)
        self.check_fault_injection_cluster_presence()
        self.step(8)
        self.check_sample_mei_cluster_presence()
        self.step(9)

        if self.problems:
            asserts.fail("One or more default values found on device")


if __name__ == "__main__":
    default_matter_test_main()
