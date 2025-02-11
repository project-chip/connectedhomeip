#
#    Copyright (c) 2023 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import chip.clusters as Clusters
import langcodes
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts


class Test_TC_LCFG_2_1(MatterBaseTest):
    def pics_TC_LCFG_2_1(self) -> list[str]:
        return ["LCFG.S"]

    def has_repeated_values(self, list):
        return len(list) != len(set(list))

    def list_has_maximum_length_of_35_bytes(self, list):
        return all(self.has_maximum_lenght_of_35_bytes(elem) for elem in list)

    def has_maximum_lenght_of_35_bytes(self, str):
        return len(str.encode('utf-8')) <= 35

    def supported_locales_has_active_locale(self, list, str):
        return str in list

    def steps_TC_LCFG_2_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT to TH", is_commissioning=True),
            TestStep(1, "TH reads SupportedLocales attribute from DUT", "Read is successful"),
            TestStep(2, "TH reads ActiveLocale attribute from the DUT", "Read is successful"),
            TestStep(3, "TH writes new string not present in SupportedLocales attribute to ActiveLocale attribute"),
            TestStep(4, "TH writes new string present in SupportedLocales attribute to ActiveALocale attribute"),
            TestStep(5, "TH reads ActiveLocale")
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.LocalizationConfiguration))
    async def test_TC_LCFG_2_1(self):

        endpoint = self.get_endpoint(default=0)
        value_not_present_in_supported_locales = "fw-GB"
        es_ES = "es-ES"

        # Step 0: Commissioning DUT (already done)
        self.step(0)

        # Step 1: TH reads SupportedLocales attribute from DUT
        self.step(1)

        initial_values_supported_locales = await self.read_single_attribute_check_success(
            cluster=Clusters.LocalizationConfiguration,
            attribute=Clusters.LocalizationConfiguration.Attributes.SupportedLocales,
            endpoint=endpoint
        )

        # Verify values in SupportedLocales attribute are not repeated
        asserts.assert_false(self.has_repeated_values(initial_values_supported_locales),
                             "SupportedLocales attribute has no repeated values")

        # Verify maximun number of elements in the SupportedLocales list is 32
        asserts.assert_true(len(initial_values_supported_locales) <= 32, "SupportedLocales attribute has less than 32 elements")

        # Verify values of SupportedLocales attribute has a maximum lenght of 35 bytes
        asserts.assert_true(self.list_has_maximum_length_of_35_bytes(initial_values_supported_locales),
                            "SupportedLocales attribute has a maximum lenght of 35 bytes")

        # Step 2: TH reads ActiveLocale attribute from the DUT
        self.step(2)

        initial_active_locale = await self.read_single_attribute_check_success(
            cluster=Clusters.LocalizationConfiguration,
            attribute=Clusters.LocalizationConfiguration.Attributes.ActiveLocale,
            endpoint=endpoint
        )

        # Verify that the ActiveLocale attribute is not empty
        asserts.assert_true(bool(initial_active_locale), "ActiveLocale attribute is not empty")

        # Verify that the ActiveLocale attribute is Language Tag as defined by BCP47
        asserts.assert_true(langcodes.tag_is_valid(initial_active_locale),
                            "ActiveLocale attribute is Language Tag as defined by BCP47")

        # Verify that the value of ActiveLocale attribute has maximum lenght of 35 bytes
        asserts.assert_true(self.has_maximum_lenght_of_35_bytes(initial_active_locale),
                            "ActiveLocale attribute has less than 35 bytes")

        # Verify that the ActiveLocale attribute value is present in the SupportedLocales attribute list
        asserts.assert_true(self.supported_locales_has_active_locale(initial_values_supported_locales,
                            initial_active_locale), "ActiveLocale attribute value is present in the SupportedLocales attribute list")

        # Step 3: TH writes new string not present in SupportedLocales attribute to ActiveLocale attribute
        self.step(3)

        result = await self.write_single_attribute(attribute_value=Clusters.LocalizationConfiguration.Attributes.ActiveLocale(value_not_present_in_supported_locales), endpoint_id=endpoint, expect_success=False)

        # DUT responds with CONTRAINT_ERROR
        asserts.assert_equal(result, Status.ConstraintError, "Trying to write an invalid value should return ConstraintError")

        # Step 4: TH writes new string present in SupportedLocales attribute to ActiveALocale attribute
        self.step(4)

        value_present_in_supported_locales = initial_values_supported_locales[initial_values_supported_locales.index(es_ES)]

        result = await self.write_single_attribute(attribute_value=Clusters.LocalizationConfiguration.Attributes.ActiveLocale(value_present_in_supported_locales), endpoint_id=endpoint, expect_success=True)

        # Verify that the DUT send the Success response
        asserts.assert_equal(result, Status.Success, "Trying to write an valid value should return Success")

        # Step 5: TH reads ActiveLocale
        self.step(5)

        active_locale = await self.read_single_attribute_check_success(
            cluster=Clusters.LocalizationConfiguration,
            attribute=Clusters.LocalizationConfiguration.Attributes.ActiveLocale,
            endpoint=endpoint
        )

        # Verify that the value of the ActiveLocale attribute is changed to SupportedLocales
        asserts.assert_not_equal(initial_active_locale, active_locale, "ActiveLocale attribute should change")


if __name__ == "__main__":
    default_matter_test_main()
