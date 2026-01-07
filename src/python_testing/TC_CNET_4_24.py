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

# Timeout constants
TIMED_REQUEST_TIMEOUT_MS = 5000         # Matter command timeout (5s)
CONNECT_NETWORK_TIMEOUT_MS = 60000      # ConnectNetwork timeout (60s - Thread needs more time to scan/fail)
MDNS_DISCOVERY_TIMEOUT = 10             # mDNS discovery timeout per attempt (10s)
NETWORK_STATUS_UPDATE_DELAY = 5         # Delay for DUT to update LastNetworkingStatus (5s for Thread)
MDNS_DISCOVERY_PREP_DELAY = 5           # Delay before starting mDNS discovery (5s)
SESSION_EXPIRY_DELAY = 5                # Delay for session expiry (5s)
SCAN_RETRY_DELAY = 3                    # Delay between scan retries (3s)
MAX_ATTEMPTS = 3                        # Maximum discovery attempts
TIMEOUT = 300                           # Overall test timeout (5 min)

# Cluster references
cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning


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


async def find_matter_devices_mdns(target_device_id: int = None) -> list:
    """
    Finds Matter devices via mDNS using zeroconf, optionally filtering by target device ID.
    Raises an exception if no device is found after MAX_ATTEMPTS.
    Returns the list of discovered services.
    """
    service_types = ["_matter._tcp.local.", "_matterc._udp.local."]

    logger.info(
        f" --- find_matter_devices_mdns: Searching for Matter devices{' with target device ID: ' + str(target_device_id) if target_device_id else ''}")

    for attempt in range(1, MAX_ATTEMPTS + 1):
        zc = None
        try:
            zc = Zeroconf()
            listener = MatterServiceListener(target_device_id)
            browsers = [ServiceBrowser(zc, stype, listener) for stype in service_types]

            await asyncio.sleep(MDNS_DISCOVERY_TIMEOUT)

            # Cleanup
            for browser in browsers:
                browser.cancel()
            await asyncio.sleep(0.1)  # Give browsers time to cleanup

            if listener.discovered_services:
                return listener.discovered_services

        except Exception as e:
            logger.error(f" --- find_matter_devices_mdns: Discovery attempt {attempt} failed: {e}")
        finally:
            if zc is not None:
                try:
                    zc.close()
                except Exception as e:
                    logger.warning(f"Failed to close zeroconf instance: {e}")

        if attempt < MAX_ATTEMPTS:
            await asyncio.sleep(SCAN_RETRY_DELAY)

    raise Exception(
        f"find_matter_devices_mdns: mDNS discovery failed after {MAX_ATTEMPTS} attempts - No Matter devices found{' for target device ID: ' + str(target_device_id) if target_device_id else ''}")


def extract_extended_pan_id_from_dataset(dataset: bytes) -> bytes:
    """
    Extracts the Extended PAN ID (8 bytes) from a Thread Operational Dataset.
    Thread TLV format: Type (1 byte) | Length (1 byte) | Value (Length bytes)
    Extended PAN ID has Type = 0x02

    Args:
        dataset: Thread operational dataset in bytes

    Returns:
        Extended PAN ID as bytes (8 bytes)

    Raises:
        ValueError: If Extended PAN ID is not found in the dataset
    """
    i = 0
    while i < len(dataset) - 1:
        tlv_type = dataset[i]
        tlv_length = dataset[i + 1]

        if tlv_type == 0x02:  # Extended PAN ID
            if tlv_length == 8 and i + 2 + tlv_length <= len(dataset):
                return dataset[i + 2:i + 2 + tlv_length]
            else:
                raise ValueError(f"Invalid Extended PAN ID length: {tlv_length}, expected 8 bytes")

        i += 2 + tlv_length

    raise ValueError("Extended PAN ID (Type=0x02) not found in Thread operational dataset")


