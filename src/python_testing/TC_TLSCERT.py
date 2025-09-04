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
import datetime
import random
import string
import test_plan_support
from typing import Optional, Union

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, rsa, utils
from cryptography.x509 import CertificateBuilder, random_serial_number
from cryptography.x509.oid import NameOID
from ecdsa.curves import curve_by_name
from mobly import asserts
from pyasn1.codec.der.decoder import decode as der_decoder
from pyasn1.error import PyAsn1Error
from pyasn1_modules import rfc2986, rfc5480

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.conversions import hex_from_bytes
from matter.testing import matter_asserts
from matter.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches,
                                           type_matches)
from matter.utils import CommissioningBuildingBlocks

class TC_TLSCERT(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def gen_cert(self, public_key: bytes | None = None) -> bytes:
        # Start generating the root certificate
        root_key = rsa.generate_private_key(
            public_exponent=65537, key_size=2048
        )
        if not public_key:
            public_key = root_key.public_key()

        rand_suffix = "".join(
            random.choices(string.ascii_letters + string.digits, k=16)
        )
        root_cert_subject = x509.Name(
            [
                x509.NameAttribute(NameOID.ORGANIZATION_NAME, "CSA"),
                x509.NameAttribute(
                    NameOID.COMMON_NAME, "TC_PAVS root " + rand_suffix
                ),
            ]
        )
        cert = (
            CertificateBuilder()
            .subject_name(root_cert_subject)
            .issuer_name(root_cert_subject)
            .public_key(public_key)
            .serial_number(random_serial_number())
            .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
            .not_valid_after(
                datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(days=365.25*20)
            )
            .add_extension(
                # We make it so that our root can only issue leaf certificates, no intermediate here.
                x509.BasicConstraints(ca=True, path_length=0), critical=True
            )
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=False,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=True,
                    crl_sign=True,
                    encipher_only=False,
                    decipher_only=False,
                ),
                critical=True,
            )
            .add_extension(
                x509.SubjectKeyIdentifier.from_public_key(
                    root_key.public_key()
                ),
                critical=False,
            )
            .sign(root_key, hashes.SHA256())
        )
        return cert.public_bytes(serialization.Encoding.DER)

    def assert_valid_caid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    def assert_valid_ccdid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CCDID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CCDID returned")

    def assert_valid_csr(self, response: Clusters.TlsCertificateManagement.Commands.ClientCSRResponse, nonce: bytes):
        # Verify der encoded and PKCS #10 (rfc2986 is PKCS #10) - next two requirements
        try:
            temp, _ = der_decoder(response.csr, asn1Spec=rfc2986.CertificationRequest())
        except PyAsn1Error:
            asserts.fail("Unable to decode CSR - improperly formatted DER")
        layer1 = dict(temp)

        # Verify public key is 256 bytes
        csr = x509.load_der_x509_csr(response.csr)
        csr_pubkey = csr.public_key()
        asserts.assert_equal(csr_pubkey.key_size, 256, "Incorrect key size")

        # Verify signature algorithm is ecdsa-with-SHA256
        signature_algorithm = dict(layer1['signatureAlgorithm'])['algorithm']
        asserts.assert_equal(signature_algorithm, rfc5480.ecdsa_with_SHA256, "CSR specifies incorrect signature key algorithm")

        # Verify signature is valid
        asserts.assert_true(csr.is_signature_valid, "Signature is invalid")

        # Verify response.nonceSignature is octet string of length 32
        try:
            # response.nonceSignature is an octet string if it can be converted to an int
            int(hex_from_bytes(response.nonceSignature), 16)
        except ValueError:
            asserts.fail("Returned CSR nonceSignature is not an octet string")

        # Verify response.nonceSignature is valid signature
        baselen = curve_by_name("NIST256p").baselen
        signature_raw_r = int(hex_from_bytes(response.nonceSignature[:baselen]), 16)
        signature_raw_s = int(hex_from_bytes(response.nonceSignature[baselen:]), 16)

        nonceSignature = utils.encode_dss_signature(signature_raw_r, signature_raw_s)
        csr_pubkey.verify(signature=nonceSignature, data=nonce, signature_algorithm=ec.ECDSA(hashes.SHA256()))
        return csr

    class TargetedFabric:
        def __init__(self, matter_test: MatterBaseTest, endpoint: Optional[int] = None,
                     dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None,
                     node_id: Optional[int] = None, fabric_index: Optional[int] = None):
            self.test = matter_test
            self.endpoint = endpoint
            self.dev_ctrl = dev_ctrl
            self.node_id = node_id
            self.fabric_index = fabric_index

        async def send_provision_root_command(
                self, certificate: bytes, caid: Union[Nullable, int] = NullValue,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(certificate=certificate, caid=caid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                asserts.assert_true(type_matches(result, Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse),
                                    "Unexpected return type for ProvisionRootCertificate")
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_find_root_command(
                self, caid: Union[Nullable, int] = NullValue,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.FindRootCertificateResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.FindRootCertificate(caid=caid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                asserts.assert_true(type_matches(result, Clusters.TlsCertificateManagement.Commands.FindRootCertificateResponse),
                                    "Unexpected return type for FindRootCertificate")
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_remove_root_command(
                self, caid: int,
                expected_status: Status = Status.Success) -> InteractionModelError:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.RemoveRootCertificate(caid=caid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_csr_command(
                self, nonce: bytes,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ClientCSRResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ClientCSR(nonce=nonce),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                asserts.assert_true(type_matches(result, Clusters.TlsCertificateManagement.Commands.ClientCSRResponse),
                                    "Unexpected return type for ClientCSR")
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_provision_client_command(
                self, certificate: bytes, ccdid: int,
                expected_status: Status = Status.Success) -> InteractionModelError:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionClientCertificate(ccdid=ccdid, clientCertificate=certificate),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_find_client_command(
                self, ccdid: Union[Nullable, int] = NullValue,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.FindClientCertificateResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.FindClientCertificate(ccdid=ccdid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                asserts.assert_true(type_matches(result, Clusters.TlsCertificateManagement.Commands.FindClientCertificateResponse),
                                    "Unexpected return type for FindClientCertificate")
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_remove_client_command(
                self, ccdid: int,
                expected_status: Status = Status.Success) -> InteractionModelError:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.RemoveClientCertificate(ccdid=ccdid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def read_tls_cert_attribute(self, attribute):
            cluster = Clusters.TlsCertificateManagement
            return await self.test.read_single_attribute_check_success(endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, cluster=cluster, attribute=attribute)


        async def read_root_certs_attribute_as_map(self, attribute):
            attribute_certs = await self.read_tls_cert_attribute(attribute)
            matter_asserts.assert_list(attribute_certs, "Expected list")
            found_certs = dict()
            for cert in attribute_certs:
                found_certs[cert.caid] = cert
            return found_certs

        async def send_remove_fabric_command(self, fabric_index: int):
            resp = await self.send_single_cmd(cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_index), endpoint=0, dev_ctrl=self.dev_ctrl)
            asserts.assert_equal(
                resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk)

    class TwoFabricData:
        def __init__(self, cr1_cmd, cr2_cmd):
            self.cr1_cmd = cr1_cmd
            self.cr2_cmd = cr2_cmd

    def get_common_substeps(self, step_prefix : string = "1") -> list[TestStep]:
        return [
            TestStep(f'{step_prefix}.1', test_plan_support.commission_if_required('CR1'), is_commissioning=True),
            TestStep(f'{step_prefix}.2', "CR1 reads ProvisionedRootCertificates attribute using a fabric-filtered read", "A list of TLSCertStruct. Store all resulting CAID in caidToClean."),
            TestStep(f'{step_prefix}.3', "CR1 sends RemoveRootCertificate command with CAID set to caidToClean[i] for all entries returned", test_plan_support.verify_success()),
            TestStep(f'{step_prefix}.4', "CR1 reads ProvisionedClientCertificates attribute using a fabric-filtered read", "A list of TLSClientCertificateDetailStruct. Store all resulting CCDID in ccdidToClean."),
            TestStep(f'{step_prefix}.5', "CR1 sends RemoveClientCertificate command with CCDID set to ccdidToClean[i] for all entries returned.", test_plan_support.verify_success()),
        ]
    async def common_setup(self, step_prefix : string = "1") -> TargetedFabric:
        self.step(f'{step_prefix}.1')
        attributes = Clusters.TlsCertificateManagement.Attributes
        endpoint = self.get_endpoint(default=1)

        # Establishing CR1 controller
        cr1_cmd = self.TargetedFabric(self, endpoint=endpoint)

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

    def get_two_fabric_substeps(self, step_prefix : string = "1") -> list[TestStep]:
        return [
            *self.get_common_substeps(f'{step_prefix}.1'),
            TestStep(f'{step_prefix}.2', test_plan_support.open_commissioning_window()),
            TestStep(f'{step_prefix}.3', test_plan_support.commission_from_existing('CR1', 'CR2')),
        ]

    async def common_two_fabric_setup(self, step_prefix : string = "1") -> TwoFabricData:
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

        cr2_cmd = self.TargetedFabric(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id, fabric_index=fabric_index_cr2)
        return self.TwoFabricData(cr1_cmd = cr1_cmd, cr2_cmd = cr2_cmd)

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
                     "Verify result a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate not populated (null)" ),
            TestStep(6, "CR2 Reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "Verify result is an empty list" ),
            TestStep(7, "CR1 sends ProvisionRootCertificate command with null CAID and Certificate set to myRootCert[0]",
                     test_plan_support.verify_status(Status.AlreadyExists)),
            TestStep(8, "CR1 Reads ProvisionedRootCertificates attribute using a fabric-filtered read on Large Message-capable transport",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[0]"),
            TestStep(9, "CR1 sends FindRootCertificate command with null CAID",
                     "Verify result is a list of TLSCertStruct with one entry. The entry should have CAID equal to myCaid[0] and Certificate equal to myRootCert[0]"),
            TestStep(10, "CR1 sends ProvisionRootCertificate command with CAID set to myCaid[0] and Certificate set to myRootCert[1]",
                     "Verify returned TLSCAID value equal to myCaid[0]"),
            TestStep(11, "CR2 Reads ProvisionedRootCertificates attribute using a fabric-filtered read",
                     "Verify result is an empty list" ),
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
            TestStep(21, "CR1 sends RemoveRootCertificate command with CAID set to myCaid[0].",
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
        my_root_cert = [self.gen_cert(), self.gen_cert(), self.gen_cert()]

        self.step(3)
        response = await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0])
        self.assert_valid_caid(response.caid)
        my_caid = [response.caid]

        self.step(4)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(attributes.ProvisionedRootCertificates)
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_equal(found_certs[my_caid[0]].certificate, my_root_cert[0], "Expected matching certificate detail")

        self.step(5)
        # Must close session so we don't re-use large payload session
        self.default_controller.ExpireSessions(self.dut_node_id)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(attributes.ProvisionedRootCertificates)
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_is_none(found_certs[my_caid[0]].certificate, "Expected no certificate over non-Large-Transport")

        self.step(6)
        attribute_certs = await cr2_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        matter_asserts.assert_list(attribute_certs, "Expected empty list", max_length=0)

        self.step(7)
        await cr1_cmd.send_provision_root_command(certificate=my_root_cert[0], expected_status = Status.AlreadyExists)

        self.step(8)
        # Must close session so we don't re-use large payload session
        self.default_controller.ExpireSessions(self.dut_node_id)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(attributes.ProvisionedRootCertificates)
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
        self.assert_valid_caid(response.caid)
        my_caid[1] = response.caid

        self.step(16)
        await cr2_cmd.send_provision_root_command(certificate=my_root_cert[2], caid=my_caid[1], expected_status = Status.AlreadyExists)

        self.step(17)
        found_certs = await cr1_cmd.read_root_certs_attribute_as_map(attributes.ProvisionedRootCertificates)
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(len(found_certs), 1, "Expected 1 certificate")
        asserts.assert_equal(found_certs[my_caid[0]].certificate, my_root_cert[1], "Expected matching certificate detail")

        self.step(18)
        found_certs = await cr2_cmd.read_root_certs_attribute_as_map(attributes.ProvisionedRootCertificates)
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
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
    #
    # def desc_TC_TLSCERT_2_9(self) -> str:
    #     return "[TC-TLSCERT-3.1.8] ProvisionClientCertificate command verification"
    #
    # def steps_TC_TLSCERT_2_9(self) -> list[TestStep]:
    #     steps = [
    #         *self.get_two_fabric_substeps(),
    #         TestStep(2, "Create two distinct, valid, self-signed, DER-encoded x509 certificates"),
    #         TestStep(3, "Sends the ClientCSR command"),
    #         TestStep(7, "Validates the CSR and nonce signature"),
    #         TestStep(8, "Sends the ProvisionClientCertificate command to the TlsCertificateManagement cluster",
    #                  "Verify that the DUT sends ProvisionClientCertificateResponse."),
    #         TestStep(9, "Sends another ProvisionClientCertificate command to the TlsCertificateManagement cluster",
    #                  "Verify a new ID is generated."),
    #         TestStep(10, "Read ProvisionedClientCertificates to from CR1"),
    #         TestStep(11, "Read ProvisionedClientCertificates to from CR2"),
    #         TestStep(12, "Read ProvisionedClientCertificates to make sure certificates from step 2 and 3 are present"),
    #         TestStep(13, "Sends the FindClientCertificate command specifying a CCDID from step 2",
    #                  "Verify the DUT sends FindClientCertificateResponse with both certificates"),
    #         TestStep(14, "Sends the FindClientCertificate command specifying a CCDID from step 3",
    #                  "Verify the DUT sends FindClientCertificateResponse with both certificates"),
    #         TestStep(15, "Sends the FindClientCertificate command specifying null for CCDID",
    #                  "Verify the DUT sends FindClientCertificateResponse with both certificates"),
    #         TestStep(16, "Sends the RemoveClientCertificate for both certificates added",
    #                  "Verify the DUT sends success status"),
    #     ]
    #     return steps
    #
    # @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    # async def test_TC_TLSCERT_2_9(self):
    #     attributes = Clusters.TlsCertificateManagement.Attributes
    #
    #     setup_data = await self.common_two_fabric_setup()
    #     cr1_cmd = setup_data.cr1_cmd
    #     cr2_cmd = setup_data.cr2_cmd
    #
    #     self.step(2)
    #
    #     first_nonce = random.randbytes(32)
    #     second_nonce = random.randbytes(32)
    #
    #     self.step(3)
    #     response = await cr1_cmd.send_csr_command(nonce=first_nonce)
    #     self.assert_valid_ccdid(response.ccdid)
    #
    #     self.step(7)
    #     first_csr = self.assert_valid_csr(response, first_nonce)
    #     first_cert = self.gen_cert(public_key=first_csr.public_key())
    #
    #     self.step(8)
    #     await cr1_cmd.send_provision_client_command(ccdid=response.ccdid, certificate=first_cert)
    #
    #     self.step(9)
    #     if not self.skip_cr2:
    #         response2 = await cr2_cmd.send_csr_command(nonce=second_nonce)
    #         self.assert_valid_ccdid(response2.ccdid)
    #         second_csr = self.assert_valid_csr(response2, second_nonce)
    #         asserts.assert_not_equal(response.ccdid, response2.ccdid, "CCDID should be unique")
    #         second_cert = self.gen_cert(public_key=second_csr.public_key())
    #
    #         await cr2_cmd.send_provision_client_command(ccdid=response2.ccdid, certificate=second_cert)
    #
    #     self.step(10)
    #     attribute_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedClientCertificates)
    #     asserts.assert_greater_equal(len(attribute_certs), 1, "Expected at least 1 certificate")
    #     found_certs = dict()
    #     # Python controller is using TCP, which supports large payloads; need a way to test
    #     # over MRP that certificate is not present
    #     for cert in attribute_certs:
    #         found_certs[cert.ccdid] = cert.clientCertificate
    #     asserts.assert_in(response.ccdid, found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
    #     asserts.assert_equal(found_certs[response.ccdid], first_cert, "Expected matching certificate detail")
    #
    #     self.step(11)
    #     if not self.skip_cr2:
    #         attribute_certs = await cr2_cmd.read_tls_cert_attribute(attribute=attributes.ProvisionedClientCertificates)
    #         asserts.assert_greater_equal(len(attribute_certs), 1, "Expected at least 1 certificate")
    #         found_certs = dict()
    #         # Python controller is using TCP, which supports large payloads; need a way to test
    #         # over MRP that certificate is not present
    #         for cert in attribute_certs:
    #             found_certs[cert.ccdid] = cert.clientCertificate
    #         asserts.assert_in(response2.ccdid, found_certs, "ProvisionedClientCertificates should contain provisioned client cert")
    #         asserts.assert_equal(found_certs[response2.ccdid], second_cert, "Expected matching certificate detail")
    #
    #     self.step(12)
    #     find_response = await cr1_cmd.send_find_client_command(ccdid=response.ccdid)
    #     asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
    #     asserts.assert_equal(find_response.certificateDetails[0].clientCertificate, first_cert)
    #
    #     self.step(13)
    #     if not self.skip_cr2:
    #         find_response2 = await cr2_cmd.send_find_client_command(ccdid=response.ccdid)
    #         asserts.assert_equal(len(find_response2.certificateDetails), 1, "Expected single certificate")
    #         asserts.assert_equal(find_response2.certificateDetails[0].clientCertificate,
    #                              second_cert, "Expected matching certificate detail")
    #
    #     self.step(14)
    #     find_all_response = await cr1_cmd.send_find_client_command()
    #     asserts.assert_greater_equal(len(find_all_response.certificateDetails), 1, "Expected at least 1 certificate")
    #     found_certs = dict()
    #     for cert in find_all_response.certificateDetails:
    #         found_certs[cert.ccdid] = cert.clientCertificate
    #     asserts.assert_in(response.ccdid, found_certs, "FindClientCertificate should contain provisioned client cert")
    #     asserts.assert_equal(found_certs[response.ccdid], first_cert, "Expected matching certificate detail")
    #
    #     self.step(15)
    #     if not self.skip_cr2:
    #         find_all_response = await cr2_cmd.send_find_client_command()
    #         asserts.assert_greater_equal(len(find_all_response.certificateDetails), 1, "Expected at least 1 certificate")
    #         found_certs = dict()
    #         for cert in find_all_response.certificateDetails:
    #             found_certs[cert.ccdid] = cert.clientCertificate
    #         asserts.assert_in(response2.ccdid, found_certs, "FindClientCertificate should contain provisioned client cert")
    #         asserts.assert_equal(found_certs[response2.ccdid], second_cert, "Expected matching certificate detail")
    #
    #     self.step(16)
    #     await cr1_cmd.send_remove_client_command(ccdid=response.ccdid)
    #     if not self.skip_cr2:
    #         await cr2_cmd.send_remove_client_command(ccdid=response2.ccdid)
    #     await cr1_cmd.send_remove_fabric_command(cr2_cmd.fabric_index)
    #

if __name__ == "__main__":
    default_matter_test_main()
