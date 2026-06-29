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
import contextlib
import logging
import socket

from mobly import asserts
from zeroconf import ServiceBrowser, ServiceListener, Zeroconf

import matter.clusters as Clusters
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout constants
TIMED_REQUEST_TIMEOUT_MS = 5000         # Matter timed-request window (5s)
MDNS_DISCOVERY_TIMEOUT = 10             # mDNS discovery timeout per attempt (10s)
SCAN_RETRY_DELAY = 3                    # Delay between scan retries (3s)
MAX_ATTEMPTS = 3                        # Maximum discovery/retry attempts
TIMEOUT = 180                           # Overall test timeout (3 min)

# Cluster references
cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning


class MatterServiceListener(ServiceListener):
    """
    Zeroconf service listener for Matter devices.
    Collects all discovered Matter services with optional device ID filtering.
    Supports early return via an asyncio.Event when a matching service is found.
    """

    def __init__(self, target_device_id=None, found_event: asyncio.Event = None):
        self.discovered_services = []
        self.target_device_id = target_device_id
        self._found_event = found_event

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        try:
            service_info = zc.get_service_info(type_, name)
            if service_info:
                addresses = []
                for addr in service_info.addresses:
                    if len(addr) == 4:  # IPv4
                        addresses.append(socket.inet_ntoa(addr))
                    elif len(addr) == 16:  # IPv6
                        addresses.append(socket.inet_ntop(socket.AF_INET6, addr))
                service_data = {
                    'name': name.replace(f'.{type_}', ''),
                    'type': type_,
                    'domain': 'local',
                    'addresses': addresses,
                    'port': service_info.port,
                    'properties': service_info.properties
                }

                if self.target_device_id:
                    service_name = service_data['name']
                    target_id_str = str(self.target_device_id)

                    if target_id_str.isdigit():
                        target_id_hex = format(int(target_id_str), 'X')
                    else:
                        target_id_hex = target_id_str

                    if (target_id_str.upper() in service_name.upper() or
                        target_id_hex.upper() in service_name.upper() or
                            target_id_hex.upper().zfill(16) in service_name.upper()):
                        self.discovered_services.append(service_data)
                        if self._found_event:
                            self._found_event.set()
                else:
                    self.discovered_services.append(service_data)
                    if self._found_event:
                        self._found_event.set()
        except Exception as e:
            logger.warning("MatterServiceListener: Failed to get service info for %s: %s", name, e)

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass


async def find_matter_devices_mdns(target_device_id: int = None) -> list:
    """
    Finds Matter devices via mDNS using zeroconf, optionally filtering by target device ID.
    Returns as soon as a matching device is found, or after MDNS_DISCOVERY_TIMEOUT per attempt.
    Raises an exception if no device is found after MAX_ATTEMPTS.
    """
    service_types = ["_matter._tcp.local.", "_matterc._udp.local."]

    logger.info(
        " --- find_matter_devices_mdns: Searching for Matter devices%s",
        ' with target device ID: ' + str(target_device_id) if target_device_id else '')

    for attempt in range(1, MAX_ATTEMPTS + 1):
        zc = None
        try:
            found_event = asyncio.Event()
            zc = Zeroconf()
            listener = MatterServiceListener(target_device_id, found_event)
            browsers = [ServiceBrowser(zc, stype, listener) for stype in service_types]

            # Wait until a device is found or the timeout expires
            with contextlib.suppress(asyncio.TimeoutError):
                await asyncio.wait_for(found_event.wait(), timeout=MDNS_DISCOVERY_TIMEOUT)

            for browser in browsers:
                browser.cancel()

            if listener.discovered_services:
                return listener.discovered_services

        except Exception as e:
            logger.error(" --- find_matter_devices_mdns: Discovery attempt %d failed: %s", attempt, e)
        finally:
            if zc is not None:
                try:
                    zc.close()
                except Exception as e:
                    logger.warning("Failed to close zeroconf instance: %s", e)

        if attempt < MAX_ATTEMPTS:
            await asyncio.sleep(SCAN_RETRY_DELAY)

    raise Exception(
        f"find_matter_devices_mdns: mDNS discovery failed after {MAX_ATTEMPTS} attempts - "
        f"No Matter devices found"
        f"{' for target device ID: ' + str(target_device_id) if target_device_id else ''}")


