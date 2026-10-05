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

import logging
import os

from mobly import asserts
from support_modules.cnet_pdc_support import FAILSAFE_EXPIRY_SECONDS, CNETPDCBaseTest, cnet
from support_modules.network_identity import (NETWORK_IDENTITY_IDENTIFIER_LENGTH, POSSESSION_NONCE_LENGTH,
                                              generate_network_client_identity, network_identity_identifier)

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# SSID for the network configuration this test case adds. The corresponding Wi-Fi network does
# not need to exist, since the test case never invokes ConnectNetwork.
_PDC_SSID_1 = b"TC-CNET-4.27-PDC-1"


class TC_CNET_4_27(CNETPDCBaseTest):

    def desc_TC_CNET_4_27(self) -> str:
        return "[TC-CNET-4.27] [Wi-Fi] Verification for Network Client Identity selection and retention [DUT-Server]"

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kPerDeviceCredentials))
    async def test_TC_CNET_4_27(self):
        endpoint = self.get_endpoint()

        self.step("precondition",
                  "DUT is commissioned on PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID. TH generates the PDC material used by "
                  "this test case: two self-signed Network Identities NI_1 and NI_2 with the associated key "
                  "identifiers NI_1_ID and NI_2_ID, a random 32 octet nonce Nonce_1, and a random 20 octet key "
                  "identifier UNKNOWN_ID that does not match any identity configured on the DUT.",
                  is_commissioning=True)
        _, ni_1 = generate_network_client_identity()
        _, ni_2 = generate_network_client_identity()
        ni_1_id = network_identity_identifier(ni_1)
        ni_2_id = network_identity_identifier(ni_2)
        nonce_1 = os.urandom(POSSESSION_NONCE_LENGTH)
        # A random 20 octet value colliding with a configured key identifier is not a scenario worth
        # guarding against, so UNKNOWN_ID is simply drawn at random.
        unknown_id = os.urandom(NETWORK_IDENTITY_IDENTIFIER_LENGTH)

        self.step(1, "TH reads the FeatureMap attribute.",
                  expectation="The PDC flag (bit 3) is set, otherwise the test case is skipped.")
        # Reaching this step means the PDC flag is set: the decorator skips the test case otherwise.

        self.step(2, "TH reads the MaxNetworks attribute and saves it as MaxNetworks. TH reads the Networks attribute "
                     "and saves it as InitialNetworks, with the number of entries as NumNetworks.",
                  expectation="DUT responds with a success status.")
        max_networks = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.MaxNetworks)
        initial_networks = await self.read_networks(endpoint)
        asserts.assert_not_in(_PDC_SSID_1, [n.networkID for n in initial_networks],
                              f"The DUT already has a network configuration for {_PDC_SSID_1!r}.")

        self.step(3, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 60.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

        self.step(4, "TH sends RemoveNetwork for as many trailing entries of InitialNetworks as are needed to leave at "
                     "least one free entry in the Networks attribute.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success to each command.")
        # Everything this test case changes is undone by the fail-safe revert in step 19, so the
        # removed entries only need to be counted, not remembered.
        remaining = list(initial_networks)
        while len(remaining) >= max_networks:
            victim = remaining.pop()
            response = await self.send_single_cmd(
                cmd=cnet.Commands.RemoveNetwork(networkID=victim.networkID), endpoint=endpoint)
            self.assert_network_config_success(response, f"RemoveNetwork for {victim.networkID!r}")
        if len(remaining) == len(initial_networks):
            log.info("Networks has %d of %d entries free, so no network configuration needs to be removed.",
                     max_networks - len(initial_networks), max_networks)

        self.step(5, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_1, and the ClientIdentifier and "
                     "PossessionNonce fields absent.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, a NetworkIndex saved "
                              "as PDC_netidx_1, and a ClientIdentity saved as NCI_A with the associated key identifier "
                              "saved as NCI_A_ID.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_1!r}")
        asserts.assert_is_not_none(response.networkIndex, "NetworkConfigResponse.NetworkIndex is missing.")
        pdc_netidx_1 = response.networkIndex
        nci_a = response.clientIdentity
        self.assert_valid_identity(nci_a, "NetworkConfigResponse.ClientIdentity")
        nci_a_id = network_identity_identifier(nci_a)

        self.step(6, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_1 and the ClientIdentifier field "
                     "set to UNKNOWN_ID.",
                  expectation="DUT responds with NOT_FOUND, since the Networks attribute does not contain an entry "
                              "with a matching ClientIdentifier.")
        await self.expect_status(
            cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1, clientIdentifier=unknown_id),
            endpoint, Status.NotFound, "AddOrUpdateWiFiNetwork with an unknown ClientIdentifier")

        self.step(7, "TH reads the Networks attribute.",
                  expectation="The entry at index PDC_netidx_1 still has ClientIdentifier NCI_A_ID, i.e. the failed "
                              "command did not modify the network configuration.")
        networks = await self.read_networks(endpoint)
        entry = self.network_at(networks, pdc_netidx_1)
        asserts.assert_equal(entry.clientIdentifier, nci_a_id,
                             f"A rejected AddOrUpdateWiFiNetwork command changed the ClientIdentifier of "
                             f"Networks[{pdc_netidx_1}].")

        self.step(8, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_1, the ClientIdentifier field set "
                     "to NCI_A_ID and the PossessionNonce field set to Nonce_1, i.e. the existing network "
                     "configuration is updated while preserving its own Network Client Identity.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, NetworkIndex "
                              "PDC_netidx_1, ClientIdentity NCI_A, and a PossessionSignature that is a valid "
                              "ec-signature over (NCI_A || Nonce_1).")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1,
                                                     clientIdentifier=nci_a_id, possessionNonce=nonce_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork reusing the identity of {_PDC_SSID_1!r}")
        asserts.assert_equal(response.networkIndex, pdc_netidx_1,
                             "Updating the network configuration moved it to a different NetworkIndex.")
        asserts.assert_equal(response.clientIdentity, nci_a,
                             "The DUT did not retain the Network Client Identity referenced by ClientIdentifier.")
        self.assert_valid_possession_signature(nci_a, nonce_1, response.possessionSignature,
                                               "NetworkConfigResponse.PossessionSignature")

        self.step(9, "TH reads the Networks attribute.",
                  expectation="The entry at index PDC_netidx_1 has NetworkID PDC_SSID_1, NetworkIdentifier NI_1_ID and "
                              "ClientIdentifier NCI_A_ID.")
        networks = await self.read_networks(endpoint)
        entry = self.network_at(networks, pdc_netidx_1)
        asserts.assert_equal(entry.networkID, _PDC_SSID_1, f"Networks[{pdc_netidx_1}].NetworkID is not PDC_SSID_1.")
        asserts.assert_equal(entry.networkIdentifier, ni_1_id,
                             f"Networks[{pdc_netidx_1}].NetworkIdentifier is not the key identifier of NI_1.")
        asserts.assert_equal(entry.clientIdentifier, nci_a_id,
                             f"Networks[{pdc_netidx_1}].ClientIdentifier is not the key identifier of NCI_A.")

        self.step(10, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field "
                      "set to an empty octet string, the NetworkIdentity field set to NI_2 and the ClientIdentifier "
                      "field set to NCI_A_ID, i.e. the Network Identity of the network configuration is rotated while "
                      "preserving the Network Client Identity.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and ClientIdentity "
                              "NCI_A.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_2,
                                                     clientIdentifier=nci_a_id),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork rotating the identity of {_PDC_SSID_1!r}")
        asserts.assert_equal(response.clientIdentity, nci_a,
                             "Rotating the Network Identity did not preserve the Network Client Identity.")

        self.step(11, "TH reads the Networks attribute.",
                  expectation="The entry at index PDC_netidx_1 has NetworkIdentifier NI_2_ID and ClientIdentifier "
                              "NCI_A_ID.")
        networks = await self.read_networks(endpoint)
        entry = self.network_at(networks, pdc_netidx_1)
        asserts.assert_equal(entry.networkIdentifier, ni_2_id,
                             f"Networks[{pdc_netidx_1}].NetworkIdentifier is not the key identifier of NI_2.")
        asserts.assert_equal(entry.clientIdentifier, nci_a_id,
                             f"Networks[{pdc_netidx_1}].ClientIdentifier is not the key identifier of NCI_A.")

        self.step(12, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_A_ID and the PossessionNonce "
                      "field set to Nonce_1.",
                  expectation="DUT sends a QueryIdentityResponse with Identity NCI_A and a PossessionSignature that is "
                              "a valid ec-signature over (NCI_A || Nonce_1), i.e. the private key associated with the "
                              "Network Client Identity was retained as well.")
        response = await self.query_identity(endpoint, nci_a_id, nonce_1)
        asserts.assert_equal(response.identity, nci_a, "QueryIdentity did not resolve NCI_A_ID to NCI_A.")
        self.assert_valid_possession_signature(nci_a, nonce_1, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")

        self.step(13, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field "
                      "set to an empty octet string, the NetworkIdentity field set to NI_2, and the ClientIdentifier "
                      "and PossessionNonce fields absent.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a ClientIdentity "
                              "saved as NCI_B, with the associated key identifier saved as NCI_B_ID. NCI_B_ID is not "
                              "equal to NCI_A_ID, i.e. a new key was generated.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_2),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork replacing the identity of "
                                           f"{_PDC_SSID_1!r}")
        nci_b = response.clientIdentity
        self.assert_valid_identity(nci_b, "NetworkConfigResponse.ClientIdentity")
        nci_b_id = network_identity_identifier(nci_b)
        asserts.assert_not_equal(nci_b_id, nci_a_id,
                                 "The DUT reused the key of the existing Network Client Identity even though the "
                                 "ClientIdentifier field was absent.")

        self.step(14, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_A_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND, since the previous Network Client Identity of the network "
                              "configuration was replaced.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_a_id), endpoint, Status.NotFound,
                                 "QueryIdentity for a replaced Network Client Identity")

        self.step(15, "TH reads the Networks attribute.",
                  expectation="The entry at index PDC_netidx_1 has NetworkIdentifier NI_2_ID and ClientIdentifier "
                              "NCI_B_ID.")
        networks = await self.read_networks(endpoint)
        entry = self.network_at(networks, pdc_netidx_1)
        asserts.assert_equal(entry.networkIdentifier, ni_2_id,
                             f"Networks[{pdc_netidx_1}].NetworkIdentifier is not the key identifier of NI_2.")
        asserts.assert_equal(entry.clientIdentifier, nci_b_id,
                             f"Networks[{pdc_netidx_1}].ClientIdentifier is not the key identifier of NCI_B.")

        self.step(16, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_B_ID and the PossessionNonce "
                      "field set to Nonce_1.",
                  expectation="DUT sends a QueryIdentityResponse with Identity NCI_B and a PossessionSignature that is "
                              "a valid ec-signature over (NCI_B || Nonce_1), i.e. a private key was generated for the "
                              "new Network Client Identity as well.")
        response = await self.query_identity(endpoint, nci_b_id, nonce_1)
        asserts.assert_equal(response.identity, nci_b, "QueryIdentity did not resolve NCI_B_ID to NCI_B.")
        self.assert_valid_possession_signature(nci_b, nonce_1, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")

        self.step(17, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field "
                      "set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS and the NetworkIdentity field absent, then "
                      "reads the Networks attribute.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, and the entry with "
                              "NetworkID PDC_SSID_1 has NetworkIdentifier and ClientIdentifier set to null, since the "
                              "network configuration no longer uses Per-Device Credentials.")
        credentials = self.non_empty_credentials()
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=credentials), endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork converting {_PDC_SSID_1!r} to a "
                                           "credentials based configuration")
        networks = await self.read_networks(endpoint)
        entry = self.find_network(networks, _PDC_SSID_1)
        asserts.assert_equal(entry.networkIdentifier, NullValue,
                             "The NetworkIdentifier of a network configuration that no longer uses PDC is not null.")
        asserts.assert_equal(entry.clientIdentifier, NullValue,
                             "The ClientIdentifier of a network configuration that no longer uses PDC is not null.")

        self.step(18, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_B_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_b_id), endpoint, Status.NotFound,
                                 "QueryIdentity for the identity of a network configuration converted away from PDC")

        self.step(19, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 0, forcibly disarming the "
                      "fail-safe so that the configuration changes made by this test case are reverted.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(0)

        self.step(20, "TH reads the Networks attribute.",
                  expectation="The list is equal to InitialNetworks, i.e. it contains NumNetworks entries, contains no "
                              "entry for PDC_SSID_1, and the entry that was connected before the test case has its "
                              "Connected field set to TRUE.")
        networks = await self.read_networks(endpoint)
        asserts.assert_equal(networks, initial_networks,
                             "Disarming the fail-safe did not restore the Networks attribute to its original contents.")


if __name__ == "__main__":
    default_matter_test_main()
