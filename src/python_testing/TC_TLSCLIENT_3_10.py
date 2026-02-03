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
from TC_TLSCLIENT_Base import TC_TLSCLIENT_Base

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCLIENT_3_10(TC_TLSCLIENT_Base):
    def pics_TC_TLSCLIENT_3_10(self):
        return ["TLSCLIENT.S", "TLSCERT.S"]

    def desc_TC_TLSCLIENT_3_10(self) -> str:
        return "[TC-TLSCLIENT-3.10] FindEndpoint command verification"

    def steps_TC_TLSCLIENT_3_10(self) -> list[TestStep]:
        return [
            *self.get_common_steps(),
            TestStep(4, "Set myHostname to a valid value."),
            TestStep(5, "Set myPort to a valid port value."),
            TestStep(6, "Set myRootCert to a valid value."),
            TestStep(7, "CR1 sends ProvisionRootCertificate command to the TLSCertificateManagementCluster with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(8, "Populate myNonce with a distinct, random 32-octet values"),
            TestStep(9, "CR1 sends ClientCSR command with Nonce set to myNonce",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid and CSR in myCsr."),
            TestStep(10,
                     "Populate myClientCert with a distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr."),
            TestStep(11, "CR1 sends ProvisionClientCertificate command to the TLSCertificateManagementCluster with CCDID set to myCcdid and ClientCertificateDetails set to myClientCert.",
                     test_plan_support.verify_success()),
            TestStep(12, "CR1 sends FindEndpoint command with EndpointID 0.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(13, "CR1 sends ProvisionEndpoint command with valid Hostname myHostname, Port myPort, CAID myCaid, CCDID myCcdid and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint."),
            TestStep(14, "CR1 sends FindEndpoint command with EndpointID set to myEndpoint.",
                     "DUT replies with a TLSEndpointStruct with value (myEndpoint, myHostname, myPort, myCaid, myCcdid, myReferenceCount), where myReferenceCount is 0."),
            TestStep(15, "CR1 sends FindEndpoint command with EndpointID set to myEndpoint + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(16, "CR2 sends FindEndpoint command with EndpointID set to myEndpoint.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(
                17, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint.", test_plan_support.verify_success()),
            TestStep(18, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsClientManagement))
    async def test_TC_TLSCLIENT_3_10(self):
        setup_data = await self.common_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(4)
        my_hostname = b"myhostname.matter.com"

        self.step(5)
        my_port = 1234

        self.step(6)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(7)
        res = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        my_caid = res.caid

        self.step(8)
        my_nonce = random.randbytes(32)

        self.step(9)
        res = await cr1_cmd.send_csr_command(nonce=my_nonce)
        my_ccdid = res.ccdid
        csr = cr1_cmd.assert_valid_csr(res, my_nonce)

        self.step(10)
        my_client_cert = cr1_cmd.gen_cert_with_key(
            cr1_cmd.get_key(), public_key=csr.public_key(), subject=csr.subject)

        self.step(11)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert)

        self.step(12)
        await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=0, expected_status=Status.NotFound)

        self.step(13)
        res = await cr1_cmd.send_provision_tls_endpoint_command(hostname=my_hostname, port=my_port, caid=my_caid, ccdid=my_ccdid)
        asserts.assert_is_not_none(res.endpointID)
        my_endpoint = res.endpointID

        self.step(14)
        res = await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint)
        asserts.assert_equal(res.endpoint.endpointID, my_endpoint)
        asserts.assert_equal(res.endpoint.hostname, my_hostname)
        asserts.assert_equal(res.endpoint.port, my_port)
        asserts.assert_equal(res.endpoint.caid, my_caid)
        asserts.assert_equal(res.endpoint.ccdid, my_ccdid)
        asserts.assert_equal(res.endpoint.referenceCount, 0)

        self.step(15)
        await cr1_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint + 1, expected_status=Status.NotFound)

        self.step(16)
        await cr2_cmd.send_find_tls_endpoint_command(endpoint_id=my_endpoint, expected_status=Status.NotFound)

        self.step(17)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint)

        self.step(18)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
