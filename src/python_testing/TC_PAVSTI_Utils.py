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

import random
import tempfile
from typing import Union

import requests
from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils
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
from matter.testing.matter_testing import type_matches
from matter.testing.tasks import Subprocess
from matter.tlv import TLVWriter, uint


class PushAvServerProcess(Subprocess):
    """Class for starting Push AV Server in a subprocess"""

    # Prefix for log messages from push av server
    PREFIX = b"[PUSH_AV_SERVER]"

    # By default this points to the push_av_server in Test Harness
    # TCs utilizing this should expect th_server_app_path otherwise
    DEFAULT_SERVER_PATH = "/root/apps/push_av_server/server.py"

    def __init__(
        self,
        server_path: str = DEFAULT_SERVER_PATH,
        port: int = 1234,
        host: str = "0.0.0.0",
    ):
        self._working_directory = tempfile.TemporaryDirectory(prefix="pavs-")
        self.host = host
        self.port = port
        self.base_url = f"https://{self.host}:{self.port}"
        # Build the command list
        command = ["python3", server_path]
        command.extend(
            [
                "--host",
                str(self.host),
                "--working-directory",
                self._working_directory.name,
            ]
        )

        # Start the server application
        super().__init__(
            *command,
            output_cb=lambda line, is_stderr: self.PREFIX + line,
        )

    def __del__(self):
        self._working_directory.cleanup()

    def get_root_cert(self):
        response = requests.get(f"{self.base_url}/certs/server/root.pem", verify=False).json()
        root_cert_response = response["cert"]["public_cert"]
        root_cert_pem = x509.load_pem_x509_certificate(
            root_cert_response.encode("utf-8")
        )
        root_cert_der = root_cert_pem.public_bytes(encoding=serialization.Encoding.DER)
        return root_cert_der

    def get_device_certificate(self, device_name: str = "DUT"):
        client_cert_response = requests.get(
            f"{self.base_url}/certs/device/{device_name}.pem", verify=False
        ).json()
        device_cert = client_cert_response["cert"]["public_cert"]
        device_cert_pem = x509.load_pem_x509_certificate(device_cert.encode("utf-8"))
        device_cert_der = device_cert_pem.public_bytes(
            encoding=serialization.Encoding.DER
        )
        return device_cert_der

    def sign_csr(self, csr_der, device_name: str = "DUT"):
        # Load DER CSR
        csr = x509.load_der_x509_csr(csr_der)
        csr_pem = csr.public_bytes(serialization.Encoding.PEM).decode("utf-8")

        # Sign the request
        # Returns the certificate path that is created in server's working directory
        csr_response = requests.post(
            f"{self.base_url}/certs/{device_name}/sign", json={"csr": csr_pem}, verify=False
        ).json()

        return csr_response

    def create_stream(self):
        response = requests.post(
            f"{self.base_url}/streams", verify=False
        ).json()
        return response['stream_id']


