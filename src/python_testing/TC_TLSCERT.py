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
#       --app-pipe_prefix /tmp/chip_all_clusters_fifo_
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
import random
import string

import test_plan_support
from mobly import asserts
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.interaction_model import Status
from matter.testing import matter_asserts
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from matter.utils import CommissioningBuildingBlocks


class TC_TLSCERT(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    class TwoFabricData:
        def __init__(self, cr1_cmd: TLSUtils, cr2_cmd: TLSUtils):
            self.cr1_cmd = cr1_cmd
            self.cr2_cmd = cr2_cmd

    def get_common_substeps(self, step_prefix: string = "1") -> list[TestStep]:
        return [
            TestStep(f'{step_prefix}.1', test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(f'{step_prefix}.2', "CR1 reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "A list of TLSCertStruct. Store all resulting CAID in caidToClean."),
            TestStep(
                f'{step_prefix}.3', "CR1 sends RemoveRootCertificate command with CAID set to caidToClean[i] for all entries returned", test_plan_support.verify_success()),
            TestStep(f'{step_prefix}.4', "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read",
                     "A list of TLSClientCertificateDetailStruct. Store all resulting CCDID in ccdidToClean."),
            TestStep(
                f'{step_prefix}.5', "CR1 sends RemoveClientCertificate command with CCDID set to ccdidToClean[i] for all entries returned.", test_plan_support.verify_success()),
        ]

    async def common_setup(self, step_prefix: string = "1") -> TLSUtils:
        self.step(f'{step_prefix}.1')
        attributes = Clusters.TlsCertificateManagement.Attributes
        endpoint = self.get_endpoint(default=1)

        # Establishing CR1 controller
        cr1_cmd = TLSUtils(self, endpoint=endpoint)

        self.step(f'{step_prefix}.2')
        root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)

        self.step(f'{step_prefix}.3')
        for cert in root_certs:
            await cr1_cmd.send_remove_root_command(caid=cert.caid)

        self.step(f'{step_prefix}.4')
        client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedClientCertificates)

        self.step(f'{step_prefix}.5')
        for cert in client_certs:
            await cr1_cmd.send_remove_client_command(ccdid=cert.ccdid)

        return cr1_cmd

    def get_two_fabric_substeps(self, step_prefix: string = "1") -> list[TestStep]:
        return [
            *self.get_common_substeps(f'{step_prefix}.1'),
            TestStep(f'{step_prefix}.2', test_plan_support.open_commissioning_window()),
            TestStep(f'{step_prefix}.3', test_plan_support.commission_from_existing('CR1', 'CR2')),
        ]

    async def common_two_fabric_setup(self, step_prefix: string = "1") -> TwoFabricData:
        cr1_cmd = await self.common_setup(f'{step_prefix}.1')
        cr1 = self.default_controller
        endpoint = self.get_endpoint(default=1)

        self.step(f'{step_prefix}.2')
        # Establishing CR2 controller
        cr2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_fabric_admin = cr2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=cr1.fabricId + 1)
        cr2 = cr2_fabric_admin.NewController(nodeId=cr1.nodeId + 1)
        cr2_dut_node_id = self.dut_node_id+1

        self.step(f'{step_prefix}.3')
        _, noc_resp, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=cr1, newFabricDevCtrl=cr2,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        fabric_index_cr2 = noc_resp.fabricIndex

        cr2_cmd = TLSUtils(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id, fabric_index=fabric_index_cr2)
        return self.TwoFabricData(cr1_cmd=cr1_cmd, cr2_cmd=cr2_cmd)

    def pics_TC_TLSCERT_2_2(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_2(self) -> str:
        return "[TC-TLSCERT-3.1] ProvisionRootCertificate command basic insertion and modification"

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
            TestStep(21, "CR1 sends RemoveRootCertificate command with CAID set to my_caid[0]",
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

    def desc_TC_TLSCERT_2_9(self) -> str:
        return "[TC-TLSCERT-2.9] ProvisionClientCertificate command verification"

    def pics_TC_TLSCERT_2_9(self):
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        # In this case - there is no feature flags needed to run this test case
        return ["TLSCERT.S"]

    def steps_TC_TLSCERT_2_9(self) -> list[TestStep]:
        steps = [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Populate my_nonce[] with 4 distinct, random 32-octet values"),
            TestStep(3, "CR1 sends TLSClientCSR command with Nonce set to my_nonce[i], for each i in [0..1]",
                     "Verify the fields CCDID, CSR and Nonce with types TLSCCDID, octstr and octstr respectively. Store TLSCCDID in my_ccdid[i] and CSR in my_csr[i]."),
            TestStep(4, "CR2 sends sends TLSClientCSR command with Nonce set to my_nonce[2]",
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
            TestStep(16, "CR1 sends TLSClientCSR command with CCDID set to my_ccdid[0] and Nonce set to my_nonce[3]",
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
            TestStep(22, "CR1 sends ProvisionClientCertificate command with CCDID set to my_ccdid[0] and ClientCertificate set to my_client_cert[2]", test_plan_support.verify_status(
                Status.AlreadyExists)),
            TestStep(23, "CR1 sends RemoveClientCertificate command with CCDID set to my_ccdid[i], for each i in [0..1]",
                     test_plan_support.verify_success()),
            TestStep(24, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]
        return steps

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
            if i > 1:
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

        self.step(12)
        found_certs = await cr2_cmd.read_client_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_in(my_ccdid[2], found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
        asserts.assert_equal(found_certs[my_ccdid[2]].clientCertificate, my_client_cert[2], "Expected matching certificate detail")

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
        my_csr[3] = cr1_cmd.assert_valid_csr(response, my_nonce[3])
        my_client_cert[3] = cr1_cmd.gen_cert_with_key(root, public_key=my_csr[3].public_key(), subject=my_csr[3].subject)

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
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid[0], certificate=my_client_cert[2], expected_status=Status.AlreadyExists)

        self.step(23)
        for i in range(2):
            await cr1_cmd.send_remove_client_command(ccdid=my_ccdid[i])

        self.step(24)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
