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

import datetime
import random
import string
from typing import Callable, List, Optional, Union

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils
from cryptography.x509 import CertificateBuilder, CertificateSigningRequest, random_serial_number
from cryptography.x509.oid import NameOID
from ecdsa.curves import curve_by_name
from mobly import asserts
from pyasn1.codec.der.decoder import decode as der_decoder
from pyasn1.error import PyAsn1Error
from pyasn1_modules import rfc2986, rfc5480

import matter.clusters as Clusters
import matter.testing.matchers as matchers
from matter import ChipDeviceCtrl
from matter.clusters.enum import MatterIntEnum
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.conversions import hex_from_bytes
from matter.testing.matter_testing import MatterBaseTest
from matter.tlv import uint


class TLSUtils:
    """Helper utility with local certificate generation & verification,
    and node/endpoint-scoped TLS commands.
    """

    def __init__(self, matter_test: MatterBaseTest, endpoint: Optional[int] = None,
                 dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None,
                 node_id: Optional[int] = None, fabric_index: Optional[int] = None):
        self.test = matter_test
        self.endpoint = endpoint
        self.dev_ctrl = dev_ctrl
        self.node_id = node_id
        self.fabric_index = fabric_index

    class CertWithKey:
        def __init__(self, cert: bytes, key: ec.EllipticCurvePrivateKey):
            self.cert = cert
            self.key = key

    def get_key(self) -> ec.EllipticCurvePrivateKey:
        return ec.generate_private_key(
            ec.SECP256R1()
        )

    def gen_cert(self, public_key: bytes | None = None) -> bytes:
        return self.gen_cert_with_key(self.get_key(), public_key)

    def gen_cert_chain(self, root: ec.EllipticCurvePrivateKey, length: int):
        signer = root
        result = []
        for i in range(length):
            key = self.get_key()
            cert = self.gen_cert_with_key(signer, key.public_key())
            result.append(self.CertWithKey(cert, key))
            signer = key
        return result

    def gen_cert_with_key(self, signer: ec.EllipticCurvePrivateKey, public_key: bytes | None = None,
                          subject: x509.Name | None = None, builder_lambda: Callable[
            [], CertificateBuilder] = lambda: CertificateBuilder()) -> bytes:
        if not public_key:
            public_key = signer.public_key()

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
        if not subject:
            subject = root_cert_subject

        cert = (
            builder_lambda()
            .subject_name(subject)
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
                    signer.public_key()
                ),
                critical=False,
            )
            .sign(signer, hashes.SHA256())
        )
        return cert.public_bytes(serialization.Encoding.DER)

    def assert_valid_caid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    def assert_valid_ccdid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CCDID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CCDID returned")

    def assert_valid_csr(self, response: Clusters.TlsCertificateManagement.Commands.ClientCSRResponse, nonce: bytes) -> CertificateSigningRequest:
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

        nonce_signature = utils.encode_dss_signature(signature_raw_r, signature_raw_s)
        csr_pubkey.verify(signature=nonce_signature, data=nonce, signature_algorithm=ec.ECDSA(hashes.SHA256()))
        return csr

    def get_fingerprint(self, cert: bytes) -> bytes:
        return x509.load_der_x509_certificate(cert).fingerprint(hashes.SHA256())

    async def send_provision_root_command(
            self, certificate: bytes, caid: Union[Nullable, int] = NullValue,
            expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse, InteractionModelError]:
        try:
            result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(certificate=certificate, caid=caid),
                                                     endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse),
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
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.FindRootCertificateResponse),
                                "Unexpected return type for FindRootCertificate")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_lookup_root_command(
            self, fingerprint: bytes,
            expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.LookupRootCertificateResponse, InteractionModelError]:
        try:
            result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.LookupRootCertificate(fingerprint=fingerprint),
                                                     endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.LookupRootCertificateResponse),
                                "Unexpected return type for LookupRootCertificate")
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
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_csr_command(
            self, nonce: bytes, ccdid: Union[Nullable, int] = NullValue,
            expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.ClientCSRResponse, InteractionModelError]:
        try:
            result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ClientCSR(ccdid=ccdid, nonce=nonce),
                                                     endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.ClientCSRResponse),
                                "Unexpected return type for ClientCSR")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_provision_client_command(
            self, certificate: bytes, ccdid: int, intermediates: List[bytes] = [],
            expected_status: Status = Status.Success) -> InteractionModelError:
        try:
            result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionClientCertificate(ccdid=ccdid, clientCertificate=certificate, intermediateCertificates=intermediates),
                                                     endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")

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
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.FindClientCertificateResponse),
                                "Unexpected return type for FindClientCertificate")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_lookup_client_command(
            self, fingerprint: bytes,
            expected_status: Status = Status.Success) -> Union[Clusters.TlsCertificateManagement.Commands.LookupClientCertificateResponse, InteractionModelError]:
        try:
            result = await self.test.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.LookupClientCertificate(fingerprint=fingerprint),
                                                     endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsCertificateManagement.Commands.LookupClientCertificateResponse),
                                "Unexpected return type for LookupClientCertificate")
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
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def read_tls_cert_attribute(self, attribute, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD):
        cluster = Clusters.TlsCertificateManagement
        return await self.test.read_single_attribute_check_success(endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, cluster=cluster, attribute=attribute, payloadCapability=payloadCapability)

    async def read_tls_client_attribute(self, attribute, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD):
        cluster = Clusters.TlsClientManagement
        return await self.test.read_single_attribute_check_success(endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id, cluster=cluster, attribute=attribute, payloadCapability=payloadCapability)

    async def read_root_certs_attribute_as_map(self, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD):
        attribute_certs = await self.read_tls_cert_attribute(Clusters.TlsCertificateManagement.Attributes.ProvisionedRootCertificates, payloadCapability)
        matter_asserts.assert_list(attribute_certs, "Expected list")
        found_certs = {}
        for cert in attribute_certs:
            found_certs[cert.caid] = cert
        return found_certs

    async def read_client_certs_attribute_as_map(self, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD):
        attribute_certs = await self.read_tls_cert_attribute(Clusters.TlsCertificateManagement.Attributes.ProvisionedClientCertificates, payloadCapability)
        matter_asserts.assert_list(attribute_certs, "Expected list")
        found_certs = {}
        for cert in attribute_certs:
            found_certs[cert.ccdid] = cert
        return found_certs

    async def send_find_client_command_as_map(
            self,
            expected_status: Status = Status.Success):
        find_response = await self.send_find_client_command(expected_status=expected_status)
        found_certs = {}
        for cert in find_response.certificateDetails:
            found_certs[cert.ccdid] = cert
        return found_certs

    async def send_remove_fabric_command(self, fabric_index: int):
        resp = await self.test.send_single_cmd(cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_index), endpoint=0, dev_ctrl=self.dev_ctrl)
        asserts.assert_equal(
            resp.statusCode, Clusters.OperationalCredentials.Enums.NodeOperationalCertStatusEnum.kOk)

    async def send_provision_tls_endpoint_command(
        self,
        hostname: bytes,
        port: uint,
        caid: uint,
        endpoint_id: Union[Nullable, int] = NullValue,
        ccdid: Union[Nullable, uint] = NullValue,
        expected_status: Union[Status, MatterIntEnum] = Status.Success,
    ) -> Union[
        Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.test.send_single_cmd(
                cmd=Clusters.TlsClientManagement.Commands.ProvisionEndpoint(
                    hostname=hostname, port=port, caid=caid, ccdid=ccdid, endpointID=endpoint_id
                ),
                endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(
                matchers.is_type(
                    result,
                    Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse,
                ),
                "Unexpected return type for ProvisionEndpoint",
            )
            return result
        except InteractionModelError as e:
            if matchers.is_type(expected_status, Status):
                asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            else:
                asserts.assert_equal(e.status, Status.Failure, "Unexpected error returned")
                asserts.assert_equal(e.clusterStatus, expected_status, "Unexpected error returned")

            return e

    async def send_remove_tls_endpoint_command(
        self,
        endpoint_id: uint,
        expected_status: Status = Status.Success,
    ) -> InteractionModelError:
        try:
            result = await self.test.send_single_cmd(
                cmd=Clusters.TlsClientManagement.Commands.RemoveEndpoint(
                    endpointID=endpoint_id
                ),
                endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_find_tls_endpoint_command(
        self,
        endpoint_id: uint,
        expected_status: Status = Status.Success,
    ) -> InteractionModelError:
        try:
            result = await self.test.send_single_cmd(
                cmd=Clusters.TlsClientManagement.Commands.FindEndpoint(
                    endpointID=endpoint_id
                ),
                endpoint=self.endpoint, dev_ctrl=self.dev_ctrl, node_id=self.node_id,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            asserts.assert_equal(Status.Success, expected_status, "Unexpected success")
            asserts.assert_true(matchers.is_type(result, Clusters.TlsClientManagement.Commands.FindEndpointResponse),
                                "Unexpected return type for FindEndpoint")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e
