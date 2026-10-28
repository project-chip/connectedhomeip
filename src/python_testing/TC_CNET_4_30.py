#
#    Copyright (c) 2026 Project CHIP Authors
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

# The DUT is a Matter commissioner that originates the commands under test, so this test
# is driven by an operator working the DUT's own interface and is not run in CI.
#
# The TH server is the all-devices-app app with a Wi-Fi Network Commissioning cluster that
# supports Per-Device Credentials and an emulated Wi-Fi driver (--wifi-emulated). The DUT
# reaches the TH server over the network the TH is on.:
#
#   python3 src/python_testing/TC_CNET_4_30.py \
#       --storage-path admin_storage.json \
#       --PICS <the DUT's PICS file> \
#       --string-arg th_server_app_path:out/linux-x64-all-devices/all-devices-app

import asyncio
import logging
import os
import random
import re
import tempfile
import threading
import time

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.testing.apps import AppServerSubprocess
from matter.testing.commissioning import SetupParameters
from matter.testing.decorators import async_test_body, pics
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterTestCommissioner
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning

# Identity of the harness-owned fabric used to observe the TH server.
_HARNESS_VENDOR_ID = 0xFFF1
_HARNESS_FABRIC_ID = 2
_HARNESS_CONTROLLER_NODE_ID = 112233
_TH_SERVER_NODE_ID = 1111

# The DUT is driven by a human operator, so the window has to stay open long enough for
# them to work through the vendor's commissioning UX.
_COMMISSIONING_WINDOW_TIMEOUT_SECONDS = 900
_COMMISSIONING_WINDOW_ITERATIONS = 10000

# How long to wait for the TH server's record of a command after the operator says the DUT has
# sent it.
_EVIDENCE_TIMEOUT_SECONDS = 30
_POLL_INTERVAL_SECONDS = 0.5

# Logged by the interaction model for every command the TH server dispatches to a cluster.
_RECEIVED_COMMAND_PATTERN = re.compile(
    rb"Received command for Endpoint=(\d+) Cluster=0x([0-9A-F]{4})_([0-9A-F]{4}) "
    rb"Command=0x([0-9A-F]{4})_([0-9A-F]{4})")

_PDC_FEATURES = cnet.Bitmaps.Feature.kWiFiNetworkInterface | cnet.Bitmaps.Feature.kPerDeviceCredentials


