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


class TC_TLSCERT_2_5(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_5(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_5(self) -> str:
        return "[TC-TLSCERT-2.5] FindRootCertificate command verification with bad arguments"

    def steps_TC_TLSCERT_2_5(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myRootCert to a valid, self-signed, DER-encoded x509 certificate."),
            TestStep(3, "CR1 sends FindRootCertificate command with CAID set to null.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(4, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(5, "CR1 sends FindRootCertificate command with CAID set to myCaid.",
                     "DUT replies with a list of TLSCertStruct with one entry. The entry should correspond to myRootCert and myCaid."),
            TestStep(6, "CR1 sends FindRootCertificate command with CAID set to myCaid + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(7, "CR2 sends FindRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(8, "CR2 sends FindRootCertificate command with CAID set to null.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(9, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_success()),
            TestStep(10, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_5(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(3)
        await cr1_cmd.send_find_root_command(expected_status=Status.NotFound)

        self.step(4)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        cr1_cmd.assert_valid_caid(response.caid)
        my_caid = response.caid

        self.step(5)
        find_response = await cr1_cmd.send_find_root_command(caid=my_caid)
        asserts.assert_equal(len(find_response.certificateDetails), 1)
        asserts.assert_equal(find_response.certificateDetails[0].caid, my_caid)
        asserts.assert_equal(find_response.certificateDetails[0].certificate, my_root_cert)

        self.step(6)
        await cr1_cmd.send_find_root_command(caid=my_caid + 1, expected_status=Status.NotFound)

        self.step(7)
        await cr2_cmd.send_find_root_command(caid=my_caid, expected_status=Status.NotFound)

        self.step(8)
        await cr2_cmd.send_find_root_command(expected_status=Status.NotFound)

        self.step(9)
        await cr1_cmd.send_remove_root_command(caid=my_caid)

        self.step(10)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
