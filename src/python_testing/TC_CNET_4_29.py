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

import asyncio
import logging
import os
import random
import sys

from mobly import asserts
from support_modules.cnet_pdc_support import (FAILSAFE_EXPIRY_SECONDS, RESPONSE_TIMEOUT, CNETPDCBaseTest, PDCAccessPointFixture,
                                              cgen, cnet)
from support_modules.network_identity import POSSESSION_NONCE_LENGTH, network_identity_identifier
from support_modules.wifi_fixture import WiFiFixtureMixin

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.exceptions import ChipStackError
from matter.interaction_model import Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# The network the Wi-Fi fixture runs for this test case, PDC_SSID in the test plan.
# Use a unique SSID so a DUT can't be confused by seeing networks from multiple TH instances.
_PDC_SSID = b"TC-CNET-4.29-PDC-%08X" % random.getrandbits(32)

# The RSN half of that network's configuration; the fixture fills in the EAP and TLS half that
# Matter PDC prescribes. Use a plain WPA3-Enterprise setup without RSN Overriding, since that is
# the most straightforward conformant PDC network setup.
_AP_OPTIONS = {"wpa": 2, "wpa_key_mgmt": "WPA-EAP-SHA256", "rsn_pairwise": "CCMP", "ieee80211w": 2}

# The fail-safe armed in step 3 has to last from the AddOrUpdateWiFiNetwork command that
# creates the Network Client Identity to the CommissioningComplete command that commits it,
# with the DUT joining a network and being rediscovered on it in between.
_CONNECT_FAILSAFE_EXPIRY_SECONDS = 900


def _has_wifi_pdc(wildcard: Clusters.Attribute.AsyncReadTransaction.ReadResponse, endpoint: int) -> bool:
    """EndpointCheckFunction for a Network Commissioning cluster with both WI and PDC."""
    # Two calls because has_feature checks that any of the given bits are set rather than all of them.
    return all(check(wildcard, endpoint) for check in
               (has_feature(cnet, cnet.Bitmaps.Feature.kWiFiNetworkInterface),
                has_feature(cnet, cnet.Bitmaps.Feature.kPerDeviceCredentials)))


