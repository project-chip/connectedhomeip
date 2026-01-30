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

import random

import test_plan_support
from mobly import asserts
from TC_TLSCERT_Base import TC_TLSCERT_Base

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_11(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_11(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_11(self) -> str:
        return "[TC-TLSCERT-2.11] FindClientCertificate command verification"

    def steps_TC_TLSCERT_2_11(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myNonce to a random 32-octet value"),
            TestStep(3, "CR1 sends FindClientCertificate command with null CCDID.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(4, "CR1 sends ClientCSR command with Nonce set to myNonce.",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid."),
            TestStep(5, "CR1 sends FindClientCertificate command with CCDID set to myCcdid.",
                     "DUT replies with a list of TLSClientCertificateDetailsStruct with one entry. The entry should have CCDID with value myCcdid with ClientCertificate and IntermediateCertificates unset."),
            TestStep(6, "CR1 sends FindClientCertificate command with CCDID set to myCcdid + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(7, "CR2 sends FindClientCertificate command with null CCDID.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(8, "CR2 sends FindClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(9, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_success()),
            TestStep(10, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_11(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = random.randbytes(32)

        self.step(3)
        await cr1_cmd.send_find_client_command(expected_status=Status.NotFound)

        self.step(4)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce)
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        cr1_cmd.assert_valid_csr(response, my_nonce)
        my_ccdid = response.ccdid

        self.step(5)
        find_response = await cr1_cmd.send_find_client_command(ccdid=my_ccdid)
        asserts.assert_equal(len(find_response.certificateDetails), 1)
        asserts.assert_equal(find_response.certificateDetails[0].ccdid, my_ccdid)
        asserts.assert_equal(find_response.certificateDetails[0].clientCertificate, NullValue)
        asserts.assert_equal(len(find_response.certificateDetails[0].intermediateCertificates), 0)

        self.step(6)
        await cr1_cmd.send_find_client_command(ccdid=my_ccdid + 1, expected_status=Status.NotFound)

        self.step(7)
        await cr2_cmd.send_find_client_command(expected_status=Status.NotFound)

        self.step(8)
        await cr2_cmd.send_find_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(9)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid)

        self.step(10)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
