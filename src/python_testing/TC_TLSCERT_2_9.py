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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_9(TC_TLSCERT_Base):
    def desc_TC_TLSCERT_2_9(self) -> str:
        return "[TC-TLSCERT-2.9] ProvisionClientCertificate command verification"

    def pics_TC_TLSCERT_2_9(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["TLSCERT.S"]

    def steps_TC_TLSCERT_2_9(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate my_nonce[] with 4 distinct, random 32-octet values"),
            TestStep(3, "CR1 sends ClientCSR command with Nonce set to my_nonce[i], for each i in [0..1]",
                     "Verify the fields CCDID, CSR and Nonce with types TLSCCDID, octstr and octstr respectively. Store TLSCCDID in my_ccdid[i] and CSR in my_csr[i]."),
            TestStep(4, "CR2 sends sends ClientCSR command with Nonce set to my_nonce[2]",
                     "Verify the fields CCDID, CSR and Nonce with types TLSCCDID, octstr and octstr respectively. Store TLSCCDID in my_ccdid[2] and CSR in my_csr[2]."),
            TestStep(5, "Populate my_intermediate_certs_1 with 10 DER-encoded x509 certificates that form a certificate chain up to (but not including) a root"),
            TestStep(6, "Populate my_intermediate_certs_2 with 1 DER-encoded x509 certificates that form a certificate chain up to (but not including) a root"),
            TestStep(7, "Populate my_client_cert[] with 3 distinct, valid, DER-encoded x509 certificates using each respective public key from my_csr[i], signed by signed by [a root, my_intermediate_certs_1[0], my_intermediate_certs_2[0]]"),
            TestStep(
                8, "CR1 sends ProvisionClientCertificate command with CCDID set to my_ccdid[0] and ClientCertificate set to my_client_cert[0]", test_plan_support.verify_success()),
            TestStep(
                9, "CR1 sends ProvisionClientCertificate command with CCDID set to my_ccdid[1], ClientCertificate set to my_client_cert[1], and IntermediateCertificates set to my_intermediate_certs_1", test_plan_support.verify_success()),
            TestStep(
                10, "CR2 sends ProvisionClientCertificate command with CCDID set to my_ccdid[2], ClientCertificate set to my_client_cert[2], and IntermediateCertificates set to my_intermediate_certs_2", test_plan_support.verify_success()),
            TestStep(11, "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify a list of TLSClientCertificateDetailStruct with two entries corresponding to my_ccdid[0..1]. The ClientCertificate value should be set to my_client_cert[0..1] and IntermediateCertificates set to my_intermediate_certs_1 for my my_ccdid[1]."),
            TestStep(12, "CR2 reads ProvisionedClientCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify a list of TLSClientCertificateDetailStruct with one entry corresponding to my_ccdid[2]. The ClientCertificate value should be set to my_client_cert[2] and IntermediateCertificates set to my_intermediate_certs_2."),
            TestStep(13, "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read",
                     "Verify a list of TLSClientCertificateDetailStruct with two entries corresponding to my_ccdid[0..1]. The ClientCertificate and IntermediateCertificates values should not be populated (non-present)."),
            TestStep(14, "CR1 sends FindClientCertificate command with null CCDID",
                     "Verify a list of TLSClientCertificateDetailStruct with two entries. The entries should correspond to my_client_cert[0..1]"),
            TestStep(15, "CR2 sends FindClientCertificate command with null CCDID",
                     "Verify a  list of TLSClientCertificateDetailStruct with one entry. The entry should correspond to my_client_cert[2]"),
            TestStep(16, "CR1 sends ClientCSR command with CCDID set to my_ccdid[0] and Nonce set to my_nonce[3]",
                     "the fields CCDID, CSR and Nonce with types TLSCCDID, octstr and octstr respectively. CCDID should equal my_ccdid[0]. The public key of the resulting CSR should be equal to the public key in my_csr[0]. NonceSignature should be a signature of my_nonce[3] using public key in CSR"),
            TestStep(
                17, "CR1 sends ProvisionClientCertificate command with CCDID set to my_ccdid[0] and ClientCertificateDetails set to my_client_cert[3]", test_plan_support.verify_success()),
            TestStep(18, "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify a list of TLSClientCertificateDetailStruct with two entries. The entries should correspond to my_client_cert[0,3]"),
            TestStep(19, "CR2 reads ProvisionedClientCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify a list of TLSClientCertificateDetailStruct with one entry. The entry should correspond to my_client_cert[2]"),
            TestStep(20, "CR1 sends FindClientCertificate command with CCDID set to my_ccdid[0]",
                     "Verify a list of TLSClientCertificateDetailStruct with one entry. The entry should correspond to my_client_cert[3]"),
            TestStep(21, "CR2 sends FindClientCertificate command with CCDID set to my_ccdid[2]",
                     "Verify a list of TLSClientCertificateDetailStruct with one entry. The entry should correspond to my_client_cert[2]"),
            TestStep(22, "CR1 sends RemoveClientCertificate command with CCDID set to my_ccdid[i], for each i in [0..1]",
                     test_plan_support.verify_success()),
            TestStep(23, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_9(self):

        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = [random.randbytes(32), random.randbytes(32), random.randbytes(32), random.randbytes(32)]
        my_ccdid = [None, None, None]
        my_csr = [None, None, None, None]

        self.step(3)
        for i in range(2):
            response = await cr1_cmd.send_csr_command(nonce=my_nonce[i])
            cr1_cmd.assert_valid_ccdid(response.ccdid)
            my_ccdid[i] = response.ccdid
            my_csr[i] = cr1_cmd.assert_valid_csr(response, my_nonce[i])
            if i > 0:
                asserts.assert_not_equal(my_ccdid[i-1], my_ccdid[i], "CCDID should be unique")

        self.step(4)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[2])
        cr2_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[2] = response.ccdid
        my_csr[2] = cr2_cmd.assert_valid_csr(response, my_nonce[2])

        self.step(5)
        # Don't have to use the same root, but may as well
        root = cr1_cmd.get_key()
        certs_with_key_1 = cr1_cmd.gen_cert_chain(root, 10)
        my_intermediate_certs_1 = [x.cert for x in certs_with_key_1]

        self.step(6)
        certs_with_key_2 = cr1_cmd.gen_cert_chain(root, 1)
        my_intermediate_certs_2 = [x.cert for x in certs_with_key_2]
        signers = [root, certs_with_key_1[0].key, certs_with_key_2[0].key]

        self.step(7)
        my_client_cert = [None, None, None, None]
        for i in range(3):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(signers[i], public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(8)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[0])

        self.step(9)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[1], certificate=my_client_cert[1], intermediates=my_intermediate_certs_1)

        self.step(10)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[2], certificate=my_client_cert[2], intermediates=my_intermediate_certs_2)

        self.step(11)
        found_certs = await cr1_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates")
        for i in range(2):
            asserts.assert_in(my_ccdid[i], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
            asserts.assert_equal(found_certs[my_ccdid[i]].clientCertificate,
                                 my_client_cert[i], "Expected matching certificate detail")
        asserts.assert_equal(found_certs[my_ccdid[1]].intermediateCertificates,
                             my_intermediate_certs_1, "Expected matching certificate detail")

        self.step(12)
        found_certs = await cr2_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_in(my_ccdid[2], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
        asserts.assert_equal(found_certs[my_ccdid[2]].clientCertificate, my_client_cert[2], "Expected matching certificate detail")
        asserts.assert_equal(found_certs[my_ccdid[2]].intermediateCertificates,
                             my_intermediate_certs_2, "Expected matching certificate detail")

        self.step(13)
        # Must close session so we don't re-use large payload session
        self.default_controller.ExpireSessions(self.dut_node_id)
        found_certs = await cr1_cmd.read_client_certs_attribute_as_map()
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates")
        for i in range(2):
            asserts.assert_in(my_ccdid[i], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
            asserts.assert_is_none(found_certs[my_ccdid[i]].clientCertificate, "Expected no certificate over non-Large-Transport")

        self.step(14)
        found_certs = await cr1_cmd.send_find_client_command_as_map()
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates")
        for i in range(2):
            asserts.assert_in(my_ccdid[i], found_certs, "FindClientCertificate should contain provisioned client cert")
            asserts.assert_equal(found_certs[my_ccdid[i]].clientCertificate,
                                 my_client_cert[i], "Expected matching certificate detail")
        asserts.assert_equal(found_certs[my_ccdid[1]].intermediateCertificates,
                             my_intermediate_certs_1, "Expected matching certificate detail")

        self.step(15)
        found_certs = await cr2_cmd.send_find_client_command_as_map()
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_in(my_ccdid[2], found_certs, "FindClientCertificate should contain provisioned client cert")
        asserts.assert_equal(found_certs[my_ccdid[2]].clientCertificate, my_client_cert[2], "Expected matching certificate detail")
        asserts.assert_equal(found_certs[my_ccdid[2]].intermediateCertificates,
                             my_intermediate_certs_2, "Expected matching certificate detail")

        self.step(16)
        response = await cr1_cmd.send_csr_command(ccdid=my_ccdid[0], nonce=my_nonce[3])
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        asserts.assert_equal(response.ccdid, my_ccdid[0], "Expected same ID")
        cr1_cmd.assert_valid_csr(response, my_nonce[3])
        my_client_cert[3] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[0].public_key(), subject=my_csr[0].subject)

        self.step(17)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[3])

        self.step(18)
        found_certs = await cr1_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates")
        expected_certs = [my_client_cert[3], my_client_cert[1]]
        for i in range(2):
            asserts.assert_in(my_ccdid[i], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
            asserts.assert_equal(found_certs[my_ccdid[i]].clientCertificate,
                                 expected_certs[i], "Expected matching certificate detail")

        self.step(19)
        found_certs = await cr2_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_in(my_ccdid[2], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
        asserts.assert_equal(found_certs[my_ccdid[2]].clientCertificate, my_client_cert[2], "Expected matching certificate detail")

        self.step(20)
        find_response = await cr1_cmd.send_find_client_command(ccdid=my_ccdid[0])
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected 1 certificate")
        single_response = find_response.certificateDetails[0]
        asserts.assert_equal(single_response.ccdid, my_ccdid[0], "FindClientCertificate should contain provisioned client cert")
        asserts.assert_equal(single_response.clientCertificate, my_client_cert[3], "Expected matching certificate detail")

        self.step(21)
        find_response = await cr2_cmd.send_find_client_command(ccdid=my_ccdid[2])
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected 1 certificate")
        single_response = find_response.certificateDetails[0]
        asserts.assert_equal(single_response.ccdid, my_ccdid[2], "FindClientCertificate should contain provisioned client cert")
        asserts.assert_equal(single_response.clientCertificate, my_client_cert[2], "Expected matching certificate detail")

        self.step(22)
        for i in range(2):
            await cr1_cmd.send_remove_client_command(ccdid=my_ccdid[i])

        self.step(23)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
