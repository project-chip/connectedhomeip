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

import ipaddress
import logging
import os
import random
import shutil
import tempfile
from typing import List, Optional, Union

import psutil
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
from matter.tlv import uint


class PushAvServerProcess(Subprocess):
    """Class for starting Push AV Server in a subprocess"""

    # Prefix for log messages from push av server
    PREFIX = b"[PUSH_AV_SERVER]"

    # By default this points to the push_av_server in Test Harness
    # TCs utilizing this should expect th_server_app_path otherwise
    DEFAULT_SERVER_PATH = "/root/apps/push_av_server/server.py"

    def __init__(
        self,
        server_path: str | None,
        port: int = 1234,
        host: str = "0.0.0.0",
    ):
        if server_path is None:
            logging.error(f"No path provided for Push AV Server, using the default path for TH: {self.DEFAULT_SERVER_PATH}")
            server_path = self.DEFAULT_SERVER_PATH
        self._working_directory = os.path.join(tempfile.gettempdir(), "pavstest")
        if os.path.exists(self._working_directory):
            shutil.rmtree(self._working_directory)
        os.makedirs(self._working_directory)
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
                self._working_directory,
            ]
        )

        # Start the server application
        super().__init__(
            *command,
            output_cb=lambda line, is_stderr: self.PREFIX + line,
        )

    def __del__(self):
        try:
            if os.path.exists(self._working_directory):
                shutil.rmtree(self._working_directory)
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

    def create_key_pair(self) -> None:
        """
        This method is a work around to create keys for camera-app
        as currently it tries to access from /tmp/pavstest/
        """
        self._post_json("/certs/dev/keypair")


class PAVSTIUtils:
    """Utils for Push AV TC's TLS requirements."""

    def assert_valid_caid(self, caid: int) -> None:
        asserts.assert_greater_equal(caid, 0, "Invalid CAID returned")
        asserts.assert_less_equal(caid, 65534, "Invalid CAID returned")

    def assert_valid_ccdid(self, ccdid: int) -> None:
        asserts.assert_greater_equal(ccdid, 0, "Invalid CCDID returned")
        asserts.assert_less_equal(ccdid, 65534, "Invalid CCDID returned")

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
        Clusters.TlsCertificateManagement.Commands.ClientCSRResponse,
        InteractionModelError,
    ]:
        try:
            result = await self.send_single_cmd(
                cmd=Clusters.TlsCertificateManagement.Commands.ClientCSR(
                    nonce=nonce
                ),
                endpoint=endpoint,
                payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD,
            )

            asserts.assert_true(
                type_matches(
                    result,
                    Clusters.TlsCertificateManagement.Commands.ClientCSRResponse,
                ),
                "Unexpected return type for TLSClientCSR",
            )
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            return e

    async def send_provision_client_command(
            self, endpoint: int, certificate: bytes, ccdid: int, intermediates: List[bytes] = [],
            expected_status: Status = Status.Success) -> InteractionModelError:
        try:
            result = await self.send_single_cmd(cmd=Clusters.TlsCertificateManagement.Commands.ProvisionClientCertificate(ccdid=ccdid, clientCertificate=certificate, intermediateCertificates=intermediates),
                                                endpoint=endpoint, payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)

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

    def _get_private_ip(self):
        candidates = {"192": [], "10": []}

        for iface, addrs in psutil.net_if_addrs().items():
            for addr in addrs:
                if addr.family.name == 'AF_INET':
                    ip = addr.address
                    ip_obj = ipaddress.ip_address(ip)
                    if ip_obj.is_private:
                        if ip.startswith("192.168."):
                            candidates["192"].append(ip)
                        elif ip.startswith("10."):
                            candidates["10"].append(ip)

        if candidates["192"]:
            return candidates["192"][0]
        if candidates["10"]:
            return candidates["10"][0]

        raise RuntimeError("No private IP found, specify using --string-arg host_ip <IPv4>")

    async def precondition_provision_tls_endpoint(
        self, endpoint: int, server: PushAvServerProcess, host_ip: str | None = None
    ) -> int:
        """Perform provisioning steps to set up TLS endpoint."""
        if host_ip is None:
            # If no host ip specified, try to get private ip
            # this is mainly required when running TCs in Test Harness
            logging.error("No host_ip provided in test arguments")
            host_ip = self._get_private_ip()
            logging.info(f"Using IP: {host_ip} as hostname to provision TLS Endpoint")

        # Create Kep Pair for camera as it currently tries to access it from /tmp/pavstest when uploading.
        # TODO: Remove once camera-app supports TLS
        server.create_key_pair()
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
            hostname=host_ip.encode('utf-8'),
            port=1234,
            expected_status=Status.Success,
            caid=prc_result.caid,
            ccdid=csr_result.ccdid,
        )
        return result.endpointID, host_ip
