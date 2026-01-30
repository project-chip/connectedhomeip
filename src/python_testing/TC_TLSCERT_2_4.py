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


class TC_TLSCERT_2_4(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_4(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_4(self) -> str:
        return "[TC-TLSCERT-2.4] ProvisionRootCertificate command verification with bad arguments"

    def steps_TC_TLSCERT_2_4(self) -> list[TestStep]:
        return [
            *self.get_common_substeps(),
            TestStep(2,
                     "Set myBigRootCert to a valid DER encoding of a valid, self-signed x509 certificate. The certificate should be large enough that the DER encoding is larger than 3000 octets."),
            TestStep(3, "Set myBadRootCert to an octstr under 3000 that is not a valid DER encoding."),
            TestStep(4, "Set myRootCert to a valid, self-signed, DER-encoded x509 certificate."),
            TestStep(5, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myBigRootCert.",
                     test_plan_support.verify_status(Status.ConstraintError)),
            TestStep(6, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myBadRootCert.",
                     test_plan_support.verify_status(Status.DynamicConstraintError)),
            TestStep(7, "CR1 sends ProvisionRootCertificate command with CAID set to 10 and Certificate set to myRootCert.",
                     test_plan_support.verify_status(Status.NotFound)),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_4(self):
        cr1_cmd = await self.common_setup()

        self.step(2)
        my_big_root_cert = cr1_cmd.gen_cert_with_key(signer=cr1_cmd.get_key(), builder_lambda=self.pad_out_certificate)
        asserts.assert_greater(len(my_big_root_cert), 3000)

        self.step(3)
        my_bad_root_cert = b"not a cert"

        self.step(4)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(5)
        await cr1_cmd.send_provision_root_command(certificate=my_big_root_cert, expected_status=Status.ConstraintError)

        self.step(6)
        await cr1_cmd.send_provision_root_command(certificate=my_bad_root_cert, expected_status=Status.DynamicConstraintError)

        self.step(7)
        await cr1_cmd.send_provision_root_command(caid=10, certificate=my_root_cert, expected_status=Status.NotFound)


if __name__ == "__main__":
    default_matter_test_main()
