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
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_10(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_10(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_10(self) -> str:
        return "[TC-TLSCERT-2.10] ProvisionClientCertificate command verification with bad arguments"

    def steps_TC_TLSCERT_2_10(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate myNonce with a random 32-octet values"),
            TestStep(3, "CR1 sends sends ClientCSR command with Nonce set to myNonce.",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid and CSR in myCsr."),
            TestStep(4,
                     "Set myBigCert to a valid DER encoding of a valid, self-signed x509 certificate using the public key from csr. The certificate should be large enough that the DER encoding is larger than 3000 octets."),
            TestStep(5, "Set myBadCert to an octstr under 3000 that is not a valid DER encoding."),
            TestStep(6,
                     "Set myWrongKey to a valid DER encoding of a valid, self-signed x509 certificate using a public key not equal to that in myCsr."),
            TestStep(7,
                     "Populate myIntermediateCerts with 1 DER-encoded x509 certificates that form a certificate chain up to (but not including) a root."),
            TestStep(8,
                     "Populate myClientCert with a distinct, valid, DER-encoded x509 certificates using the public key from myCsr and signed by myIntermediateCerts[0]."),
            TestStep(9, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificate set to myBigCert.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(10, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid, ClientCertificate set to myClientCert, and IntermediateCertificates set to myBigCert.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(11, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificate set to myBadCert.",
                     test_plan_support.verify_status(Status.DynamicConstraintError)),
            TestStep(12, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificate set to myWrongKey.",
                     test_plan_support.verify_status(Status.DynamicConstraintError)),
            TestStep(13, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid, ClientCertificate set to myClientCert, and IntermediateCertificates set to myBadCert.",
                     test_plan_support.verify_status(Status.DynamicConstraintError)),
            TestStep(14, "CR1 sends ProvisionClientCertificate command with CCDID set to a value not equal to myCcdid and ClientCertificate set to myClientCert[0].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(15, "CR2 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificate set to myClientCert.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(16, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_success()),
            TestStep(17, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_10(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = random.randbytes(32)

        self.step(3)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce)
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid = response.ccdid
        my_csr = cr1_cmd.assert_valid_csr(response, my_nonce)

        self.step(4)
        root = cr1_cmd.get_key()
        my_big_cert = cr1_cmd.gen_cert_with_key(signer=root, public_key=my_csr.public_key(),
                                                builder_lambda=self.pad_out_certificate)
        asserts.assert_greater(len(my_big_cert), 3000)

        self.step(5)
        my_bad_cert = b"not a cert"

        self.step(6)
        key_for_wrong_cert = cr1_cmd.get_key()
        my_wrong_key_cert = cr1_cmd.gen_cert_with_key(key_for_wrong_cert, public_key=key_for_wrong_cert.public_key())

        self.step(7)
        certs_with_key = cr1_cmd.gen_cert_chain(key_for_wrong_cert, 1)
        signer_key = certs_with_key[0].key

        self.step(8)
        my_client_cert = cr1_cmd.gen_cert_with_key(
            signer_key, public_key=my_csr.public_key(), subject=my_csr.subject)

        self.step(9)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_big_cert, expected_status=Status.ConstraintError)

        self.step(10)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert, intermediates=[my_big_cert], expected_status=Status.ConstraintError)

        self.step(11)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_bad_cert, expected_status=Status.DynamicConstraintError)

        self.step(12)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_wrong_key_cert, expected_status=Status.DynamicConstraintError)

        self.step(13)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert, intermediates=[my_bad_cert], expected_status=Status.DynamicConstraintError)

        self.step(14)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid + 1, certificate=my_client_cert, expected_status=Status.NotFound)

        self.step(15)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert, expected_status=Status.NotFound)

        self.step(16)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid)

        self.step(17)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
