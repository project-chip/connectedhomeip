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
from support_modules.network_identity import POSSESSION_NONCE_LENGTH, generate_network_client_identity, network_identity_identifier

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, pics, run_if_endpoint_matches
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# SSIDs for the network configurations this test case adds. The corresponding Wi-Fi networks do
# not need to exist, since the test case never invokes ConnectNetwork.
_PDC_SSID_1 = b"TC-CNET-4.28-PDC-1"
_PDC_SSID_2 = b"TC-CNET-4.28-PDC-2"


class TC_CNET_4_28(CNETPDCBaseTest):

    @pics("CNET.S.F00", "CNET.S.F03", "CNET.S.C09.Rsp")
    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kPerDeviceCredentials))
    async def test_TC_CNET_4_28(self):
        """[TC-CNET-4.28] [Wi-Fi] Verification for Network Client Identity reuse across network configurations [DUT-Server]"""
        endpoint = self.get_endpoint()

        self.step("precondition",
                  "DUT is commissioned on PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID. TH generates the PDC material used by "
                  "this test case: two self-signed Network Identities NI_1 and NI_2 with the associated key "
                  "identifiers NI_1_ID and NI_2_ID, and a random 32 octet nonce Nonce_1.")
        _, ni_1 = generate_network_client_identity()
        _, ni_2 = generate_network_client_identity()
        ni_1_id = network_identity_identifier(ni_1)
        ni_2_id = network_identity_identifier(ni_2)
        nonce_1 = os.urandom(POSSESSION_NONCE_LENGTH)

        self.step(1, "TH reads the FeatureMap attribute.",
                  expectation="The PDC flag (bit 3) is set, otherwise the test case is skipped.")
        # Reaching this step means the PDC flag is set: the decorator skips the test case otherwise.

        self.step(2, "TH reads the MaxNetworks attribute and saves it as MaxNetworks. If MaxNetworks is less than 2, "
                     "the remaining steps are skipped. TH reads the Networks attribute and saves it as "
                     "InitialNetworks, with the number of entries as NumNetworks.",
                  expectation="DUT responds with a success status.")
        max_networks = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.MaxNetworks)
        if max_networks < 2:
            log.info("MaxNetworks is %d, so the DUT cannot hold two network configurations at once.", max_networks)
            self.mark_all_remaining_steps_skipped(3)
            return
        initial_networks = await self.read_networks(endpoint)
        for ssid in (_PDC_SSID_1, _PDC_SSID_2):
            asserts.assert_not_in(ssid, [n.networkID for n in initial_networks],
                                  f"The DUT already has a network configuration for {ssid!r}.")

        self.step(3, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 60.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

        self.step(4, "TH sends RemoveNetwork for as many trailing entries of InitialNetworks as are needed to leave at "
                     "least two free entries in the Networks attribute.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success to each command.")
        # Everything this test case changes is undone by the fail-safe revert in step 12, so the
        # removed entries only need to be counted, not remembered.
        remaining = list(initial_networks)
        while len(remaining) > max_networks - 2:
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
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a ClientIdentity "
                              "saved as NCI_A, with the associated key identifier saved as NCI_A_ID.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_1!r}")
        nci_a = response.clientIdentity
        self.assert_valid_identity(nci_a, "NetworkConfigResponse.ClientIdentity")
        nci_a_id = network_identity_identifier(nci_a)

        self.step(6, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_2, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_2, the ClientIdentifier field set "
                     "to NCI_A_ID and the PossessionNonce field set to Nonce_1, i.e. the Network Client Identity is "
                     "copied from the network configuration for PDC_SSID_1.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, ClientIdentity NCI_A, "
                              "and a PossessionSignature that is a valid ec-signature over (NCI_A || Nonce_1).")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_2, credentials=b"", networkIdentity=ni_2,
                                                     clientIdentifier=nci_a_id, possessionNonce=nonce_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_2!r} reusing the "
                                           f"identity of {_PDC_SSID_1!r}")
        asserts.assert_equal(response.clientIdentity, nci_a,
                             "The DUT did not copy the Network Client Identity referenced by ClientIdentifier.")
        self.assert_valid_possession_signature(nci_a, nonce_1, response.possessionSignature,
                                               "NetworkConfigResponse.PossessionSignature")

        self.step(7, "TH reads the Networks attribute.",
                  expectation="The list contains an entry with NetworkID PDC_SSID_1, NetworkIdentifier NI_1_ID and "
                              "ClientIdentifier NCI_A_ID, and an entry with NetworkID PDC_SSID_2, NetworkIdentifier "
                              "NI_2_ID and ClientIdentifier NCI_A_ID.")
        networks = await self.read_networks(endpoint)
        for ssid, ni_id, ni_name in ((_PDC_SSID_1, ni_1_id, "NI_1"), (_PDC_SSID_2, ni_2_id, "NI_2")):
            entry = self.find_network(networks, ssid)
            asserts.assert_equal(entry.networkIdentifier, ni_id,
                                 f"The NetworkIdentifier of {ssid!r} is not the key identifier of {ni_name}.")
            asserts.assert_equal(entry.clientIdentifier, nci_a_id,
                                 f"The ClientIdentifier of {ssid!r} is not the key identifier of NCI_A.")

        self.step(8, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_A_ID and the PossessionNonce "
                     "field set to Nonce_1.",
                  expectation="DUT sends a QueryIdentityResponse with Identity NCI_A and a PossessionSignature that is "
                              "a valid ec-signature over (NCI_A || Nonce_1).")
        response = await self.query_identity(endpoint, nci_a_id, nonce_1)
        asserts.assert_equal(response.identity, nci_a, "QueryIdentity did not resolve NCI_A_ID to NCI_A.")
        self.assert_valid_possession_signature(nci_a, nonce_1, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")

        self.step(9, "TH sends RemoveNetwork with the NetworkID field set to PDC_SSID_1.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(networkID=_PDC_SSID_1), endpoint=endpoint)
        self.assert_network_config_success(response, f"RemoveNetwork for {_PDC_SSID_1!r}")

        self.step(10, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_A_ID and the PossessionNonce "
                      "field set to Nonce_1.",
                  expectation="DUT sends a QueryIdentityResponse with Identity NCI_A and a valid PossessionSignature, "
                              "i.e. the Network Client Identity is retained as long as it is referenced by the network "
                              "configuration for PDC_SSID_2.")
        response = await self.query_identity(endpoint, nci_a_id, nonce_1)
        asserts.assert_equal(response.identity, nci_a,
                             "QueryIdentity did not resolve NCI_A_ID to NCI_A after the network configuration for "
                             "PDC_SSID_1 was removed, although the one for PDC_SSID_2 still refers to it.")
        self.assert_valid_possession_signature(nci_a, nonce_1, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")

        self.step(11, "TH sends QueryIdentity with the KeyIdentifier field set to NI_1_ID and the PossessionNonce "
                      "field absent.",
                  expectation="DUT responds with NOT_FOUND, since the only network configuration referring to that "
                              "Network Identity was removed.")
        await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=ni_1_id), endpoint, Status.NotFound,
                                 "QueryIdentity for the Network Identity of a removed network configuration")

        self.step(12, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 0, forcibly disarming the "
                      "fail-safe so that the configuration changes made by this test case are reverted, or waits for "
                      "the fail-safe timer armed in step 3 to expire if it cannot reach the DUT.",
                  expectation="If an ArmFailSafeResponse arrives, it reports no error.")
        await self.disarm_failsafe()

        self.step(13, "TH re-discovers the DUT if it has become unreachable, then reads the Networks attribute.",
                  expectation="The list is equal to InitialNetworks.")
        await self.wait_until_reachable()
        networks = await self.read_networks(endpoint)
        asserts.assert_equal(networks, initial_networks,
                             "Disarming the fail-safe did not restore the Networks attribute to its original contents.")


if __name__ == "__main__":
    default_matter_test_main()
