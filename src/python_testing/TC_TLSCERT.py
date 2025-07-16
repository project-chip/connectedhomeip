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
from typing import Dict, Optional, Set, Union

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import Nullable, NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.commissioning import CustomCommissioningParameters
from chip.testing.matter_testing import (MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches,
                                         type_matches)
from chip.tlv import uint
from chip.utils import CommissioningBuildingBlocks
from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509 import CertificateBuilder, random_serial_number
from cryptography.x509.oid import NameOID
from mobly import asserts


class TC_TLSCERT(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        # See https://github.com/project-chip/connectedhomeip/issues/39690
        self.skip_cr2 = True

    def gen_root_cert(self) -> bytes:
        # Start generating the root certificate
        root_key = rsa.generate_private_key(
            public_exponent=65537, key_size=2048
        )
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
            .public_key(root_key.public_key())
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

    def assert_valid_id(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    class TargetedFabric:
        def __init__(self, matter_test: MatterBaseTest, endpoint: Optional[int] = None,
                     dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None,
                     node_id: Optional[int] = None):
            self.test = matter_test
            self.endpoint = endpoint
            self.dev_ctrl = dev_ctrl
            self.node_id = node_id

        async def send_provision_root_command(
                self, certificate: bytes,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(certificate=certificate),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

                asserts.assert_true(type_matches(result, Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse),
                                    "Unexpected return type for ProvisionRootCertificate")
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def send_find_root_command(
                self, caid: Union[Nullable, int] = NullValue,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse, InteractionModelError]:
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
                self, caid: Union[Nullable, int] = NullValue,
                expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse, InteractionModelError]:
            try:
                result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.RemoveRootCertificate(caid=caid),
                                                         endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
                return result
            except InteractionModelError as e:
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
                return e

        async def read_tls_cert_attribute(self, attribute):
            cluster = Clusters.TlsCertificateManagement
            return await self.test.read_single_attribute_check_success(endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, cluster=cluster, attribute=attribute)

    def desc_TC_TLSCERT_3_1(self) -> str:
        return "[TC-TLSCERT-1.1] Primary functionality with DUT as Server"

    def steps_TC_TLSCERT_3_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "CR1 opens commissioning window on DUT",
                     "Commissioning window should open"),
            TestStep(3, "CR2 fully commissions DUT_CE", "DUT should fully commission"),
            TestStep(4, "Create two distinct, valid, self-signed, DER-encoded x509 certificates"),
            TestStep(5, "Read ProvisionedRootCertificates"),
            TestStep(6, "Sends the RemoveRootCertificate command for any certificates found in step 4"),
            TestStep(7, "Sends the ProvisionRootCertificate command to the TlsCertificateManagement cluster",
                     "Verify that the DUT sends ProvisionRootCertificateResponse."),
            TestStep(8, "Sends another ProvisionRootCertificate command to the TlsCertificateManagement cluster",
                     "Verify a new ID is generated."),
            TestStep(9, "Read ProvisionedRootCertificates to from CR1"),
            TestStep(10, "Read ProvisionedRootCertificates to from CR2"),
            TestStep(11, "Read ProvisionedRootCertificates to make sure certificates from step 2 and 3 are present"),
            TestStep(12, "Sends the FindRootCertificate command specifying a CAID from step 2",
                     "Verify the DUT sends FindRootCertificateResponse with both certificates"),
            TestStep(13, "Sends the FindRootCertificate command specifying a CAID from step 3",
                     "Verify the DUT sends FindRootCertificateResponse with both certificates"),
            TestStep(14, "Sends the FindRootCertificate command specifying null for CAID",
                     "Verify the DUT sends FindRootCertificateResponse with both certificates"),
            TestStep(15, "Sends the RemoveRootCertificate for both certificates added",
                     "Verify the DUT sends success status"),
        ]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.TlsCertificateManagement))
    async def test_TC_TLSCERT_3_1(self):
        endpoint = self.get_endpoint(default=1)
        attributes = Clusters.TlsCertificateManagement.Attributes

        self.step(1)
        # Establishing CR1 controller
        cr1 = self.default_controller
        cr1_cmd = self.TargetedFabric(self, endpoint=endpoint)

        # Establishing CR2 controller
        cr2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_fabric_admin = cr2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=cr1.fabricId + 1)
        cr2 = cr2_fabric_admin.NewController(nodeId=cr1.nodeId + 1)
        cr2_dut_node_id = self.dut_node_id+1
        cr2_cmd = self.TargetedFabric(self, endpoint=endpoint, dev_ctrl=cr2, node_id=cr2_dut_node_id)

        self.step(2)

        _, noc_resp, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=cr1, newFabricDevCtrl=cr2,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        fabric_index_cr2 = noc_resp.fabricIndex

        self.step(3)

        self.step(4)
        first_cert = self.gen_root_cert()
        second_cert = self.gen_root_cert()

        self.step(5)
        attribute_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)

        self.step(6)
        for cert in attribute_certs:
            await cr1_cmd.send_remove_root_command(caid=cert.caid)

        self.step(7)
        response = await cr1_cmd.send_provision_root_command(certificate=first_cert)
        self.assert_valid_id(response.caid)

        self.step(8)
        if not self.skip_cr2:
            response2 = await cr2_cmd.send_provision_root_command(certificate=second_cert)
            self.assert_valid_id(response2.caid)
            asserts.assert_not_equal(response.caid, response2.caid, "CAID should be unique")

        self.step(9)
        attribute_certs = await cr1_cmd.read_tls_cert_attribute(attributes.ProvisionedRootCertificates)
        asserts.assert_greater_equal(len(attribute_certs), 1, "Expected at least 1 certificates")
        found_certs = dict()
        # Python controller is using TCP, which supports large payloads; need a way to test
        # over MRP that certificate is not present
        for cert in attribute_certs:
            found_certs[cert.caid] = cert.certificate
        asserts.assert_in(response.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
        asserts.assert_equal(found_certs[response.caid], first_cert, "Expected matching certificate detail")

        self.step(10)
        if not self.skip_cr2:
            attribute_certs = await cr2_cmd.read_tls_cert_attribute(attribute=attributes.ProvisionedRootCertificates)
            asserts.assert_greater_equal(len(attribute_certs), 1, "Expected at least 1 certificates")
            found_certs = dict()
            # Python controller is using TCP, which supports large payloads; need a way to test
            # over MRP that certificate is not present
            for cert in attribute_certs:
                found_certs[cert.caid] = cert.certificate
            asserts.assert_in(response2.caid, found_certs, "ProvisionedRootCertificates should contain provisioned root cert")
            asserts.assert_equal(found_certs[response2.caid], second_cert, "Expected matching certificate detail")

        self.step(11)
        find_response = await cr1_cmd.send_find_root_command(caid=response.caid)
        asserts.assert_equal(len(find_response.certificateDetails), 1, "Expected single certificate")
        asserts.assert_equal(find_response.certificateDetails[0].certificate, first_cert)

        self.step(12)
        if not self.skip_cr2:
            find_response2 = await cr2_cmd.send_find_root_command(caid=response2.caid)
            asserts.assert_equal(len(find_response2.certificateDetails), 1, "Expected single certificate")
            asserts.assert_equal(find_response2.certificateDetails[0].certificate, second_cert, "Expected matching certificate detail")

        self.step(13)
        find_all_response = await cr1_cmd.send_find_root_command()
        asserts.assert_greater_equal(len(find_all_response.certificateDetails), 1, "Expected at least 1 certificates")
        found_certs = dict()
        for cert in find_all_response.certificateDetails:
            found_certs[cert.caid] = cert.certificate
        asserts.assert_in(response.caid, found_certs, "FindRootCertificate should contain provisioned root cert")
        asserts.assert_equal(found_certs[response.caid], first_cert, "Expected matching certificate detail")

        self.step(14)
        if not self.skip_cr2:
            find_all_response = await cr2_cmd.send_find_root_command()
            asserts.assert_greater_equal(len(find_all_response.certificateDetails), 1, "Expected at least 1 certificates")
            found_certs = dict()
            for cert in find_all_response.certificateDetails:
                found_certs[cert.caid] = cert.certificate
            asserts.assert_in(response2.caid, found_certs, "FindRootCertificate should contain provisioned root cert")
            asserts.assert_equal(found_certs[response2.caid], second_cert, "Expected matching certificate detail")

        self.step(15)
        await cr1_cmd.send_remove_root_command(caid=response.caid)
        if not self.skip_cr2:
            await cr2_cmd.send_remove_root_command(caid=response2.caid)
        resp = await self.send_single_cmd(cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_index_cr2), endpoint=0)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk)


if __name__ == "__main__":
    default_matter_test_main()
