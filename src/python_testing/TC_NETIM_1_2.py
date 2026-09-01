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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${NETWORK_MANAGEMENT_APP}
#     factory-reset: true
#     quiet: true
#     app-args: >
#       --discriminator 1234 --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
from dataclasses import dataclass

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipUtility
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.network_identity import (corrupt_network_client_identity, generate_network_client_identity,
                                             network_identity_identifier, regenerate_network_client_identity)
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# AddClient and RemoveClient require a Timed Interaction.
_TIMED_REQUEST_TIMEOUT_MS = 5000


# The controller enforces must_use_timed_invoke client-side (it raises before sending). To verify the
# DUT's own NeedsTimedInteraction enforcement, these subclasses clear that flag so the command is sent
# untimed and the DUT is the one that rejects it.
@dataclass
class _AddClientNoTimedInvoke(Clusters.NetworkIdentityManagement.Commands.AddClient):
    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


@dataclass
class _RemoveClientNoTimedInvoke(Clusters.NetworkIdentityManagement.Commands.RemoveClient):
    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


class TC_NETIM_1_2(MatterBaseTest):

    def desc_TC_NETIM_1_2(self) -> str:
        return "[TC-NETIM-1.2] Client Manipulation Commands Verification [DUT-Server]"

    def steps_TC_NETIM_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done. TH removes any clients from prior runs and generates the client "
                        "identities used by this test: a valid ClientIdentityA (keeping its private key), a valid "
                        "ClientIdentityB, an invalid identity (corrupted signature), and a colliding identity "
                        "(re-signed with ClientIdentityA's key so it shares A's identifier but differs in bytes).",
                        is_commissioning=True),
            TestStep(2, "TH sends AddClient with ClientIdentityA without using a Timed Interaction.",
                        "DUT responds with NEEDS_TIMED_INTERACTION."),
            TestStep(3, "TH sends AddClient with ClientIdentityA using a Timed Interaction.",
                        "DUT responds with AddClientResponse containing a ClientIndex in 1..2047; stored as idxA."),
            TestStep(4, "TH reads the Clients attribute.",
                        "The list includes an entry with ClientIndex idxA and the 20-byte identifier of ClientIdentityA."),
            TestStep(5, "TH sends AddClient with ClientIdentityA again using a Timed Interaction.",
                        "DUT responds with AddClientResponse and the same ClientIndex idxA (idempotent)."),
            TestStep(6, "TH sends AddClient with an invalid client identity using a Timed Interaction.",
                        "DUT responds with DYNAMIC_CONSTRAINT_ERROR (identity fails validation)."),
            TestStep(7, "TH sends AddClient with a colliding client identity (ClientIdentityA's identifier, different "
                        "bytes) using a Timed Interaction.", "DUT responds with ALREADY_EXISTS."),
            TestStep(8, "TH reads the ClientTableSize attribute and stores as clientTableSize.",
                        "DUT responds with a value in range 500..2047."),
            TestStep(9, "TH fills the Client Table with unique valid identities up to the clientTableSize read in step "
                        "8, then sends one additional AddClient using a Timed Interaction.",
                        "The additional AddClient (beyond clientTableSize) responds with RESOURCE_EXHAUSTED."),
            TestStep(10, "TH sends RemoveClient with ClientIndex idxA without using a Timed Interaction.",
                         "DUT responds with NEEDS_TIMED_INTERACTION."),
            TestStep(11, "TH removes every entry it added (idxA and all fill entries) via RemoveClient using a Timed "
                         "Interaction.", "Each removal of an existing entry succeeds."),
            TestStep(12, "TH reads the Clients attribute.", "The list does not include any of the removed entries."),
            TestStep(13, "TH sends AddClient with a new unique valid identity (ClientIdentityB) using a Timed "
                         "Interaction.", "DUT responds with AddClientResponse containing a ClientIndex in 1..2047."),
            TestStep(14, "TH sends RemoveClient with a ClientIndex that is not allocated using a Timed Interaction.",
                         "DUT responds with NOT_FOUND."),
            TestStep(15, "TH sends RemoveClient with ClientIdentifier of ClientIdentityB using a Timed Interaction.",
                         "DUT responds with SUCCESS."),
            TestStep(16, "TH sends RemoveClient with neither ClientIndex nor ClientIdentifier using a Timed "
                         "Interaction.", "DUT responds with INVALID_COMMAND."),
            TestStep(17, "TH sends RemoveClient with both ClientIndex and ClientIdentifier using a Timed Interaction.",
                         "DUT responds with INVALID_COMMAND."),
        ]

    def pics_TC_NETIM_1_2(self) -> list[str]:
        return [
            "NETIM.S",
            "NETIM.S.C00.Rsp",
        ]

    async def _send_timed(self, cmd, endpoint):
        """Sends a command using a Timed Interaction and returns the response."""
        return await self.send_single_cmd(cmd=cmd, endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)

    async def _expect_status_untimed(self, cmd, endpoint, expected_status: Status, failure_message: str):
        """Sends a command with the client-side timed-invoke requirement cleared and asserts the DUT's status.

        ``cmd`` must be an instance of the no-timed-invoke command subclasses above, so the controller
        forwards it untimed and the DUT is the party that rejects it.
        """
        try:
            await self.default_controller.SendCommand(self.dut_node_id, endpoint, cmd)
            asserts.fail(failure_message)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, failure_message)

    async def _expect_status_timed(self, cmd, endpoint, expected_status: Status, failure_message: str):
        """Sends a command using a Timed Interaction and asserts it fails with the expected status."""
        try:
            await self._send_timed(cmd, endpoint)
            asserts.fail(failure_message)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, failure_message)

    @run_if_endpoint_matches(has_cluster(Clusters.NetworkIdentityManagement))
    async def test_TC_NETIM_1_2(self):
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        # Remove any clients left over from a prior run so the table starts empty.
        preexisting = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        for client in preexisting:
            await self._send_timed(commands.RemoveClient(clientIndex=client.clientIndex), endpoint)

        key_a, client_identity_a = generate_network_client_identity()
        identifier_a = network_identity_identifier(client_identity_a)
        # Same key as A -> same identifier, but a fresh signature so the bytes differ (a true collision).
        client_identity_collision = regenerate_network_client_identity(key_a)
        # A structurally-valid identity whose signature no longer verifies.
        client_identity_invalid = corrupt_network_client_identity(generate_network_client_identity()[1])
        _, client_identity_b = generate_network_client_identity()
        identifier_b = network_identity_identifier(client_identity_b)

        self.step(2)
        await self._expect_status_untimed(
            _AddClientNoTimedInvoke(clientIdentity=client_identity_a), endpoint, Status.NeedsTimedInteraction,
            "AddClient without a Timed Interaction should fail with NeedsTimedInteraction.")

        self.step(3)
        response = await self._send_timed(commands.AddClient(clientIdentity=client_identity_a), endpoint)
        asserts.assert_is_instance(response, commands.AddClientResponse, "AddClient did not return an AddClientResponse.")
        idx_a = response.clientIndex
        matter_asserts.assert_int_in_range(idx_a, 1, 2047, "AddClientResponse.ClientIndex")

        self.step(4)
        clients_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        matching = [c for c in clients_list if c.clientIndex == idx_a and c.clientIdentifier == identifier_a]
        asserts.assert_equal(len(matching), 1,
                             "Clients does not contain the added ClientIdentityA (matching ClientIndex and identifier).")

        self.step(5)
        response = await self._send_timed(commands.AddClient(clientIdentity=client_identity_a), endpoint)
        asserts.assert_is_instance(response, commands.AddClientResponse, "AddClient did not return an AddClientResponse.")
        asserts.assert_equal(response.clientIndex, idx_a,
                             "Re-adding the identical ClientIdentityA must be idempotent and return the same ClientIndex.")

        self.step(6)
        await self._expect_status_timed(
            commands.AddClient(clientIdentity=client_identity_invalid), endpoint, Status.DynamicConstraintError,
            "AddClient with an invalid client identity should fail with DynamicConstraintError.")

        self.step(7)
        await self._expect_status_timed(
            commands.AddClient(clientIdentity=client_identity_collision), endpoint, Status.AlreadyExists,
            "AddClient with an identifier that collides with an existing client should fail with AlreadyExists.")

        self.step(8)
        client_table_size = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ClientTableSize)
        matter_asserts.assert_int_in_range(client_table_size, 500, 2047, "ClientTableSize")

        self.step(9)
        # Fill the table to the capacity read in step 8, then confirm one more AddClient is rejected.
        current_clients = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        fill_needed = client_table_size - len(current_clients)
        asserts.assert_greater(fill_needed, 0, "Client Table is already at capacity before the fill step.")
        log.info("Filling the Client Table to its capacity of %d entries (%d additional); this issues ~%d timed invokes.",
                 client_table_size, fill_needed, fill_needed + 1)
        fill_indices = []
        for _ in range(fill_needed):
            _, fill_identity = generate_network_client_identity()
            fill_response = await self._send_timed(commands.AddClient(clientIdentity=fill_identity), endpoint)
            asserts.assert_is_instance(fill_response, commands.AddClientResponse,
                                       "AddClient while filling the table did not return an AddClientResponse.")
            fill_indices.append(fill_response.clientIndex)

        # One additional client beyond capacity must be rejected.
        _, overflow_identity = generate_network_client_identity()
        await self._expect_status_timed(
            commands.AddClient(clientIdentity=overflow_identity), endpoint, Status.ResourceExhausted,
            "AddClient beyond the Client Table capacity should fail with ResourceExhausted.")

        self.step(10)
        await self._expect_status_untimed(
            _RemoveClientNoTimedInvoke(clientIndex=idx_a), endpoint, Status.NeedsTimedInteraction,
            "RemoveClient without a Timed Interaction should fail with NeedsTimedInteraction.")

        self.step(11)
        # A successful RemoveClient carries no response payload; success is the absence of an error status.
        for index in [idx_a, *fill_indices]:
            await self._send_timed(commands.RemoveClient(clientIndex=index), endpoint)

        self.step(12)
        clients_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        remaining_indices = {c.clientIndex for c in clients_list}
        removed_indices = {idx_a, *fill_indices}
        asserts.assert_equal(remaining_indices & removed_indices, set(),
                             "Clients still contains entries that were removed.")

        self.step(13)
        response = await self._send_timed(commands.AddClient(clientIdentity=client_identity_b), endpoint)
        asserts.assert_is_instance(response, commands.AddClientResponse, "AddClient did not return an AddClientResponse.")
        idx_b = response.clientIndex
        matter_asserts.assert_int_in_range(idx_b, 1, 2047, "AddClientResponse.ClientIndex")

        self.step(14)
        # The plan removes idxA here; since the allocator may have reused idxA for ClientIdentityB, use an
        # index that is currently unallocated so the NOT_FOUND path is exercised deterministically.
        stale_index = idx_a if idx_a != idx_b else (idx_b % client_table_size) + 1
        await self._expect_status_timed(
            commands.RemoveClient(clientIndex=stale_index), endpoint, Status.NotFound,
            "RemoveClient with an unallocated ClientIndex should fail with NotFound.")

        self.step(15)
        # Success is the absence of an error status (RemoveClient has no response payload).
        await self._send_timed(commands.RemoveClient(clientIdentifier=identifier_b), endpoint)

        self.step(16)
        await self._expect_status_timed(
            commands.RemoveClient(), endpoint, Status.InvalidCommand,
            "RemoveClient with neither ClientIndex nor ClientIdentifier should fail with InvalidCommand.")

        self.step(17)
        await self._expect_status_timed(
            commands.RemoveClient(clientIndex=idx_a, clientIdentifier=identifier_b), endpoint, Status.InvalidCommand,
            "RemoveClient with both ClientIndex and ClientIdentifier should fail with InvalidCommand.")


if __name__ == "__main__":
    default_matter_test_main()
