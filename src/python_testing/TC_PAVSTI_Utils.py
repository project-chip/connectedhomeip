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
from typing import Optional

import psutil
import requests
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from TC_TLS_Utils import TLSUtils

from matter.interaction_model import Status
from matter.testing.tasks import Subprocess

log = logging.getLogger(__name__)


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
        server_ip: str | None = None,
    ):
        if server_path is None:
            log.error(f"No path provided for Push AV Server, using the default path for TH: {self.DEFAULT_SERVER_PATH}")
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
                "--strict-mode"
            ]
        )

        if server_ip:
            command.extend(
                [
                    "--server-ip",
                    server_ip
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

    def update_track_name(self, stream_id: str, trackName: str) -> None:
        """
            Request the server to add a track name associated with stream_id.
            This is required to validate trackName of the segments that are uploaded.
        """
        self._post_json(endpoint=f"/streams/{stream_id}/trackName", data={"trackName": trackName})


class PAVSTIUtils:
    """Utils for Push AV TC's TLS requirements."""

    # ----------------------------------------------------------------------
    # Precondition setup
    # ----------------------------------------------------------------------

    def get_private_ip(self):
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
            log.error("No host_ip provided in test arguments")
            host_ip = self.get_private_ip()

        tls_utils = TLSUtils(self, endpoint=endpoint)
        log.info(f"Using IP: {host_ip} as hostname to provision TLS Endpoint")
        root_cert_der = server.get_root_cert()
        prc_result = await tls_utils.send_provision_root_command(certificate=root_cert_der)
        tls_utils.assert_valid_caid(prc_result.caid)

        csr_nonce = random.randbytes(32)
        csr_result = await tls_utils.send_csr_command(nonce=csr_nonce)
        tls_utils.assert_valid_ccdid(csr_result.ccdid)
        tls_utils.assert_valid_csr(csr_result, csr_nonce)

        server.sign_csr(csr_result.csr)
        device_cert_der = server.get_device_certificate()

        await tls_utils.send_provision_client_command(
            certificate=device_cert_der, ccdid=csr_result.ccdid
        )
        result = await tls_utils.send_provision_tls_endpoint_command(
            hostname=host_ip.encode('utf-8'),
            port=1234,
            expected_status=Status.Success,
            caid=prc_result.caid,
            ccdid=csr_result.ccdid,
        )
        return result.endpointID, host_ip
