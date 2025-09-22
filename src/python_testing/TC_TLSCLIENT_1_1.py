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
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_TLSCLIENT_1_1(MatterBaseTest):
    def desc_TC_TLSCLIENT_1_1(self) -> str:
        return "[TC-TLSCLIENT-1.1] Primary functionality with DUT as Server"

    def steps_TC_TLSCLIENT_1_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Sends the ProvisionEndpoint command to the TlsClientNamanement cluster",
                     "Verify that the DUT sends ProvisionEndpointResponse."),
        ]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_1_1(self):

        endpoint = self.get_endpoint(default=1)
        util = TLSUtils(self, endpoint=endpoint)

        self.step(1)

        self.step(2)
        e = await util.send_provision_tls_endpoint_command(expected_status=Status.Failure, hostname=b"my_hostname", port=1000, caid=10)
        asserts.assert_equal(
            e.clusterStatus, Clusters.TlsClientManagement.Enums.StatusCodeEnum.kRootCertificateNotFound, "Unexpected error returned")


if __name__ == "__main__":
    default_matter_test_main()
