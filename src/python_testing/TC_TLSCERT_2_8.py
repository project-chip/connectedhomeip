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
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_8(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_8(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_8(self) -> str:
        return "[TC-TLSCERT-2.8] ClientCSR command verification"

    def steps_TC_TLSCERT_2_8(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "CR2 reads MaxClientCertificates attribute into myMaxClientCerts."),
            TestStep(3, "Populate myNonce[] with myMaxClientCerts+1 distinct, random 32-octet values."),
            TestStep(4, "Set myBigNonce to a value exceeding 32 octets."),
            TestStep(5, "CR1 sends ClientCSR command with Nonce set to myBigNonce.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(6, "CR1 sends ClientCSR command with Nonce set to myNonce[0].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[0]."),
            TestStep(7, "CR2 sends ClientCSR command with Nonce set to myNonce[i], for each i in [1..myMaxClientCerts].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[i]."),
            TestStep(8, "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read.",
                     "DUT replies with a list of TLSClientCertificateDetailStruct with one entry for myCcdid[0]."),
            TestStep(9, "CR2 reads ProvisionedClientCertificates attribute using a fabric-filtered read.",
                     "DUT replies with a list of TLSClientCertificateDetailStruct with myMaxClientCerts entries."),
            TestStep(10, "CR2 sends ClientCSR command with Nonce set to myNonce[myMaxClientCerts].",
                     test_plan_support.verify_status(Status.ResourceExhausted)),
            TestStep(11, "CR2 sends RemoveClientCertificate command with CCDID set to myCcdid[1].",
                     test_plan_support.verify_success()),
            TestStep(12, "CR2 sends ClientCSR command with Nonce set to myNonce[myMaxClientCerts].",
                     "DUT replies with CCDID, CSR and Nonce."),
            TestStep(13, "CR2 reads ProvisionedClientCertificates attribute.",
                     "DUT replies with a list of TLSClientCertificateDetailStruct with myMaxClientCerts entries."),
            TestStep(14, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
            TestStep(15, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid[0].",
                     test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_8(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_max_client_certs = await cr2_cmd.read_tls_cert_attribute(attributes.MaxClientCertificates)

        self.step(3)
        my_nonce = [random.randbytes(32) for _ in range(my_max_client_certs + 1)]
        my_ccdid = [None] * (my_max_client_certs + 2)

        self.step(4)
        my_big_nonce = random.randbytes(33)

        self.step(5)
        await cr1_cmd.send_csr_command(nonce=my_big_nonce, expected_status=Status.ConstraintError)

        self.step(6)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce[0])
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        cr1_cmd.assert_valid_csr(response, my_nonce[0])
        my_ccdid[0] = response.ccdid

        self.step(7)
        for i in range(1, my_max_client_certs + 1):
            response = await cr2_cmd.send_csr_command(nonce=my_nonce[i])
            cr2_cmd.assert_valid_ccdid(response.ccdid)
            cr1_cmd.assert_valid_csr(response, my_nonce[i])
            my_ccdid[i] = response.ccdid

        self.step(8)
        client_certs = await cr1_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(client_certs), 1)
        asserts.assert_in(my_ccdid[0], client_certs)
        asserts.assert_equal(client_certs[my_ccdid[0]].clientCertificate, NullValue,
                             "Expected no certificate for unprovisioned certificate")
        asserts.assert_equal(len(client_certs[my_ccdid[0]].intermediateCertificates), 0,
                             "Expected no intermediate certificates for unprovisioned certificate")

        self.step(9)
        client_certs = await cr2_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(client_certs), my_max_client_certs)
        for i in range(1, my_max_client_certs + 1):
            asserts.assert_in(my_ccdid[i], client_certs)
            asserts.assert_equal(client_certs[my_ccdid[i]].clientCertificate, NullValue,
                                 "Expected no certificate for unprovisioned certificate")
            asserts.assert_equal(len(client_certs[my_ccdid[i]].intermediateCertificates), 0,
                                 "Expected no intermediate certificates for unprovisioned certificate")

        self.step(10)
        await cr2_cmd.send_csr_command(nonce=my_nonce[my_max_client_certs], expected_status=Status.ResourceExhausted)

        self.step(11)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid[1])

        self.step(12)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[my_max_client_certs])
        cr2_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[my_max_client_certs + 1] = response.ccdid
        cr1_cmd.assert_valid_csr(response, my_nonce[my_max_client_certs])

        self.step(13)
        client_certs = await cr2_cmd.read_client_certs_attribute_as_map()
        asserts.assert_equal(len(client_certs), my_max_client_certs)
        for i in range(2, my_max_client_certs + 2):
            asserts.assert_in(my_ccdid[i], client_certs)
            asserts.assert_equal(client_certs[my_ccdid[i]].clientCertificate, NullValue,
                                 "Expected no certificate for unprovisioned certificate")
            asserts.assert_equal(len(client_certs[my_ccdid[i]].intermediateCertificates), 0,
                                 "Expected no intermediate certificates for unprovisioned certificate")

        self.step(14)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

        self.step(15)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid[0])


if __name__ == "__main__":
    default_matter_test_main()
