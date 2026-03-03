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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --secured-device-port 5540
#       --autoApplyImage
#       --KVS /tmp/chip_kvs_requestor
#       --requestorCanConsent true
#       --userConsentState granted
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --nodeId 2
#       --admin-vendor-id 65521
#       --int-arg product-id:32769
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --string-arg provider_app_path:${OTA_PROVIDER_APP}
#       --string-arg ota_image:${SU_OTA_REQUESTOR_V2}
#       --int-arg ota_provider_port:5541
#     factory-reset: true
#     quiet: false
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import tempfile
import subprocess
from pathlib import Path
import queue
import socket
import textwrap
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_SU_2_3(SoftwareUpdateBaseTest):

    def desc_TC_SU_2_3(self) -> str:
        return "[TC-SU-2.3] Transfer of Software Update Images between DUT and TH/OTA-P"

    def pics_TC_SU_2_3(self):
        """Return the PICS definitions associated with this test."""
        return [
            "MCORE.OTA",
            "MCORE.OTA.HTTPS"
        ]

    async def wait_for_requestor_state(self, event_cb, target_state, timeout_sec=120.0):
        """
        Espera hasta recibir un StateTransition cuyo newState == target_state.
        Ignora cualquier otro StateTransition que llegue antes.
        """
        deadline = time.time() + timeout_sec
        while True:
            remaining = deadline - time.time()
            if remaining <= 0:
                asserts.fail(f"Timeout esperando StateTransition a {target_state}")

            try:
                ev = event_cb.event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                asserts.fail(f"Timeout esperando StateTransition a {target_state}")

            data = ev.Data
            if getattr(data, "newState", None) == target_state:
                return data

    class SimpleHttpsServer:
        def __init__(self, serve_dir: str, port: int, bind: str = "0.0.0.0"):
            self.serve_dir = serve_dir
            self.port = port
            self.bind = bind
            self.proc = None
            self.tmp = None
            self.cert_path = None
            self.key_path = None
            self.access_log = None

        def _run_cmd(self, cmd, timeout=30):
            p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=timeout)
            if p.returncode != 0:
                raise RuntimeError(f"Command failed: {' '.join(cmd)}\nstdout={p.stdout}\nstderr={p.stderr}")
            return p.stdout

        def _wait_port(self, host, port, timeout=3.0):
            deadline = time.time() + timeout
            last_err = None
            while time.time() < deadline:
                try:
                    with socket.create_connection((host, port), timeout=0.3):
                        return
                except OSError as e:
                    last_err = e
                    time.sleep(0.1)
            raise RuntimeError(f"HTTPS server not listening on {host}:{port} (last error: {last_err})")

        def start(self):
            self.tmp = tempfile.TemporaryDirectory(prefix="tc_su_2_3_https_")
            tmpdir = Path(self.tmp.name)
            self.cert_path = str(tmpdir / "cert.pem")
            self.key_path = str(tmpdir / "key.pem")
            self.access_log = str(tmpdir / "access.log")

            # Self-signed cert (CN=localhost)
            self._run_cmd([
                "openssl", "req", "-x509", "-newkey", "rsa:2048", "-sha256", "-days", "1",
                "-nodes",
                "-subj", "/CN=localhost",
                "-addext", "subjectAltName = DNS:localhost,IP:127.0.0.1",
                "-keyout", self.key_path,
                "-out", self.cert_path,
            ])

            # Custom HTTP handler with access log
            handler_py = tmpdir / "https_server.py"
            handler_py.write_text(textwrap.dedent("""\
            import http.server, ssl, sys, os, time
            from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer

            ACCESS_LOG = os.environ.get("ACCESS_LOG", "access.log")

            class Handler(SimpleHTTPRequestHandler):
                def log_message(self, fmt, *args):
                    line = "%s - - [%s] %s\\n" % (
                        self.client_address[0],
                        time.strftime("%d/%b/%Y:%H:%M:%S"),
                        fmt % args,
                    )
                    with open(ACCESS_LOG, "a", encoding="utf-8") as f:
                        f.write(line)

            def main():
                bind = sys.argv[1]
                port = int(sys.argv[2])
                cert = sys.argv[3]
                key = sys.argv[4]

                httpd = ThreadingHTTPServer((bind, port), Handler)
                ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
                ctx.load_cert_chain(certfile=cert, keyfile=key)
                httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
                httpd.serve_forever()

            if __name__ == "__main__":
                main()
            """), encoding="utf-8")

            env = os.environ.copy()
            env["ACCESS_LOG"] = self.access_log

            # Serve directory must be current working directory for SimpleHTTPRequestHandler
            self.proc = subprocess.Popen(
                ["python3", str(handler_py), self.bind, str(self.port), self.cert_path, self.key_path],
                cwd=self.serve_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
                text=True
            )

            time.sleep(0.2)
            rc = self.proc.poll()
            if rc is not None:
                out, err = self.proc.communicate(timeout=1)
                raise RuntimeError(
                    f"HTTPS server failed to start (rc={rc}).\nstdout:\n{out}\nstderr:\n{err}"
                )

            self._wait_port("127.0.0.1", self.port, timeout=3.0)

        def stop(self):
            if self.proc is not None:
                self.proc.terminate()
                try:
                    self.proc.wait(timeout=3)
                except Exception:
                    self.proc.kill()
            self.proc = None
            if self.tmp is not None:
                self.tmp.cleanup()
                self.tmp = None

        def read_access_log(self) -> str:
            if not self.access_log or not os.path.exists(self.access_log):
                return ""
            with open(self.access_log, "r", encoding="utf-8") as f:
                return f.read()

    def steps_TC_SU_2_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.",
                     "Verify that the DUT obtains the User Consent from the user prior to transfer of software update image. This step is vendor specific."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI to the location where the image is located.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT. "
                     "Verify that the Maximum Block Size requested by DUT should be: "
                     "- no larger than 1024 (2^10) bytes over non-TCP transports. "
                     "- no larger than 8192 (2^13) bytes over TCP transport."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI with the https url of the software image.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT "
                     "from the https url and not from the OTA-P."),
            TestStep(4, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Wait for the Idle timeout so that reading the UpdateState Attribute of the OTA Requestor returns the value as Idle. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P.",
                     "Verify that the BDX Idle timeout should be no less than 5 minutes. "
                     "Verify that the DUT starts a new transfer of software image when sending another QueryImage request."),
            TestStep(5, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P. "
                     "Set the RC[STARTOFS] bit and associated STARTOFS field in the ReceiveInit Message to indicate the resumption of a transfer previously aborted.",
                     "ImageURI should have the https url from where the image can be downloaded.",
                     "Verify that the DUT starts receiving the rest of the software image after resuming the image transfer.")
        ]

    @async_test_body
    async def teardown_test(self):
        if hasattr(self, "https_srv") and self.https_srv is not None:
            logger.info("Stopping HTTPS server")
            self.https_srv.stop()
            self.https_srv = None
        if hasattr(self, "current_provider_app_proc") and self.current_provider_app_proc is not None:
            logger.info("Terminating existing OTA Provider")
            self.current_provider_app_proc.terminate()
            self.current_provider_app_proc = None
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_3(self):

        requestorCluster = Clusters.Objects.OtaSoftwareUpdateRequestor

        self.LOG_FILE_PATH = "provider.log"
        self.KVS_PATH = "/tmp/chip_kvs_provider"
        self.provider_app_path = self.user_params.get('provider_app_path', None)
        if not self.provider_app_path:
            asserts.fail("Missing provider app path.")
        self.ota_image = self.user_params.get('ota_image')
        self.endpoint = self.get_endpoint()

        self.fifo_in = self.user_params.get('app_pipe', None)
        if not self.fifo_in:
            asserts.fail("Fifo input missing. Speficy using --string-arg app_pipe:<FIFO_APP_PIPE_INPUT>")

        self.fifo_out = self.user_params.get('app_pipe_out', None)
        if not self.fifo_out:
            asserts.fail("Fifo output missing. Speficy using --string-arg app_pipe_out:<FIFO_APP_PIPE_OUTPUT>")

        self.step(0)

        # Requestor (DUT) info
        controller = self.default_controller
        requestor_node_id = self.dut_node_id

        # Provider info
        provider_node_id = 1
        provider_discriminator = 1111
        provider_setupPinCode = 20202021
        provider_port = self.user_params.get('ota_provider_port', 5541)

        self.provider_data = {
            "discriminator": provider_discriminator,
            "setup_pincode": provider_setupPinCode,
            "port": provider_port,
        }

        # DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.
        self.step(1)

        # userConsentNeeded flag
        provider_extra_args_updateconsent = [
            '--userConsentNeeded',
            '--app-pipe', self.fifo_in, '--app-pipe-out', self.fifo_out
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=provider_extra_args_updateconsent,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

        # Commission Provider (Only one time)
        logger.info("Commissioning provider/TH")
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'Provider Commissioning response: {resp}')

        logger.info("Starting EventSubscription handlers")
        event_cb = EventSubscriptionHandler(expected_cluster=Clusters.Objects.OtaSoftwareUpdateRequestor,
                                            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id)
        await event_cb.start(dev_ctrl=controller, node_id=requestor_node_id, endpoint=self.endpoint, fabric_filtered=False, min_interval_sec=0, max_interval_sec=5)

        await self.create_acl_entry(dev_ctrl=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)

        await self.set_default_ota_providers_list(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await self.announce_ota_provider(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await asyncio.sleep(0.5)

        # Waiting for querying event
        querying_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info(f"Quering event: {querying_event}")
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kQuerying, querying_event.newState,
                             f"New state is {querying_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kQuerying}")

        # Waiting for downloading event
        downloading_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info(f"Downloading event: {downloading_event}")
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kDownloading, downloading_event.newState,
                             f"New state is {downloading_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kDownloading}")

        # Getting QueryImageSnapshot using out-of-band communication channel
        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logger.info(f"Out of band command response: {response_data}")

        # Verify that the DUT obtains the User Consent from the user prior to transfer of software update image
        user_consent_needed = response_data['Payload']['UserConsentNeeded']
        asserts.assert_true(user_consent_needed, "UserConsentNeeded should be True")

        self.terminate_provider()

        # Waiting foe idle state (back to normal)
        logger.info("Waiting for idle state")
        idle_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info(f"Idle: {idle_event}")
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kIdle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kIdle}")

        # DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to "UpdateAvailable". Set ImageURI to the location where the image is located.
        self.step(2)

        provider_extra_args_updateAvailable = [
            '--queryImageStatus', 'updateAvailable',
            '--app-pipe', self.fifo_in, '--app-pipe-out', self.fifo_out
        ]

        self.start_provider(
            provider_app_path=self.provider_app_path,
            ota_image_path=self.ota_image,
            setup_pincode=self.provider_data["setup_pincode"],
            discriminator=self.provider_data["discriminator"],
            port=self.provider_data["port"],
            extra_args=provider_extra_args_updateAvailable,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH,
            expected_output="Server initialization complete",
            timeout=30
        )

        await self.announce_ota_provider(controller=controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id, endpoint=self.endpoint)

        await asyncio.sleep(0.5)

        # Waiting for downloading state
        downloading = await self.wait_for_requestor_state(
            event_cb,
            requestorCluster.Enums.UpdateStateEnum.kDownloading,
            timeout_sec=120
        )
        logger.info(f"Reached Downloading (ignoring intermediate states). Event: {downloading}")

        # Getting QueryImageSnapshot using out-of-band communication channel
        command = {"Name": "QueryImageSnapshot", "Cluster": "OtaSoftwareUpdateProvider", "Endpoint": self.endpoint}
        self.write_to_app_pipe(command, self.fifo_in)
        response_data = self.read_from_app_pipe(self.fifo_out)

        logger.info(f"Out of band command response: {response_data}")

        block_size = response_data['Payload']['BlockSize']
        logger.info(f"Block size: {block_size}")

        # TODO: Get transport protocol
        # Verify the Maximum Block Size requested by DUT
        tcp = True
        max_block_size_tcp = 8192
        max_block_size_non_tcp = 1024

        if tcp:
            asserts.assert_less_equal(block_size, max_block_size_tcp,
                                      f"{block_size} should be less than {max_block_size_tcp} bytes in TCP transport")
        else:
            asserts.assert_less_equal(block_size, max_block_size_non_tcp,
                                      f"{block_size} should be less than {max_block_size_non_tcp} bytes in non TCP transport")

        self.terminate_provider()

        # Waiting foe idle state (back to normal)
        logger.info("Waiting for idle state")
        idle_event = event_cb.wait_for_event_report(Clusters.Objects.OtaSoftwareUpdateRequestor.Events.StateTransition, 50)
        logger.info(f"Idle: {idle_event}")
        asserts.assert_equal(requestorCluster.Enums.UpdateStateEnum.kIdle, idle_event.newState,
                             f"New state is {idle_event.newState} and it should be {requestorCluster.Enums.UpdateStateEnum.kIdle}")

        await asyncio.sleep(5)

        # DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to "UpdateAvailable". Set ImageURI with the https url of the software image.
        self.step(3)

        if self.pics_guard(self.check_pics("MCORE.OTA.HTTPS")):
            https_port = self.user_params.get("https_port", 8443)
            ota_path = Path(self.ota_image).expanduser()

            asserts.assert_true(ota_path.exists(), f"ota_image does not exist: {ota_path}")
            asserts.assert_true(ota_path.is_file(), f"ota_image is not a file: {ota_path}")

            serve_dir = str(ota_path.parent.resolve())
            image_filename = ota_path.name

            self.https_srv = self.SimpleHttpsServer(serve_dir=serve_dir, port=https_port, bind="127.0.0.1")
            self.https_srv.start()

            # Choose host:
            # - If the DUT is in the same machine, "localhost" works.
            # - If the DUT is remote, you will need a IP/hostname reachable for the DUT.
            https_host = self.user_params.get("https_host", "127.0.0.1")

            image_uri = f"https://{https_host}:{https_port}/{image_filename}"
            logger.info(f"Serving OTA image over HTTPS: {image_uri}")

            provider_extra_args_https = [
                "--queryImageStatus", "updateAvailable",
                "--imageUri", image_uri,
                "--app-pipe", self.fifo_in, "--app-pipe-out", self.fifo_out
            ]

            self.start_provider(
                provider_app_path=self.provider_app_path,
                ota_image_path=self.ota_image,
                setup_pincode=self.provider_data["setup_pincode"],
                discriminator=self.provider_data["discriminator"],
                port=self.provider_data["port"],
                extra_args=provider_extra_args_https,
                kvs_path=self.KVS_PATH,
                log_file=self.LOG_FILE_PATH,
                timeout=30
            )

            await self.announce_ota_provider(
                controller=controller,
                provider_node_id=provider_node_id,
                requestor_node_id=requestor_node_id,
                endpoint=self.endpoint
            )

            await asyncio.sleep(3)

            # Waiting for downloading state
            downloading = await self.wait_for_requestor_state(
                event_cb,
                requestorCluster.Enums.UpdateStateEnum.kDownloading,
                timeout_sec=120
            )
            logger.info(f"Reached Downloading (ignoring intermediate states). Event: {downloading}")

            await asyncio.sleep(0.5)

            deadline = time.time() + 60
            while time.time() < deadline:
                access_log = self.https_srv.read_access_log()
                if f"GET /{image_filename}" in access_log:
                    break
                await asyncio.sleep(0.5)

            logger.info(f"HTTPS access log:\n{access_log}")
            asserts.assert_true(
                f"GET /{image_filename}" in access_log,
                "Expected DUT to download the image via HTTPS (no GET seen in HTTPS server access log)."
            )

            # # Check there are no BDX logs
            # try:
            #     with open(self.LOG_FILE_PATH, "r", encoding="utf-8") as f:
            #         prov_log = f.read()
            #     suspicious = ["ReceiveInit", "SendInit", "BlockQuery", "TransferSession", "BDX"]
            #     bdx_hits = [p for p in suspicious if p in prov_log]
            #     asserts.assert_true(
            #         len(bdx_hits) == 0,
            #         f"Provider log suggests BDX transfer happened (should NOT in HTTPS step). Hits={bdx_hits}"
            #     )
            # except FileNotFoundError:
            #     logger.warning("Provider log file not found; skipping BDX log verification.")

            self.https_srv.stop()

        self.step(4)
        # NOTE: Step 4 behavior is already covered by TC_SU_2_2 Step #1 (idle transition after cancel)
        # and TC_SU_2_7 Step #4 (forced cancel and 5-min timeout validation).
        # Reference PR: #685 for the provider cancelation implementation.
        self.step(5)
        # NOTE: Step skipped not implemented in spec.


if __name__ == "__main__":
    default_matter_test_main()
