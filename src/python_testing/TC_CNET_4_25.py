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
from support_modules.cnet_pdc_support import FAILSAFE_EXPIRY_SECONDS, MAX_DEBUG_TEXT_LENGTH, CNETPDCBaseTest, cgen, cnet
from support_modules.network_identity import (POSSESSION_NONCE_LENGTH, corrupt_network_client_identity,
                                              generate_network_client_identity, network_identity_identifier)

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# SSIDs for the network configurations this test case adds. The corresponding Wi-Fi networks
# do not need to exist, since the test case never invokes ConnectNetwork.
_PDC_SSID_1 = b"TC-CNET-4.25-PDC-1"
_PDC_SSID_2 = b"TC-CNET-4.25-PDC-2"
_PDC_SSID_3 = b"TC-CNET-4.25-PDC-3"


class TC_CNET_4_25(CNETPDCBaseTest):

    def desc_TC_CNET_4_25(self) -> str:
        return ("[TC-CNET-4.25] [Wi-Fi] Verification for AddOrUpdateWiFiNetwork command with Per-Device Credentials "
                "[DUT-Server]")

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning,
                                         Clusters.NetworkCommissioning.Bitmaps.Feature.kPerDeviceCredentials))
    async def test_TC_CNET_4_25(self):
        endpoint = self.get_endpoint()

        self.step("precondition",
                  "DUT is commissioned on PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID. TH generates the PDC material used by "
                  "this test case: a self-signed Network Identity NI_1 with the associated key identifier NI_1_ID, an "
                  "octet string NI_INVALID that is a copy of NI_1 with one octet of its signature modified, and a "
                  "random 32 octet nonce Nonce_1.",
                  is_commissioning=True)
        _, ni_1 = generate_network_client_identity()
        ni_1_id = network_identity_identifier(ni_1)
        ni_invalid = corrupt_network_client_identity(ni_1)
        nonce_1 = os.urandom(POSSESSION_NONCE_LENGTH)

        self.step(1, "TH reads the FeatureMap attribute.",
                  expectation="The PDC flag (bit 3) is set, otherwise the test case is skipped. The WI flag (bit 0) is "
                              "also set, since PDC can only be supported in combination with WI.")
        # Reaching this step means the PDC flag is set: the decorator skips the test case otherwise.
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.FeatureMap)
        asserts.assert_true(feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface,
                            "The WI feature is not set even though the DUT supports PDC.")

        self.step(2, "TH reads the MaxNetworks attribute and saves it as MaxNetworks.",
                  expectation="DUT responds with a success status.")
        max_networks = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.MaxNetworks)

        self.step(3, "TH reads the Networks attribute and saves it as InitialNetworks, with the number of entries as "
                     "NumNetworks.",
                  expectation="For every entry, NetworkIdentifier and ClientIdentifier are each either null or an "
                              "octstr of length 20, and NetworkIdentifier is null if and only if ClientIdentifier is "
                              "null.")
        initial_networks = await self.read_networks(endpoint)
        num_networks = len(initial_networks)
        for index, network in enumerate(initial_networks):
            network_identifier = self.assert_nullable_identifier(
                network.networkIdentifier, f"Networks[{index}].NetworkIdentifier")
            client_identifier = self.assert_nullable_identifier(
                network.clientIdentifier, f"Networks[{index}].ClientIdentifier")
            asserts.assert_equal(network_identifier is None, client_identifier is None,
                                 f"Networks[{index}] has only one of NetworkIdentifier and ClientIdentifier set to "
                                 "null; a network configuration either uses PDC or it does not.")
        configured_ssids = [n.networkID for n in initial_networks]
        for ssid in (_PDC_SSID_1, _PDC_SSID_2, _PDC_SSID_3):
            asserts.assert_not_in(ssid, configured_ssids,
                                  f"The DUT already has a network configuration for {ssid!r}, which this test case "
                                  "requires to be absent.")

        self.step(4, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 60.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

        self.step(5, "If NumNetworks is equal to MaxNetworks, TH sends RemoveNetwork with the NetworkID of the last "
                     "entry of InitialNetworks, to make room for the network configuration added in step 6.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
        if num_networks == max_networks:
            response = await self.send_single_cmd(
                cmd=cnet.Commands.RemoveNetwork(networkID=initial_networks[-1].networkID), endpoint=endpoint)
            self.assert_network_config_success(response, "RemoveNetwork for the last entry of InitialNetworks")
        else:
            log.info("Networks has %d of %d entries free, so no network configuration needs to be removed.",
                     max_networks - num_networks, max_networks)

        self.step(6, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_1, the Credentials field set "
                     "to an empty octet string, the NetworkIdentity field set to NI_1, the ClientIdentifier field "
                     "absent, the PossessionNonce field set to Nonce_1 and the Breadcrumb field set to 1.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, a DebugText of at most "
                              "512 characters or absent, a NetworkIndex (saved as PDC_netidx_1), a ClientIdentity that "
                              "is a valid self-signed PDC identity of length 137 (saved as NCI_1), and a "
                              "PossessionSignature that is a valid ec-signature of length 64 over (NCI_1 || Nonce_1).")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=_PDC_SSID_1, credentials=b"", networkIdentity=ni_1, possessionNonce=nonce_1, breadcrumb=1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_1!r}")
        if response.debugText is not None:
            asserts.assert_less_equal(len(response.debugText), MAX_DEBUG_TEXT_LENGTH,
                                      "NetworkConfigResponse.DebugText is longer than 512 characters.")
        asserts.assert_is_not_none(response.networkIndex, "NetworkConfigResponse.NetworkIndex is missing.")
        pdc_netidx_1 = response.networkIndex
        nci_1 = response.clientIdentity
        self.assert_valid_identity(nci_1, "NetworkConfigResponse.ClientIdentity")
        nci_1_id = network_identity_identifier(nci_1)
        self.assert_valid_possession_signature(nci_1, nonce_1, response.possessionSignature,
                                               "NetworkConfigResponse.PossessionSignature")

        self.step(7, "TH reads the Networks attribute.",
                  expectation="The entry at index PDC_netidx_1 has NetworkID PDC_SSID_1, Connected FALSE, "
                              "NetworkIdentifier NI_1_ID and ClientIdentifier NCI_1_ID.")
        networks = await self.read_networks(endpoint)
        added = self.network_at(networks, pdc_netidx_1)
        asserts.assert_equal(added.networkID, _PDC_SSID_1, f"Networks[{pdc_netidx_1}].NetworkID is not PDC_SSID_1.")
        asserts.assert_false(added.connected, f"Networks[{pdc_netidx_1}].Connected is TRUE for a network the DUT was "
                             "never asked to connect to.")
        asserts.assert_equal(added.networkIdentifier, ni_1_id,
                             f"Networks[{pdc_netidx_1}].NetworkIdentifier is not the key identifier of NI_1.")
        asserts.assert_equal(added.clientIdentifier, nci_1_id,
                             f"Networks[{pdc_netidx_1}].ClientIdentifier is not the key identifier of NCI_1.")

        self.step(8, "TH reads the Breadcrumb attribute from the General Commissioning cluster.",
                  expectation="The Breadcrumb attribute is set to 1.")
        breadcrumb = await self.read_single_attribute_check_success(
            endpoint=0, cluster=cgen, attribute=cgen.Attributes.Breadcrumb)
        asserts.assert_equal(breadcrumb, 1, "AddOrUpdateWiFiNetwork did not update the Breadcrumb attribute.")

        self.step(9, "TH sends RemoveNetwork with the NetworkID field set to PDC_SSID_1 and the Breadcrumb field set "
                     "to 1, to make room for the network configuration added in step 10.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(networkID=_PDC_SSID_1, breadcrumb=1), endpoint=endpoint)
        self.assert_network_config_success(response, f"RemoveNetwork for {_PDC_SSID_1!r}")

        self.step(10, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_2, the Credentials field "
                      "set to an empty octet string, the NetworkIdentity field set to NI_1, and the ClientIdentifier "
                      "and PossessionNonce fields absent.",
                  expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a ClientIdentity "
                              "that is a valid self-signed PDC identity of length 137 (saved as NCI_2). The key "
                              "identifier of NCI_2 differs from NCI_1_ID, since a new unique Network Client Identity "
                              "is created when the ClientIdentifier field is absent. PossessionSignature is absent.")
        response = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_2, credentials=b"", networkIdentity=ni_1),
            endpoint=endpoint)
        self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID_2!r}")
        nci_2 = response.clientIdentity
        self.assert_valid_identity(nci_2, "NetworkConfigResponse.ClientIdentity")
        asserts.assert_not_equal(network_identity_identifier(nci_2), nci_1_id,
                                 "The DUT reused the key of the Network Client Identity of another network "
                                 "configuration even though the ClientIdentifier field was absent.")
        asserts.assert_is_none(response.possessionSignature,
                               "NetworkConfigResponse carries a PossessionSignature even though the "
                               "AddOrUpdateWiFiNetwork command had no PossessionNonce field.")

        self.step(11, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_3, the Credentials field "
                      "set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS and the NetworkIdentity field set to NI_1.",
                  expectation="DUT responds with INVALID_COMMAND, since the Credentials field is required to be empty "
                              "when the NetworkIdentity field is present.")
        credentials = self.non_empty_credentials()
        await self.expect_status(
            cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_3, credentials=credentials, networkIdentity=ni_1),
            endpoint, Status.InvalidCommand, "AddOrUpdateWiFiNetwork with non-empty Credentials and a NetworkIdentity")

        self.step(12, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID_3, the Credentials field "
                      "set to an empty octet string and the NetworkIdentity field set to NI_INVALID.",
                  expectation="DUT responds with DYNAMIC_CONSTRAINT_ERROR.")
        await self.expect_status(
            cnet.Commands.AddOrUpdateWiFiNetwork(ssid=_PDC_SSID_3, credentials=b"", networkIdentity=ni_invalid),
            endpoint, Status.DynamicConstraintError, "AddOrUpdateWiFiNetwork with an invalid NetworkIdentity")

        self.step(13, "TH reads the Networks attribute.",
                  expectation="The list does not contain an entry with the NetworkID field set to PDC_SSID_3, i.e. "
                              "neither of the two failed commands modified the network configuration.")
        networks = await self.read_networks(endpoint)
        asserts.assert_not_in(_PDC_SSID_3, [n.networkID for n in networks],
                              f"A rejected AddOrUpdateWiFiNetwork command still added a configuration for "
                              f"{_PDC_SSID_3!r}.")

        self.step(14, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 0, forcibly disarming the "
                      "fail-safe so that the configuration changes made by this test case are reverted.",
                  expectation="DUT sends an ArmFailSafeResponse.")
        await self.arm_failsafe(0)

        self.step(15, "TH reads the Networks attribute.",
                  expectation="The list is equal to InitialNetworks, i.e. it contains NumNetworks entries, contains no "
                              "entry for PDC_SSID_1 or PDC_SSID_2, and the entry that was connected before the test "
                              "case has its Connected field set to TRUE.")
        networks = await self.read_networks(endpoint)
        asserts.assert_equal(networks, initial_networks,
                             "Disarming the fail-safe did not restore the Networks attribute to its original contents.")


if __name__ == "__main__":
    default_matter_test_main()