class TC_CNET_4_30(MatterTestCommissioner):

    # The harness never opens a Matter session to the DUT: the DUT originates every command in
    # this test and is driven through its own interface, while all of the harness's traffic
    # goes to the TH server.
    requires_dut = False

    @property
    def default_timeout(self) -> int:
        # Every step waits on a human driving the DUT's commissioning and command UX.
        return 3600

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.th_server = None
        self.storage = None
        self.networks_subscription = None
        # (endpoint, cluster, command) of each command the TH server has received, in order.
        self._received_commands: list[tuple[int, int, int]] = []
        self._received_commands_lock = threading.Lock()

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a TH server app with a Wi-Fi Network Commissioning cluster that supports "
                         "Per-Device Credentials. Specify the path with --string-arg th_server_app_path:<path_to_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        log.info("Temporary storage directory: %s", self.storage.name)

        self.th_server_setup_params = SetupParameters(
            discriminator=random.randint(0, 4095),
            passcode=20202021)
        self.th_server = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            port=self.user_params.get("th_server_port", 5543),
            discriminator=self.th_server_setup_params.discriminator,
            passcode=self.th_server_setup_params.passcode,
            extra_args=["--wifi-emulated"])
        forward_output = self.th_server.output_cb
        self.th_server.output_cb = lambda line, is_stderr: forward_output(self._record_command(line), is_stderr)
        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

        certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin = certificate_authority.NewFabricAdmin(vendorId=_HARNESS_VENDOR_ID, fabricId=_HARNESS_FABRIC_ID)
        self.th_server_controller = fabric_admin.NewController(
            nodeId=_HARNESS_CONTROLLER_NODE_ID,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        await self.th_server_controller.CommissionOnNetwork(
            nodeId=_TH_SERVER_NODE_ID,
            setupPinCode=self.th_server_setup_params.passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_setup_params.discriminator)
        log.info("Commissioned the TH server onto the harness fabric")

    def teardown_class(self):
        if self.networks_subscription is not None:
            self.networks_subscription.cancel()
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    def _record_command(self, line: bytes) -> bytes:
        if match := _RECEIVED_COMMAND_PATTERN.search(line):
            endpoint = int(match[1])
            cluster = int(match[2] + match[3], 16)
            command = int(match[4] + match[5], 16)
            with self._received_commands_lock:
                self._received_commands.append((endpoint, cluster, command))
        return line

    def _forget_received_commands(self) -> None:
        with self._received_commands_lock:
            self._received_commands.clear()

    async def _find_wifi_pdc_endpoint(self) -> int:
        """Returns the TH server endpoint hosting a Network Commissioning cluster with Wi-Fi and PDC."""
        read = await self.th_server_controller.ReadAttribute(_TH_SERVER_NODE_ID, [cnet.Attributes.FeatureMap])
        endpoints = [endpoint for endpoint, clusters in read.items()
                     if clusters[cnet][cnet.Attributes.FeatureMap] & _PDC_FEATURES == _PDC_FEATURES]
        asserts.assert_true(endpoints,
                            "The TH server app does not host a Network Commissioning cluster with the WI and PDC "
                            "features. Build it with chip_device_config_enable_wifi_pdc=true.")
        return endpoints[0]

    async def _ask_dut_to_commission_th_server(self) -> None:
        params = await self.th_server_controller.OpenCommissioningWindow(
            nodeId=_TH_SERVER_NODE_ID,
            timeout=_COMMISSIONING_WINDOW_TIMEOUT_SECONDS,
            iteration=_COMMISSIONING_WINDOW_ITERATIONS,
            discriminator=random.randint(0, 4095),
            option=self.th_server_controller.CommissioningWindowPasscode.kTokenWithRandomPin)

        self.wait_for_user_input(
            prompt_msg="Using the DUT's commissioning interface, commission the TH server using the following "
                       "parameters:\n"
                       f"- setupPinCode:   {params.setupPinCode}\n"
                       f"- setupManualCode: {params.setupManualCode}\n"
                       f"- setupQRCode:     {params.setupQRCode}\n"
                       "\nPress enter once the DUT reports that commissioning completed.\n")

        read = await self.th_server_controller.ReadAttribute(
            _TH_SERVER_NODE_ID, [(0, Clusters.OperationalCredentials.Attributes.CommissionedFabrics)])
        fabrics = read[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CommissionedFabrics]
        asserts.assert_equal(fabrics, 2, "The TH server does not have a fabric for the DUT, so the DUT did not "
                                         "commission it.")

    async def _ask_dut_to_send(self, instruction: str, endpoint: int,
                               command: type[Clusters.ClusterObjects.ClusterCommand]) -> None:
        """Has the operator send a command from the DUT, then checks that the TH server received it.

        A command the DUT sent earlier, e.g. as part of one UI action that covers several steps,
        also counts, so the operator is only prompted if the TH server has not received it yet.
        """
        expected = (endpoint, command.cluster_id, command.command_id)
        name = command.__name__
        with self._received_commands_lock:
            already_received = expected in self._received_commands
        if not already_received:
            self.wait_for_user_input(prompt_msg=f"{instruction}\n\nPress enter once the DUT has sent the command.\n")

        deadline = time.monotonic() + _EVIDENCE_TIMEOUT_SECONDS
        while True:
            with self._received_commands_lock:
                if expected in self._received_commands:
                    return
            if time.monotonic() > deadline:
                asserts.fail(f"The TH server did not receive a {name} command on endpoint {endpoint}.")
            await asyncio.sleep(_POLL_INTERVAL_SECONDS)

    async def _wait_for_pdc_network(self, initial_networks: list[cnet.Structs.NetworkInfoStruct]) -> None:
        """Waits for the TH server to report a network configuration added for Per-Device Credentials.

        The TH server only accepts AddOrUpdateWiFiNetwork with a NetworkIdentity if the
        Credentials field is empty, so a new entry with a NetworkIdentifier shows that the DUT's
        command had both properties. All reports since the precondition are checked rather than
        the current value, because the DUT may already have let the fail-safe expire.
        """
        deadline = time.monotonic() + _EVIDENCE_TIMEOUT_SECONDS
        while True:
            for report in self.networks_subscription.attribute_reports[cnet.Attributes.Networks]:
                added = [n for n in report.value if n not in initial_networks and n.networkIdentifier is not None
                         and n.networkIdentifier is not NullValue]
                if added:
                    log.info("The DUT added a PDC network configuration for %r", added[0].networkID)
                    return
            if time.monotonic() > deadline:
                asserts.fail("The TH server did not report a network configuration that uses Per-Device Credentials, "
                             "so the AddOrUpdateWiFiNetwork command did not have the NetworkIdentity field present "
                             "and the Credentials field set to an empty octet string.")
            await asyncio.sleep(_POLL_INTERVAL_SECONDS)

    @pics("CNET.C.F00", "CNET.C.F03")
    @async_test_body
    async def test_TC_CNET_4_30(self):
        """[TC-CNET-4.30] [Wi-Fi] Verification for Per-Device Credentials commands check [DUT-Client]"""

        self.step("precondition",
                  "TH has a Network Commissioning cluster on endpoint PIXIT.CNET.ENDPOINT_WIFI with WI and PDC set in "
                  "its FeatureMap attribute, and the DUT can communicate with the TH.",
                  expectation="The DUT commissions the TH server.")
        endpoint = await self._find_wifi_pdc_endpoint()
        log.info("TH server hosts a Wi-Fi PDC Network Commissioning cluster on endpoint %d", endpoint)
        await self._ask_dut_to_commission_th_server()
        initial_networks = await self.th_server_controller.ReadAttribute(
            _TH_SERVER_NODE_ID, [(endpoint, cnet.Attributes.Networks)])
        initial_networks = initial_networks[endpoint][cnet][cnet.Attributes.Networks]
        self.networks_subscription = AttributeSubscriptionHandler(cnet, cnet.Attributes.Networks)
        await self.networks_subscription.start(self.th_server_controller, _TH_SERVER_NODE_ID, endpoint)
        self._forget_received_commands()

        self.step(1, "DUT sends the ArmFailSafe command to the TH.",
                  expectation="TH receives the ArmFailSafe command from the DUT.")
        await self._ask_dut_to_send(
            "Using the DUT's interface, send an ArmFailSafe command to the TH server.",
            0, cgen.Commands.ArmFailSafe)

        self.step(2, "DUT sends the AddOrUpdateWiFiNetwork command to the TH to configure a network that uses "
                     "Per-Device Credentials.",
                  expectation="TH receives the AddOrUpdateWiFiNetwork command from the DUT with the NetworkIdentity "
                              "field present and the Credentials field set to an empty octet string.")
        if self.pics_guard(self.check_pics("CNET.C.C02.Tx")):
            await self._ask_dut_to_send(
                f"Using the DUT's interface, send an AddOrUpdateWiFiNetwork command to endpoint {endpoint} of the TH "
                "server that configures a Wi-Fi network using Per-Device Credentials.",
                endpoint, cnet.Commands.AddOrUpdateWiFiNetwork)
            await self._wait_for_pdc_network(initial_networks)

        self.step(3, "DUT sends the QueryIdentity command to the TH.",
                  expectation="TH receives the QueryIdentity command from the DUT.")
        if self.pics_guard(self.check_pics("CNET.C.C09.Tx")):
            await self._ask_dut_to_send(
                f"Using the DUT's interface, send a QueryIdentity command to endpoint {endpoint} of the TH server.",
                endpoint, cnet.Commands.QueryIdentity)

        self.step(4, "DUT sends the ConnectNetwork command to the TH.",
                  expectation="TH receives the ConnectNetwork command from the DUT.")
        if self.pics_guard(self.check_pics("CNET.C.C06.Tx")):
            await self._ask_dut_to_send(
                f"Using the DUT's interface, send a ConnectNetwork command to endpoint {endpoint} of the TH server.",
                endpoint, cnet.Commands.ConnectNetwork)


if __name__ == "__main__":
    default_matter_test_main()
