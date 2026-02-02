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
from TC_TLSCERT_Base import TC_TLSCERT_Base

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_13(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_13(self):
        return ["TLSCERT.S", "TLSCLIENT.S"]

    def desc_TC_TLSCERT_2_13(self) -> str:
        return "[TC-TLSCERT-2.13] RemoveClientCertificate command verification"

    def steps_TC_TLSCERT_2_13(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myNonce to a random 32-octet value"),
            TestStep(3, "Set myRootCert to a valid, self-signed, DER-encoded x509 certificate"),
            TestStep(4, "CR1 sends RemoveClientCertificate command with CCDID set to 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(5, "CR1 sends ClientCSR command with Nonce set to myNonce.",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid."),
            TestStep(6, "Populate myClientCert with a valid, self-signed, DER-encoded x509 certificate using the public key from the CSR."),
            TestStep(7, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificate set to myClientCert.",
                     test_plan_support.verify_success()),
            TestStep(8, "CR2 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(9, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(10, "CR1 sends ProvisionEndpoint command with valid Hostname, Port, CAID myCaid and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint."),
            TestStep(11, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.InvalidInState)),
            TestStep(12, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint.",
                     test_plan_support.verify_success()),
            TestStep(13, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_success()),
            TestStep(14, "CR1 sends FindClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(15, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(16, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_success()),
            TestStep(17, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_13(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = random.randbytes(32)

        self.step(3)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(4)
        await cr1_cmd.send_remove_client_command(ccdid=1, expected_status=Status.NotFound)

        self.step(5)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce)
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid = response.ccdid
        my_csr = cr1_cmd.assert_valid_csr(response, my_nonce)

        self.step(6)
        root = cr1_cmd.get_key()
        my_client_cert = cr1_cmd.gen_cert_with_key(root, public_key=my_csr.public_key(), subject=my_csr.subject)

        self.step(7)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert)

        self.step(8)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(9)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        cr1_cmd.assert_valid_caid(response.caid)
        my_caid = response.caid

        self.step(10)
        endpoint_response = await cr1_cmd.send_provision_tls_endpoint_command(hostname=b"my_hostname", port=1000, caid=my_caid, ccdid=my_ccdid)
        my_endpoint = endpoint_response.endpointID

        self.step(11)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.InvalidInState)

        self.step(12)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint)

        self.step(13)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid)

        self.step(14)
        await cr1_cmd.send_find_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(15)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(16)
        await cr1_cmd.send_remove_root_command(caid=my_caid)

        self.step(17)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
