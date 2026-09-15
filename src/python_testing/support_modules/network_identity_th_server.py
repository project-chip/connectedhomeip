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

"""The TH-side Network Identity Management server used by the [DUT-Client] test cases.

In TC-NETIM-1.5 and TC-NETIM-1.6 the DUT is a Matter commissioner that originates
Network Identity Management commands, so the TH has to present a Network Identity
Management *server* for the DUT to talk to. The Python controller cannot host a
server cluster of its own, so the TH runs the reference network-manager-app as a
subprocess and:

* commissions it onto a harness-owned fabric, which is the session every assertion
  about the effect of a DUT command is made over, and
* opens a commissioning window so the DUT can commission the same app, which puts
  the TH server and the DUT on one fabric as the Device Topology requires.

Nothing in the cluster is fabric-scoped (see
``src/app/clusters/network-identity-management-server/NetworkIdentityManagementCluster.cpp``,
where the accessing fabric only ever reaches a log line), so the harness fabric
observes exactly the identities, clients and secrets the DUT installs over its own.

The responses themselves are recovered from the TH server's output. Every failure status
is logged by ``CommandHandlerImpl::FallibleAddStatus`` with the endpoint, cluster and
command that produced it, and the cluster logs its own successes with the accessing fabric
index -- so each step asserts what the server actually answered rather than relying on the
operator's report. This follows TC_SC_3_5, which arms the same match around the prompt that
asks an operator to drive a commissioner DUT.
"""

import logging
import os
import random
import re
import tempfile
from dataclasses import dataclass

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.apps import AppServerSubprocess
from matter.testing.commissioning import SetupParameters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest

log = logging.getLogger(__name__)

# AddClient, RemoveClient and ImportAdminSecret require a Timed Interaction.
TIMED_REQUEST_TIMEOUT_MS = 5000

# Network Identity Management (0x0450) as the interaction model logs a cluster id.
_CLUSTER_LOG_ID = "0x0000_0450"

# How long to wait for an expected line after the operator says the DUT has sent the command.
_LOG_MATCH_TIMEOUT_SECONDS = 30


@dataclass(frozen=True)
class THServerLogExpectation:
    """A line the TH server must emit for a step's response to have actually been sent.

    ``pattern`` is a bytes regex because the harness matches raw output lines, which carry
    the server's ANSI colouring around the text. ``description`` is used in the failure
    message when the line never appears.
    """

    pattern: re.Pattern
    description: str


# Identity of the harness-owned fabric used to observe the TH server.
_HARNESS_VENDOR_ID = 0xFFF1
_HARNESS_FABRIC_ID = 2
_HARNESS_CONTROLLER_NODE_ID = 112233
_TH_SERVER_NODE_ID = 1111

# The DUT is driven by a human operator, so the window has to stay open long enough for
# them to work through the vendor's commissioning UX.
_COMMISSIONING_WINDOW_TIMEOUT_SECONDS = 900
_COMMISSIONING_WINDOW_ITERATIONS = 10000


