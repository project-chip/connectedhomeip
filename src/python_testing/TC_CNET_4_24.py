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

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout constants
TIMED_REQUEST_TIMEOUT_MS = 5000  # Matter command timeout (5s)
CONNECT_NETWORK_TIMEOUT_MS = 60000  # ConnectNetwork timeout (60s - Thread needs more time to scan/fail)
NETWORK_STATUS_UPDATE_DELAY = 5  # Delay for DUT to update LastNetworkingStatus (5s for Thread)
TIMEOUT = 300  # Overall test timeout (5 min)

# Cluster references
cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning

# Thread TLV types (from Thread Operational Dataset specification)
EXTENDED_PAN_ID_TLV_TYPE = 0x02  # 8 bytes
NETWORK_KEY_TLV_TYPE = 0x05  # 16 bytes


def get_thread_tlv(dataset: bytes, tlv_type: int, expected_length: int = None) -> bytes:
    """
    Extracts a specific TLV value from a Thread Operational Dataset.
    Thread TLV format: Type (1 byte) | Length (1 byte) | Value (Length bytes)

    Common TLV types:
    - EXTENDED_PAN_ID_TLV_TYPE (0x02): Extended PAN ID (8 bytes)
    - 0x03: Network Name (variable length, max 16 bytes)
    - NETWORK_KEY_TLV_TYPE (0x05): Network Key (16 bytes)

    Args:
        dataset: Thread operational dataset in bytes
        tlv_type: TLV type to extract
        expected_length: Expected length of the TLV value (optional, for validation)

    Returns:
        TLV value as bytes

    Raises:
        ValueError: If TLV is not found or has invalid length
    """
    i = 0
    while i < len(dataset) - 1:
        current_type = dataset[i]
        current_length = dataset[i + 1]

        if current_type == tlv_type:
            if expected_length is not None and current_length != expected_length:
                raise ValueError(
                    f"Invalid TLV length for type 0x{tlv_type:02x}: {current_length}, expected {expected_length} bytes"
                )
            if i + 2 + current_length <= len(dataset):
                return dataset[i + 2: i + 2 + current_length]
            raise ValueError(f"TLV type 0x{tlv_type:02x} extends beyond dataset boundary")

        i += 2 + current_length

    raise ValueError(f"TLV type 0x{tlv_type:02x} not found in Thread operational dataset")


def modify_thread_tlv(dataset: bytes, tlv_type: int, modifier_fn) -> bytes:
    """
    Modifies a specific TLV value in a Thread Operational Dataset.

    Args:
        dataset: Original Thread operational dataset in bytes
        tlv_type: TLV type to modify (e.g., 0x02 for Extended PAN ID)
        modifier_fn: Function that takes the original value bytes and returns modified bytes.
                    Example: lambda v: bytes(b ^ 0xAA for b in v)

    Returns:
        Modified dataset as bytes

    Raises:
        ValueError: If the TLV type is not found or modified value has different length
    """
    result = bytearray(dataset)
    i = 0

    while i < len(result) - 1:
        current_type = result[i]
        current_length = result[i + 1]

        if current_type == tlv_type:
            # Extract current value
            value_start = i + 2
            value_end = value_start + current_length

            if value_end > len(result):
                raise ValueError(f"TLV type 0x{tlv_type:02x} extends beyond dataset boundary")

            original_value = bytes(result[value_start:value_end])
            # Apply modification function
            modified_value = modifier_fn(original_value)

            # Ensure modified value has same length
            if len(modified_value) != current_length:
                raise ValueError(
                    f"Modified TLV value for type 0x{tlv_type:02x} has different length: "
                    f"{len(modified_value)} vs original {current_length}"
                )

            # Replace value in dataset
            result[value_start:value_end] = modified_value
            logger.info(f" --- Modified TLV type 0x{tlv_type:02x} ({current_length} bytes)")
            return bytes(result)

        i += 2 + current_length

    raise ValueError(f"TLV type 0x{tlv_type:02x} not found in dataset")


