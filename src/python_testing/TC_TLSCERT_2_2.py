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
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.runner import TestStep, default_matter_test_main


class TC_TLSCERT_2_2(TC_TLSCERT_Base):
    def pics_TC_TLSCERT_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_2(self) -> str:
        return "[TC-TLSCERT-2.2] ProvisionRootCertificate command basic insertion and modification"

    def steps_TC_TLSCERT_2_2(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate myRootCert[] with 3 distinct, valid, self-signed, DER-encoded x509 certificates"),
            TestStep(3, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[0]",
                     "Verify returned TLSCAID value. Store the returned value as myCaid[0]."),
            TestStep(4, "CR1 Reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[0]"),
            TestStep(5, "CR1 Reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "Verify result a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate not populated (null)"),
            TestStep(6, "CR2 Reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "Verify result is an empty list"),
            TestStep(7, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[0]",
                     test_plan_support.verify_status(Status.AlreadyExists)),
            TestStep(8, "CR1 Reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[0]"),
            TestStep(9, "CR1 sends FindRootCertificate command with null CAID",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[0]"),
            TestStep(10, "CR1 sends ProvisionRootCertificate command with CAID set to myCaid[0] and Certificate set to myRootCert[1]",
                     "Verify returned TLSCAID value equal to myCaid[0]"),
            TestStep(11, "CR2 Reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "Verify result is an empty list"),
            TestStep(12, "CR1 sends FindRootCertificate command with null CAID",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[1]"),
            TestStep(13, "CR2 sends FindRootCertificate command with null CAID",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(14, "CR2 sends ProvisionRootCertificate command with CAID set to myCaid[0] and Certificate set to myRootCert[2]",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(15, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2]",
                     "Verify returned TLSCAID value. Store the returned value as myCaid[1]."),
            TestStep(16, "CR2 sends ProvisionRootCertificate command with CAID set to myCaid[1] and Certificate set to myRootCert[2]",
                     test_plan_support.verify_status(Status.AlreadyExists)),
            TestStep(17, "CR1 Reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[1]"),
            TestStep(18, "CR2 Reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[1] and Certificate equal to myRootCert[2]"),
            TestStep(19, "CR1 sends FindRootCertificate command with null CAID",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[1]"),
            TestStep(20, "CR2 sends FindRootCertificate command with null CAID",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[1] and Certificate equal to myRootCert[2]"),
            TestStep(21, "CR1 sends RemoveRootCertificate command with CAID set to myCaid[0]",
                     test_plan_support.verify_success()),
            TestStep(22, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_2(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_root_cert = [cr1_cmd.gen_cert(), cr1_cmd.gen_cert(), cr1_cmd.gen_cert()]

        self.step(3)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        cr1_cmd.assert_valid_caid(response.caid)
        my_caid = [response.caid, None, None]

        self.step(4)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map()
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_equal(found_certs[my_caid[0]].certificate, my_root_cert[0], "Expected matching certificate detail")

        self.step(5)
        # Must close session so we don't re-use large payload session
        self.default_controller.ExpireSessions(self.dut_node_id)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map()
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_is_none(found_certs[my_caid[0]].certificate, "Expected no certificate over non-Large-Transport")

        self.step(6)
        attribute_certs = await cr2_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        matter_asserts.assert_list(attribute_certs, "Expected empty list", max_length=0)

        self.step(7)
        await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0], expected_status=Status.AlreadyExists)

        self.step(8)
        # Must close session so we don't re-use large payload session
        self.default_controller.ExpireSessions(self.dut_node_id)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map()
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_is_none(found_certs[my_caid[0]].certificate, "Expected no certificate over non-Large-Transport")

        self.step(9)
        find_response = await cr1_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
        asserts.assert_equal(find_response.certificateDetails[0].caid, my_caid[0])
        asserts.assert_equal(find_response.certificateDetails[0].certificate, my_root_cert[0])

        self.step(10)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[1], caid=my_caid[0])
        asserts.assert_equal(response.caid, my_caid[0])

        self.step(11)
        attribute_certs = await cr2_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        matter_asserts.assert_list(attribute_certs, "Expected empty list", max_length=0)

        self.step(12)
        find_response = await cr1_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
        asserts.assert_equal(find_response.certificateDetails[0].caid, my_caid[0])
        asserts.assert_equal(find_response.certificateDetails[0].certificate, my_root_cert[1])

        self.step(13)
        await cr2_cmd.send_find_root_command(caid=my_caid[0], expected_status=Status.NotFound)

        self.step(14)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2], caid=my_caid[0], expected_status=Status.NotFound)

        self.step(15)
        response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2])
        cr2_cmd.assert_valid_caid(response.caid)
        my_caid[1] = response.caid

        self.step(16)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2], caid=my_caid[1], expected_status=Status.AlreadyExists)

        self.step(17)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map()
        asserts.assert_in(my_caid[0], found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_equal(found_certs[my_caid[0]].certificate, my_root_cert[1], "Expected matching certificate detail")

        self.step(18)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map()
        asserts.assert_in(my_caid[1], found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_equal(found_certs[my_caid[1]].certificate, my_root_cert[2], "Expected matching certificate detail")

        self.step(19)
        find_response = await cr1_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
        asserts.assert_equal(find_response.certificateDetails[0].certificate, my_root_cert[1])
        asserts.assert_equal(find_response.certificateDetails[0].caid, my_caid[0])

        self.step(20)
        find_response = await cr2_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
        asserts.assert_equal(find_response.certificateDetails[0].certificate, my_root_cert[2])
        asserts.assert_equal(find_response.certificateDetails[0].caid, my_caid[1])

        self.step(21)
        await cr1_cmd.send_remove_root_command(caid=my_caid[0])

        self.step(22)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
