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
from typing import Optional, Union

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
        try:
            self._working_directory.cleanup()
        except Exception:
            pass

    def _get_json(self, endpoint: str) -> dict:
        url = f"{self.base_url}{endpoint}"
        response = requests.get(url, verify=False, timeout=5)
        response.raise_for_status()
        return response.json()

    def _post_json(self, endpoint: str, data: Optional[dict] = None) -> dict:
        url = f"{self.base_url}{endpoint}"
        response = requests.post(url, json=data or {}, verify=False, timeout=5)
        response.raise_for_status()
        return response.json()

    def get_root_cert(self) -> bytes:
        """Retrieve the root certificate in DER format."""
        response = self._get_json("/certs/server/root.pem")
        root_cert_pem = response["cert"]["public_cert"].encode("utf-8")
        root_cert = x509.load_pem_x509_certificate(root_cert_pem)
        return root_cert.public_bytes(encoding=serialization.Encoding.DER)

    def get_device_certificate(self, device_name: str = "DUT") -> bytes:
        """Retrieve a device certificate in DER format."""
        response = self._get_json(f"/certs/device/{device_name}.pem")
        device_cert_pem = response["cert"]["public_cert"].encode("utf-8")
        device_cert = x509.load_pem_x509_certificate(device_cert_pem)
        return device_cert.public_bytes(encoding=serialization.Encoding.DER)

    def sign_csr(self, csr_der: bytes, device_name: str = "DUT") -> dict:
        """Submit a CSR for signing and return the server's response."""
        csr = x509.load_der_x509_csr(csr_der)
        csr_pem = csr.public_bytes(serialization.Encoding.PEM).decode("utf-8")
        return self._post_json(f"/certs/{device_name}/sign", {"csr": csr_pem})

    def create_stream(self) -> str:
        """Request the server to create a new stream."""
        response = self._post_json("/streams")
        return response["stream_id"]


class PAVSTIUtils:
    """Utils for Push AV TC's TLS requirements."""

    def assert_valid_caid(self, caid: int) -> None:
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    def assert_valid_ccdid(self, ccdid: int) -> None:
        asserts.assert_greater_equal(ccdid, 0, "Invalid CCDID returned")
        asserts.assert_less_equal(ccdid, 65534, "Invalid CCDID returned")

    def assert_valid_csr(
        self,
        response: Clusters.TlsCertificateManagement.Commands.TLSClientCSRResponse,
        nonce: bytes,
    ) -> None:
        try:
            temp, _ = der_decoder(response.csr, asn1Spec=rfc2986.CertificationRequest())
        except PyAsn1Error:
            asserts.fail("Unable to decode CSR - improperly formatted DER")
        layer1 = dict(temp)

        # Verify public key is 256 bytes
        csr = x509.load_der_x509_csr(response.csr)
        csr_pubkey = csr.public_key()

        # Ensure key size is 256 bits
        asserts.assert_equal(csr_pubkey.key_size, 256, "Incorrect key size")

        # Ensure signature algorithm is ecdsa-with-SHA256
        signature_algorithm = dict(layer1["signatureAlgorithm"])["algorithm"]
        asserts.assert_equal(
            signature_algorithm,
            rfc5480.ecdsa_with_SHA256,
            "CSR specifies incorrect signature key algorithm",
        )

        # Validate signature
        asserts.assert_true(csr.is_signature_valid, "CSR signature is invalid")

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

    # ----------------------------------------------------------------------
    # Command helpers
    # ----------------------------------------------------------------------

    async def send_provision_root_command(
        self, endpoint: int, certificate: bytes, expected_status: Status = Status.Success
    ) -> Union[
        Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificateResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.ProvisionRootCertificate(
                    certificate=certificate
                ),
                endpoint=endpoint,
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
        self, endpoint: int, caid: int, expected_status: Status = Status.Success
    ) -> InteractionModelError:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.RemoveRootCertificate(
                    caid=caid
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_csr_command(
        self, endpoint: int, nonce: bytes, expected_status: Status = Status.Success
    ) -> Union[
        Clusters.TlsCertificateManagement.Commands.TLSClientCSRResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.TLSClientCSR(
                    nonce=nonce
                ),
                endpoint=endpoint,
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
        self, endpoint: int, certificate: bytes, ccdid: int, expected_status: Status = Status.Success
    ) -> InteractionModelError:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.ProvisionClientCertificate(
                    ccdid=ccdid,
                    clientCertificateDetails=Clusters.TlsCertificateManagement.Structs.TLSClientCertificateDetailStruct(
                        clientCertificate=certificate
                    ),
                ),
                endpoint=endpoint,
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

    # ----------------------------------------------------------------------
    # Precondition setup
    # ----------------------------------------------------------------------

    async def precondition_provision_tls_endpoint(
        self, endpoint: int, server: PushAvServerProcess
    ) -> int:
        """Perform provisioning steps to set up TLS endpoint."""
        root_cert_der = server.get_root_cert()
        prc_result = await self.send_provision_root_command(endpoint=endpoint, certificate=root_cert_der)
        self.assert_valid_caid(prc_result.caid)

        csr_nonce = random.randbytes(32)
        csr_result = await self.send_csr_command(endpoint=endpoint, nonce=csr_nonce)
        self.assert_valid_ccdid(csr_result.ccdid)
        self.assert_valid_csr(csr_result, csr_nonce)

        server.sign_csr(csr_result.csr)
        device_cert_der = server.get_device_certificate()

        await self.send_provision_client_command(
            endpoint=endpoint, certificate=device_cert_der, ccdid=csr_result.ccdid
        )
        result = await self.send_provision_tls_endpoint_command(
            endpoint=endpoint,
            hostname=b"localhost",
            port=1234,
            expected_status=Status.Success,
            caid=prc_result.caid,
            ccdid=csr_result.ccdid,
        )
        return result.endpointID
