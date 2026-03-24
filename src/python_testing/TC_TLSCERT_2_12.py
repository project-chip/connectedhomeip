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


class TC_TLSCERT_2_12(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_12(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_12(self) -> str:
        return "[TC-TLSCERT-2.12] LookupClientCertificate command verification"

    def steps_TC_TLSCERT_2_12(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate myNonce[] with 3 distinct, random 32-octet values"),
            TestStep(3, "CR1 sends LookupClientCertificate command with Fingerprint set to the empty octstr.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(4, "CR1 sends LookupClientCertificate command with Fingerprint set to and arbitrary octstr.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(5, "CR1 sends ClientCSR command with Nonce set to myNonce[i], for each i in [0..1].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[i] and CSR in myCsr[i]."),
            TestStep(6, "CR2 sends ClientCSR command with Nonce set to myNonce[2].",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid[2] and CSR in myCsr[2]."),
            TestStep(7,
                     "Populate myClientCert[] with 3 distinct, valid, self-signed, DER-encoded x509 certificates using each respective public key from myCsr[i]."),
            TestStep(8, "Populate myClientCertFingerprint[] with the fingerprints corresponding to myClientCert[]."),
            TestStep(9, "Set myBigFingerprint to myClientCertFingerprint[0] concatenated with enough characters to exceed 64."),
            TestStep(10, "CR1 sends LookupClientCertificate command with Fingerprint set to the empty octstr.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(11, "CR1 sends LookupClientCertificate command with Fingerprint set to and arbitrary octstr.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(12, "CR1 sends LookupClientCertificate command with Fingerprint set to the myClientCertFingerprint[0]",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(13, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid[i] and ClientCertificate set to myClientCert[i], for each i in [0..1].",
                     test_plan_support.verify_success()),
            TestStep(14, "CR2 sends ProvisionClientCertificate command with CCDID set to myCcdid[2] and ClientCertificate set to myClientCert[2].",
                     test_plan_support.verify_success()),
            TestStep(15, "CR1 sends LookupClientCertificate command with Fingerprint set to myClientCertFingerprint[0].",
                     "DUT replies with a TLSCCDID value equal to myCcdid[0]."),
            TestStep(16, "CR1 sends LookupClientCertificate command with Fingerprint set to myClientCertFingerprint[1].",
                     "DUT replies with a TLSCCDID value equal to myCcdid[1]."),
            TestStep(17, "CR1 sends LookupClientCertificate command with Fingerprint set to myClientCertFingerprint[2].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(18, "CR2 sends LookupClientCertificate command with Fingerprint set to myClientCertFingerprint[2].",
                     "DUT replies with a TLSCCDID value equal to myCcdid[2]."),
            TestStep(19, "CR1 sends LookupClientCertificate command with Fingerprint set to myBigFingerprint.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(20, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid[i], for each i in [0..1].",
                     test_plan_support.verify_success()),
            TestStep(21, "CR2 sends RemoveClientCertificate command with CCDID set to myCcdid[2].",
                     test_plan_support.verify_success()),
            TestStep(22, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_12(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = [random.randbytes(32) for _ in range(3)]
        my_ccdid = [None] * 3
        my_csr = [None] * 3
        my_client_cert = [None] * 3

        self.step(3)
        await cr1_cmd.send_lookup_client_command(fingerprint=b'', expected_status=Status.NotFound)

        self.step(4)
        await cr1_cmd.send_lookup_client_command(fingerprint=b'arbitrary', expected_status=Status.NotFound)

        self.step(5)
        for i in range(2):
            response = await cr1_cmd.send_csr_command(nonce=my_nonce[i])
            cr1_cmd.assert_valid_ccdid(response.ccdid)
            my_ccdid[i] = response.ccdid
            my_csr[i] = cr1_cmd.assert_valid_csr(response, my_nonce[i])

        self.step(6)
        response = await cr2_cmd.send_csr_command(nonce=my_nonce[2])
        cr2_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid[2] = response.ccdid
        my_csr[2] = cr2_cmd.assert_valid_csr(response, my_nonce[2])

        self.step(7)
        root = cr1_cmd.get_key()
        for i in range(3):
            my_client_cert[i] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[i].public_key(), subject=my_csr[i].subject)

        self.step(8)
        my_client_cert_fingerprint = [cr1_cmd.get_fingerprint(cert) for cert in my_client_cert]

        self.step(9)
        my_big_fingerprint = my_client_cert_fingerprint[0] + b'\x00' * (65 - len(my_client_cert_fingerprint[0]))

        self.step(10)
        await cr1_cmd.send_lookup_client_command(fingerprint=b'', expected_status=Status.NotFound)

        self.step(11)
        await cr1_cmd.send_lookup_client_command(fingerprint=b'arbitrary', expected_status=Status.NotFound)

        self.step(12)
        await cr1_cmd.send_lookup_client_command(fingerprint=my_client_cert_fingerprint[0], expected_status=Status.NotFound)

        self.step(13)
        for i in range(2):
            await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[i], certificate=my_client_cert[i])

        self.step(14)
        await cr2_cmd.send_provision_client_command(ccdid=my_ccdid[2], certificate=my_client_cert[2])

        self.step(15)
        response = await cr1_cmd.send_lookup_client_command(fingerprint=my_client_cert_fingerprint[0])
        asserts.assert_equal(response.ccdid, my_ccdid[0])

        self.step(16)
        response = await cr1_cmd.send_lookup_client_command(fingerprint=my_client_cert_fingerprint[1])
        asserts.assert_equal(response.ccdid, my_ccdid[1])

        self.step(17)
        await cr1_cmd.send_lookup_client_command(fingerprint=my_client_cert_fingerprint[2], expected_status=Status.NotFound)

        self.step(18)
        response = await cr2_cmd.send_lookup_client_command(fingerprint=my_client_cert_fingerprint[2])
        asserts.assert_equal(response.ccdid, my_ccdid[2])

        self.step(19)
        await cr1_cmd.send_lookup_client_command(fingerprint=my_big_fingerprint, expected_status=Status.ConstraintError)

        self.step(20)
        for i in range(2):
            await cr1_cmd.send_remove_client_command(ccdid=my_ccdid[i])

        self.step(21)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid[2])

        self.step(22)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
