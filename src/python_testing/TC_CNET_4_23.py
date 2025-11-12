#
#    Copyright (c) 2025 Project CHIP Authors
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
import socket

from mobly import asserts
from zeroconf import ServiceBrowser, ServiceListener, Zeroconf

import matter.clusters as Clusters
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

# Timeout constants
TIMED_REQUEST_TIMEOUT_MS = 5000         # Matter command timeout (5s)
CONNECT_NETWORK_TIMEOUT_MS = 30000      # ConnectNetwork timeout (30s)
MDNS_DISCOVERY_TIMEOUT = 10             # mDNS discovery timeout per attempt (10s)
SCAN_RETRY_DELAY = 3                    # Delay between scan retries (3s)
MAX_ATTEMPTS = 3                        # Maximum discovery attempts
TIMEOUT = 180                           # Overall test timeout (3 min)

# Global variable to store target device ID for mDNS discovery
_target_device_id = None

# Cluster references
cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning


def get_target_device_id():
    """Get the cached target device ID for mDNS discovery."""
    return _target_device_id


def set_target_device_id(device_id):
    """Set the target device ID for mDNS discovery."""
    global _target_device_id
    _target_device_id = str(device_id) if device_id is not None else None


class MatterServiceListener(ServiceListener):
    """
    Zeroconf service listener for Matter devices.
    Collects all discovered Matter services with optional device ID filtering.
    """

    def __init__(self, target_device_id=None):
        self.discovered_services = []
        self.target_device_id = target_device_id

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        try:
            service_info = zc.get_service_info(type_, name)
            if service_info:
                addresses = [
                    socket.inet_ntoa(addr) for addr in service_info.addresses if len(addr) == 4
                ]
                service_data = {
                    'name': name.replace(f'.{type_}', ''),
                    'type': type_,
                    'domain': 'local',
                    'addresses': addresses,
                    'port': service_info.port,
                    'properties': service_info.properties
                }

                # Filter by target device ID if specified
                if self.target_device_id:
                    # The device ID can be in the service name or properties
                    service_name = service_data['name']
                    target_id_str = str(self.target_device_id)

                    # Convert numeric node ID to hex format if needed
                    if target_id_str.isdigit():
                        target_id_hex = format(int(target_id_str), 'X')
                    else:
                        target_id_hex = target_id_str

                    # Check if the target device ID (in various formats) is in the service name
                    if (target_id_str.upper() in service_name.upper() or
                        target_id_hex.upper() in service_name.upper() or
                            target_id_hex.upper().zfill(16) in service_name.upper()):
                        self.discovered_services.append(service_data)
                    else:
                        pass
                else:
                    # No filtering, add all services
                    self.discovered_services.append(service_data)
        except Exception as e:
            logger.warning(f"MatterServiceListener: Failed to get service info for {name}: {e}")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass


async def find_matter_devices_mdns():
    """
    Finds Matter devices via mDNS using zeroconf, optionally filtering by target device ID.
    Raises an exception if no device is found after MAX_ATTEMPTS.
    Returns the list of discovered services.
    """
    service_types = ["_matter._tcp.local.", "_matterc._udp.local."]
    target_device_id = get_target_device_id()

    logger.info(
        f"find_matter_devices_mdns: Searching for Matter devices{' with target device ID: ' + target_device_id if target_device_id else ''}")

    for attempt in range(1, MAX_ATTEMPTS + 1):
        try:
            zc = Zeroconf()
            listener = MatterServiceListener(target_device_id)
            browsers = [ServiceBrowser(zc, stype, listener) for stype in service_types]

            await asyncio.sleep(MDNS_DISCOVERY_TIMEOUT)

            # Cleanup
            for browser in browsers:
                browser.cancel()
            zc.close()

            if listener.discovered_services:
                return listener.discovered_services

        except Exception as e:
            logger.error(f"find_matter_devices_mdns: Discovery attempt {attempt} failed: {e}")
            try:
                zc.close()
            except Exception:
                pass

        if attempt < MAX_ATTEMPTS:
            await asyncio.sleep(2)

    raise Exception(
        f"find_matter_devices_mdns: mDNS discovery failed after {MAX_ATTEMPTS} attempts - No Matter devices found{' for target device ID: ' + target_device_id if target_device_id else ''}")


