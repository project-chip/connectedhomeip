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
# Run it with the path to a Network Identity Management server app for the TH, e.g.:
#
#   python3 src/python_testing/TC_NETIM_1_5.py \
#       --storage-path admin_storage.json \
#       --PICS <the DUT's PICS file> \
#       --string-arg th_server_app_path:out/linux-x64-network-manager/matter-network-manager-app
#
# Every command a client can generate is optional, so the test plan gates steps 2-5 on
# AddClient, steps 6-7 on RemoveClient and steps 8-11 on QueryIdentity. The PICS file decides
# which of those groups run; without --PICS all three read as unsupported and the test skips.
import secrets

from mobly import asserts
from support_modules.network_identity import (NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH, NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                              derive_ecdsa_network_identity, encode_network_administrator_secret,
                                              generate_network_client_identity, matter_epoch_now, network_identity_identifier)
from support_modules.network_identity_th_server import NetworkIdentityTHServerTest

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing import matter_asserts
from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main

# The per-step PICS from the test plan's procedure table. All three are optional for a client,
# so each group of steps runs only if the DUT claims the command that group exercises.
PICS_ADD_CLIENT = 'NETIM.C.C00.Tx'
PICS_REMOVE_CLIENT = 'NETIM.C.C02.Tx'
PICS_QUERY_IDENTITY = 'NETIM.C.C03.Tx'