class TC_CNET_4_23(MatterBaseTest):

    async def _validate_network_config_response(
        self,
        response: cnet.Commands.NetworkConfigResponse,
        expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None
    ) -> None:
        expected_status = expected_status or cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected response type from NetworkConfig command")
        asserts.assert_equal(response.networkingStatus, expected_status,
                             f"Expected NetworkingStatus {expected_status}, got {response.networkingStatus}")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _validate_connect_network_response(
        self,
        response: cnet.Commands.ConnectNetworkResponse,
        expect_success: bool = True
    ) -> None:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected response type from ConnectNetwork command")
        if expect_success:
            asserts.assert_equal(response.networkingStatus,
                                 cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                 f"Expected success, got {response.networkingStatus}")
        else:
            asserts.assert_not_equal(response.networkingStatus,
                                     cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                     "Expected failure, got success")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _read_networks(self, endpoint: int) -> list:
        return await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks,
        )

    async def _read_last_networking_status(
        self,
        endpoint: int,
        expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None,
        valid_statuses: list = None
    ) -> cnet.Enums.NetworkCommissioningStatusEnum:
        status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(" --- LastNetworkingStatus = %s", status)
        if expected_status is not None:
            asserts.assert_equal(status, expected_status,
                                 f"Expected {expected_status}, got {status}")
        elif valid_statuses is not None:
            asserts.assert_in(status, valid_statuses,
                              f"Expected one of {valid_statuses}, got {status}")
        return status

    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_23(self):
        return [
            TestStep(0, "TH begins commissioning the DUT over the initial commissioning radio (PASE):\n"
                        "Skip CommissioningComplete to manually configure WiFi with incorrect then correct credentials\n"
                        "Extend fail-safe to 300 seconds to allow time for credential testing",
                        is_commissioning=False),
            TestStep(1, "TH reads Networks attribute and removes all configured networks",
                        "Verify that DUT successfully removed all networks configured during commissioning"),
            TestStep(2, "TH sends AddOrUpdateWiFiNetwork with incorrect SSID and correct password, Breadcrumb = 1",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(3, "TH sends ConnectNetwork command with incorrect SSID, Breadcrumb = 2",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(4, "TH reads LastNetworkingStatus (should be kNetworkNotFound)",
                        "Verify LastNetworkingStatus is kNetworkNotFound (5)"),
            TestStep(5, "TH reads Networks attribute",
                        "Verify incorrect SSID is in the network list"),
            TestStep(6, "TH sends RemoveNetwork command with incorrect SSID, Breadcrumb = 3",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)"),
            TestStep(7, "TH reads Networks attribute",
                        "Verify Networks list is empty after removal"),
            TestStep(8, "TH sends AddOrUpdateWiFiNetwork with incorrect password and Breadcrumb = 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(9, "TH sends ConnectNetwork command and Breadcrumb = 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(10, "TH reads LastNetworkingStatus (should be kAuthFailure or kOtherConnectionFailure)",
                         "Verify LastNetworkingStatus to be 'kAuthFailure' or 'kOtherConnectionFailure'"),
            TestStep(11, "TH sends ScanNetworks command with Breadcrumb = 6",
                         "Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:\n"
                         "1. wiFiScanResults contains the target network SSID"),
            TestStep(12, "TH sends AddOrUpdateWiFiNetwork with correct credentials and Breadcrumb = 7",
                         "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                         "1. NetworkingStatus is kSuccess (0)\n"
                         "2. DebugText is of type string with max length 512 or empty"),
            TestStep(13, "TH sends ConnectNetwork command and Breadcrumb = 8",
                         "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                         "1. NetworkingStatus is kSuccess (0)\n"
                         "2. DebugText is of type string with max length 512 or empty"),
            TestStep(14, "TH reads LastNetworkingStatus (should be kSuccess)",
                         "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(15, "TH reads Networks attribute",
                         "Verify the device is connected to the correct network"),
            TestStep(16, "TH sends CommissioningComplete to finalize commissioning",
                         "Verify that DUT sends CommissioningCompleteResponse with the following fields:\n"
                         "1. ErrorCode field set to OK (0)"),
        ]

    def desc_TC_CNET_4_23(self):
        return "[TC-CNET-4.23] [Wi-Fi] Network Commissioning Success After Connection Failures [DUT-Server]"

    @async_test_body
    async def test_TC_CNET_4_23(self):

        endpoint = ROOT_ENDPOINT_ID

        correct_ssid = self.matter_test_config.wifi_ssid.encode('utf-8')
        correct_password = self.matter_test_config.wifi_passphrase.encode('utf-8')

        incorrect_ssid = b"IncorrectSSID_12345"
        incorrect_password = b"IncorrectPassword123"

        # Step 0: Commission over PASE, skip CommissioningComplete
        self.step(0)

        # Temporarily inject an incorrect password so the automated commissioning flow
        # does NOT successfully connect the DUT to WiFi. The framework will reach the
        # WiFiNetworkEnable stage, fail (because the password is wrong), and then enter
        # its Cleanup stage — but it does NOT disarm the fail-safe during cleanup, so the
        # PASE session stays alive and we can continue with manual network-commissioning
        # steps below.

        self.matter_test_config.wifi_passphrase = incorrect_password.decode('utf-8', errors='replace')
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        try:
            await self.commission_devices()
        finally:
            self.matter_test_config.wifi_passphrase = correct_password.decode('utf-8', errors='replace')

        # Extend fail-safe to 300 seconds to allow time for credential testing
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0)
        )
        logger.info(" --- Extended fail-safe timer to 300 seconds")

        # Step 1: Remove all configured networks
        self.step(1)

        networks = await self._read_networks(endpoint)
        logger.info(" --- Found %d network(s) configured during commissioning", len(networks))
        for network in networks:
            network_id = network.networkID
            logger.info(" --- Removing network: %s", network_id.decode('utf-8', errors='replace'))
            remove_response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.RemoveNetwork(networkID=network_id, breadcrumb=0)
            )
            await self._validate_network_config_response(remove_response)
            logger.info(" --- Network removed successfully: %s", network_id.decode('utf-8', errors='replace'))

        networks_after = await self._read_networks(endpoint)
        asserts.assert_equal(len(networks_after), 0,
                             f"Expected empty network list after cleanup, but found {len(networks_after)} network(s)")
        logger.info(" --- All networks successfully removed.")

        # Step 2: AddOrUpdateWiFiNetwork with incorrect SSID
        self.step(2)
        logger.info(" --- Using incorrect SSID: %s with correct password", incorrect_ssid.decode())
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=incorrect_ssid, credentials=correct_password, breadcrumb=1),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        # Step 3: ConnectNetwork with incorrect SSID — expect failure
        self.step(3)
        logger.info(" --- Sending ConnectNetwork with incorrect SSID...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=incorrect_ssid, breadcrumb=2))
        await self._validate_connect_network_response(response, expect_success=False)
        logger.info(" --- ConnectNetwork failed as expected for incorrect SSID: %s", incorrect_ssid.decode())

        # Step 4: Read LastNetworkingStatus — expect kNetworkNotFound
        self.step(4)
        await self._read_last_networking_status(
            endpoint, expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound)

        # Step 5: Read Networks — verify incorrect SSID is stored
        self.step(5)
        networks = await self._read_networks(endpoint)
        logger.info(" --- Networks attribute has %d network(s)", len(networks))
        network_ids = [net.networkID for net in networks]
        asserts.assert_in(incorrect_ssid, network_ids,
                          f"Expected incorrect SSID {incorrect_ssid.decode()} to be in Networks list")

        # Step 6: RemoveNetwork with incorrect SSID
        self.step(6)
        logger.info(" --- Removing incorrect SSID: %s", incorrect_ssid.decode())
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(networkID=incorrect_ssid, breadcrumb=3),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        # Step 7: Read Networks — verify list is empty
        self.step(7)
        networks = await self._read_networks(endpoint)
        logger.info(" --- Networks attribute has %d network(s) after removal", len(networks))
        asserts.assert_equal(len(networks), 0,
                             f"Expected Networks list to be empty, but has {len(networks)} network(s)")

        # Step 8: AddOrUpdateWiFiNetwork with correct SSID but incorrect password
        self.step(8)
        logger.info(" --- Using correct SSID: %s with incorrect password", correct_ssid.decode('utf-8', errors='replace'))
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid, credentials=incorrect_password, breadcrumb=4),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        # Step 9: ConnectNetwork with incorrect password — expect failure
        self.step(9)
        logger.info(" --- Sending ConnectNetwork with incorrect credentials...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=correct_ssid, breadcrumb=5))
        await self._validate_connect_network_response(response, expect_success=False)
        logger.info(" --- ConnectNetwork failed as expected with incorrect credentials")

        # Step 10: Read LastNetworkingStatus — expect kAuthFailure or kOtherConnectionFailure
        self.step(10)
        await self._read_last_networking_status(
            endpoint,
            valid_statuses=[
                cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
                cnet.Enums.NetworkCommissioningStatusEnum.kOtherConnectionFailure])

        # Step 11: ScanNetworks — verify target SSID is visible
        self.step(11)
        target_ssid_found = False

        for attempt in range(1, MAX_ATTEMPTS + 1):
            logger.info(" --- ScanNetworks attempt %d/%d", attempt, MAX_ATTEMPTS)
            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ScanNetworks(breadcrumb=6))
            asserts.assert_true(isinstance(response, cnet.Commands.ScanNetworksResponse),
                                "Unexpected value returned from ScanNetworks")

            ssids_found = [network.ssid for network in response.wiFiScanResults]
            logger.info(" --- Scan results: Found %d networks", len(ssids_found))

            if len(ssids_found) > 0:
                for i, ssid in enumerate(ssids_found):
                    logger.info(" --- Network %d: %s (type: %s)", i, ssid, type(ssid))
                if correct_ssid in ssids_found:
                    target_ssid_found = True
                    logger.info(" --- Found target SSID: %s on attempt %d", correct_ssid.decode('utf-8', errors='replace'), attempt)
                    break
                logger.warning(" --- Target SSID not found in scan results (attempt %d)", attempt)
            else:
                logger.warning(" --- No networks found in scan (attempt %d)", attempt)

            if not target_ssid_found and attempt < MAX_ATTEMPTS:
                logger.info(" --- Waiting %ds before retry...", SCAN_RETRY_DELAY)
                await asyncio.sleep(SCAN_RETRY_DELAY)

        asserts.assert_true(target_ssid_found,
                            f"ScanNetworks did not find target SSID "
                            f"'{correct_ssid.decode('utf-8', errors='replace')}' after {MAX_ATTEMPTS} attempts")

        # Step 12: AddOrUpdateWiFiNetwork with correct credentials
        self.step(12)
        logger.info("Using correct SSID: %s with correct password", correct_ssid.decode('utf-8', errors='replace'))
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid, credentials=correct_password, breadcrumb=7),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        # Step 13: ConnectNetwork with correct credentials — expect success
        self.step(13)
        logger.info(" --- Sending ConnectNetwork with correct credentials...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=correct_ssid, breadcrumb=8))
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(" --- ConnectNetwork succeeded for SSID: %s", correct_ssid.decode('utf-8', errors='replace'))

        # Step 14: Read LastNetworkingStatus — expect kSuccess
        self.step(14)
        await self._read_last_networking_status(
            endpoint, expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)

        # Step 15: Read Networks — verify connected to correct network
        self.step(15)
        response = await self._read_networks(endpoint)
        connected_networks = [network.networkID for network in response if network.connected]
        asserts.assert_true(correct_ssid in connected_networks,
                            f"Expected device to be connected to SSID '{correct_ssid.decode('utf-8', errors='replace')}'")
        logger.info(" --- Device is connected to SSID: %s", correct_ssid.decode('utf-8', errors='replace'))

        # Step 16: CommissioningComplete over CASE
        self.step(16)

        # Discover device on WiFi network via mDNS to verify it is reachable
        # before switching from PASE to CASE
        logger.info(" --- Discovering device on WiFi network via mDNS...")
        discovered_devices = await find_matter_devices_mdns(self.dut_node_id)
        logger.info(" --- Found %d device(s) via mDNS", len(discovered_devices))
        asserts.assert_greater(len(discovered_devices), 0,
                               "No devices found via mDNS after WiFi connection")

        # Close PASE session to force CASE session establishment over WiFi.
        # CloseBLEConnection() and ExpireSessions() are synchronous operations
        # on the controller side — no additional delay is needed.
        logger.info(" --- Closing BLE connection and expiring PASE session...")
        self.default_controller.CloseBLEConnection()
        self.default_controller.ExpireSessions(self.dut_node_id)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse),
                            "Expected CommissioningCompleteResponse")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected errorCode OK (0), got {response.errorCode}")
        logger.info(" --- CommissioningComplete sent successfully, commissioning finalized.")


if __name__ == "__main__":
    default_matter_test_main()