class TC_CNET_4_23(MatterBaseTest):

    async def _validate_network_config_response(self, response, expected_status=None):
        expected_status = expected_status or cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected response type from NetworkConfig command")
        asserts.assert_equal(response.networkingStatus, expected_status,
                             f"Expected NetworkingStatus {expected_status}, got {response.networkingStatus}")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _validate_connect_network_response(self, response, expect_success=True):
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

    async def _read_networks(self, endpoint):
        return await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks,
        )

    async def _read_last_networking_status(self, endpoint, expected_status=None, valid_statuses=None):
        status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(f"LastNetworkingStatus = {status}")
        if expected_status is not None:
            asserts.assert_equal(status, expected_status,
                                 f"Expected {expected_status}, got {status}")
        elif valid_statuses is not None:
            asserts.assert_in(status, valid_statuses,
                              f"Expected one of {valid_statuses}, got {status}")
        return status

    # Overrides default_timeout
    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_23(self):
        return [
            TestStep(0, "TH begins commissioning the DUT over the initial commissioning radio (PASE):\n"
                        "Skip CommissioningComplete to manually configure WiFi with incorrect then correct credentials\n"
                        "TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint 0",
                        is_commissioning=False),
            TestStep(1, "TH reads Networks attribute and removes all configured networks",
                        "Verify that DUT successfully removes all networks configured during commissioning"),
            TestStep(2, "TH sends AddOrUpdateWiFiNetwork with WRONG SSID and correct password, Breadcrumb = 1",
                        "Verify that DUT sends NetworkConfigResponse:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(3, "TH sends ConnectNetwork command with wrong SSID, Breadcrumb = 2",
                        "Verify that DUT sends ConnectNetworkResponse:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(4, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus is kNetworkNotFound (5)"),
            TestStep(5, "TH reads Networks attribute",
                        "Verify wrong SSID is in the network list"),
            TestStep(6, "TH sends RemoveNetwork command with wrong SSID, Breadcrumb = 3",
                        "Verify that DUT sends NetworkConfigResponse:\n"
                        "1. NetworkingStatus is kSuccess (0)"),
            TestStep(7, "TH reads Networks attribute",
                        "Verify Networks list is empty after removal"),
            TestStep(8, "TH sends AddOrUpdateWiFiNetwork with INCORRECT PASSWORD and Breadcrumb field set to 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty\n"),
            TestStep(9, "TH sends ConnectNetwork command and Breadcrumb field set to 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(10, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                     "Verify LastNetworkingStatus to be 'kAuthFailure'"),
            TestStep(11, "TH sends ScanNetworks command with Breadcrumb field set to 6",
                     "Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:\n"
                     "1. wiFiScanResults contains the target network SSID"),
            TestStep(12, "TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 7",
                     "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                     "1. NetworkingStatus is kSuccess which is '0'\n"
                     "2. DebugText is of type string with max length 512 or empty"),
            TestStep(13, "TH sends ConnectNetwork command and Breadcrumb field set to 8",
                     "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                     "1. NetworkingStatus is kSuccess which is '0'\n"
                     "2. DebugText is of type string with max length 512 or empty"),
            TestStep(14, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                     "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(15, "TH reads Networks attribute",
                     "Verify device is connected to correct network"),
            TestStep(16, "TH sends CommissioningComplete to finalize commissioning",
                         "Verify that DUT sends CommissioningCompleteResponse with the following fields:\n"
                         "1. ErrorCode field set to OK (0)"),
        ]

    def desc_TC_CNET_4_23(self):
        return "[TC-CNET-4.23] [Wi-Fi] Verification for Commissioning [DUT-Server]"

    @async_test_body
    async def test_TC_CNET_4_23(self):

        # Network Commissioning cluster is always on root endpoint (0) during commissioning
        endpoint = ROOT_ENDPOINT_ID

        # Save correct credentials (used by commissioning framework and for final connection)
        correct_ssid = self.matter_test_config.wifi_ssid.encode('utf-8')
        correct_password = self.matter_test_config.wifi_passphrase.encode('utf-8')

        # Create wrong credentials for test commands
        wrong_ssid = b"WrongSSID_12345"
        self.matter_test_config.wifi_passphrase = "WrongPassword123"
        wrong_password = self.matter_test_config.wifi_passphrase.encode('utf-8')

        # Step 0: TH begins commissioning the DUT over the initial commissioning radio (PASE):
        self.step(0)

        # Skip CommissioningComplete to manually configure WiFi with incorrect then correct credentials
        self.default_controller.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        await self.commission_devices()

        # Extend fail-safe to 300 seconds to allow time for credential testing
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0)
        )
        logger.info("Extended fail-safe timer to 300 seconds")

        # Set target device ID for mDNS discovery
        set_target_device_id(self.dut_node_id)
        logger.info(f"Set target device ID for mDNS filtering: {self.dut_node_id}")

        # TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint 0
        feature_map = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.FeatureMap,
        )

        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info("Device does not support WiFi on endpoint 0, skipping remaining steps")
            self.skip_all_remaining_steps(1)
            return

        # Step 1: TH reads Networks attribute and removes all configured networks
        self.step(1)

        logger.info(" --- Step 1: Reading and removing all configured networks from commissioning...")
        networks = await self._read_networks(endpoint)

        logger.info(f"Found {len(networks)} network(s) configured during commissioning")
        for network in networks:
            network_id = network.networkID
            logger.info(f" --- Removing network: {network_id.decode('utf-8', errors='replace')}")
            remove_response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.RemoveNetwork(
                    networkID=network_id,
                    breadcrumb=0
                )
            )
            await self._validate_network_config_response(remove_response)
            logger.info(f" --- Network removed successfully: {network_id.decode('utf-8', errors='replace')}")

        # Verify networks list is now empty
        networks_after = await self._read_networks(endpoint)
        asserts.assert_equal(len(networks_after), 0,
                             f"Expected empty network list after cleanup, but found {len(networks_after)} network(s)")
        logger.info(" --- All networks successfully removed. Ready for manual WiFi configuration tests.")

        # Step 2: TH sends AddOrUpdateWiFiNetwork with WRONG SSID and correct password
        self.step(2)

        logger.info(f" --- Step 2: Using WRONG SSID: {wrong_ssid.decode('utf-8')} with correct password")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=wrong_ssid,
                credentials=correct_password,
                breadcrumb=1
            )
        )
        await self._validate_network_config_response(response)

        # Step 3: TH sends ConnectNetwork command with wrong SSID
        self.step(3)

        logger.info(" --- Step 3: Sending ConnectNetwork with wrong SSID...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=wrong_ssid,
                breadcrumb=2
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_connect_network_response(response, expect_success=False)
        logger.info(f"ConnectNetwork failed as expected for wrong SSID: {wrong_ssid.decode('utf-8')}")

        # Wait for DUT to complete connection attempt and update LastNetworkingStatus
        logger.info(" --- Waiting 3 seconds for DUT to update network status...")
        await asyncio.sleep(3)

        # Step 4: TH reads LastNetworkingStatus (should be kNetworkNotFound)
        self.step(4)

        await self._read_last_networking_status(
            endpoint,
            expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound
        )

        # Step 5: TH reads Networks attribute (verify wrong SSID is stored)
        self.step(5)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Step 5: Networks attribute has {len(networks)} network(s)")
        network_ids = [net.networkID for net in networks]
        asserts.assert_in(wrong_ssid, network_ids,
                          f"Expected wrong SSID {wrong_ssid.decode('utf-8')} to be in Networks list")

        # Step 6: TH sends RemoveNetwork to clean up wrong SSID
        self.step(6)

        logger.info(f" --- Step 6: Removing wrong SSID: {wrong_ssid.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wrong_ssid,
                breadcrumb=3
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # Step 7: TH reads Networks attribute (verify list is empty)
        self.step(7)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Step 7: Networks attribute has {len(networks)} network(s) after removal")
        asserts.assert_equal(len(networks), 0,
                             f"Expected Networks list to be empty, but has {len(networks)} network(s)")

        # TH sends AddOrUpdateWiFiNetwork with INCORRECT credentials and Breadcrumb field set to 1
        self.step(8)

        logger.info(
            f" --- Step 8: Using correct SSID: {correct_ssid.decode('utf-8')} with incorrect Password: {wrong_password.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=wrong_password,
                breadcrumb=4
            )
        )
        await self._validate_network_config_response(response)

        # TH sends ConnectNetwork command and Breadcrumb field set to 2
        self.step(9)

        logger.info(" --- Step 9: Sending ConnectNetwork with incorrect credentials...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=5
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_connect_network_response(response, expect_success=False)
        logger.info(f"ConnectNetwork failed as expected for SSID: {correct_ssid.decode('utf-8')} with incorrect credentials")

        # Wait for DUT to complete connection attempt and update LastNetworkingStatus
        logger.info(" --- Waiting 3 seconds for DUT to update network status...")
        await asyncio.sleep(3)

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(10)

        # Verify LastNetworkingStatus indicates a failure
        # Can be kAuthFailure (wrong password), kNetworkNotFound (network not in range),
        # or kOtherConnectionFailure (general connection failure)
        valid_failure_statuses = [
            cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
            cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound,
            cnet.Enums.NetworkCommissioningStatusEnum.kOtherConnectionFailure
        ]
        await self._read_last_networking_status(
            endpoint,
            valid_statuses=valid_failure_statuses
        )

        # TH sends ScanNetworks command
        self.step(11)

        # Retry scan up to MAX_ATTEMPTS times if no networks found or target SSID not found
        ssids_found = []
        target_ssid_found = False

        for attempt in range(1, MAX_ATTEMPTS + 1):
            logger.info(f"ScanNetworks attempt {attempt}/{MAX_ATTEMPTS}")

            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ScanNetworks(
                    breadcrumb=6
                )
            )

            # Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:
            asserts.assert_true(isinstance(response, cnet.Commands.ScanNetworksResponse),
                                "Unexpected value returned from ScanNetworks")

            # 1. wiFiScanResults contains the target network SSID
            ssids_found = [network.ssid for network in response.wiFiScanResults]
            logger.info(f"Scan results: Found {len(ssids_found)} networks")

            if len(ssids_found) > 0:
                for i, ssid in enumerate(ssids_found):
                    logger.info(f"Network {i}: {ssid} (type: {type(ssid)})")

                # Check if target SSID is in the results
                if correct_ssid in ssids_found:
                    target_ssid_found = True
                    logger.info(f"ScanNetworks found target SSID: {correct_ssid.decode('utf-8')} on attempt {attempt}")
                    break
                else:
                    logger.warning(
                        f"Target SSID '{correct_ssid.decode('utf-8')}' not found in scan results (attempt {attempt})")
            else:
                logger.warning(f"No networks found in scan (attempt {attempt})")

            # If not found and not last attempt, wait before retry
            if not target_ssid_found and attempt < MAX_ATTEMPTS:
                logger.info(f"Waiting {SCAN_RETRY_DELAY} seconds before retry...")
                await asyncio.sleep(SCAN_RETRY_DELAY)

        asserts.assert_true(target_ssid_found,
                            f"ScanNetworks did not find target SSID '{correct_ssid.decode('utf-8')}' after {MAX_ATTEMPTS} attempts")

        # TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 5
        self.step(12)

        logger.info(
            f"Using correct SSID: {correct_ssid.decode('utf-8')} with correct Password")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=correct_password,
                breadcrumb=7
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # TH sends ConnectNetwork command and Breadcrumb field set to 6
        self.step(13)

        logger.info("Sending ConnectNetwork with correct credentials...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=8
            ),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
        )
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(f"ConnectNetwork succeeded for SSID: {correct_ssid.decode('utf-8')} with correct credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(14)

        await self._read_last_networking_status(
            endpoint,
            expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        )

        # TH reads Networks attribute
        self.step(15)

        response = await self._read_networks(endpoint)

        # Verify device is connected to correct network
        connected_networks = [network.networkID for network in response if network.connected]
        asserts.assert_true(correct_ssid in connected_networks,
                            f"Expected device to be connected to SSID '{correct_ssid.decode('utf-8')}'")
        logger.info(f"Device is connected to SSID: {correct_ssid.decode('utf-8')}")

        # TH sends CommissioningComplete to finalize commissioning
        self.step(16)

        await asyncio.sleep(5)  # Short delay before mDNS discovery

        # Discover device on WiFi network via mDNS to establish CASE session
        logger.info("Discovering device on WiFi network via mDNS...")
        discovered_devices = await find_matter_devices_mdns()
        logger.info(f"Found {len(discovered_devices)} device(s) via mDNS")
        if discovered_devices:
            for device in discovered_devices:
                logger.info(f"  Device: {device['name']} at {device['addresses']}")

        # Close PASE session to force CASE session establishment over WiFi
        logger.info("Closing BLE connection and expiring PASE session...")
        self.default_controller.CloseBLEConnection()
        self.default_controller.ExpireSessions(self.dut_node_id)
        logger.info("PASE session closed, next command will establish CASE session over WiFi")

        # Send CommissioningComplete over CASE session (will be auto-established over WiFi)
        logger.info("Sending CommissioningComplete (will establish CASE session over WiFi)...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends CommissioningCompleteResponse with the following fields:
        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse),
                            "Expected CommissioningCompleteResponse")
        # 1. ErrorCode field set to OK (0)
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse errorCode to be OK (0), but got {response.errorCode}")
        logger.info("CommissioningComplete command sent successfully, commissioning finalized.")


if __name__ == "__main__":
    default_matter_test_main()
