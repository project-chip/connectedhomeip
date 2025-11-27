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

import logging
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
import string

import test_plan_support
from cryptography.x509 import CertificateBuilder, UnrecognizedExtension
from cryptography.x509.oid import ObjectIdentifier
from mobly import asserts
from TC_TLS_Utils import TLSUtils

import matter.clusters as Clusters
from matter.ChipDeviceCtrl import TransportPayloadCapability
from matter.clusters.Types import NullValue
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
        endpoint = self.get_endpoint()

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
        endpoint = self.get_endpoint()

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

    def pad_out_certificate(self) -> CertificateBuilder:
        builder = CertificateBuilder()
        for i in range(100):
            builder = builder.add_extension(
                UnrecognizedExtension(
                    oid=ObjectIdentifier(f"1.1.{i}"),
                    value=b'\x23' * 20
                ),
                critical=False
            )
        return builder

    def pics_TC_TLSCERT_2_1(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_1(self) -> str:
        return "[TC-TLSCERT-2.1] Attributes with Server as DUT"

    def steps_TC_TLSCERT_2_1(self) -> list[TestStep]:
        return [
            *self.get_common_substeps(),
            TestStep(2, "TH reads MaxRootCertificates attribute", "DUT replies with an uint8 value between 5 and 254."),
            TestStep(3, "TH reads ProvisionedRootCertificates attribute", "DUT replies with an empty list of TLSCertStruct."),
            TestStep(4, "TH reads MaxClientCertificates attribute", "DUT replies with an uint8 value between 2 and 254."),
            TestStep(5, "TH reads ProvisionedClientCertificates attribute",
                     "DUT replies with an empty list of TLSClientCertificateDetailStruct."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_1(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        cr1_cmd = await self.common_setup()

        self.step(2)
        max_root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.MaxRootCertificates)
        asserts.assert_greater_equal(max_root_certs, 5, "MaxRootCertificates should be >= 5")
        asserts.assert_less_equal(max_root_certs, 254, "MaxRootCertificates should be <= 254")

        self.step(3)
        provisioned_root_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        asserts.assert_equal(len(provisioned_root_certs), 0, "ProvisionedRootCertificates should be empty")

        self.step(4)
        max_client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.MaxClientCertificates)
        asserts.assert_greater_equal(max_client_certs, 2, "MaxClientCertificates should be >= 2")
        asserts.assert_less_equal(max_client_certs, 254, "MaxClientCertificates should be <= 254")

        self.step(5)
        provisioned_client_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedClientCertificates)
        asserts.assert_equal(len(provisioned_client_certs), 0, "ProvisionedClientCertificates should be empty")

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

    def pics_TC_TLSCERT_2_3(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_3(self) -> str:
        return "[TC-TLSCERT-2.3] ProvisionRootCertificate command verification with several entries"

    def steps_TC_TLSCERT_2_3(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "CR2 reads MaxRootCertificates attribute into myMaxRootCerts."),
            TestStep(3, "Populate myRootCert[] with myMaxRootCerts + 3 distinct, valid, self-signed, DER-encoded x509 certificates."),
            TestStep(4, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[i], for each i in [0..1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[i]."),
            TestStep(5, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[i], for each i in [2..myMaxRootCerts+1].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[i]."),
            TestStep(6, "CR1 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with 2 entries. The entries should correspond to the first two entries in myRootCert and myCaid."),
            TestStep(7, "CR2 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries. The entries should correspond to the entries 2 through myMaxRootCerts+1 of myRootCert and myCaid."),
            TestStep(8, "CR1 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with 2 entries. The entries should correspond to the first two entries in myRootCert and myCaid."),
            TestStep(9, "CR2 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries. The entries should correspond to the entries 2 through myMaxRootCerts + 1 of myRootCert and myCaid."),
            TestStep(10, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[myMaxRootCerts+2].",
                     test_plan_support.verify_status(Status.ResourceExhausted)),
            TestStep(11, "CR2 sends RemoveRootCertificate command with CAID set to myCaid[2].",
                     test_plan_support.verify_success()),
            TestStep(12, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[myMaxRootCerts+2].",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid[myMaxRootCerts+2]."),
            TestStep(13, "CR2 reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries."),
            TestStep(14, "CR2 sends FindRootCertificate command with null CAID.",
                     "DUT replies with a list of TLSCertStruct with myMaxRootCerts entries."),
            TestStep(15, "CR2 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2].",
                     test_plan_support.verify_status(Status.ResourceExhausted)),
            TestStep(16, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
            TestStep(17, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[2].",
                     "DUT replies with a TLSCAID value."),
            TestStep(18, "CR1 sends RemoveRootCertificate command with CAID set to myCaid[i], for each i in [0..1].",
                     test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_3(self):
        attributes = Clusters.TlsCertificateManagement.Attributes

        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_max_root_certs = await cr2_cmd.read_tls_cert_attribute(attributes.MaxRootCertificates)

        self.step(3)
        my_root_cert = [cr1_cmd.gen_cert() for _ in range(my_max_root_certs + 3)]
        my_caid = [None] * (my_max_root_certs + 3)

        self.step(4)
        for i in range(2):
            response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[i])
            cr1_cmd.assert_valid_caid(response.caid)
            my_caid[i] = response.caid

        self.step(5)
        logging.warning(f"maximum cert {my_max_root_certs}")
        for i in range(2, my_max_root_certs + 2):
            logging.warning(f"Root cert {i}")
            response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[i])
            cr2_cmd.assert_valid_caid(response.caid)
            my_caid[i] = response.caid

        self.step(6)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), 2, "Expected 2 certificates for CR1")
        for i in range(2):
            asserts.assert_in(my_caid[i], found_certs)
            asserts.assert_equal(found_certs[my_caid[i]].certificate, my_root_cert[i])

        self.step(7)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), my_max_root_certs, "Expected myMaxRootCerts certificates for CR2")
        for i in range(2, my_max_root_certs + 2):
            asserts.assert_in(my_caid[i], found_certs)
            asserts.assert_equal(found_certs[my_caid[i]].certificate, my_root_cert[i])

        self.step(8)
        find_response = await cr1_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), 2, "Expected 2 certificates for CR1")

        self.step(9)
        find_response = await cr2_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), my_max_root_certs,
                             "Expected myMaxRootCerts certificates for CR2")

        self.step(10)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[my_max_root_certs + 2], expected_status=Status.ResourceExhausted)

        self.step(11)
        await cr2_cmd.send_remove_root_command(caid=my_caid[2])

        self.step(12)
        response = await cr2_cmd.send_provision_root_command(certificate=my_root_cert[my_max_root_certs + 2])
        cr2_cmd.assert_valid_caid(response.caid)
        my_caid[my_max_root_certs + 2] = response.caid

        self.step(13)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map(TransportPayloadCapability.LARGE_PAYLOAD)
        asserts.assert_equal(len(found_certs), my_max_root_certs, "Expected myMaxRootCerts certificates for CR2")

        self.step(14)
        find_response = await cr2_cmd.send_find_root_command()
        asserts.assert_equal(len(find_response.certificateDetails), my_max_root_certs,
                             "Expected myMaxRootCerts certificates for CR2")

        self.step(15)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2], expected_status=Status.ResourceExhausted)

        self.step(16)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

        self.step(17)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[2])
        cr1_cmd.assert_valid_caid(response.caid)

        self.step(18)
        for i in range(2):
            await cr1_cmd.send_remove_root_command(caid=my_caid[i])

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

    def pics_TC_TLSCERT_2_7(self):
        return ["TLSCERT.S", "TLSCLIENT.S"]

    def desc_TC_TLSCERT_2_7(self) -> str:
        return "[TC-TLSCERT-2.7] RemoveRootCertificate command verification"

    def steps_TC_TLSCERT_2_7(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myRootCert to a valid, self-signed, DER-encoded x509 certificate."),
            TestStep(3, "CR1 sends RemoveRootCertificate command with CAID set to 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(4, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(5, "CR2 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(6, "CR1 sends ProvisionEndpoint command with valid Hostname, Port, CAID myCaid and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint."),
            TestStep(7, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_status(Status.InvalidInState)),
            TestStep(8, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint.",
                     test_plan_support.verify_success()),
            TestStep(9, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_success()),
            TestStep(10, "CR1 sends FindRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(11, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(12, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_7(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(3)
        await cr1_cmd.send_remove_root_command(caid=1, expected_status=Status.NotFound)

        self.step(4)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        cr1_cmd.assert_valid_caid(response.caid)
        my_caid = response.caid

        self.step(5)
        await cr2_cmd.send_remove_root_command(caid=my_caid, expected_status=Status.NotFound)

        self.step(6)
        endpoint_response = await cr1_cmd.send_provision_tls_endpoint_command(hostname=b"my_hostname", port=1000, caid=my_caid)
        my_endpoint = endpoint_response.endpointID

        self.step(7)
        await cr1_cmd.send_remove_root_command(caid=my_caid, expected_status=Status.InvalidInState)

        self.step(8)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint)

        self.step(9)
        await cr1_cmd.send_remove_root_command(caid=my_caid)

        self.step(10)
        await cr1_cmd.send_find_root_command(caid=my_caid, expected_status=Status.NotFound)

        self.step(11)
        await cr1_cmd.send_remove_root_command(caid=my_caid, expected_status=Status.NotFound)

        self.step(12)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

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

    def pics_TC_TLSCERT_2_11(self):
        return ["TLSCERT.S"]

    def desc_TC_TLSCERT_2_11(self) -> str:
        return "[TC-TLSCERT-2.11] FindClientCertificate command verification"

    def steps_TC_TLSCERT_2_11(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myNonce to a random 32-octet value"),
            TestStep(3, "CR1 sends FindClientCertificate command with null CCDID.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(4, "CR1 sends ClientCSR command with Nonce set to myNonce.",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid."),
            TestStep(5, "CR1 sends FindClientCertificate command with CCDID set to myCcdid.",
                     "DUT replies with a list of TLSClientCertificateDetailsStruct with one entry. The entry should have CCDID with value myCcdid with ClientCertificate and IntermediateCertificates unset."),
            TestStep(6, "CR1 sends FindClientCertificate command with CCDID set to myCcdid + 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(7, "CR2 sends FindClientCertificate command with null CCDID.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(8, "CR2 sends FindClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(9, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_success()),
            TestStep(10, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_11(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = random.randbytes(32)

        self.step(3)
        await cr1_cmd.send_find_client_command(expected_status=Status.NotFound)

        self.step(4)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce)
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        cr1_cmd.assert_valid_csr(response, my_nonce)
        my_ccdid = response.ccdid

        self.step(5)
        find_response = await cr1_cmd.send_find_client_command(ccdid=my_ccdid)
        asserts.assert_equal(len(find_response.certificateDetails), 1)
        asserts.assert_equal(find_response.certificateDetails[0].ccdid, my_ccdid)
        asserts.assert_equal(find_response.certificateDetails[0].clientCertificate, NullValue)
        asserts.assert_equal(len(find_response.certificateDetails[0].intermediateCertificates), 0)

        self.step(6)
        await cr1_cmd.send_find_client_command(ccdid=my_ccdid + 1, expected_status=Status.NotFound)

        self.step(7)
        await cr2_cmd.send_find_client_command(expected_status=Status.NotFound)

        self.step(8)
        await cr2_cmd.send_find_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(9)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid)

        self.step(10)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)

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
            TestStep(14, "CR2 sends ProvisionClientCertificate command with CCDID set to myCcdid[2] and ClientCertificateDetails set to myClientCert[2].",
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

    def pics_TC_TLSCERT_2_13(self):
        return ["TLSCERT.S", "TLSCLIENT.S"]

    def desc_TC_TLSCERT_2_13(self) -> str:
        return "[TC-TLSCERT-2.13] RemoveClientCertificate command verification"

    def steps_TC_TLSCERT_2_13(self) -> list[TestStep]:
        return [
            *self.get_two_fabric_substeps(),
            TestStep(2, "Set myNonce to a random 32-octet value"),
            TestStep(3, "Set myRootCert to a valid, self-signed, DER-encoded x509 certificate"),
            TestStep(4, "CR1 sends RemoveClientCertificate command with CCDID set to 1.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(5, "CR1 sends ClientCSR command with Nonce set to myNonce.",
                     "DUT replies with CCDID, CSR and Nonce. Store TLSCCDID in myCcdid."),
            TestStep(6, "Populate myClientCert with a valid, self-signed, DER-encoded x509 certificate using the public key from the CSR."),
            TestStep(7, "CR1 sends ProvisionClientCertificate command with CCDID set to myCcdid and ClientCertificateDetails set to myClientCert.",
                     test_plan_support.verify_success()),
            TestStep(8, "CR2 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(9, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert.",
                     "DUT replies with a TLSCAID value. Store the returned value as myCaid."),
            TestStep(10, "CR1 sends ProvisionEndpoint command with valid Hostname, Port, CAID myCaid and null EndpointID.",
                     "DUT replies with a TLSEndpointID value. Store the returned value as myEndpoint."),
            TestStep(11, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.InvalidInState)),
            TestStep(12, "CR1 sends RemoveEndpoint command with EndpointID set to myEndpoint.",
                     test_plan_support.verify_success()),
            TestStep(13, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_success()),
            TestStep(14, "CR1 sends FindClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(15, "CR1 sends RemoveClientCertificate command with CCDID set to myCcdid.",
                     test_plan_support.verify_status(Status.NotFound)),
            TestStep(16, "CR1 sends RemoveRootCertificate command with CAID set to myCaid.",
                     test_plan_support.verify_success()),
            TestStep(17, test_plan_support.remove_fabric('CR2', 'CR1'), test_plan_support.verify_success()),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_2_13(self):
        setup_data = await self.common_two_fabric_setup()
        cr1_cmd = setup_data.cr1_cmd
        cr2_cmd = setup_data.cr2_cmd

        self.step(2)
        my_nonce = random.randbytes(32)

        self.step(3)
        my_root_cert = cr1_cmd.gen_cert()

        self.step(4)
        await cr1_cmd.send_remove_client_command(ccdid=1, expected_status=Status.NotFound)

        self.step(5)
        response = await cr1_cmd.send_csr_command(nonce=my_nonce)
        cr1_cmd.assert_valid_ccdid(response.ccdid)
        my_ccdid = response.ccdid
        my_csr = cr1_cmd.assert_valid_csr(response, my_nonce)

        self.step(6)
        root = cr1_cmd.get_key()
        my_client_cert = cr1_cmd.gen_cert_with_key(root, public_key=my_csr.public_key(), subject=my_csr.subject)

        self.step(7)
        await cr1_cmd.send_provision_client_command(ccdid=my_ccdid, certificate=my_client_cert)

        self.step(8)
        await cr2_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(9)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert)
        cr1_cmd.assert_valid_caid(response.caid)
        my_caid = response.caid

        self.step(10)
        endpoint_response = await cr1_cmd.send_provision_tls_endpoint_command(hostname=b"my_hostname", port=1000, caid=my_caid, ccdid=my_ccdid)
        my_endpoint = endpoint_response.endpointID

        self.step(11)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.InvalidInState)

        self.step(12)
        await cr1_cmd.send_remove_tls_endpoint_command(endpoint_id=my_endpoint)

        self.step(13)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid)

        self.step(14)
        await cr1_cmd.send_find_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(15)
        await cr1_cmd.send_remove_client_command(ccdid=my_ccdid, expected_status=Status.NotFound)

        self.step(16)
        await cr1_cmd.send_remove_root_command(caid=my_caid)

        self.step(17)
        await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