class TC_NETIM_1_5(NetworkIdentityTHServerTest):

    async def _add_client_from_harness(self) -> tuple[int, bytes]:
        """Adds a client to the TH server over the harness fabric, returning its index and identifier."""
        cluster = Clusters.NetworkIdentityManagement
        _, client_identity = generate_network_client_identity()
        response = await self.send_th_server_command(cluster.Commands.AddClient(clientIdentity=client_identity))
        asserts.assert_is_instance(response, cluster.Commands.AddClientResponse,
                                   "AddClient did not return an AddClientResponse.")
        clients = await self.read_th_server_attribute(cluster.Attributes.Clients)
        added = [client for client in clients if client.clientIndex == response.clientIndex]
        asserts.assert_equal(len(added), 1, "The client the harness added is not in the TH server's Client Table.")
        return added[0].clientIndex, added[0].clientIdentifier

    @pics('NETIM.C')
    @async_test_body
    async def test_TC_NETIM_1_5(self):
        """[TC-NETIM-1.5] Client Command Generation and Command-Specific Responses [DUT-Client]"""
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        self.require_any_pics(PICS_ADD_CLIENT, PICS_REMOVE_CLIENT, PICS_QUERY_IDENTITY)

        self.step("precondition", "TH imports a Network Administrator Shared Secret into its Network Identity Management "
                  "server and removes any clients left over from a prior run, so that the server holds one "
                  "queryable Network Identity and an empty Client Table.",
                  expectation="The TH server reports exactly one current active network identity and no clients.")
        raw_secret = secrets.token_bytes(NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
        await self.send_th_server_command(commands.ImportAdminSecret(
            networkAdministratorSharedSecret=encode_network_administrator_secret(
                created=matter_epoch_now(), raw_secret=raw_secret)))
        for client in await self.read_th_server_attribute(attributes.Clients):
            await self.send_th_server_command(commands.RemoveClient(clientIndex=client.clientIndex))

        active_identities = await self.read_th_server_attribute(attributes.ActiveNetworkIdentities)
        current = [identity for identity in active_identities if identity.current]
        asserts.assert_equal(len(current), 1, "The TH server should hold exactly one current active network identity.")
        network_identity_index = current[0].index
        network_identity_identifier_value = current[0].identifier
        # The TH server derives this identity from the raw secret, so the harness knows the exact
        # bytes a successful QueryIdentity has to return to the DUT in step 8.
        _, expected_network_identity = derive_ecdsa_network_identity(raw_secret)
        asserts.assert_equal(network_identity_identifier_value, network_identity_identifier(expected_network_identity),
                             "The TH server's active network identity does not match the identity derived from the NASS.")
        asserts.assert_equal(await self.read_th_server_attribute(attributes.Clients), [],
                             "The TH server's Client Table should be empty at the start of the test.")

        self.step(1, "Commission TH to DUT.",
                  expectation="DUT and TH can communicate over CASE.")
        await self.ask_dut_to_commission_th_server()

        # Carried from the AddClient steps to the RemoveClient steps, or seeded by the harness in
        # step 6 when the AddClient steps did not run.
        idx_a = None
        client_identifier_a = None

        self.step(2, "DUT sends AddClient with a valid client identity.",
                  expectation="TH responds with AddClientResponse and a valid ClientIndex. Store as idxA. Client identity "
                              "is stored as ClientIdentifierA.")
        if self.pics_guard(self.check_pics(PICS_ADD_CLIENT)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send an AddClient command to the TH Network Identity Management server with a "
                "valid client identity.",
                "the TH responded with an AddClientResponse carrying a ClientIndex.",
                expected_log=self.expect_client_table_change("added"))
            clients = await self.read_th_server_attribute(attributes.Clients)
            asserts.assert_equal(len(clients), 1,
                                 "AddClient with a valid client identity should have added exactly one client.")
            idx_a = clients[0].clientIndex
            client_identifier_a = clients[0].clientIdentifier
            matter_asserts.assert_int_in_range(idx_a, 1, 2047, "ClientStruct.ClientIndex")
            asserts.assert_equal(len(client_identifier_a), NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                 "ClientStruct.ClientIdentifier must be 20 bytes.")

        self.step(3, "DUT resends AddClient with ClientIdentifierA.",
                  expectation="TH returns the same ClientIndex=idxA.")
        if self.pics_guard(self.check_pics(PICS_ADD_CLIENT)):
            # The idempotent AddClient path returns the existing index without logging, so there is
            # no server line to match here; the Client Table check below is the objective evidence.
            self.ask_dut_to_send(
                "Using the DUT's interface, send AddClient to the TH Network Identity Management server again with the "
                f"exact same client identity used in the previous step (its Identifier is {client_identifier_a.hex()}).",
                f"the TH responded with an AddClientResponse carrying ClientIndex={idx_a}.")
            clients = await self.read_th_server_attribute(attributes.Clients)
            asserts.assert_equal([client.clientIndex for client in clients], [idx_a],
                                 "Re-adding ClientIdentifierA should leave the Client Table holding only idxA.")

        self.step(4, "DUT sends AddClient with a non-valid client identity.",
                  expectation="TH responds with DYNAMIC_CONSTRAINT_ERROR.")
        if self.pics_guard(self.check_pics(PICS_ADD_CLIENT)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send AddClient to the TH Network Identity Management server with a non-valid "
                "client identity, such as a compact-pdc-identity whose self-signature does not verify.",
                "the TH responded with DYNAMIC_CONSTRAINT_ERROR.",
                expected_log=self.expect_command_status(commands.AddClient.command_id, Status.DynamicConstraintError))
            clients = await self.read_th_server_attribute(attributes.Clients)
            asserts.assert_equal([client.clientIndex for client in clients], [idx_a],
                                 "A non-valid client identity must not be added to the Client Table.")

        self.step(5, "DUT sends AddClient with a client identity that already exists.",
                  expectation="TH responds with ALREADY_EXISTS.")
        if self.pics_guard(self.check_pics(PICS_ADD_CLIENT)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send AddClient to the TH Network Identity Management server with a client "
                f"identity that already exists: one whose Identifier is {client_identifier_a.hex()} but whose encoding "
                "differs from the identity added in step 2 (for example, the same public key carrying a freshly generated "
                "signature).",
                "the TH responded with ALREADY_EXISTS.",
                expected_log=self.expect_command_status(commands.AddClient.command_id, Status.AlreadyExists))
            clients = await self.read_th_server_attribute(attributes.Clients)
            asserts.assert_equal([client.clientIndex for client in clients], [idx_a],
                                 "A client identity that already exists must not add a second Client Table entry.")

        self.step(6, "DUT sends RemoveClient with ClientIndex=idxA.",
                  expectation="TH responds SUCCESS.")
        if self.pics_guard(self.check_pics(PICS_REMOVE_CLIENT)):
            if idx_a is None:
                # A DUT that generates RemoveClient but not AddClient skipped the steps that would
                # have created idxA, so the harness adds the client the DUT is asked to remove. The
                # Client Table is not fabric-scoped, so this is the same entry the DUT sees.
                idx_a, client_identifier_a = await self._add_client_from_harness()
            self.ask_dut_to_send(
                "Using the DUT's interface, send RemoveClient to the TH Network Identity Management server with "
                f"ClientIndex={idx_a}.",
                "the TH responded with SUCCESS.",
                expected_log=self.expect_client_table_change("removed", client_index=idx_a,
                                                             client_identifier=client_identifier_a))
            asserts.assert_equal(await self.read_th_server_attribute(attributes.Clients), [],
                                 "RemoveClient with ClientIndex=idxA should have emptied the Client Table.")

        self.step(7, "DUT sends RemoveClient with an invalid client index.",
                  expectation="TH responds NOT_FOUND.")
        if self.pics_guard(self.check_pics(PICS_REMOVE_CLIENT)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send RemoveClient to the TH Network Identity Management server with an invalid "
                f"client index, such as ClientIndex={idx_a} now that it has been removed.",
                "the TH responded with NOT_FOUND.",
                expected_log=self.expect_command_status(commands.RemoveClient.command_id, Status.NotFound))

        self.step(8, "DUT sends QueryIdentity with one valid selector supported by TH test setup.",
                  expectation="TH responds with QueryIdentityResponse containing Identity.")
        if self.pics_guard(self.check_pics(PICS_QUERY_IDENTITY)):
            # A successful QueryIdentity leaves no trace on the server -- no log line, and it changes
            # no attribute -- so the operator's report is the only evidence here. The Identity they
            # are asked to match is exact: the harness derived it from the NASS it imported.
            self.ask_dut_to_send(
                "Using the DUT's interface, send QueryIdentity to the TH Network Identity Management server with exactly "
                f"one selector: NetworkIdentityIndex={network_identity_index} (or, equivalently, "
                f"Identifier={network_identity_identifier_value.hex()}).",
                f"the TH responded with a QueryIdentityResponse whose Identity is {expected_network_identity.hex()}.")

        self.step(9, "DUT sends QueryIdentity with no fields present.",
                  expectation="TH responds INVALID_COMMAND.")
        if self.pics_guard(self.check_pics(PICS_QUERY_IDENTITY)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send QueryIdentity to the TH Network Identity Management server with none of "
                "the selector fields present.",
                "the TH responded with INVALID_COMMAND.",
                expected_log=self.expect_command_status(commands.QueryIdentity.command_id, Status.InvalidCommand))

        self.step(10, "DUT sends QueryIdentity with multiple selector fields present.",
                  expectation="TH responds INVALID_COMMAND.")
        if self.pics_guard(self.check_pics(PICS_QUERY_IDENTITY)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send QueryIdentity to the TH Network Identity Management server with more than "
                f"one selector field present, such as NetworkIdentityIndex={network_identity_index} together with "
                f"Identifier={network_identity_identifier_value.hex()}.",
                "the TH responded with INVALID_COMMAND.",
                expected_log=self.expect_command_status(commands.QueryIdentity.command_id, Status.InvalidCommand))

        self.step(11, "DUT sends QueryIdentity with unknown selector value(s).",
                  expectation="TH responds NOT_FOUND.")
        if self.pics_guard(self.check_pics(PICS_QUERY_IDENTITY)):
            unknown_identifier = secrets.token_bytes(NETWORK_IDENTITY_IDENTIFIER_LENGTH)
            self.ask_dut_to_send(
                "Using the DUT's interface, send QueryIdentity to the TH Network Identity Management server with an "
                f"unknown selector value, such as Identifier={unknown_identifier.hex()}.",
                "the TH responded with NOT_FOUND.",
                expected_log=self.expect_command_status(commands.QueryIdentity.command_id, Status.NotFound))


if __name__ == "__main__":
    default_matter_test_main()
