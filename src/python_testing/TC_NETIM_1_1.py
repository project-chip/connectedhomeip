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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.network_identity import (encode_network_administrator_secret, generate_network_client_identity,
                                             matter_epoch_now, network_identity_identifier)
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_NETWORK_INFRASTRUCTURE_MANAGER_DEVICE_TYPE_ID = 0x0090
# NetworkIdentityManagement commands (AddClient, ImportAdminSecret) require a Timed Interaction.
_TIMED_REQUEST_TIMEOUT_MS = 5000


class TC_NETIM_1_1(MatterBaseTest):

    def desc_TC_NETIM_1_1(self) -> str:
        return "[TC-NETIM-1.1] Cluster Conformance Verification [DUT-Server]"

    def steps_TC_NETIM_1_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done. TH configures the DUT with one known Network Administrator "
                        "Shared Secret (via ImportAdminSecret) and one known client identity (via AddClient), storing "
                        "the expected active network identity and client fields.", is_commissioning=True),
            TestStep(2, "TH reads the ServerList from the Descriptor cluster on the endpoint.",
                        "Network Identity Management cluster ID 0x0450 is present."),
            TestStep(3, "TH reads the DeviceTypeList from the Descriptor cluster on the endpoint.",
                        "Network Infrastructure Manager device type ID 0x0090 is present."),
            TestStep(4, "TH reads the ActiveNetworkIdentities attribute and stores as activeList.",
                        "DUT responds with a list."),
            TestStep(5, "TH validates the length of activeList.", "activeList is a list with length 1."),
            TestStep(6, "TH validates each element in activeList.",
                        "For each element: Index in 1..65534, Type is a valid enum8, Identifier is a 20-byte octstr, "
                        "CreatedTimestamp is epoch-s, Current is a bool, RemainingClients is null when Current is true "
                        "else a uint16."),
            TestStep(7, "TH verifies activeList contains the expected active network identity configured during setup.",
                        "activeList contains an entry with the expected CreatedTimestamp, Type ECDSA, a 20-byte "
                        "Identifier and Current true."),
            TestStep(8, "TH reads the Clients attribute and stores as clientsList.", "DUT responds with a list."),
            TestStep(9, "TH validates the length of clientsList.", "clientsList is a list with length 1."),
            TestStep(10, "TH validates each element in clientsList.",
                         "For each element: ClientIndex in 1..2047, ClientIdentifier is a 20-byte octstr, "
                         "NetworkIdentityIndex is null or in range 1..65534."),
            TestStep(11, "TH verifies clientsList contains the expected client configured during setup.",
                         "clientsList contains an entry with the expected ClientIndex and ClientIdentifier."),
            TestStep(12, "TH reads the ClientTableSize attribute and stores as clientTableSize.",
                         "DUT responds with a value."),
            TestStep(13, "TH validates the ClientTableSize value.", "clientTableSize is a uint16 in range 500..2047."),
        ]

    def pics_TC_NETIM_1_1(self) -> list[str]:
        return [
            "NETIM.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.NetworkIdentityManagement))
    async def test_TC_NETIM_1_1(self):
        cluster = Clusters.NetworkIdentityManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        # Test Setup: configure one known active network identity and one known client.
        # The active identity's CreatedTimestamp is taken verbatim from the imported NASS, so it
        # is predictable. The client's ClientIdentifier is the 20-byte key identifier derived from
        # the client identity we generate, so it is predictable too.
        expected_created_timestamp = matter_epoch_now()
        nass = encode_network_administrator_secret(created=expected_created_timestamp)
        await self.send_single_cmd(
            cmd=cluster.Commands.ImportAdminSecret(networkAdministratorSharedSecret=nass),
            endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)

        _, client_identity = generate_network_client_identity()
        expected_client_identifier = network_identity_identifier(client_identity)
        add_client_response = await self.send_single_cmd(
            cmd=cluster.Commands.AddClient(clientIdentity=client_identity),
            endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        asserts.assert_is_instance(add_client_response, cluster.Commands.AddClientResponse,
                                   "AddClient did not return an AddClientResponse.")
        expected_client_index = add_client_response.clientIndex
        matter_asserts.assert_int_in_range(expected_client_index, 1, 2047, "AddClientResponse.ClientIndex")

        self.step(2)
        server_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList)
        asserts.assert_in(cluster.id, server_list, "Network Identity Management cluster (0x0450) not in the ServerList.")

        self.step(3)
        device_type_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
        asserts.assert_true(
            any(dt.deviceType == _NETWORK_INFRASTRUCTURE_MANAGER_DEVICE_TYPE_ID for dt in device_type_list),
            "Network Infrastructure Manager device type (0x0090) not in the DeviceTypeList.")

        self.step(4)
        active_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ActiveNetworkIdentities)
        matter_asserts.assert_list(active_list, "ActiveNetworkIdentities")

        self.step(5)
        asserts.assert_equal(len(active_list), 1, "ActiveNetworkIdentities should contain exactly one entry.")

        self.step(6)
        for identity in active_list:
            matter_asserts.assert_int_in_range(identity.index, 1, 65534, "ActiveNetworkIdentityStruct.Index")
            matter_asserts.assert_valid_enum(identity.type, "ActiveNetworkIdentityStruct.Type", cluster.Enums.IdentityTypeEnum)
            matter_asserts.assert_is_octstr(identity.identifier, "ActiveNetworkIdentityStruct.Identifier")
            asserts.assert_equal(len(identity.identifier), 20, "ActiveNetworkIdentityStruct.Identifier must be 20 bytes.")
            matter_asserts.assert_valid_uint32(identity.createdTimestamp, "ActiveNetworkIdentityStruct.CreatedTimestamp")
            matter_asserts.assert_valid_bool(identity.current, "ActiveNetworkIdentityStruct.Current")
            if identity.current:
                asserts.assert_equal(identity.remainingClients, NullValue,
                                     "RemainingClients must be null when Current is true.")
            else:
                matter_asserts.assert_valid_uint16(identity.remainingClients, "ActiveNetworkIdentityStruct.RemainingClients")

        self.step(7)
        matching = [identity for identity in active_list
                    if identity.createdTimestamp == expected_created_timestamp and identity.current]
        asserts.assert_equal(len(matching), 1,
                             "activeList does not contain the expected active network identity (Current entry with the "
                             "CreatedTimestamp imported during setup).")
        expected_identity = matching[0]
        asserts.assert_equal(expected_identity.type, cluster.Enums.IdentityTypeEnum.kEcdsa,
                             "The imported network identity should be of type ECDSA.")
        asserts.assert_equal(len(expected_identity.identifier), 20, "The imported network identity Identifier must be 20 bytes.")

        self.step(8)
        clients_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.Clients)
        matter_asserts.assert_list(clients_list, "Clients")

        self.step(9)
        asserts.assert_equal(len(clients_list), 1, "Clients should contain exactly one entry.")

        self.step(10)
        for client in clients_list:
            matter_asserts.assert_int_in_range(client.clientIndex, 1, 2047, "ClientStruct.ClientIndex")
            matter_asserts.assert_is_octstr(client.clientIdentifier, "ClientStruct.ClientIdentifier")
            asserts.assert_equal(len(client.clientIdentifier), 20, "ClientStruct.ClientIdentifier must be 20 bytes.")
            if client.networkIdentityIndex is not NullValue:
                matter_asserts.assert_int_in_range(client.networkIdentityIndex, 1, 65534, "ClientStruct.NetworkIdentityIndex")

        self.step(11)
        matching_clients = [client for client in clients_list
                            if client.clientIndex == expected_client_index
                            and client.clientIdentifier == expected_client_identifier]
        asserts.assert_equal(len(matching_clients), 1,
                             "clientsList does not contain the expected client (matching ClientIndex and "
                             "ClientIdentifier configured during setup).")

        self.step(12)
        client_table_size = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ClientTableSize)

        self.step(13)
        matter_asserts.assert_valid_uint16(client_table_size, "ClientTableSize")
        matter_asserts.assert_int_in_range(client_table_size, 500, 2047, "ClientTableSize")


if __name__ == "__main__":
    default_matter_test_main()
