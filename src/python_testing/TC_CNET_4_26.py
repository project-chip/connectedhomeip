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
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# SSID for the network configuration this test case adds. The corresponding Wi-Fi network does
# not need to exist, since the test case never invokes ConnectNetwork.
_PDC_SSID_1 = b"TC-CNET-4.26-PDC-1"


class TC_CNET_4_26(CNETPDCBaseTest):

    def desc_TC_CNET_4_26(self) -> str:
        return "[TC-CNET-4.26] [Wi-Fi] Verification for QueryIdentity command [DUT-Server]"

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kPerDeviceCredentials))
    async def test_TC_CNET_4_26(self):
        endpoint = self.get_endpoint()

        self.step("precondition",
                  "DUT is commissioned. TH generates the PDC material used by this test case: a self-signed Network "
                  "Identity NI_1 with the associated key identifier NI_1_ID, two different random 32 octet nonces "
                  "Nonce_1 and Nonce_2, and a random 20 octet key identifier UNKNOWN_ID that does not match any "
                  "identity configured on the DUT.",
                  is_commissioning=True)
        _, ni_1 = generate_network_client_identity()
        ni_1_id = network_identity_identifier(ni_1)
        nonce_1 = os.urandom(POSSESSION_NONCE_LENGTH)
        nonce_2 = os.urandom(POSSESSION_NONCE_LENGTH)
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
        num_networks = len(initial_networks)
        asserts.assert_not_in(_PDC_SSID_1, [n.networkID for n in initial_networks],
                              f"The DUT already has a network configuration for {_PDC_SSID_1!r}.")

        self.step(3, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 60.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

        self.step(4, "If NumNetworks is equal to MaxNetworks, TH sends RemoveNetwork with the NetworkID of the last "
                     "entry of InitialNetworks, to make room for the network configuration added in step 5.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
        if num_networks == max_networks:
            response = await self.send_single_cmd(
                cmd=cnet.Commands.RemoveNetwork(networkID=initial_networks[-1].networkID), endpoint=endpoint)
            self.assert_network_config_success(response, "RemoveNetwork for the last entry of InitialNetworks")
        else:
            log.info("Networks has %d of %d entries free, so no network configuration needs to be removed.",
                     max_networks - num_networks, max_networks)

        self.step(5, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_1, and the ClientIdentifier and "
                     "PossessionNonce fields absent.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a ClientIdentity "
                              "saved as NCI_1, with the associated key identifier saved as NCI_1_ID.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_1!r}")
        nci_1 = response.clientIdentity
        self.assert_valid_identity(nci_1, "NetworkConfigResponse.ClientIdentity")
        nci_1_id = network_identity_identifier(nci_1)

        self.step(6, "TH reads the Networks attribute.",
                  expectation="The entry with the NetworkID field set to PDC_SSID_1 has NetworkIdentifier NI_1_ID and "
                              "ClientIdentifier NCI_1_ID.")
        networks = await self.read_networks(endpoint)
        added = self.find_network(networks, _PDC_SSID_1)
        asserts.assert_equal(added.networkIdentifier, ni_1_id,
                             f"The NetworkIdentifier of the {_PDC_SSID_1!r} entry is not the key identifier of NI_1.")
        asserts.assert_equal(added.clientIdentifier, nci_1_id,
                             f"The ClientIdentifier of the {_PDC_SSID_1!r} entry is not the key identifier of NCI_1.")

        self.step(7, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_1_ID and the PossessionNonce "
                     "field absent.",
                  expectation="DUT sends a QueryIdentityResponse with the Identity field equal to NCI_1 and the "
                              "PossessionSignature field absent.")
        response = await self.query_identity(endpoint, nci_1_id)
        asserts.assert_equal(response.identity, nci_1, "QueryIdentity did not resolve NCI_1_ID to NCI_1.")
        asserts.assert_is_none(response.possessionSignature,
                               "QueryIdentityResponse carries a PossessionSignature even though the QueryIdentity "
                               "command had no PossessionNonce field.")

        self.step(8, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_1_ID and the PossessionNonce "
                     "field set to Nonce_1.",
                  expectation="DUT sends a QueryIdentityResponse with the Identity field equal to NCI_1 and a "
                              "PossessionSignature that is a valid ec-signature of length 64 over (NCI_1 || Nonce_1), "
                              "saved as Signature_1.")
        response = await self.query_identity(endpoint, nci_1_id, nonce_1)
        asserts.assert_equal(response.identity, nci_1, "QueryIdentity did not resolve NCI_1_ID to NCI_1.")
        self.assert_valid_possession_signature(nci_1, nonce_1, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")
        signature_1 = response.possessionSignature

        self.step(9, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_1_ID and the PossessionNonce "
                     "field set to Nonce_2.",
                  expectation="DUT sends a QueryIdentityResponse with the Identity field equal to NCI_1 and a "
                              "PossessionSignature that is a valid ec-signature over (NCI_1 || Nonce_2) and differs "
                              "from Signature_1.")
        response = await self.query_identity(endpoint, nci_1_id, nonce_2)
        asserts.assert_equal(response.identity, nci_1, "QueryIdentity did not resolve NCI_1_ID to NCI_1.")
        self.assert_valid_possession_signature(nci_1, nonce_2, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")
        asserts.assert_not_equal(response.possessionSignature, signature_1,
                                 "The DUT produced the same PossessionSignature for two different nonces.")

        self.step(10, "TH sends QueryIdentity with the KeyIdentifier field set to NI_1_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT sends a QueryIdentityResponse with the Identity field equal to NI_1 and the "
                              "PossessionSignature field absent.")
        response = await self.query_identity(endpoint, ni_1_id)
        asserts.assert_equal(response.identity, ni_1, "QueryIdentity did not resolve NI_1_ID to NI_1.")
        asserts.assert_is_none(response.possessionSignature,
                               "QueryIdentityResponse carries a PossessionSignature even though the QueryIdentity "
                               "command had no PossessionNonce field.")

        self.step(11, "TH sends QueryIdentity with the KeyIdentifier field set to NI_1_ID and the PossessionNonce "
                      "field set to Nonce_1.",
                  expectation="DUT responds with NOT_FOUND, since a Network Identity is only resolved if no "
                              "PossessionNonce is present in the request.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=ni_1_id, possessionNonce=nonce_1),
                                 endpoint, Status.NotFound, "QueryIdentity for a Network Identity with a nonce")

        self.step(12, "TH sends QueryIdentity with the KeyIdentifier field set to UNKNOWN_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=unknown_id),
                                 endpoint, Status.NotFound, "QueryIdentity for an unknown key identifier")

        self.step(13, "TH sends RemoveNetwork with the NetworkID field set to PDC_SSID_1.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(networkID=_PDC_SSID_1), endpoint=endpoint)
        self.assert_network_config_success(response, f"RemoveNetwork for {_PDC_SSID_1!r}")

        self.step(14, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_1_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND, since the Network Client Identity and the associated "
                              "private key are removed together with the network configuration referencing them.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_1_id), endpoint, Status.NotFound,
                                 "QueryIdentity for the Network Client Identity of a removed network configuration")

        self.step(15, "TH sends QueryIdentity with the KeyIdentifier field set to NI_1_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=ni_1_id), endpoint, Status.NotFound,
                                 "QueryIdentity for the Network Identity of a removed network configuration")

        self.step(16, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field "
                      "set to an empty octet string, the NetworkIdentity field set to NI_1, and the ClientIdentifier "
                      "and PossessionNonce fields absent.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a ClientIdentity "
                              "saved as NCI_2, with the associated key identifier saved as NCI_2_ID.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_1!r}")
        nci_2 = response.clientIdentity
        self.assert_valid_identity(nci_2, "NetworkConfigResponse.ClientIdentity")
        nci_2_id = network_identity_identifier(nci_2)

        self.step(17, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 0, forcibly disarming the "
                      "fail-safe so that the configuration changes made by this test case are reverted.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(0)

        self.step(18, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_2_ID and the PossessionNonce "
                      "field absent. This step is performed without an armed fail-safe context.",
                  expectation="DUT responds with NOT_FOUND, since the Network Client Identity created within the "
                              "fail-safe context is discarded when the changes are reverted. In particular the DUT "
                              "does not respond with FAILSAFE_REQUIRED, since QueryIdentity does not require an armed "
                              "fail-safe context.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_2_id), endpoint, Status.NotFound,
                                 "QueryIdentity for a Network Client Identity discarded by the fail-safe revert")

        self.step(19, "TH reads the Networks attribute.",
                  expectation="The list is equal to InitialNetworks.")
        networks = await self.read_networks(endpoint)
        asserts.assert_equal(networks, initial_networks,
                             "Disarming the fail-safe did not restore the Networks attribute to its original contents.")


if __name__ == "__main__":
    default_matter_test_main()
