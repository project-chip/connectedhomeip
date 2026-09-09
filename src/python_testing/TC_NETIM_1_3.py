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
import secrets

from mobly import asserts
from support_modules.network_identity import (NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH, NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                              derive_ecdsa_network_identity, encode_network_administrator_secret,
                                              generate_network_client_identity, matter_epoch_now, network_identity_identifier)

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, pics, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# AddClient and ImportAdminSecret require a Timed Interaction; QueryIdentity does not.
_TIMED_REQUEST_TIMEOUT_MS = 5000
# A compact-pdc-identity for an ECDSA key is at most 140 octets.
_MAX_ECDSA_IDENTITY_LENGTH = 140
# Valid ranges for the table indices, used to pick an index no entry occupies.
_MAX_CLIENT_INDEX = 2047
_MAX_NETWORK_IDENTITY_INDEX = 65534


class TC_NETIM_1_3(MatterBaseTest):

    async def _query(self, endpoint, **selector):
        """Sends a QueryIdentity command and returns the QueryIdentityResponse (QueryIdentity is untimed)."""
        return await self.send_single_cmd(
            cmd=Clusters.NetworkIdentityManagement.Commands.QueryIdentity(**selector), endpoint=endpoint)

    async def _expect_query_status(self, endpoint, expected_status, failure_message: str, **selector):
        """Sends a QueryIdentity command and asserts it fails with the expected status."""
        try:
            await self._query(endpoint, **selector)
            asserts.fail(failure_message)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, failure_message)

    @pics('NETIM.S', 'NETIM.S.C03.Rsp')
    @run_if_endpoint_matches(has_cluster(Clusters.NetworkIdentityManagement))
    async def test_TC_NETIM_1_3(self):
        """[TC-NETIM-1.3] QueryIdentity Command Verification [DUT-Server]"""
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        # Collects every Identity returned by a successful QueryIdentity for validation in step 16.
        returned_identities = []

        self.step(1, "Commissioning, already done.",
                  is_commissioning=True)

        self.step(2, "TH sends ImportAdminSecret with a valid Network Administrator Shared Secret to establish an active "
                     "identity.",
                  expectation="DUT responds with SUCCESS.")
        raw_secret = secrets.token_bytes(NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
        nass = encode_network_administrator_secret(created=matter_epoch_now(), raw_secret=raw_secret)
        await self.send_single_cmd(cmd=commands.ImportAdminSecret(networkAdministratorSharedSecret=nass),
                                   endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        # The DUT must derive this exact identity from the NASS raw secret.
        _, expected_network_identity = derive_ecdsa_network_identity(raw_secret)
        expected_network_identifier = network_identity_identifier(expected_network_identity)

        self.step(3, "TH reads ActiveNetworkIdentities and stores one entry's fields as aniIndex, aniType, aniIdentifier.",
                  expectation="The fields are present and valid; aniIdentifier matches the identifier TH derives from the "
                              "imported NASS.")
        active_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ActiveNetworkIdentities)
        current = [identity for identity in active_list if identity.current]
        asserts.assert_equal(len(current), 1, "Expected exactly one current active network identity after import.")
        ani = current[0]
        ani_index = ani.index
        ani_type = ani.type
        ani_identifier = ani.identifier
        matter_asserts.assert_int_in_range(ani_index, 1, 65534, "ActiveNetworkIdentityStruct.Index")
        asserts.assert_equal(ani_type, cluster.Enums.IdentityTypeEnum.kEcdsa, "Active network identity should be ECDSA.")
        asserts.assert_equal(ani_identifier, expected_network_identifier,
                             "Active network identity Identifier does not match the identity derived from the NASS.")

        self.step(4, "TH sends AddClient with a new unique valid client identity.",
                  expectation="DUT responds with AddClientResponse; the client identity is stored as ClientIdentifierA.")
        _, client_identity_a = generate_network_client_identity()
        expected_client_identifier = network_identity_identifier(client_identity_a)
        add_response = await self.send_single_cmd(cmd=commands.AddClient(clientIdentity=client_identity_a),
                                                  endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        asserts.assert_is_instance(add_response, commands.AddClientResponse, "AddClient did not return an AddClientResponse.")

        self.step(5, "TH reads Clients and stores one client's ClientIdentifier as clientIdentifier and ClientIndex as "
                     "clientIndex.",
                  expectation="clientIdentifier is 20 bytes and matches the identifier of ClientIdentifierA.")
        clients_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        matching = [c for c in clients_list if c.clientIdentifier == expected_client_identifier]
        asserts.assert_equal(len(matching), 1, "Clients does not contain the added ClientIdentifierA.")
        client = matching[0]
        client_index = client.clientIndex
        client_identifier = client.clientIdentifier
        asserts.assert_equal(len(client_identifier), NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                             "ClientIdentifier must be 20 bytes.")

        self.step(6, "TH sends QueryIdentity with only NetworkIdentityIndex=aniIndex.",
                  expectation="DUT responds with QueryIdentityResponse whose Identity is the active network identity.")
        response = await self._query(endpoint, networkIdentityIndex=ani_index)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by NetworkIdentityIndex returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(7, "TH sends QueryIdentity with only NetworkIdentityType=aniType.",
                  expectation="DUT responds with QueryIdentityResponse whose Identity is the current network identity.")
        response = await self._query(endpoint, networkIdentityType=ani_type)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by NetworkIdentityType returned the wrong current Identity.")
        returned_identities.append(response.identity)

        self.step(8, "TH sends QueryIdentity with only Identifier=aniIdentifier.",
                  expectation="DUT responds with QueryIdentityResponse whose Identity is the active network identity.")
        response = await self._query(endpoint, identifier=ani_identifier)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by network Identifier returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(9, "TH sends QueryIdentity with only Identifier=clientIdentifier.",
                  expectation="DUT responds with QueryIdentityResponse whose Identity is ClientIdentifierA.")
        response = await self._query(endpoint, identifier=client_identifier)
        asserts.assert_equal(response.identity, client_identity_a,
                             "QueryIdentity by client Identifier returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(10, "TH sends QueryIdentity with only ClientIndex=clientIndex.",
                  expectation="DUT responds with QueryIdentityResponse whose Identity matches ClientIdentifierA.")
        response = await self._query(endpoint, clientIndex=client_index)
        asserts.assert_equal(response.identity, client_identity_a,
                             "QueryIdentity by ClientIndex returned an Identity that does not match ClientIdentifierA.")
        returned_identities.append(response.identity)

        self.step(11, "TH sends QueryIdentity with only ClientIndex set to an index no entry in Clients occupies.",
                  expectation="DUT responds with NOT_FOUND.")
        # Incrementing clientIndex is not enough: on a DUT that already holds another client at that index
        # the query would succeed and fail the step for an unrelated reason.
        allocated_client_indices = {c.clientIndex for c in clients_list}
        unknown_client_index = next(i for i in range(1, _MAX_CLIENT_INDEX + 1) if i not in allocated_client_indices)
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unallocated ClientIndex should fail with NotFound.",
            clientIndex=unknown_client_index)

        self.step(12, "TH sends QueryIdentity with no fields present.",
                  expectation="DUT responds with INVALID_COMMAND.")
        await self._expect_query_status(
            endpoint, Status.InvalidCommand, "QueryIdentity with no selector should fail with InvalidCommand.")

        self.step(13, "TH sends QueryIdentity with both NetworkIdentityIndex and NetworkIdentityType present.",
                  expectation="DUT responds with INVALID_COMMAND.")
        await self._expect_query_status(
            endpoint, Status.InvalidCommand,
            "QueryIdentity with both NetworkIdentityIndex and NetworkIdentityType should fail with InvalidCommand.",
            networkIdentityIndex=ani_index, networkIdentityType=ani_type)

        self.step(14, "TH sends QueryIdentity with an unknown NetworkIdentityIndex.",
                  expectation="DUT responds with NOT_FOUND.")
        # A retired identity may still occupy aniIndex+1, so pick an index absent from ActiveNetworkIdentities.
        allocated_ni_indices = {identity.index for identity in active_list}
        unknown_ni_index = next(i for i in range(1, _MAX_NETWORK_IDENTITY_INDEX + 1) if i not in allocated_ni_indices)
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unknown NetworkIdentityIndex should fail with NotFound.",
            networkIdentityIndex=unknown_ni_index)

        self.step(15, "TH sends QueryIdentity with an unknown Network Identity (unknown Identifier).",
                  expectation="DUT responds with NOT_FOUND.")
        unknown_identifier = secrets.token_bytes(NETWORK_IDENTITY_IDENTIFIER_LENGTH)
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unknown Identifier should fail with NotFound.",
            identifier=unknown_identifier)

        self.step(16, "TH validates each successful QueryIdentityResponse.Identity.",
                  expectation="Each Identity is an octstr of length <= 140 (ECDSA).")
        for identity in returned_identities:
            matter_asserts.assert_is_octstr(identity, "QueryIdentityResponse.Identity")
            asserts.assert_less_equal(len(identity), _MAX_ECDSA_IDENTITY_LENGTH,
                                      "QueryIdentityResponse.Identity for ECDSA must be at most 140 octets.")


if __name__ == "__main__":
    default_matter_test_main()