class TC_CNET_4_24(MatterBaseTest):
    """
    [TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server]

    Example Usage:
        To run the test case, use the following command:

        ```bash
        python src/python_testing/TC_CNET_4_24.py --in-test-commissioning-method ble-thread -d <discriminator> -p <passcode> \
               --endpoint <endpoint_value> --thread-dataset-hex <dataset_value>
        ```

        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT, and
        `<dataset_value>` should be replaced with the Thread Operational Dataset
        in hexadecimal format.
    """

    async def _validate_network_config_response(
        self, response: cnet.Commands.NetworkConfigResponse, expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None
    ) -> None:
        expected_status = expected_status or cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_true(
            isinstance(response, cnet.Commands.NetworkConfigResponse), "Unexpected response type from NetworkConfig command"
        )
        asserts.assert_equal(
            response.networkingStatus,
            expected_status,
            f"Expected NetworkingStatus {expected_status}, got {response.networkingStatus}",
        )
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512, f"debugText too long: {len(response.debugText)} bytes")

    async def _validate_connect_network_response(
        self, response: cnet.Commands.ConnectNetworkResponse, expect_success: bool = True
    ) -> None:
        asserts.assert_true(
            isinstance(response, cnet.Commands.ConnectNetworkResponse), "Unexpected response type from ConnectNetwork command"
        )
        if expect_success:
            asserts.assert_equal(
                response.networkingStatus,
                cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                f"Expected success, got {response.networkingStatus}",
            )
        else:
            asserts.assert_not_equal(
                response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Expected failure, got success"
            )
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512, f"debugText too long: {len(response.debugText)} bytes")

    async def _read_networks(self, endpoint: int) -> list:
        return await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks,
        )

    async def _read_last_networking_status(
        self, endpoint: int, expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None, valid_statuses: list = None
    ) -> cnet.Enums.NetworkCommissioningStatusEnum:
        status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(f" --- LastNetworkingStatus = {status}")
        if expected_status is not None:
            asserts.assert_equal(status, expected_status, f"Expected {expected_status}, got {status}")
        elif valid_statuses is not None:
            asserts.assert_in(status, valid_statuses, f"Expected one of {valid_statuses}, got {status}")
        return status

    # Overrides default_timeout
    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_24(self):
        return [
            TestStep(
                0,
                "Commission device if not already done\n"
                "Then opens a new fail-safe window (300 seconds) for network reconfiguration testing\n",
            ),
            TestStep(
                1,
                "TH reads Networks attribute and removes all configured networks",
                "Verify that DUT successfully removed all networks configured during commissioning\n"
                "Verify LastNetworkingStatus and LastConnectErrorValue are Null after network removal",
            ),
            TestStep(
                2,
                "TH sends AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID, Breadcrumb = 1",
                "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                "1. NetworkingStatus is kSuccess (0)\n"
                "2. DebugText is of type string with max length 512 or empty",
            ),
            TestStep(
                3,
                "TH sends ConnectNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 2",
                "Verify DUT returns ConnectNetworkResponse with NetworkingStatus kSuccess (0)",
            ),
            TestStep(
                4,
                "TH reads LastNetworkingStatus after Extended PAN ID connection failure",
                "Verify LastNetworkingStatus is kNetworkNotFound (5)",
            ),
            TestStep(5, "TH reads Networks attribute", "Verify dataset with incorrect Extended PAN ID is in the network list"),
            TestStep(
                6,
                "TH sends RemoveNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 3",
                "Verify that DUT sends NetworkConfigResponse command to the TH with the following response fields:\n"
                "1. NetworkingStatus is kSuccess (0)",
            ),
            TestStep(7, "TH reads Networks attribute", "Verify Networks list is empty after removal"),
            TestStep(
                8,
                "TH sends AddOrUpdateThreadNetwork with valid format but incorrect Network Key, Breadcrumb = 4",
                "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                "1. NetworkingStatus is kSuccess (0)\n"
                "2. DebugText is of type string with max length 512 or empty\n",
            ),
            TestStep(
                9,
                "TH sends ConnectNetwork command with dataset containing incorrect Network Key, Breadcrumb = 5",
                "Verify DUT returns ConnectNetworkResponse with NetworkingStatus kSuccess (0)",
            ),
            TestStep(
                10,
                "TH reads LastNetworkingStatus after Network Key connection failure",
                "Verify LastNetworkingStatus is kAuthFailure (7)",
            ),
            TestStep(
                11,
                "TH sends AddOrUpdateThreadNetwork with correct operational dataset and Breadcrumb = 6",
                "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                "1. NetworkingStatus is kSuccess (0)\n"
                "2. DebugText is of type string with max length 512 or empty",
            ),
            TestStep(
                12,
                "TH sends ConnectNetwork command and Breadcrumb = 7",
                "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                "1. NetworkingStatus is kSuccess (0)\n"
                "2. DebugText is of type string with max length 512 or empty",
            ),
            TestStep(13, "TH reads LastNetworkingStatus (should be kSuccess)", "Verify LastNetworkingStatus is kSuccess (0)"),
            TestStep(14, "TH reads Networks attribute", "Verify the device is connected to the correct network"),
            TestStep(
                15,
                "TH sends ArmFailSafe(0) to close the fail-safe window and commit network changes",
                "Verify that DUT sends ArmFailSafeResponse with the following fields:\n1. ErrorCode field set to OK (0)",
            ),
        ]

    def desc_TC_CNET_4_24(self):
        return "[TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server]"

    @async_test_body
    async def test_TC_CNET_4_24(self):
        # Network Commissioning cluster is always on root endpoint (0) during commissioning
        endpoint = ROOT_ENDPOINT_ID

        # Save correct Thread operational dataset from test config (used by commissioning framework and for final connection)
        correct_thread_dataset = self.matter_test_config.thread_operational_dataset
        asserts.assert_is_not_none(
            correct_thread_dataset,
            "Thread operational dataset must be provided via --thread-dataset-hex parameter for this test"
        )
        logger.info(f" --- Correct Thread operational dataset: {correct_thread_dataset.hex()}")

        # Create incorrect Thread operational datasets for testing
        # First incorrect dataset: valid format but with modified Extended PAN ID
        # This should cause kNetworkNotFound since the Extended PAN ID identifies the network
        # Extended PAN ID: Type=0x02, Length=0x08, Value=8 bytes
        incorrect_thread_dataset_1 = modify_thread_tlv(
            correct_thread_dataset,
            EXTENDED_PAN_ID_TLV_TYPE,
            lambda v: bytes(b ^ 0xAA for b in v),  # XOR with 0xAA
        )
        logger.info(f" --- Incorrect Thread dataset 1 (modified Extended PAN ID only): {incorrect_thread_dataset_1.hex()}")

        # Second incorrect dataset: valid format but with modified Network Key
        # This should cause kAuthFailure since the network exists but credentials are wrong
        # Network Key: Type=0x05, Length=0x10, Value=16 bytes
        incorrect_thread_dataset_2 = modify_thread_tlv(
            correct_thread_dataset,
            NETWORK_KEY_TLV_TYPE,
            lambda v: bytes(b ^ 0xCC for b in v),  # XOR with 0xCC
        )
        logger.info(f" --- Incorrect Thread dataset 2 (modified Network Key only): {incorrect_thread_dataset_2.hex()}")

        # Step 0: Commission device if not already done
        self.step(0)

        self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        self.matter_test_config.tc_version_to_simulate = None
        self.matter_test_config.tc_user_response_to_simulate = None

        try:
            await self.commission_devices()
            logger.info(" --- Device commissioned successfully, now on Thread network")
        except Exception as e:
            logger.error(f" --- Commissioning failed: {e}")
            asserts.fail(f"Step 0 failed: Device commissioning did not complete successfully. Error: {e}")

        # Open a new fail-safe window (300 seconds) for network reconfiguration testing
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )
        logger.info(" --- Opened new fail-safe window (300 seconds) for network reconfiguration testing")

        # Step 1: TH reads Networks attribute and removes all configured networks
        self.step(1)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Found {len(networks)} network(s) configured during commissioning")
        for network in networks:
            network_id = network.networkID
            logger.info(f" --- Removing network with Extended PAN ID: {network_id.hex()}")
            remove_response = await self.send_single_cmd(
                endpoint=endpoint, cmd=cnet.Commands.RemoveNetwork(networkID=network_id, breadcrumb=0)
            )
            await self._validate_network_config_response(remove_response)
            logger.info(f" --- Network removed successfully (Extended PAN ID: {network_id.hex()})")

        # Verify that DUT successfully removed all networks configured during commissioning
        networks_after = await self._read_networks(endpoint)
        asserts.assert_equal(
            len(networks_after), 0, f"Expected empty network list after cleanup, but found {len(networks_after)} network(s)"
        )
        logger.info(" --- All networks successfully removed. Ready for manual Thread configuration tests.")

        # Wait for device to update LastNetworkingStatus and LastConnectErrorValue
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to update status...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Verify LastNetworkingStatus and LastConnectErrorValue are cleared to NULL after network removal
        last_networking_status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(f" --- LastNetworkingStatus after network removal: {last_networking_status}")

        # Read current connectivity status
        last_connect_error = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastConnectErrorValue,
        )
        logger.info(f" --- LastConnectErrorValue after removal: {last_connect_error}")

        # Verify both status attributes are Null after network removal
        asserts.assert_is(
            last_networking_status,
            NullValue,
            f"Expected LastNetworkingStatus to be Null after network removal, got {last_networking_status}",
        )
        asserts.assert_is(
            last_connect_error,
            NullValue,
            f"Expected LastConnectErrorValue to be Null after network removal, got {last_connect_error}",
        )

        # Step 2: TH sends AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID, Breadcrumb = 1
        self.step(2)
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(operationalDataset=incorrect_thread_dataset_1, breadcrumb=1),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )
        await self._validate_network_config_response(response)

        # Step 3: TH sends ConnectNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 2
        self.step(3)

        # Extract Extended PAN ID (8 bytes) from the dataset to use as networkID
        network_id_1 = get_thread_tlv(incorrect_thread_dataset_1, tlv_type=EXTENDED_PAN_ID_TLV_TYPE, expected_length=8)
        logger.info(f" --- Step 3: Sending ConnectNetwork with incorrect Extended PAN ID: {network_id_1.hex()}")

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=network_id_1, breadcrumb=2),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS,
        )
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(" --- ConnectNetwork returned kSuccess")

        # Wait for device to complete connection attempt
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to update network status...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 4: TH reads LastNetworkingStatus after Extended PAN ID connection failure
        self.step(4)

        # Verify that LastNetworkingStatus is kNetworkNotFound
        # The device should detect that the Extended PAN ID does not exist in the environment
        await self._read_last_networking_status(
            endpoint, expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound
        )

        # Step 5: TH reads Networks attribute (verify dataset with incorrect Extended PAN ID is stored)
        self.step(5)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Step 5: Networks attribute has {len(networks)} network(s)")
        network_ids = [net.networkID for net in networks]
        asserts.assert_in(network_id_1, network_ids, "Incorrect Extended PAN ID not found in Networks attribute")

        # Step 6: TH sends RemoveNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 3
        self.step(6)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(networkID=network_id_1, breadcrumb=3),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )
        await self._validate_network_config_response(response)

        # Step 7: TH reads Networks attribute (verify list is empty)
        self.step(7)

        networks = await self._read_networks(endpoint)
        logger.info(f" --- Step 7: Networks attribute has {len(networks)} network(s) after removal")
        asserts.assert_equal(len(networks), 0, f"Expected Networks list to be empty, but has {len(networks)} network(s)")

        # Step 8: TH sends AddOrUpdateThreadNetwork with valid format but incorrect Network Key, Breadcrumb = 4
        self.step(8)
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(operationalDataset=incorrect_thread_dataset_2, breadcrumb=4),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )
        await self._validate_network_config_response(response)

        # Step 9: TH sends ConnectNetwork command with dataset containing incorrect Network Key, Breadcrumb = 5
        self.step(9)

        # Extract Extended PAN ID (8 bytes) from the dataset to use as networkID
        network_id_2 = get_thread_tlv(incorrect_thread_dataset_2, tlv_type=EXTENDED_PAN_ID_TLV_TYPE, expected_length=8)
        logger.info(f" --- Step 9: Sending ConnectNetwork with incorrect Network Key: {network_id_2.hex()}")

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=network_id_2, breadcrumb=5),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS,
        )
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(" --- ConnectNetwork returned kSuccess")

        # Wait for device to complete connection attempt
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for device to update network status...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 10: TH reads LastNetworkingStatus after Network Key connection failure
        self.step(10)

        # Verify that LastNetworkingStatus is kAuthFailure
        # The device should detect incorrect Network Key authentication failure
        await self._read_last_networking_status(endpoint, expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure)

        # Step 11: TH sends AddOrUpdateThreadNetwork with correct operational dataset and Breadcrumb = 6
        self.step(11)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(operationalDataset=correct_thread_dataset, breadcrumb=6),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )
        await self._validate_network_config_response(response)

        # Wait for DUT to stabilize Thread configuration before attempting connection
        logger.info(f" --- Waiting {NETWORK_STATUS_UPDATE_DELAY} seconds for DUT to stabilize Thread configuration...")
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Step 12: TH sends ConnectNetwork command and Breadcrumb = 7
        self.step(12)

        # Extract Extended PAN ID (8 bytes) from the dataset to use as networkID
        correct_network_id = get_thread_tlv(correct_thread_dataset, tlv_type=EXTENDED_PAN_ID_TLV_TYPE, expected_length=8)
        logger.info(f" --- Step 12: Sending ConnectNetwork with correct dataset: {correct_network_id.hex()}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=correct_network_id, breadcrumb=7),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS,
        )
        await self._validate_connect_network_response(response, expect_success=True)
        logger.info(" --- ConnectNetwork succeeded")

        # Step 13: TH reads LastNetworkingStatus (should be kSuccess)
        self.step(13)

        await self._read_last_networking_status(endpoint, expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)

        # Step 14: TH reads Networks attribute
        self.step(14)

        response = await self._read_networks(endpoint)
        logger.info(f" --- Step 14: Networks attribute has {len(response)} network(s)")
        asserts.assert_greater_equal(
            len(response), 1, "Expected at least one network in Networks attribute after successful connection"
        )
        # Verify the device is connected to the correct network (correct_network_id from Step 12)
        connected_networks = [network.networkID for network in response if network.connected]
        logger.info(f" --- Connected networks: {[net.hex() for net in connected_networks]}")
        logger.info(f" --- Expected network ID: {correct_network_id.hex()}")
        asserts.assert_true(
            correct_network_id in connected_networks,
            f"Expected device to be connected to Thread network with Extended PAN ID '{correct_network_id.hex()}'",
        )

        # Step 15: Close fail-safe to commit the network reconfiguration
        self.step(15)

        # Commissioning completed in Step 0 (CommissioningComplete was already sent)
        # Close the fail-safe by sending ArmFailSafe(0) to commit the network configuration changes
        logger.info(" --- Closing fail-safe window with ArmFailSafe(0) to commit network reconfiguration...")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=99),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS,
        )

        # Verify that DUT sends ArmFailSafeResponse with ErrorCode OK
        asserts.assert_true(isinstance(response, cgen.Commands.ArmFailSafeResponse), "Expected ArmFailSafeResponse")
        asserts.assert_equal(
            response.errorCode,
            cgen.Enums.CommissioningErrorEnum.kOk,
            f"Expected ArmFailSafeResponse errorCode to be OK (0), but got {response.errorCode}",
        )
        logger.info(" --- Fail-safe closed successfully, network reconfiguration committed.")


if __name__ == "__main__":
    default_matter_test_main()
