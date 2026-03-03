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
from TC_TLSCERT_Base import TC_TLSCERT_Base

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_6(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_6(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_6(self) -> str:
        return "[TC-TLSCERT-2.6] LookupRootCertificate command verification"

    def steps_TC_TLSCERT_2_6(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate myRootCert[] with 3 distinct, valid, self-signed, DER-encoded x509 certificates."),
            TestStep(3, "Populate myRootCertFingerprint[] with the fingerprints corresponding to myRootCert[]."),
            TestStep(4, "Set myBigFingerprint to myRootCertFingerprint[0] concatenated with enough characters to exceed 64."),
            TestStep(5, "CR1 sends LookupRootCertificate command with Fingerprint set to the empty octstr.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(6, "CR1 sends LookupRootCertificate command with Fingerprint set to myRootCertFingerprint[0].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(7, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[i], for each i in [0..1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[i]."),
            TestStep(8, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[2]."),
            TestStep(9, "CR1 sends LookupRootCertificate command with Fingerprint set to myRootCertFingerprint[0].",
                     "DUT replies with a TLSCAID value equal to myCaid[0]."),
            TestStep(10, "CR1 sends LookupRootCertificate command with Fingerprint set to myRootCertFingerprint[1].",
                     "DUT replies with a TLSCAID value equal to myCaid[1]."),
            TestStep(11, "CR1 sends LookupRootCertificate command with Fingerprint set to myRootCertFingerprint[2].",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(12, "CR2 sends LookupRootCertificate command with Fingerprint set to myRootCertFingerprint[2].",
                     "DUT replies with a TLSCAID value equal to myCaid[2]."),
            TestStep(13, "CR1 sends LookupRootCertificate command with Fingerprint set to myBigFingerprint.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(14, "CR1 sends RemoveRootCertificate command with CAID set to myCaid[i], for each i in [0..1].",
                     test_plan_support.verify_success()),
            TestStep(15, "CR2 sends RemoveRootCertificate command with CAID set to myCaid[2].",
                     test_plan_support.verify_success()),
            TestStep(16, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_6(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_root_cert = [cr1_cmd.gen_cert() for _ in range(3)]
        my_caid = [None, None, None]

        self.step(3)
        my_root_cert_fingerprint = [cr1_cmd.get_fingerprint(cert) for cert in my_root_cert]

        self.step(4)
        my_big_fingerprint = my_root_cert_fingerprint[0] + b'\x00' * (65 - len(my_root_cert_fingerprint[0]))

        self.step(5)
        await cr1_cmd.send_lookup_root_command(fingerprint=b'', expected_status=Status.NotFound)

        self.step(6)
        await cr1_cmd.send_lookup_root_command(fingerprint=my_root_cert_fingerprint[0], expected_status=Status.NotFound)

        self.step(7)
        for i in range(2):
            response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[i])
            cr1_cmd.assert_valid_caid(response.caid)
            my_caid[i] = response.caid

        self.step(8)
        response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2])
        cr2_cmd.assert_valid_caid(response.caid)
        my_caid[2] = response.caid

        self.step(9)
        response = await cr1_cmd.send_lookup_root_command(fingerprint=my_root_cert_fingerprint[0])
        asserts.assert_equal(response.caid, my_caid[0])

        self.step(10)
        response = await cr1_cmd.send_lookup_root_command(fingerprint=my_root_cert_fingerprint[1])
        asserts.assert_equal(response.caid, my_caid[1])

        self.step(11)
        await cr1_cmd.send_lookup_root_command(fingerprint=my_root_cert_fingerprint[2], expected_status=Status.NotFound)

        self.step(12)
        response = await cr2_cmd.send_lookup_root_command(fingerprint=my_root_cert_fingerprint[2])
        asserts.assert_equal(response.caid, my_caid[2])

        self.step(13)
        await cr1_cmd.send_lookup_root_command(fingerprint=my_big_fingerprint, expected_status=Status.ConstraintError)

        self.step(14)
        for i in range(2):
            await cr1_cmd.send_remove_root_command(caid=my_caid[i])

        self.step(15)
        await cr2_cmd.send_remove_root_command(caid=my_caid[2])

        self.step(16)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
