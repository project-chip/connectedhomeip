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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --int-arg PIXIT.BINFO.Finish:2
#       --int-arg PIXIT.BINFO.PrimaryColor:5
#
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_BINFO_3_1(MatterBaseTest):
    async def read_binfo_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BasicInformation
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BINFO_3_1(self) -> str:
        return "12.3.3. [TC-BINFO-3.1] Appearance Attribute DUT as Server"

    def steps_TC_BINFO_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH reads ProductAppearance from the DUT.",
                     ("Verify that TH reads ProductAppearance attribute from the DUT successfully "
                      "and the value is in ProductAppearanceStruct. "
                      "Finish value is in range 0-5.")),
            TestStep(2, "TH reads ProductAppearance from the DUT.",
                     ("Verify that TH reads ProductAppearance attribute from the DUT successfully "
                      "and the value is in ProductAppearanceStruct. "
                      "PrimaryColor is in range 0-20.")),
            TestStep(3, "TH reads ProductAppearance from the DUT.",
                     ("Vendor specific test-step. "
                      "Verify that TH reads ProductAppearance attribute "
                      "from the DUT successfully.")),
            TestStep(4, "TH reads ProductAppearance from the DUT.",
                     ("Vendor specific test-step. "
                      "Verify that TH reads ProductAppearance attribute "
                      "from the DUT successfully.")),
        ]
        return steps

    def pics_TC_BINFO_3_1(self) -> list[str]:
        pics = [
            "BINFO.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BINFO_3_1(self):

        endpoint = self.get_endpoint(default=0)
        attributes = Clusters.BasicInformation.Attributes

        # Read ProductAppearance once and reuse for all steps since the attribute value doesn't change
        productAppearance = await self.read_binfo_attribute_expect_success(
            endpoint=endpoint, attribute=attributes.ProductAppearance)
        asserts.assert_is_not_none(productAppearance, "ProductAppearance attribute read failed")

        self.step(1)
        asserts.assert_greater_equal(productAppearance.finish, 0, "Finish value is below minimum range")
        asserts.assert_less_equal(productAppearance.finish, 5, "Finish value is above maximum range")

        self.step(2)
        asserts.assert_greater_equal(productAppearance.primaryColor, 0, "PrimaryColor value is below minimum range")
        asserts.assert_less_equal(productAppearance.primaryColor, 20, "PrimaryColor value is above maximum range")

        self.step(3)
        # Vendor specific test: This step should verify the actual Finish value matches PIXIT.BINFO.Finish
        # For example, if PIXIT.BINFO.Finish is satin(2), the test should verify finish == 2
        asserts.assert_true("PIXIT.BINFO.Finish" in self.matter_test_config.global_test_params,
                            "PIXIT.BINFO.Finish must be included on the command line in "
                            "the --int-arg flag as PIXIT.BINFO.Finish:<finish_value>")
        pixit_finish = self.matter_test_config.global_test_params["PIXIT.BINFO.Finish"]
        asserts.assert_equal(productAppearance.finish, pixit_finish,
                             (f"ProductAppearance.Finish ({productAppearance.finish}) "
                              f"does not match PIXIT.BINFO.Finish ({pixit_finish})\n"
                              "Please verify the PIXIT.BINFO.Finish setting and try again."))

        self.step(4)
        # Vendor specific test: This step should verify the actual PrimaryColor value matches PIXIT.BINFO.PrimaryColor
        # For example, if PIXIT.BINFO.PrimaryColor is purple(5), the test should verify primaryColor == 5
        asserts.assert_true("PIXIT.BINFO.PrimaryColor" in self.matter_test_config.global_test_params,
                            "PIXIT.BINFO.PrimaryColor must be included on the command line in "
                            "the --int-arg flag as PIXIT.BINFO.PrimaryColor:<primary_color_value>")
        pixit_primary_color = self.matter_test_config.global_test_params["PIXIT.BINFO.PrimaryColor"]
        asserts.assert_equal(productAppearance.primaryColor, pixit_primary_color,
                             (f"ProductAppearance.PrimaryColor ({productAppearance.primaryColor})\n"
                              f"does not match PIXIT.BINFO.PrimaryColor ({pixit_primary_color})\n"
                              "Please verify the PIXIT.BINFO.PrimaryColor setting and try again."))


if __name__ == "__main__":
    default_matter_test_main()
