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

from mobly import asserts
from TC_TLSCERT_Base import TC_TLSCERT_Base

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_1(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_1(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_1(self) -> str:
        return "[TC-TLSCERT-2.1] Attributes with Server as DUT"

    def steps_TC_TLSCERT_2_1(self) -> list[TestStep]:
        return [
            *self.get_common_substeps(),
            TestStep(2, "TH reads MaxRootCertificates attribute", "DUT replies with an uint8 value between 5 and 254."),
            TestStep(3, "TH reads ProvisionedRootCertificates attribute", "DUT replies with an empty list of TLSCertStruct."),
            TestStep(4, "TH reads MaxClientCertificates attribute", "DUT replies with an uint8 value between 2 and 254."),
            TestStep(5, "TH reads ProvisionedClientCertificates attribute",
                     "DUT replies with an empty list of TLSClientCertificateDetailStruct."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_1(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        cr1_cmd = await self.common_setup()

        self.step(2)
        max_root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.MaxRootCertificates)
        asserts.assert_greater_equal(max_root_certs, 5, "MaxRootCertificates should be >= 5")
        asserts.assert_less_equal(max_root_certs, 254, "MaxRootCertificates should be <= 254")

        self.step(3)
        provisioned_root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        asserts.assert_equal(len(provisioned_root_certs), 0, "ProvisionedRootCertificates should be empty")

        self.step(4)
        max_client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.MaxClientCertificates)
        asserts.assert_greater_equal(max_client_certs, 2, "MaxClientCertificates should be >= 2")
        asserts.assert_less_equal(max_client_certs, 254, "MaxClientCertificates should be <= 254")

        self.step(5)
        provisioned_client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedClientCertificates)
        asserts.assert_equal(len(provisioned_client_certs), 0, "ProvisionedClientCertificates should be empty")


if __name__ == "__main__":
    default_matter_test_main()
