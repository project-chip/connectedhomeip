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
CONNECT_NETWORK_TIMEOUT_MS = 30000      # ConnectNetwork timeout (30s)
MDNS_DISCOVERY_TIMEOUT = 10             # mDNS discovery timeout per attempt (10s)
NETWORK_STATUS_UPDATE_DELAY = 3         # Delay for DUT to update LastNetworkingStatus (3s)
MDNS_DISCOVERY_PREP_DELAY = 5           # Delay before starting mDNS discovery (5s)
SESSION_EXPIRY_DELAY = 5                # Delay for session expiry (5s)
SCAN_RETRY_DELAY = 3                    # Delay between scan retries (3s)
MAX_ATTEMPTS = 3                        # Maximum discovery attempts
TIMEOUT = 180                           # Overall test timeout (3 min)

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
            TestStep(0, "TH begins commissioning the DUT over the initial commissioning radio (PASE):\n"
                        "Skip CommissioningComplete to manually configure Thread with incorrect then correct credentials\n"
                        "Extend fail-safe to 300 seconds to allow time for credential testing\n"
                        "TH reads FeatureMap attribute from the DUT and verifies if DUT supports Thread on endpoint 0",
                        is_commissioning=False),
            TestStep(1, "TH reads Networks attribute and removes all configured networks",
                        "Verify that DUT successfully removed all networks configured during commissioning"),
            TestStep(2, "TH sends AddOrUpdateThreadNetwork with incorrect operational dataset (invalid network), Breadcrumb = 1",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(3, "TH sends ConnectNetwork command with incorrect network ID, Breadcrumb = 2",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(4, "TH reads LastNetworkingStatus (should be kNetworkNotFound or kOtherConnectionFailure)",
                        "Verify LastNetworkingStatus is kNetworkNotFound (5) or kOtherConnectionFailure (7)"),
            TestStep(5, "TH reads Networks attribute",
                        "Verify incorrect network ID is in the network list"),
            TestStep(6, "TH sends RemoveNetwork command with incorrect network ID, Breadcrumb = 3",
                        "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)"),
            TestStep(7, "TH reads Networks attribute",
                        "Verify Networks list is empty after removal"),
            TestStep(8, "TH sends AddOrUpdateThreadNetwork with incorrect operational dataset (valid format but wrong network), Breadcrumb = 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty\n"),
            TestStep(9, "TH sends ConnectNetwork command and Breadcrumb = 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(10, "TH reads LastNetworkingStatus (should be kOtherConnectionFailure)",
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
            TestStep(15, "TH sends CommissioningComplete to finalize commissioning",
                         "Verify that DUT sends CommissioningCompleteResponse with the following fields:\n"
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
        # First incorrect dataset: completely invalid/non-existent network (truncated/malformed)
        incorrect_thread_dataset_1 = bytes.fromhex("1111111122222222")
        logger.info(f" --- Incorrect Thread dataset 1 (invalid/truncated): {incorrect_thread_dataset_1.hex()}")

        # Second incorrect dataset: valid format but with modified Extended PAN ID to create a non-existent network
        incorrect_thread_dataset_2 = bytearray(correct_thread_dataset)

        # Find and modify the Extended PAN ID TLV in the dataset
        # Extended PAN ID: Type=0x02, Length=0x08, Value=8 bytes
        i = 0
        while i < len(incorrect_thread_dataset_2) - 1:
            tlv_type = incorrect_thread_dataset_2[i]
            tlv_length = incorrect_thread_dataset_2[i + 1]

            if tlv_type == 0x02:  # Extended PAN ID
                # Modify the Extended PAN ID value to create a non-existent network
                # XOR the bytes to change them while maintaining valid format
                for j in range(8):
                    if i + 2 + j < len(incorrect_thread_dataset_2):
                        incorrect_thread_dataset_2[i + 2 + j] ^= 0xFF
                logger.info(f" --- Modified Extended PAN ID in dataset 2")
                break

            i += 2 + tlv_length

        incorrect_thread_dataset_2 = bytes(incorrect_thread_dataset_2)
        logger.info(f" --- Incorrect Thread dataset 2 (modified Extended PAN ID): {incorrect_thread_dataset_2.hex()}")

        # Step 0: TH begins commissioning the DUT over the initial commissioning radio (PASE):
        self.step(0)

        # Temporarily use incorrect Thread operational dataset during auto-commissioning to prevent automatic Thread connection
        # This ensures the DUT doesn't connect to Thread during the automated commissioning steps,
        # allowing the test to manually verify incorrect and correct credential handling
        self.matter_test_config.thread_operational_dataset = incorrect_thread_dataset_1

        # Skip CommissioningComplete to manually configure Thread with incorrect then correct credentials
        self.default_controller.SetSkipCommissioningComplete(True)
        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        try:
            await self.commission_devices()
        finally:
            # Restore correct Thread operational dataset for later use
            self.matter_test_config.thread_operational_dataset = correct_thread_dataset

        # Extend fail-safe to 300 seconds to allow time for credential testing
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0)
        )
        logger.info(" --- Extended fail-safe timer to 300 seconds")

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
        logger.info(" --- All networks successfully removed. Ready for manual WiFi configuration tests.")

        # Step 2: TH sends AddOrUpdateThreadNetwork with incorrect operational dataset (invalid network), Breadcrumb = 1
        self.step(2)

        logger.info(" --- Step 2: Sending AddOrUpdateThreadNetwork with incorrect operational dataset (invalid network)")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=incorrect_thread_dataset_1,
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(response)

        # Step 3: TH sends ConnectNetwork command with incorrect network ID, Breadcrumb = 2
        self.step(3)

        # Wait for DUT to complete connection attempt and update LastNetworkingStatus

        # Step 4: TH reads LastNetworkingStatus (should be kNetworkNotFound or kOtherConnectionFailure)
        self.step(4)

        # Step 5: TH reads Networks attribute (verify incorrect network ID is stored)
        self.step(5)

        # Step 6: TH sends RemoveNetwork command with incorrect network ID, Breadcrumb = 3
        self.step(6)

        # Step 7: TH reads Networks attribute (verify list is empty)
        self.step(7)

        # Step 8: TH sends AddOrUpdateThreadNetwork with incorrect operational dataset (valid format but wrong network), Breadcrumb = 4
        self.step(8)

        # Step 9: TH sends ConnectNetwork command and Breadcrumb = 5
        self.step(9)

        # Wait for DUT to complete connection attempt and update LastNetworkingStatus

        # Step 10: TH reads LastNetworkingStatus (should be kOtherConnectionFailure)
        self.step(10)

        # When connection fails due to wrong network, the DUT should return kOtherConnectionFailure

        # Step 11: TH sends AddOrUpdateThreadNetwork with correct operational dataset and Breadcrumb = 6
        self.step(11)

        # Wait for DUT to stabilize Thread configuration before attempting connection

        # Step 12: TH sends ConnectNetwork command and Breadcrumb = 7
        self.step(12)

        # Step 13: TH reads LastNetworkingStatus (should be kSuccess)
        self.step(13)

        # Step 14: TH reads Networks attribute
        self.step(14)

        # Verify the device is connected to the correct network

        # Step 15: TH sends CommissioningComplete to finalize commissioning
        self.step(15)

        # Discover device on Thread network via mDNS to establish CASE session

        # Close PASE session to force CASE session establishment over Thread

        # Verify that DUT sends CommissioningCompleteResponse with the following fields:

        # 1. ErrorCode field set to OK (0)


if __name__ == "__main__":
    default_matter_test_main()