class TC_CNET_4_29(WiFiFixtureMixin, CNETPDCBaseTest):

    @property
    def default_timeout(self) -> int:
        # The 90 seconds of the base class cover a test case that talks to a DUT sitting
        # still. This one moves the DUT to another network and back again, and rolls a
        # fail-safe back twice, with operational discovery following most of those.
        return 600

    @run_if_endpoint_matches(_has_wifi_pdc)
    async def test_TC_CNET_4_29(self):
        """[TC-CNET-4.29] [Wi-Fi] Verification for ConnectNetwork command using Per-Device Credentials [DUT-Server]"""

        endpoint = self.get_endpoint()

        self.step("precondition",
                  "DUT is commissioned on PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID. TH claims a radio from the Wi-Fi "
                  "fixture and brings up an access point on it that authenticates clients with Per-Device "
                  "Credentials, with the SSID PDC_SSID and the Network Identity NI_AP. TH generates a random 32 "
                  "octet nonce Nonce_1.",
                  is_commissioning=True)
        nonce_1 = os.urandom(POSSESSION_NONCE_LENGTH)
        # Checked here rather than where the cleanup needs it, since by then the DUT is on
        # PDC_SSID and this is the only way of getting it off again.
        credentials = self.get_credentials()
        asserts.assert_true(credentials, "This test case commits the DUT to the network the TH runs, so it needs "
                                         "PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS (--wifi-passphrase) to put it "
                                         "back on PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID afterwards.")

        radio = self.wifi_fixture.require_radios(1)[0]
        access_point = PDCAccessPointFixture(radio, _PDC_SSID.decode(), _AP_OPTIONS)
        operational_ssid: bytes | None = None
        try:
            access_point.start()
            log.info("Running %r on %s (%s) with the Network Identity %s",
                     _PDC_SSID, radio.ifname, radio.phy, access_point.network_identifier.hex())

            self.step(1, "TH reads the FeatureMap attribute.",
                      expectation="The PDC flag (bit 3) is set, otherwise the test case is skipped.")
            # Reaching this step means both the WI and the PDC flag are set: the decorator
            # skips the test case otherwise.

            self.step(2, "TH reads the Networks attribute and saves it as InitialNetworks.",
                      expectation="Exactly one entry has Connected set to TRUE, which is the network the DUT was "
                                  "commissioned on and whose NetworkID is saved as OPERATIONAL_SSID. No entry refers "
                                  "to PDC_SSID.")
            initial_networks = await self.read_networks(endpoint)
            connected = [network.networkID for network in initial_networks if network.connected]
            asserts.assert_equal(len(connected), 1,
                                 f"Expected exactly one entry of the Networks attribute to be connected, found "
                                 f"{len(connected)}.")
            operational_ssid = connected[0]
            log.info("The DUT is commissioned on %r", operational_ssid)
            asserts.assert_not_in(_PDC_SSID, [network.networkID for network in initial_networks],
                                  f"The DUT already has a network configuration for {_PDC_SSID!r}, which this test "
                                  "case requires to be absent.")

            self.step(3, "TH sends ArmFailSafe to the DUT with ExpiryLengthSeconds set to 900.",
                      expectation="DUT sends an ArmFailSafeResponse.")
            await self.arm_failsafe(_CONNECT_FAILSAFE_EXPIRY_SECONDS)

            self.step(4, "TH sends RemoveNetwork with the NetworkID field set to OPERATIONAL_SSID and the Breadcrumb "
                         "field set to 1.",
                      expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
            response = await self.send_single_cmd(
                cmd=cnet.Commands.RemoveNetwork(networkID=operational_ssid, breadcrumb=1), endpoint=endpoint)
            self.assert_network_config_success(response, f"RemoveNetwork for {operational_ssid!r}")

            self.step(5, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID, the Credentials field "
                         "set to an empty octet string, the NetworkIdentity field set to NI_AP, the PossessionNonce "
                         "field set to Nonce_1 and the Breadcrumb field set to 1.",
                      expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success, a ClientIdentity "
                                  "that is a valid self-signed PDC identity saved as NCI with the associated key "
                                  "identifier saved as NCI_ID, and a PossessionSignature that is a valid ec-signature "
                                  "over the message (NCI || Nonce_1).")
            response = await self.send_single_cmd(
                cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                    ssid=_PDC_SSID, credentials=b"", networkIdentity=access_point.network_identity,
                    possessionNonce=nonce_1, breadcrumb=1),
                endpoint=endpoint)
            self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID!r}")
            nci = response.clientIdentity
            self.assert_valid_identity(nci, "NetworkConfigResponse.ClientIdentity")
            self.assert_valid_possession_signature(nci, nonce_1, response.possessionSignature,
                                                   "NetworkConfigResponse.PossessionSignature")
            nci_id = network_identity_identifier(nci)

            self.step(6, "TH authorizes NCI as a client of the network PDC_SSID.",
                      expectation="The access point accepts the certificate NCI stands for from a client of that "
                                  "network.")
            access_point.authorize(nci)

            self.step(7, "TH sends ConnectNetwork with the NetworkID field set to PDC_SSID and the Breadcrumb field "
                         "set to 2.",
                      expectation="If a ConnectNetworkResponse arrives, its NetworkingStatus is Success and its "
                                  "ErrorValue is null. The DUT is not expected to be able to answer, since carrying "
                                  "the command out takes it off the network the command was sent over.")
            connect_status = await self._connect_network(endpoint, _PDC_SSID, breadcrumb=2)
            if connect_status is not None:
                asserts.assert_equal(connect_status, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                     "The ConnectNetworkResponse of step 7 does not report NetworkingStatus Success.")

            self.step(8, "TH discovers and connects to the DUT on the PDC_SSID operational network, and examines the "
                         "authentication exchange recorded at the access point.",
                      expectation="The TH reaches the DUT again, and the DUT authenticated using EAP-TLS with NCI as "
                                  "its client certificate and an EAP Identity Response carrying the NAI for the "
                                  "Network Identifier of NI_AP.")
            # A station hostapd reports as authorized is its account of the EAP-TLS exchange
            # having succeeded, so NCI having been accepted as the client certificate follows
            # from the station being there at all: it is the only authorized client of this
            # network. Waiting for it also turns a DUT that never associated into a clear
            # failure rather than a discovery timeout.
            station = access_point.wait_for_station()
            log.info("Station %s joined %r", station, _PDC_SSID)
            # The NAI is read back rather than pinned in the eap_user file, so that a wrong one
            # reports itself instead of presenting as a DUT that never associated.
            expected_nai = f"@{access_point.network_identifier.hex().upper()}.pdc.csa-iot.org"
            asserts.assert_equal(station.eap_identity, expected_nai,
                                 "The DUT did not identify itself with the Network Access Identifier for NI_AP.")
            await self.wait_until_reachable()

            self.step(9, "TH reads the Breadcrumb attribute from the General Commissioning cluster.",
                      expectation="The Breadcrumb attribute is 2, i.e. the value carried by the ConnectNetwork "
                                  "command.")
            breadcrumb = await self.read_single_attribute_check_success(
                endpoint=0, cluster=cgen, attribute=cgen.Attributes.Breadcrumb)
            asserts.assert_equal(breadcrumb, 2, "The Breadcrumb attribute does not carry the value of the "
                                                "ConnectNetwork command.")

            self.step(10, "TH reads the Networks attribute.",
                      expectation="The entry with the NetworkID PDC_SSID has Connected set to TRUE, its "
                                  "NetworkIdentifier is the key identifier of NI_AP and its ClientIdentifier is "
                                  "NCI_ID. All other entries have Connected set to FALSE.")
            await self._assert_connected_to_pdc_network(endpoint, access_point, nci_id)

            self.step(11, "TH reads the LastNetworkingStatus, LastNetworkID and LastConnectErrorValue attributes.",
                      expectation="LastNetworkingStatus is Success, LastNetworkID is PDC_SSID and "
                                  "LastConnectErrorValue is null.")
            last_status, last_network_id = await self._read_last_connect_result(endpoint)
            asserts.assert_equal(last_status, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                 "LastNetworkingStatus is not Success.")
            asserts.assert_equal(last_network_id, _PDC_SSID, "LastNetworkID is not PDC_SSID.")
            last_error = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.LastConnectErrorValue)
            asserts.assert_true(last_error is NullValue, f"LastConnectErrorValue is {last_error} rather than null.")

            self.step(12, "TH sends the CommissioningComplete command to the DUT.",
                      expectation="DUT sends a CommissioningCompleteResponse with the ErrorCode field set to OK.")
            # The DUT is committed to PDC_SSID from here, so the cleanup in step 28 is what
            # gets it back onto a network that outlives this test case.
            await self.commissioning_complete()

            self.step(13, "TH sends ArmFailSafe with ExpiryLengthSeconds set to 0, to ensure the CommissioningComplete "
                          "command properly persisted the fail-safe context.",
                      expectation="DUT sends an ArmFailSafeResponse, and the command has no effect.")
            await self.arm_failsafe(0)

            self.step(14, "TH reads the Networks attribute.",
                      expectation="The entry with the NetworkID PDC_SSID still has Connected set to TRUE and its "
                                  "ClientIdentifier set to NCI_ID.")
            await self._assert_connected_to_pdc_network(endpoint, access_point, nci_id)

            self.step(15, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_ID and the PossessionNonce "
                          "field set to Nonce_1.",
                      expectation="DUT sends a QueryIdentityResponse whose Identity is NCI and whose "
                                  "PossessionSignature is a valid ec-signature over the message (NCI || Nonce_1), "
                                  "i.e. the Network Client Identity and its private key were retained across the "
                                  "CommissioningComplete command.")
            await self._assert_identity_retained(endpoint, nci, nci_id, nonce_1)

            self.step(16, "TH sends ArmFailSafe with ExpiryLengthSeconds set to 60. Steps 16 to 27 verify that the DUT "
                          "retains a committed Network Client Identity and its private key while the network "
                          "configuration referring to them is removed or given a new identity within a fail-safe "
                          "period that is subsequently rolled back.",
                      expectation="DUT sends an ArmFailSafeResponse.")
            await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

            self.step(17, "TH sends RemoveNetwork with the NetworkID field set to PDC_SSID.",
                      expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success.")
            response = await self.send_single_cmd(
                cmd=cnet.Commands.RemoveNetwork(networkID=_PDC_SSID), endpoint=endpoint)
            self.assert_network_config_success(response, f"RemoveNetwork for {_PDC_SSID!r}")

            self.step(18, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_ID and the PossessionNonce "
                          "field absent.",
                      expectation="DUT responds with NOT_FOUND, since the only network configuration referring to "
                                  "that Network Client Identity was removed.")
            await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_id), endpoint, Status.NotFound,
                                     "QueryIdentity for the removed network configuration")

            self.step(19, "TH sends ArmFailSafe with ExpiryLengthSeconds set to 0, or waits for the fail-safe timer "
                          "armed in step 16 to expire if it cannot reach the DUT, which is expected to revert the "
                          "removal of the network configuration.",
                      expectation="If an ArmFailSafeResponse arrives, it reports no error.")
            await self.disarm_failsafe()

            self.step(20, "TH re-discovers the DUT on the PDC_SSID operational network if it has become unreachable, "
                          "then reads the Networks attribute.",
                      expectation="The entry with the NetworkID PDC_SSID has Connected set to TRUE, its "
                                  "NetworkIdentifier is the key identifier of NI_AP and its ClientIdentifier is "
                                  "NCI_ID.")
            # The DUT stays associated with PDC_SSID across the rollback, since the connection
            # in place when the fail-safe was armed is also the one the reverted configuration
            # calls for. Some implementations re-establish it anyway, so a transient loss of
            # connectivity here is tolerated rather than treated as a failure.
            await self.wait_until_reachable()
            await self._assert_connected_to_pdc_network(endpoint, access_point, nci_id)

            self.step(21, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_ID and the PossessionNonce "
                          "field set to Nonce_1.",
                      expectation="DUT sends a QueryIdentityResponse whose Identity is NCI and whose "
                                  "PossessionSignature is valid, i.e. the private key was retained as well and the "
                                  "restored network configuration is usable.")
            await self._assert_identity_retained(endpoint, nci, nci_id, nonce_1)

            self.step(22, "TH sends ArmFailSafe with ExpiryLengthSeconds set to 60.",
                      expectation="DUT sends an ArmFailSafeResponse.")
            await self.arm_failsafe(FAILSAFE_EXPIRY_SECONDS)

            self.step(23, "TH sends AddOrUpdateWiFiNetwork with the SSID field set to PDC_SSID, the Credentials field "
                          "set to an empty octet string, the NetworkIdentity field set to NI_AP, and the "
                          "ClientIdentifier and PossessionNonce fields absent.",
                      expectation="DUT sends a NetworkConfigResponse with NetworkingStatus Success and a "
                                  "ClientIdentity whose key identifier, saved as NCI_2_ID, is not NCI_ID, i.e. a new "
                                  "key was generated.")
            # The new identity is never authorized as a client of PDC_SSID, and does not need
            # to be: the DUT stays associated throughout steps 16 to 27 and never reconnects,
            # so NCI_2 never reaches the air.
            response = await self.send_single_cmd(
                cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                    ssid=_PDC_SSID, credentials=b"", networkIdentity=access_point.network_identity),
                endpoint=endpoint)
            self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {_PDC_SSID!r}")
            self.assert_valid_identity(response.clientIdentity, "NetworkConfigResponse.ClientIdentity")
            nci_2_id = network_identity_identifier(response.clientIdentity)
            asserts.assert_not_equal(nci_2_id, nci_id,
                                     "The DUT reused the existing Network Client Identity rather than generating a "
                                     "new key for a network configuration added without a ClientIdentifier.")

            self.step(24, "TH sends ArmFailSafe with ExpiryLengthSeconds set to 0, or waits for the fail-safe timer "
                          "armed in step 22 to expire if it cannot reach the DUT, which is expected to revert the "
                          "change made in step 23.",
                      expectation="If an ArmFailSafeResponse arrives, it reports no error.")
            await self.disarm_failsafe()

            self.step(25, "TH re-discovers the DUT on the PDC_SSID operational network if it has become unreachable, "
                          "then reads the Networks attribute.",
                      expectation="The entry with the NetworkID PDC_SSID has Connected set to TRUE and its "
                                  "ClientIdentifier set to NCI_ID again.")
            await self.wait_until_reachable()
            await self._assert_connected_to_pdc_network(endpoint, access_point, nci_id)

            self.step(26, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_2_ID and the PossessionNonce "
                          "field absent.",
                      expectation="DUT responds with NOT_FOUND, since the Network Client Identity created in step 23 "
                                  "was discarded when the fail-safe was rolled back.")
            await self.expect_status(cnet.Commands.QueryIdentity(keyIdentifier=nci_2_id), endpoint, Status.NotFound,
                                     "QueryIdentity for the Network Client Identity discarded by the rollback")

            self.step(27, "TH sends QueryIdentity with the KeyIdentifier field set to NCI_ID and the PossessionNonce "
                          "field set to Nonce_1.",
                      expectation="DUT sends a QueryIdentityResponse whose Identity is NCI and whose "
                                  "PossessionSignature is valid.")
            await self._assert_identity_retained(endpoint, nci, nci_id, nonce_1)

            self.step(28, "(Cleanup) TH restores the network configuration saved as InitialNetworks, connects the DUT "
                          "back to OPERATIONAL_SSID and sends the CommissioningComplete command. TH ensures that NCI is "
                          "no longer an authorized client of the network PDC_SSID.",
                      expectation="The TH can communicate with the DUT on OPERATIONAL_SSID.")
        finally:
            # The cleanup runs whether the test case passed or not, since a failure from step
            # 12 onwards otherwise strands the DUT on a network that is about to disappear. A
            # failure inside it is worth reporting, but not at the price of replacing the
            # failure that brought us here.
            pending_failure = sys.exc_info()[1]
            try:
                await self._restore_operational_network(endpoint, access_point, operational_ssid, credentials)
            except Exception:
                if pending_failure is None:
                    raise
                log.exception("The DUT could not be put back on %r", operational_ssid)
            finally:
                access_point.close()

    async def _connect_network(self, endpoint: int, network_id: bytes,
                               breadcrumb: int = 0) -> cnet.Enums.NetworkCommissioningStatusEnum | None:
        """Sends ConnectNetwork, and returns its NetworkingStatus or None if it was not answered.

        The DUT answers over the network it is joining and nothing routes between that network
        and the one the command was sent over, so the response is expected to be lost rather
        than slow. The test plan asks for it to be verified where it does arrive, and leaves
        the outcome of the connection attempt itself to the steps that follow.
        """
        try:
            response = await asyncio.wait_for(
                self.send_single_cmd(cmd=cnet.Commands.ConnectNetwork(networkID=network_id, breadcrumb=breadcrumb),
                                     endpoint=endpoint),
                timeout=RESPONSE_TIMEOUT)
        except (TimeoutError, ChipStackError) as error:
            log.info("No ConnectNetworkResponse arrived (%s); the steps that follow go on the observable outcome",
                     error)
            return None
        asserts.assert_is_instance(response, cnet.Commands.ConnectNetworkResponse,
                                   "ConnectNetwork did not return a ConnectNetworkResponse.")
        if response.networkingStatus == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess:
            asserts.assert_true(response.errorValue is NullValue,
                                f"ConnectNetworkResponse.ErrorValue is {response.errorValue} rather than null.")
        return response.networkingStatus

    async def _read_last_connect_result(self, endpoint: int) -> tuple[cnet.Enums.NetworkCommissioningStatusEnum, bytes]:
        """Reads the LastNetworkingStatus and LastNetworkID attributes, neither of which may be null."""
        status = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.LastNetworkingStatus)
        asserts.assert_true(status is not NullValue, "LastNetworkingStatus is null after a connection attempt.")
        network_id = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.LastNetworkID)
        asserts.assert_true(network_id is not NullValue, "LastNetworkID is null after a connection attempt.")
        return status, network_id

    async def _assert_connected_to_pdc_network(self, endpoint: int, access_point: PDCAccessPointFixture,
                                               client_identifier: bytes) -> None:
        """Checks that the Networks attribute reports PDC_SSID as the connected network."""
        networks = await self.read_networks(endpoint)
        network = self.find_network(networks, _PDC_SSID)
        asserts.assert_true(network.connected, f"The entry for {_PDC_SSID!r} does not have Connected set to TRUE.")
        asserts.assert_equal(self.assert_nullable_identifier(network.networkIdentifier, "NetworkIdentifier"),
                             access_point.network_identifier,
                             f"The NetworkIdentifier of the entry for {_PDC_SSID!r} is not the key identifier of "
                             "NI_AP.")
        asserts.assert_equal(self.assert_nullable_identifier(network.clientIdentifier, "ClientIdentifier"),
                             client_identifier,
                             f"The ClientIdentifier of the entry for {_PDC_SSID!r} is not the expected Network "
                             "Client Identity.")
        for other in networks:
            if other.networkID != _PDC_SSID:
                asserts.assert_false(other.connected,
                                     f"The entry for {other.networkID!r} has Connected set to TRUE as well.")

    async def _assert_identity_retained(self, endpoint: int, nci: bytes, nci_id: bytes, nonce: bytes) -> None:
        """Checks that the DUT still holds the Network Client Identity NCI and its private key."""
        response = await self.query_identity(endpoint, nci_id, nonce)
        asserts.assert_equal(response.identity, nci,
                             "QueryIdentityResponse.Identity is not the Network Client Identity the DUT returned "
                             "when the network configuration was added.")
        self.assert_valid_possession_signature(nci, nonce, response.possessionSignature,
                                               "QueryIdentityResponse.PossessionSignature")

    async def _restore_operational_network(self, endpoint: int, access_point: PDCAccessPointFixture,
                                           operational_ssid: bytes | None, credentials: str) -> None:
        """Puts the DUT back on the network it was commissioned on, and takes the fixture down.

        What needs doing depends on where the test case got to, so this goes on the state the
        DUT is actually in rather than on the steps that ran. Where the DUT is on
        OPERATIONAL_SSID already, whether because the test case never took it away or because
        a fail-safe rollback brought it back, there is nothing to restore.
        """
        if operational_ssid is None:
            return  # Never got as far as finding out which network that is.

        await self.wait_until_reachable()
        networks = await self.read_networks(endpoint)
        operational = next((network for network in networks if network.networkID == operational_ssid), None)
        needs_restore = operational is None or not operational.connected

        if not needs_restore:
            log.info("The DUT is on %r already, so there is nothing to restore", operational_ssid)
        else:
            log.info("Putting the DUT back on %r", operational_ssid)
            await self.arm_failsafe(_CONNECT_FAILSAFE_EXPIRY_SECONDS)
            if any(network.networkID == _PDC_SSID for network in networks):
                # Not only tidiness: the DUT may have no room for another network otherwise.
                response = await self.send_single_cmd(
                    cmd=cnet.Commands.RemoveNetwork(networkID=_PDC_SSID), endpoint=endpoint)
                self.assert_network_config_success(response, f"RemoveNetwork for {_PDC_SSID!r}")
            response = await self.send_single_cmd(
                cmd=cnet.Commands.AddOrUpdateWiFiNetwork(ssid=operational_ssid, credentials=credentials.encode()),
                endpoint=endpoint)
            self.assert_network_config_success(response, f"AddOrUpdateWiFiNetwork for {operational_ssid!r}")
            # A lost ConnectNetworkResponse is the expected case, here as everywhere else in this
            # test case, and whether the DUT arrived is what the re-discovery below answers. A
            # response that did arrive and reports a failure is worth stopping on, since none of
            # what follows can succeed after it.
            status = await self._connect_network(endpoint, operational_ssid)
            if status is not None:
                asserts.assert_equal(status, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                     f"ConnectNetwork for {operational_ssid!r} did not report NetworkingStatus "
                                     "Success.")

        # Taking the access point down before we attempt to re-discover the DUT on the original
        # network gets the DUT's address on this link out of the way: it would otherwise stay in
        # the mDNS resolver's cache for the record's TTL, and a link-local address is what the
        # sorter scores highest, so for as long as the interface is there the TH keeps being handed
        # the address the DUT has left.
        access_point.close()

        if needs_restore:
            await self.wait_until_reachable()
            await self.commissioning_complete()
            log.info("The DUT is back on %r", operational_ssid)


if __name__ == "__main__":
    default_matter_test_main()
