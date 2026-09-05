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

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.network_identity import (NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH, NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                             derive_ecdsa_network_identity, encode_network_administrator_secret,
                                             generate_network_client_identity, matter_epoch_now, network_identity_identifier)
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# AddClient and ImportAdminSecret require a Timed Interaction; QueryIdentity does not.
_TIMED_REQUEST_TIMEOUT_MS = 5000
# A compact-pdc-identity for an ECDSA key is at most 140 octets.
_MAX_ECDSA_IDENTITY_LENGTH = 140


class TC_NETIM_1_3(MatterBaseTest):

    def desc_TC_NETIM_1_3(self) -> str:
        return "[TC-NETIM-1.3] QueryIdentity Command Verification [DUT-Server]"

    def steps_TC_NETIM_1_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done.", is_commissioning=True),
            TestStep(2, "TH sends ImportAdminSecret with a valid Network Administrator Shared Secret to establish an "
                        "active identity.", "DUT responds with SUCCESS."),
            TestStep(3, "TH reads ActiveNetworkIdentities and stores one entry's fields as aniIndex, aniType, "
                        "aniIdentifier.", "The fields are present and valid; aniIdentifier matches the identifier TH "
                        "derives from the imported NASS."),
            TestStep(4, "TH sends AddClient with a new unique valid client identity.",
                        "DUT responds with AddClientResponse; the client identity is stored as ClientIdentifierA."),
            TestStep(5, "TH reads Clients and stores one client's ClientIdentifier as clientIdentifier and ClientIndex "
                        "as clientIndex.", "clientIdentifier is 20 bytes and matches the identifier of ClientIdentifierA."),
            TestStep(6, "TH sends QueryIdentity with only NetworkIdentityIndex=aniIndex.",
                        "DUT responds with QueryIdentityResponse whose Identity is the active network identity."),
            TestStep(7, "TH sends QueryIdentity with only NetworkIdentityType=aniType.",
                        "DUT responds with QueryIdentityResponse whose Identity is the current network identity."),
            TestStep(8, "TH sends QueryIdentity with only Identifier=aniIdentifier.",
                        "DUT responds with QueryIdentityResponse whose Identity is the active network identity."),
            TestStep(9, "TH sends QueryIdentity with only Identifier=clientIdentifier.",
                        "DUT responds with QueryIdentityResponse whose Identity is ClientIdentifierA."),
            TestStep(10, "TH sends QueryIdentity with only ClientIndex=clientIndex.",
                         "DUT responds with QueryIdentityResponse whose Identity matches ClientIdentifierA."),
            TestStep(11, "TH sends QueryIdentity with only ClientIndex=clientIndex+1.", "DUT responds with NOT_FOUND."),
            TestStep(12, "TH sends QueryIdentity with no fields present.", "DUT responds with INVALID_COMMAND."),
            TestStep(13, "TH sends QueryIdentity with both NetworkIdentityIndex and NetworkIdentityType present.",
                         "DUT responds with INVALID_COMMAND."),
            TestStep(14, "TH sends QueryIdentity with an unknown NetworkIdentityIndex.", "DUT responds with NOT_FOUND."),
            TestStep(15, "TH sends QueryIdentity with an unknown Network Identity (unknown Identifier).",
                         "DUT responds with NOT_FOUND."),
            TestStep(16, "TH validates each successful QueryIdentityResponse.Identity.",
                         "Each Identity is an octstr of length <= 140 (ECDSA)."),
        ]

    def pics_TC_NETIM_1_3(self) -> list[str]:
        return [
            "NETIM.S",
            "NETIM.S.C03.Rsp",
        ]

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

    @run_if_endpoint_matches(has_cluster(Clusters.NetworkIdentityManagement))
    async def test_TC_NETIM_1_3(self):
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        # Collects every Identity returned by a successful QueryIdentity for validation in step 16.
        returned_identities = []

        self.step(1)

        self.step(2)
        raw_secret = secrets.token_bytes(NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
        nass = encode_network_administrator_secret(created=matter_epoch_now(), raw_secret=raw_secret)
        await self.send_single_cmd(cmd=commands.ImportAdminSecret(networkAdministratorSharedSecret=nass),
                                   endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        # The DUT must derive this exact identity from the NASS raw secret.
        _, expected_network_identity = derive_ecdsa_network_identity(raw_secret)
        expected_network_identifier = network_identity_identifier(expected_network_identity)

        self.step(3)
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

        self.step(4)
        _, client_identity_a = generate_network_client_identity()
        expected_client_identifier = network_identity_identifier(client_identity_a)
        add_response = await self.send_single_cmd(cmd=commands.AddClient(clientIdentity=client_identity_a),
                                                  endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        asserts.assert_is_instance(add_response, commands.AddClientResponse, "AddClient did not return an AddClientResponse.")

        self.step(5)
        clients_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        matching = [c for c in clients_list if c.clientIdentifier == expected_client_identifier]
        asserts.assert_equal(len(matching), 1, "Clients does not contain the added ClientIdentifierA.")
        client = matching[0]
        client_index = client.clientIndex
        client_identifier = client.clientIdentifier
        asserts.assert_equal(len(client_identifier), NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                             "ClientIdentifier must be 20 bytes.")

        self.step(6)
        response = await self._query(endpoint, networkIdentityIndex=ani_index)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by NetworkIdentityIndex returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(7)
        response = await self._query(endpoint, networkIdentityType=ani_type)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by NetworkIdentityType returned the wrong current Identity.")
        returned_identities.append(response.identity)

        self.step(8)
        response = await self._query(endpoint, identifier=ani_identifier)
        asserts.assert_equal(response.identity, expected_network_identity,
                             "QueryIdentity by network Identifier returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(9)
        response = await self._query(endpoint, identifier=client_identifier)
        asserts.assert_equal(response.identity, client_identity_a,
                             "QueryIdentity by client Identifier returned the wrong Identity.")
        returned_identities.append(response.identity)

        self.step(10)
        response = await self._query(endpoint, clientIndex=client_index)
        asserts.assert_equal(response.identity, client_identity_a,
                             "QueryIdentity by ClientIndex returned an Identity that does not match ClientIdentifierA.")
        returned_identities.append(response.identity)

        self.step(11)
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unallocated ClientIndex should fail with NotFound.",
            clientIndex=client_index + 1)

        self.step(12)
        await self._expect_query_status(
            endpoint, Status.InvalidCommand, "QueryIdentity with no selector should fail with InvalidCommand.")

        self.step(13)
        await self._expect_query_status(
            endpoint, Status.InvalidCommand,
            "QueryIdentity with both NetworkIdentityIndex and NetworkIdentityType should fail with InvalidCommand.",
            networkIdentityIndex=ani_index, networkIdentityType=ani_type)

        self.step(14)
        unknown_ni_index = ani_index + 1 if ani_index < 65534 else ani_index - 1
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unknown NetworkIdentityIndex should fail with NotFound.",
            networkIdentityIndex=unknown_ni_index)

        self.step(15)
        unknown_identifier = secrets.token_bytes(NETWORK_IDENTITY_IDENTIFIER_LENGTH)
        await self._expect_query_status(
            endpoint, Status.NotFound, "QueryIdentity with an unknown Identifier should fail with NotFound.",
            identifier=unknown_identifier)

        self.step(16)
        for identity in returned_identities:
            matter_asserts.assert_is_octstr(identity, "QueryIdentityResponse.Identity")
            asserts.assert_less_equal(len(identity), _MAX_ECDSA_IDENTITY_LENGTH,
                                      "QueryIdentityResponse.Identity for ECDSA must be at most 140 octets.")


if __name__ == "__main__":
    default_matter_test_main()