class PAVSTIUtils:
    """Utils Push AV TCs for TLS requirements"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def assert_valid_caid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    def assert_valid_ccdid(self, caid):
        asserts.assert_greater_equal(caid, 0, "Invalid CCDID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CCDID returned")

    def assert_valid_csr(
        self,
        response: Clusters.TlsCertificateManagement.Commands.TLSClientCSRResponse,
        nonce: bytes,
    ):
        try:
            temp, _ = der_decoder(response.csr, asn1Spec=rfc2986.CertificationRequest())
        except PyAsn1Error:
            asserts.fail("Unable to decode CSR - improperly formatted DER")
        layer1 = dict(temp)

        # Verify public key is 256 bytes
        csr = x509.load_der_x509_csr(response.csr)
        csr_pubkey = csr.public_key()
        asserts.assert_equal(csr_pubkey.key_size, 256, "Incorrect key size")

        # Verify signature algorithm is ecdsa-with-SHA156
        signature_algorithm = dict(layer1["signatureAlgorithm"])["algorithm"]
        asserts.assert_equal(
            signature_algorithm,
            rfc5480.ecdsa_with_SHA256,
            "CSR specifies incorrect signature key algorithm",
        )

        # Verify signature is valid
        asserts.assert_true(csr.is_signature_valid, "Signature is invalid")

        # Verify response.nonce is octet string of length 32
        try:
            # response.nonce is an octet string if it can be converted to an int
            int(hex_from_bytes(response.nonce), 16)
        except ValueError:
            asserts.fail("Returned CSR nonce is not an octet string")

        # Verify response.nonce is valid signature
        nocsr_elements = dict(
            [
                (1, response.csr),
                (2, nonce),
            ]
        )
        writer = TLVWriter()
        writer.put(None, nocsr_elements)

        baselen = curve_by_name("NIST256p").baselen
        signature_raw_r = int(hex_from_bytes(response.nonce[:baselen]), 16)
        signature_raw_s = int(hex_from_bytes(response.nonce[baselen:]), 16)

        nocsr_signature = utils.encode_dss_signature(signature_raw_r, signature_raw_s)
        csr_pubkey.verify(
            signature=nocsr_signature,
            data=writer.encoding,
            signature_algorithm=ec.ECDSA(hashes.SHA256()),
        )

    async def send_provision_root_command(
        self, certificate: bytes, expected_status: Status = Status.Success
    ) -> Union[
        Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(
                    certificate=certificate
                ),
                endpoint=self.endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )

            asserts.assert_true(
                type_matches(
                    result,
                    Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse,
                ),
                "Unexpected return type for ProvisionRootCertificate",
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_remove_root_command(
        self, caid: int, expected_status: Status = Status.Success
    ) -> InteractionModelError:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.RemoveRootCertificate(
                    caid=caid
                ),
                endpoint=self.endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_csr_command(
        self, nonce: bytes, expected_status: Status = Status.Success
    ) -> Union[
        Clusters.TlsCertificateManagement.Commands.TLSClientCSRResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.TLSClientCSR(
                    nonce=nonce
                ),
                endpoint=self.endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )

            asserts.assert_true(
                type_matches(
                    result,
                    Clusters.TlsCertificateManagement.Commands.TLSClientCSRResponse,
                ),
                "Unexpected return type for TLSClientCSR",
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_provision_client_command(
        self, certificate: bytes, ccdid: int, expected_status: Status = Status.Success
    ) -> InteractionModelError:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.ProvisionClientCertificate(
                    ccdid=ccdid,
                    clientCertificateDetails=Clusters.TlsCertificateManagement.Structs.TLSClientCertificateDetailStruct(
                        clientCertificate=certificate
                    ),
                ),
                endpoint=self.endpoint,
                node_id=self.node_id,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_provision_tls_endpoint_command(
        self,
        endpoint: int,
        hostname: bytes,
        port: uint,
        caid: uint,
        ccdid: Union[Nullable, uint] = NullValue,
        expected_status: Status = Status.Success,
    ) -> Union[
        Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsClientManagement.Commands.ProvisionEndpoint(
                    hostname=hostname, port=port, caid=caid, ccdid=ccdid
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )

            asserts.assert_true(
                type_matches(
                    result,
                    Clusters.TlsClientManagement.Commands.ProvisionEndpointResponse,
                ),
                "Unexpected return type for ProvisionEndpoint",
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def precondition_provision_tls_endpoint(self, endpoint: int, server: PushAvServerProcess):
        root_cert_der = server.get_root_cert()
        prc_result = await self.send_provision_root_command(certificate=root_cert_der)
        self.assert_valid_caid(prc_result.caid)

        csr_nonce = random.randbytes(32)
        csr_result = await self.send_csr_command(nonce=csr_nonce)
        self.assert_valid_ccdid(csr_result.ccdid)
        self.assert_valid_csr(csr_result, csr_nonce)

        server.sign_csr(csr_result.csr)
        device_cert_der = server.get_device_certificate()

        await self.send_provision_client_command(
            certificate=device_cert_der, ccdid=csr_result.ccdid
        )
        result = await self.send_provision_tls_endpoint_command(
            endpoint=endpoint,
            hostname=b"locahost",
            port=1234,
            expected_status=Status.Success,
            caid=prc_result.caid,
            ccdid=csr_result.ccdid,
        )
        return result.endpointID