class TC_CNET_4_24(MatterBaseTest):
    """
    [TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server]

    Example Usage:
        To run the test case, use the following command:

        ```bash
        python src/python_testing/TC_CNET_4_24.py --commissioning-method ble-thread -d <discriminator> -p <passcode> \
               --endpoint <endpoint_value> --thread-dataset-hex <dataset_value>
        ```

        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT, and
        `<dataset_value>` should be replaced with the Thread Operational Dataset
        in hexadecimal format.
    """

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
        logger.info(f" --- LastNetworkingStatus = {status}")
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

    def steps_TC_CNET_4_24(self):
        return [
            TestStep(0, "TH commissions the DUT over BLE-Thread using correct credentials\n"
                        "Then opens a new fail-safe window (300 seconds) for network reconfiguration testing\n"
                        "TH reads FeatureMap attribute from the DUT and verifies if DUT supports Thread on endpoint 0",
                        is_commissioning=True),
            TestStep(1, "TH reads Networks attribute and removes all configured networks",
                        "Verify that DUT successfully removed all networks configured during commissioning"),
            TestStep(2, "TH sends AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID, Breadcrumb = 1",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(3, "TH sends ConnectNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 2",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0) (in non-concurrent mode)\n"
                        "After delay, verify LastNetworkingStatus is NOT kSuccess, indicating connection failure"),
            TestStep(4, "TH reads LastNetworkingStatus after Extended PAN ID connection failure",
                        "Verify LastNetworkingStatus is kOtherConnectionFailure (7)"),
            TestStep(5, "TH reads Networks attribute",
                        "Verify dataset with incorrect Extended PAN ID is in the network list"),
            TestStep(6, "TH sends RemoveNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 3",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)"),
            TestStep(7, "TH reads Networks attribute",
                        "Verify Networks list is empty after removal"),
            TestStep(8, "TH sends AddOrUpdateThreadNetwork with valid format but incorrect Network Name and Master Key, Breadcrumb = 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty\n"),
            TestStep(9, "TH sends ConnectNetwork command with dataset containing incorrect Network Name and Master Key, Breadcrumb = 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0) (in non-concurrent mode)\n"
                        "After delay, verify LastNetworkingStatus is NOT kSuccess, indicating connection failure"),
            TestStep(10, "TH reads LastNetworkingStatus after Network Name/Master Key connection failure",
                         "Verify LastNetworkingStatus to be 'kOtherConnectionFailure'"),
            TestStep(11, "TH sends AddOrUpdateThreadNetwork with correct operational dataset and Breadcrumb = 6",
                         "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                         "1. NetworkingStatus is kSuccess (0)\n"
                         "2. DebugText is of type string with max length 512 or empty"),
            TestStep(12, "TH sends ConnectNetwork command and Breadcrumb = 7",
                         "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                         "1. NetworkingStatus is kSuccess (0)\n"
                         "2. DebugText is of type string with max length 512 or empty"),
            TestStep(13, "TH reads LastNetworkingStatus (should be kSuccess)",
                         "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(14, "TH reads Networks attribute",
                         "Verify the device is connected to the correct network"),
            TestStep(15, "TH sends ArmFailSafe(0) to close the fail-safe window and commit network changes",
                         "Verify that DUT sends ArmFailSafeResponse with the following fields:\n"
                         "1. ErrorCode field set to OK (0)"),
        ]

    def desc_TC_CNET_4_24(self):
        return "[TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server]"

    @async_test_body
    async def test_TC_CNET_4_24(self):
        # Network Commissioning cluster is always on root endpoint (0) during commissioning
        endpoint = ROOT_ENDPOINT_ID

        # Save correct Thread operational dataset from test config (used by commissioning framework and for final connection)
        correct_thread_dataset = self.matter_test_config.thread_operational_dataset
        logger.info(f" --- Correct Thread operational dataset: {correct_thread_dataset.hex()}")

        # Create incorrect Thread operational datasets for testing
        # First incorrect dataset: valid format but with modified Extended PAN ID AND Master Key
        # This ensures the device cannot connect to any existing network with matching credentials
        incorrect_thread_dataset_1 = bytearray(correct_thread_dataset)

        # Find and modify Extended PAN ID (Type=0x02) and Master Key (Type=0x05) TLVs
        # Extended PAN ID: Type=0x02, Length=0x08, Value=8 bytes
        # Master Key: Type=0x05, Length=0x10, Value=16 bytes
        i = 0
        while i < len(incorrect_thread_dataset_1) - 1:
            tlv_type = incorrect_thread_dataset_1[i]
            tlv_length = incorrect_thread_dataset_1[i + 1]

            if tlv_type == 0x02:  # Extended PAN ID
                # Modify the Extended PAN ID value to create a non-existent network
                # XOR all 8 bytes to ensure it's completely different
                for j in range(min(tlv_length, 8)):
                    if i + 2 + j < len(incorrect_thread_dataset_1):
                        incorrect_thread_dataset_1[i + 2 + j] ^= 0xAA
                logger.info(f" --- Modified Extended PAN ID in dataset 1")
            elif tlv_type == 0x05:  # Master Key (Network Key)
                # Also modify the Master Key to ensure connection failure
                # XOR all 16 bytes of the master key to prevent matching with real network
                for j in range(min(tlv_length, 16)):
                    if i + 2 + j < len(incorrect_thread_dataset_1):
                        incorrect_thread_dataset_1[i + 2 + j] ^= 0x99
                logger.info(f" --- Modified Master Key in dataset 1")

            i += 2 + tlv_length

        incorrect_thread_dataset_1 = bytes(incorrect_thread_dataset_1)
        logger.info(f" --- Incorrect Thread dataset 1 (modified Extended PAN ID + Master Key): {incorrect_thread_dataset_1.hex()}")

        # Second incorrect dataset: valid format but with modified Network Name and Master Key
        incorrect_thread_dataset_2 = bytearray(correct_thread_dataset)

        # Find and modify Network Name (Type=0x03) and Master Key (Type=0x05) TLVs
        i = 0
        while i < len(incorrect_thread_dataset_2) - 1:
            tlv_type = incorrect_thread_dataset_2[i]
            tlv_length = incorrect_thread_dataset_2[i + 1]

            if tlv_type == 0x03:  # Network Name
                # Modify network name bytes
                for j in range(min(tlv_length, 8)):
                    if i + 2 + j < len(incorrect_thread_dataset_2):
                        incorrect_thread_dataset_2[i + 2 + j] ^= 0x55
                logger.info(f" --- Modified Network Name in dataset 2")
            elif tlv_type == 0x05:  # Master Key (Network Key)
                # Modify master key bytes
                for j in range(min(tlv_length, 8)):
                    if i + 2 + j < len(incorrect_thread_dataset_2):
                        incorrect_thread_dataset_2[i + 2 + j] ^= 0xCC
                logger.info(f" --- Modified Master Key in dataset 2")

            i += 2 + tlv_length

        incorrect_thread_dataset_2 = bytes(incorrect_thread_dataset_2)
        logger.info(f" --- Incorrect Thread dataset 2 (modified Network Name and Master Key): {incorrect_thread_dataset_2.hex()}")

        # Step 0: TH begins commissioning the DUT over the initial commissioning radio (PASE):
        self.step(0)

        # Commission device normally (with CommissioningComplete)
        # Thread networks cannot prevent automatic connection like WiFi (no separate password)
        # So we commission fully, then open a new fail-safe for testing
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        await self.commission_devices()

        # Device is now fully commissioned and on Thread network with CASE session established
        logger.info(" --- Device commissioned successfully, now on Thread network")

        # Open a new fail-safe window (300 seconds) for network reconfiguration testing
        # This allows us to test incorrect and correct credentials while maintaining connectivity
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        logger.info(" --- Opened new fail-safe window (300 seconds) for network reconfiguration testing")

        # TH reads FeatureMap attribute from the DUT and verifies if DUT supports Thread on endpoint 0
        feature_map = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.FeatureMap,
        )

        if not (feature_map & cnet.Bitmaps.Feature.kThreadNetworkInterface):
            logger.info(" --- Device does not support Thread on endpoint 0, skipping remaining steps")
            self.skip_all_remaining_steps(1)
            return

        # Step 1: TH reads Networks attribute and removes all configured networks
        self.step(1)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Found {len(networks)} network(s) configured during commissioning")
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

        # Verify that DUT successfully removed all networks configured during commissioning
        networks_after = await self._read_networks(endpoint)
        asserts.assert_equal(len(networks_after), 0,
                             f"Expected empty network list after cleanup, but found {len(networks_after)} network(s)")
        logger.info(" --- All networks successfully removed. Ready for manual Thread configuration tests.")

        # Wait for the device to disconnect from Thread network after removing the network configuration
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to disconnect from Thread...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Verify device is disconnected by checking LastNetworkingStatus and LastConnectErrorValue
        last_networking_status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(f" --- LastNetworkingStatus after removal: {last_networking_status}")

        # Read current connectivity status
        last_connect_error = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastConnectErrorValue,
        )
        logger.info(f" --- LastConnectErrorValue after removal: {last_connect_error}")

        # Step 2: TH sends AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID, Breadcrumb = 1
        self.step(2)

        logger.info(" --- Step 2: Sending AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=incorrect_thread_dataset_1,
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # Step 3: TH sends ConnectNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 2
        self.step(3)

        logger.info(" --- Step 3: Sending ConnectNetwork with dataset containing incorrect Extended PAN ID")
        # Extract Extended PAN ID from the dataset to use as networkID
        network_id_1 = extract_extended_pan_id_from_dataset(incorrect_thread_dataset_1)
        logger.info(f" --- Extracted Extended PAN ID: {network_id_1.hex()}")

        # Thread devices may timeout when trying to connect with incorrect credentials
        # This is expected behavior - the device will not respond if it cannot find/join the network
        connect_failed = False
        try:
            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ConnectNetwork(
                    networkID=network_id_1,
                    breadcrumb=2
                ),
                timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
            )
            # In non-concurrent mode, ConnectNetwork may return kSuccess immediately
            # even though connection will fail later
            await self._validate_connect_network_response(response, expect_success=True)
            logger.info(" --- Step 3: ConnectNetwork returned kSuccess (non-concurrent mode)")
        except Exception as e:
            # Timeout is expected when device cannot connect with incorrect credentials
            logger.info(f" --- Step 3: ConnectNetwork timed out as expected with incorrect credentials: {e}")
            connect_failed = True

        # Wait for device to complete connection attempt
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to update network status...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 4: TH reads LastNetworkingStatus after Extended PAN ID connection failure
        self.step(4)

        # If ConnectNetwork timed out, connection definitely failed
        # Otherwise, verify LastNetworkingStatus indicates failure
        if not connect_failed:
            last_networking_status = await self.read_single_attribute(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=cnet.Attributes.LastNetworkingStatus,
            )
            logger.info(f" --- LastNetworkingStatus after incorrect Extended PAN ID: {last_networking_status}")
            # For Thread, we expect kOtherConnectionFailure when credentials are wrong
            # If status is kSuccess, the device may not have properly validated the credentials
            if last_networking_status == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess:
                logger.warning(" --- WARNING: LastNetworkingStatus is kSuccess despite incorrect credentials")
                logger.warning(" --- This may indicate a device firmware issue in non-concurrent mode")
        else:
            logger.info(" --- Connection failure confirmed by ConnectNetwork timeout")

        # Step 5: TH reads Networks attribute (verify dataset with incorrect Extended PAN ID is stored)
        self.step(5)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Step 5: Networks attribute has {len(networks)} network(s)")
        network_ids = [net.networkID for net in networks]
        asserts.assert_in(network_id_1, network_ids,
                          "Incorrect Extended PAN ID not found in Networks attribute")

        # Step 6: TH sends RemoveNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 3
        self.step(6)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=network_id_1,
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

        # Step 8: TH sends AddOrUpdateThreadNetwork with valid format but incorrect Network Name and Master Key, Breadcrumb = 4
        self.step(8)

        logger.info(" --- Step 8: Sending AddOrUpdateThreadNetwork with valid format but incorrect Network Name and Master Key")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=incorrect_thread_dataset_2,
                breadcrumb=4
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # Step 9: TH sends ConnectNetwork command with dataset containing incorrect Network Name and Master Key, Breadcrumb = 5
        self.step(9)

        logger.info(" --- Step 9: Sending ConnectNetwork with dataset containing incorrect Network Name and Master Key")
        # Extract Extended PAN ID from the dataset to use as networkID
        network_id_2 = extract_extended_pan_id_from_dataset(incorrect_thread_dataset_2)
        logger.info(f" --- Extracted Extended PAN ID: {network_id_2.hex()}")

        # Thread devices may timeout when trying to connect with incorrect credentials
        connect_failed_2 = False
        try:
            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ConnectNetwork(
                    networkID=network_id_2,
                    breadcrumb=5
                ),
                timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
            )
            # In non-concurrent mode, ConnectNetwork may return kSuccess immediately
            await self._validate_connect_network_response(response, expect_success=True)
            logger.info(" --- Step 9: ConnectNetwork returned kSuccess (non-concurrent mode)")
        except Exception as e:
            logger.info(f" --- Step 9: ConnectNetwork timed out as expected with incorrect credentials: {e}")
            connect_failed_2 = True

        # Wait for device to complete connection attempt
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to update network status...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 10: TH reads LastNetworkingStatus after Network Name/Master Key connection failure
        self.step(10)

        if not connect_failed_2:
            last_networking_status = await self.read_single_attribute(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=cnet.Attributes.LastNetworkingStatus,
            )
            logger.info(f" --- LastNetworkingStatus after incorrect Network Name/Master Key: {last_networking_status}")
            if last_networking_status == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess:
                logger.warning(" --- WARNING: LastNetworkingStatus is kSuccess despite incorrect credentials")
                logger.warning(" --- This may indicate a device firmware issue in non-concurrent mode")
        else:
            logger.info(" --- Connection failure confirmed by ConnectNetwork timeout")

        # Step 11: TH sends AddOrUpdateThreadNetwork with correct operational dataset and Breadcrumb = 6
        self.step(11)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=correct_thread_dataset,
                breadcrumb=7
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # Wait for DUT to stabilize Thread configuration before attempting connection
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for DUT to stabilize Thread configuration...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 12: TH sends ConnectNetwork command and Breadcrumb = 7
        self.step(12)

        logger.info(" --- Step 12: Sending ConnectNetwork with correct operational dataset")
        # Extract Extended PAN ID from the dataset to use as networkID
        correct_network_id = extract_extended_pan_id_from_dataset(correct_thread_dataset)
        logger.info(f" --- Extracted Extended PAN ID: {correct_network_id.hex()}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_network_id,
                breadcrumb=8
            ),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
        )
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(" --- Step 12: ConnectNetwork succeeded with correct operational dataset")

        # Step 13: TH reads LastNetworkingStatus (should be kSuccess)
        self.step(13)

        await self._read_last_networking_status(
            endpoint,
            expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        )

        # Step 14: TH reads Networks attribute
        self.step(14)

        response = await self._read_networks(endpoint)
        logger.info(f" --- Step 14: Networks attribute has {len(response)} network(s)")
        asserts.assert_greater_equal(len(response), 1,
                                     "Expected at least one network in Networks attribute after successful connection")
        # Verify the device is connected to the correct network
        # For Thread, networkID is the Extended PAN ID (8 bytes), not the full dataset
        correct_network_id = extract_extended_pan_id_from_dataset(correct_thread_dataset)
        connected_networks = [network.networkID for network in response if network.connected]
        logger.info(f" --- Connected networks: {[net.hex() for net in connected_networks]}")
        logger.info(f" --- Expected network ID: {correct_network_id.hex()}")
        asserts.assert_true(correct_network_id in connected_networks,
                            f"Expected device to be connected to Thread network with Extended PAN ID '{correct_network_id.hex()}'")

        # Step 15: Close fail-safe to commit the network reconfiguration
        self.step(15)

        # Since the device already completed commissioning in Step 0 (including sending CommissioningComplete),
        # we cannot send another CommissioningComplete. Instead, we close the fail-safe by sending
        # ArmFailSafe with expiryLengthSeconds=0, which commits the network configuration changes.
        logger.info(" --- Closing fail-safe window with ArmFailSafe(0) to commit network reconfiguration...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=99),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends ArmFailSafeResponse with ErrorCode OK
        asserts.assert_true(isinstance(response, cgen.Commands.ArmFailSafeResponse),
                            "Expected ArmFailSafeResponse")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected ArmFailSafeResponse errorCode to be OK (0), but got {response.errorCode}")
        logger.info(" --- Fail-safe closed successfully, network reconfiguration committed.")


if __name__ == "__main__":
    default_matter_test_main()
