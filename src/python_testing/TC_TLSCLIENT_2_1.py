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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import test_plan_support
from mobly import asserts
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCLIENT_2_1(MatterBaseTest):
    def pics_TC_TLSCLIENT_2_1(self):
        return ["TLSCLIENT.S"]

    def desc_TC_TLSCLIENT_2_1(self) -> str:
        return "[TC-TLSCLIENT-2.1] Attributes with Server as DUT"

    def steps_TC_TLSCLIENT_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(2, "TH reads MaxProvisioned attribute", "DUT replies with an uint8 value between 5 and 254."),
            TestStep(3, "TH reads ProvisionedEndpoints attribute", "DUT replies with an empty list of TLSEndpointStruct."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_2_1(self):
        self.step(1)
        attributes = Clusters.TlsClientManagement.Attributes
        cr1_cmd = TLSUtils(self, endpoint=self.get_endpoint())

        self.step(2)
        max_provisioned = await cr1_cmd.read_tls_client_attribute(attributes.MaxProvisioned)
        asserts.assert_greater_equal(max_provisioned, 5, "MaxProvisioned should be >= 5")
        asserts.assert_less_equal(max_provisioned, 254, "MaxProvisioned should be <= 254")

        self.step(3)
        provisioned_endpoints = await cr1_cmd.read_tls_client_attribute(attributes.ProvisionedEndpoints)
        asserts.assert_equal(len(provisioned_endpoints), 0, "ProvisionedEndpoints should be empty")


if __name__ == "__main__":
    default_matter_test_main()