class NetworkIdentityTHServerTest(MatterBaseTest):
    """Base class that owns the TH-side Network Identity Management server."""

    # The harness never opens a Matter session to the DUT: the DUT originates every command
    # in these tests and is driven through its own interface, while all of the harness's
    # traffic goes to the TH server. Skipping the background wildcard subscription and the
    # pre-test DUT-state capture keeps both from reaching for a node that was never
    # commissioned onto a harness fabric.
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
        # Set by ask_dut_to_commission_th_server() once the DUT has its own fabric.
        self.dut_fabric_index = None

        th_server_app = self.user_params.get("th_server_app_path", None)
        if not th_server_app:
            asserts.fail("This test requires a Network Identity Management server app on the TH. Specify the path with "
                         "--string-arg th_server_app_path:<path_to_network_manager_app>")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        # Keep the server's KVS out of the way so each run starts from a clean cluster state.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        log.info("Temporary storage directory: %s", self.storage.name)

        self.th_server_setup_params = SetupParameters(
            discriminator=random.randint(0, 4095),
            passcode=20202021)
        self.th_server_port = self.user_params.get("th_server_port", 5543)

        self.th_server = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            port=self.th_server_port,
            discriminator=self.th_server_setup_params.discriminator,
            passcode=self.th_server_setup_params.passcode)
        self.th_server.start(
            expected_output="Server initialization complete",
            timeout=30)

        certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        fabric_admin = certificate_authority.NewFabricAdmin(vendorId=_HARNESS_VENDOR_ID, fabricId=_HARNESS_FABRIC_ID)
        self.th_server_controller = fabric_admin.NewController(
            nodeId=_HARNESS_CONTROLLER_NODE_ID,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))
        self.th_server_node_id = _TH_SERVER_NODE_ID

        await self.th_server_controller.CommissionOnNetwork(
            nodeId=self.th_server_node_id,
            setupPinCode=self.th_server_setup_params.passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_server_setup_params.discriminator)
        log.info("Commissioned the TH server onto the harness fabric")

        self.th_server_endpoint = await self._find_network_identity_endpoint()
        log.info("TH server hosts Network Identity Management on endpoint %d", self.th_server_endpoint)

    def teardown_class(self):
        if self.th_server is not None:
            self.th_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    async def _find_network_identity_endpoint(self) -> int:
        """Returns the TH server endpoint hosting the Network Identity Management cluster."""
        read = await self.th_server_controller.ReadAttribute(
            self.th_server_node_id, [Clusters.Descriptor.Attributes.ServerList])
        endpoints = [endpoint for endpoint, clusters in read.items()
                     if Clusters.NetworkIdentityManagement.id
                     in clusters[Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]]
        asserts.assert_true(endpoints,
                            "The TH server app does not host a Network Identity Management cluster on any endpoint.")
        return endpoints[0]

    async def read_th_server_attribute(self, attribute):
        """Reads one Network Identity Management attribute from the TH server."""
        read = await self.th_server_controller.ReadAttribute(
            self.th_server_node_id, [(self.th_server_endpoint, attribute)])
        return read[self.th_server_endpoint][Clusters.NetworkIdentityManagement][attribute]

    async def send_th_server_command(self, cmd):
        """Invokes a Network Identity Management command on the TH server using a Timed Interaction."""
        return await self.th_server_controller.SendCommand(
            self.th_server_node_id, self.th_server_endpoint, cmd,
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

    async def read_th_server_commissioned_fabrics(self) -> int:
        read = await self.th_server_controller.ReadAttribute(
            self.th_server_node_id, [(0, Clusters.OperationalCredentials.Attributes.CommissionedFabrics)])
        return read[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CommissionedFabrics]

    async def _read_th_server_fabric_indices(self) -> set[int]:
        """Returns the fabric index of every fabric on the TH server.

        The Fabrics attribute is fabric-scoped, so an ordinary read would return only the
        harness's own entry; the read is unfiltered to see the fabric the DUT commissioned.
        """
        read = await self.th_server_controller.ReadAttribute(
            self.th_server_node_id, [(0, Clusters.OperationalCredentials.Attributes.Fabrics)], fabricFiltered=False)
        fabrics = read[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.Fabrics]
        return {fabric.fabricIndex for fabric in fabrics}

    async def ask_dut_to_commission_th_server(self) -> None:
        """Has the operator commission the TH server from the DUT, and confirms the DUT joined.

        Once this returns, the DUT holds administrative access to the TH server on a fabric
        it commissioned itself, which is what lets it originate the commands under test, and
        ``self.dut_fabric_index`` names that fabric so later steps can tell the DUT's commands
        apart from the harness's own on the same server.
        """
        fabrics_before = await self._read_th_server_fabric_indices()
        params = await self.th_server_controller.OpenCommissioningWindow(
            nodeId=self.th_server_node_id,
            timeout=_COMMISSIONING_WINDOW_TIMEOUT_SECONDS,
            iteration=_COMMISSIONING_WINDOW_ITERATIONS,
            discriminator=random.randint(0, 4095),
            option=self.th_server_controller.CommissioningWindowPasscode.kTokenWithRandomPin)

        self.wait_for_user_input(
            prompt_msg="Using the DUT's commissioning interface, commission the TH Network Identity Management server "
                       "using the following parameters:\n"
                       f"- setupPinCode:   {params.setupPinCode}\n"
                       f"- setupManualCode: {params.setupManualCode}\n"
                       f"- setupQRCode:     {params.setupQRCode}\n"
                       "\nPress enter once the DUT reports that commissioning completed.\n")

        fabrics_after = await self._read_th_server_fabric_indices()
        added = fabrics_after - fabrics_before
        asserts.assert_equal(len(added), 1,
                             "The TH server did not gain exactly one fabric, so the DUT did not commission it.")
        self.dut_fabric_index = added.pop()
        log.info("The DUT commissioned the TH server on fabric index %d", self.dut_fabric_index)

    def require_any_pics(self, *pics_keys: str) -> None:
        """Skips the test unless the DUT claims at least one of the given PICS.

        Every command a Network Identity Management client can generate is optional, and the
        test plan gates each step on the PICS for the command it exercises, so a DUT claiming
        none of them has nothing to run here. Skipping up front keeps the operator from being
        asked to commission the TH server for a test that would then skip every step -- which
        is also what a run that forgot ``--PICS`` looks like, because ``check_pics`` reports
        False for every key when no PICS file was supplied.
        """
        if not any(self.check_pics(key) for key in pics_keys):
            asserts.skip("The DUT claims none of " + ", ".join(pics_keys) + ", so every step of this test would be "
                         "skipped. If the DUT does generate one of these commands, say so in the PICS file passed "
                         "with --PICS.")

    def expect_command_status(self, command_id: int, status: Status) -> THServerLogExpectation:
        """The line the TH server emits when it answers a command with a failure status.

        ``CommandHandlerImpl::FallibleAddStatus`` logs every non-success status at error
        severity, naming the endpoint, cluster and command, so the pattern identifies which
        command produced the status rather than matching a bare status name that any other
        traffic on the server could trip.
        """
        text = (f"Endpoint={self.th_server_endpoint} Cluster={_CLUSTER_LOG_ID} "
                f"Command=0x0000_{command_id:04X} status 0x{status:02x} (")
        return THServerLogExpectation(
            pattern=re.compile(re.escape(text.encode())),
            description=f"{status.name} for command 0x{command_id:04X} on endpoint {self.th_server_endpoint}")

    def expect_client_table_change(self, action: str, *, client_index: int | None = None,
                                   client_identifier: bytes | None = None) -> THServerLogExpectation:
        """The line the cluster emits when AddClient or RemoveClient changes the Client Table.

        Pinned to the DUT's fabric, so the harness's own AddClient/RemoveClient calls against
        the same server cannot satisfy it. ``action`` is "added" or "removed".

        Note this does not cover the idempotent AddClient path: re-adding the identical
        identity returns the existing index without logging, so a step that exercises it has
        to be verified from the Client Table instead.
        """
        index = r"\d+" if client_index is None else str(client_index)
        identifier = "[0-9A-Fa-f]+" if client_identifier is None else client_identifier.hex().upper()
        pattern = rf"Client {index} \({identifier}\) {action} by fabric {self.dut_fabric_index} node 0x"
        return THServerLogExpectation(
            pattern=re.compile(pattern.encode()),
            description=f"a client {action} by the DUT's fabric ({self.dut_fabric_index})")

    def expect_admin_secret_transfer(self, action: str) -> THServerLogExpectation:
        """The line the cluster emits on a successful ImportAdminSecret or ExportAdminSecret.

        Pinned to the DUT's fabric, so the harness's own imports and exports cannot satisfy it.
        ``action`` is "imported" or "exported".
        """
        text = f"NASS successfully {action} by fabric {self.dut_fabric_index} node 0x"
        return THServerLogExpectation(
            pattern=re.compile(re.escape(text.encode())),
            description=f"a NASS {action} by the DUT's fabric ({self.dut_fabric_index})")

    def ask_dut_to_send(self, instruction: str, expected_outcome: str, *,
                        expected_log: THServerLogExpectation | None = None) -> None:
        """Has the operator send a command from the DUT, then checks what the TH server did.

        When ``expected_log`` is given the match is armed before the operator is prompted and
        asserted afterwards, so the TH server's own record of the response it sent is the
        evidence and the operator's answer is a cross-check. A handful of outcomes leave no
        trace on the server -- a QueryIdentityResponse, and the idempotent AddClient path --
        and for those the operator's answer is all there is; the payload the DUT reports is
        still pinned, because the prompt quotes the exact bytes the harness computed.
        """
        if expected_log is not None:
            self.th_server.arm_output_match(expected_log.pattern)

        response = self.wait_for_user_input(
            prompt_msg=f"{instruction}\n\n"
            f"Input 'Y' if {expected_outcome}\n"
            f"Input 'N' if it did not.\n")

        if expected_log is not None:
            # The operator has already acted, so the line is normally present by now; the wait
            # only absorbs the lag of the server's output being forwarded to the harness.
            asserts.assert_true(
                self.th_server.wait_for_output(_LOG_MATCH_TIMEOUT_SECONDS),
                f"The TH server never logged {expected_log.description}, so it did not answer the DUT as the test "
                "plan requires (or the DUT never sent the command).")

        asserts.assert_equal((response or "").strip().lower(), "y",
                             f"The operator reported that the DUT did not observe: {expected_outcome}")
