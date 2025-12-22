#
#   Copyright (c) 2025 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --trace-to json:${TRACE_APP}.json
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
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_CGEN_2_1(MatterBaseTest):
    def desc_TC_CGEN_2_1(self) -> str:
        return "[TC-CGEN-2.1] Breadcrumb, BasicCommissioningInfo, RegulatoryConfig, LocationCapability and SupportsConcurrentConnection attributes [{DUT_Server}]"

    def pics_TC_CGEN_2_1(self):
        return ["CGEN.S"]

    def steps_TC_CGEN_2_1(self):
        return [
            TestStep("1", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep("2", "TH reads the Breadcrumb attribute"),
            TestStep("3", "TH writes Breadcrumb = 1"),
            TestStep("4", "TH reads Breadcrumb again and verifies = 1"),
            TestStep("5", "TH reads the RegulatoryConfig attribute"),
            TestStep("6", "TH reads the LocationCapability attribute"),
            TestStep("7", "TH reads the BasicCommissioningInfo attribute"),
            TestStep("8", "TH reads the SupportsConcurrentConnection attribute"),
        ]

    @async_test_body
    async def test_TC_CGEN_2_1(self):
        self.endpoint = self.get_endpoint()
        cluster = Clusters.GeneralCommissioning
        attributes = cluster.Attributes

        self.step("1")

        self.step("2")
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.Breadcrumb)
        matter_asserts.assert_valid_uint64(breadcrumb, "Breadcrumb must be uint64")
        log.info(f"Breadcrumb initial value: {breadcrumb}")

        self.step("3")
        await self.write_single_attribute(attributes.Breadcrumb(1), expect_success=True)
        log.info("Breadcrumb set to 1")

        self.step("4")
        breadcrumb_val = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.Breadcrumb)
        matter_asserts.assert_valid_uint64(breadcrumb_val, "Breadcrumb must be uint64 after write")
        asserts.assert_equal(breadcrumb_val, 1, "Breadcrumb should be 1 after write")

        self.step("5")
        reg_cfg = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.RegulatoryConfig)
        matter_asserts.assert_valid_enum(
            reg_cfg, "RegulatoryConfig must be a valid RegulatoryLocationTypeEnum",
            Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum)
        log.info(f"RegulatoryConfig value: {Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum(reg_cfg).name}")

        self.step("6")
        loc_cap = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.LocationCapability)
        matter_asserts.assert_valid_enum(
            loc_cap, "LocationCapability must be a valid RegulatoryLocationTypeEnum",
            Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum)
        log.info(f"LocationCapability value: {Clusters.GeneralCommissioning.Enums.RegulatoryLocationTypeEnum(loc_cap).name}")

        self.step("7")
        basic_info = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.BasicCommissioningInfo)
        log.info(f"BasicCommissioningInfo: {basic_info}")

        failsafe = basic_info.failSafeExpiryLengthSeconds
        max_cumulative = basic_info.maxCumulativeFailsafeSeconds
        matter_asserts.assert_valid_uint16(failsafe, "failSafeExpiryLengthSeconds must be uint16")
        matter_asserts.assert_valid_uint16(max_cumulative, "maxCumulativeFailsafeSeconds must be uint16")
        asserts.assert_greater(failsafe, 0, "failSafeExpiryLengthSeconds must be greater than 0")
        asserts.assert_greater(max_cumulative, 0, "maxCumulativeFailsafeSeconds must be greater than 0")

        self.step("8")
        supports_con = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=attributes.SupportsConcurrentConnection)
        asserts.assert_is_instance(supports_con, bool, "SupportsConcurrentConnection must be boolean")


if __name__ == "__main__":
    default_matter_test_main()
