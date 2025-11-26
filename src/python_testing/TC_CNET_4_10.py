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

import logging

import test_plan_support
from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

# Thread TLV Type mapping for operational dataset parsing
THREAD_TLV_TYPE_MAP = {
    0x00: "Channel",
    0x01: "PanId",
    0x02: "ExtendedPanId",
    0x03: "NetworkName",
    0x04: "PSKc",
    0x05: "NetworkKey",
    0x07: "MeshLocalPrefix",
    0x0c: "SecurityPolicy",
    0x0e: "ActiveTimestamp",
    0x35: "ChannelMask",
    0x4a: "WakeupChannel"
}


def parse_openthread_dataset_stream(dataset_hex: str) -> dict[str, str] | None:
    """
    Parses a flat stream of OpenThread TLVs from a hex string into a dictionary.
    Handles the simple [Type][Length][Value] format.
    """
    tlvs = {}
    i = 0
    while i < len(dataset_hex):
        try:
            # Read Type (1 byte)
            tlv_type = int(dataset_hex[i:i+2], 16)
            i += 2

            # Read Length (1 byte)
            tlv_length = int(dataset_hex[i:i+2], 16)
            i += 2

            # Read Value ('tlv_length' bytes)
            value_end = i + (tlv_length * 2)
            if value_end > len(dataset_hex):
                logging.error(f"Error: TLV (type 0x{tlv_type:02x}) length is out of bounds.")
                return None

            value_hex = dataset_hex[i:value_end]
            i = value_end

            # Get the name from the map or use a default for unknown types
            key_name = THREAD_TLV_TYPE_MAP.get(tlv_type, f"Unknown_Type_0x{tlv_type:02x}")
            tlvs[key_name] = value_hex

        except (ValueError, IndexError) as e:
            logging.error(f"Error parsing OpenThread stream at index {i}: {e}")
            return None

    return tlvs


class TC_CNET_4_10(MatterBaseTest):
    """
    [TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]

    Pre-Conditions:
        1. DUT supports CNET.S.F01(TH).
        2. DUT has a Network Commissioning cluster on the endpoint specified
           in the --endpoint command-line argument, with a FeatureMap attribute of 2.
        3. DUT is commissioned on the operational network specified
           in the --thread-dataset-hex command-line argument.
        4. TH can communicate with the DUT on the commissioned network.

    Example usage:
        To run the test case, use the following command:

        ```bash
        python src/python_testing/TC_CNET_4_10.py --commissioning-method ble-thread --discriminator 3840 --passcode 20202021 \
        --endpoint <endpoint_value> --thread-dataset-hex <dataset_value>
        ```

        Where `<endpoint_value>` should be replaced with the actual endpoint number
        for the Network Commissioning cluster on the DUT, and
        `dataset_value` should be replaced with the operational dataset of the DUT in hexadecimal format.
        The Extended PAN ID will be automatically extracted from the provided dataset.
    """

    def desc_TC_CNET_4_10(self):
        return '[TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_10(self):
        return [
            'CNET.S',
            'CNET.S.F01'
        ]

    def steps_TC_CNET_4_10(self):
        return [
            TestStep(1, test_plan_support.commission_if_required(),
                     "DUT is commissioned, TH can communicate with the DUT on thread dataset provided in --thread-dataset-hex parameter.", is_commissioning=True),
            TestStep(2, "TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters).",
                     "Verify that there is a single connected network across ALL network commissioning clusters. "),
            TestStep(3, "Skip remaining steps if the connected network is NOT on the cluster currently being verified."),
            TestStep(4, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900 and Breadcrumb set to 0",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(5, "TH reads Networks attribute from the DUT and save the number of entries as 'NumNetworks'",
                     "Verify that the Networks attribute list has an entry with the following values: "
                     "1. NetworkID field value set as the Extended PAN ID from the operational dataset "
                     "2. Connected field value is of type boolean and has the value True"),
            TestStep(6, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to the Extended PAN ID from the operational dataset and Breadcrumb field set to 1",
                     "Verify that DUT sends NetworkConfigResponse to command with the following fields: "
                     "1. NetworkingStatus is success "
                     "2. NetworkIndex is 'Userth_netidx'"),
            TestStep(7, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list has 'NumNetworks' - 1 entries"),
            TestStep(8, "TH reads LastNetworkingStatus attribute from the DUT",
                     "Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(9, "TH reads LastNetworkID attribute from the DUT",
                     "Verify that DUT sends LastNetworkID as the Extended PAN ID from the operational dataset or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(10, "TH reads Breadcrumb attribute from the General Commissioning cluster",
                     "Verify that the breadcrumb value is set to 1"),
            TestStep(11, "TH sends ConnectNetwork command to the DUT with NetworkID field set to the Extended PAN ID from the operational dataset and Breadcrumb set to 2",
                     "Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound"),
            TestStep(12, "TH reads Breadcrumb attribute from the General Commissioning cluster",
                     "Verify that the breadcrumb value is set to 1"),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(14, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list contains 'NumNetworks' entries and has an entry with the following values:"
                     "1. NetworkID field value set as the Extended PAN ID from the operational dataset "
                     "2. Connected field value is of type boolean and has the value True"),
            TestStep(15, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(16, "TH sends RemoveNetwork Command to the DUT with NetworkID set to the Extended PAN ID from the operational dataset and Breadcrumb set to 1",
                     "Verify that the DUT sends NetworkConfigResponse to command with the following fields: "
                     "1. NetworkingStatus is success "
                     "2. NetworkIndex is 'Userth_netidx'"),
            TestStep(17, "TH sends CommissioningComplete command to the DUT",
                     "Verify that the DUT sends CommissioningCompleteResponse to the command with the ErrorCode field set to OK (0)"),
            TestStep(18, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(19, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list contains 'NumNetworks' -1 entries and does not contain the Extended PAN ID from the operational dataset"),
            TestStep(20, "(Cleanup) TH adds the Thread network back to the DUT.")
        ]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_10(self):
        # Commissioning is already done
        self.step(1)

        cnet = Clusters.NetworkCommissioning
        gen_comm = Clusters.GeneralCommissioning

        # Parse Extended PAN ID from the Thread operational dataset
        operational_dataset_hex = self.matter_test_config.thread_operational_dataset.hex()
        logging.info(f"Parsing Thread operational dataset: {operational_dataset_hex}")

        parsed_dataset = parse_openthread_dataset_stream(operational_dataset_hex)
        asserts.assert_is_not_none(parsed_dataset, "Failed to parse Thread operational dataset")

        ext_pan_id_hex = parsed_dataset.get("ExtendedPanId")
        asserts.assert_is_not_none(ext_pan_id_hex, "Extended PAN ID not found in Thread operational dataset")
        asserts.assert_equal(len(ext_pan_id_hex), 16,
                             f"Extended PAN ID must be 16 hex characters (8 bytes), got {len(ext_pan_id_hex)} characters")

        thread_network_id_bytes = bytes.fromhex(ext_pan_id_hex)
        logging.info(f"Extracted Extended PAN ID from dataset: {thread_network_id_bytes.hex()}")

        # Step 2: Read Networks and verify thread network
        self.step(2)
        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        logging.info(f"Networks by endpoint: {networks_dict}")
        connected_network_count = {}
        for ep in networks_dict:
            connected_network_count[ep] = sum((x.connected for x in networks_dict[ep]))
        logging.info(f"Connected networks count by endpoint: {connected_network_count}")
        asserts.assert_equal(sum(connected_network_count.values()), 1,
                             "Verify that only one entry has connected status as TRUE across ALL endpoints")

        # Step 3: Skip remaining steps if the connected network is NOT on the cluster currently being verified.
        self.step(3)
        current_cluster_connected = connected_network_count[self.get_endpoint()] == 1
        if not current_cluster_connected:
            logging.info("Current cluster is not connected, skipping all remaining test steps")
            self.mark_all_remaining_steps_skipped(4)
            return

        # Step 4: Arm failsafe and verify response
        self.step(4)
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
                                   )
        # Successful command execution is implied if no exception is raised.

        # Step 5: Read Networks and verify thread network
        self.step(5)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        num_networks = len(networks)

        # Find network index
        userth_netidx = None
        for idx, network in enumerate(networks):
            if network.networkID == thread_network_id_bytes:
                userth_netidx = idx
                asserts.assert_true(network.connected, "Thread network not connected")
                break
        asserts.assert_true(userth_netidx is not None, "Thread network not found")

        # Step 6: Remove network
        self.step(6)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        asserts.assert_equal(response.networkIndex, userth_netidx,
                             "Incorrect network index in response")

        # Step 7: Verify network count reduced and store the result
        self.step(7)
        networks_after_removal = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks_after_removal), num_networks - 1,
                             "Network count not reduced")

        # Step 8: Check LastNetworkingStatus
        self.step(8)
        status = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkingStatus
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(status, NullValue, "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            expected_status = cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
            asserts.assert_equal(status, expected_status, "LastNetworkingStatus should be Success when Networks list is not empty")

        # Step 9: Check LastNetworkID
        self.step(9)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkID
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "LastNetworkID should be Null when Networks list is empty")
        else:
            expected_network_id = thread_network_id_bytes
            asserts.assert_equal(last_network_id, expected_network_id, "LastNetworkID incorrect when Networks list is not empty")

        # Step 10: Verify breadcrumb
        self.step(10)
        breadcrumb = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 11: Try to connect to removed network
        self.step(11)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound)

        # Step 12: Verify breadcrumb unchanged
        self.step(12)
        breadcrumb = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 13: Disable failsafe
        self.step(13)
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
                                   )
        # Successful command execution is implied if no exception is raised.

        # Step 14: Verify network restored
        self.step(14)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks,
                             "Network count not restored")
        found = False
        for network in networks:
            if (network.networkID == thread_network_id_bytes and
                    network.connected):
                found = True
                break
        asserts.assert_true(found, "Thread network not restored")

        # Step 15: Re-arm failsafe
        self.step(15)
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
                                   )
        # Successful command execution is implied if no exception is raised.

        # Step 16: Remove network again
        self.step(16)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
        asserts.assert_equal(response.networkIndex, userth_netidx)

        # Step 17: Complete commissioning
        self.step(17)
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.CommissioningComplete()
                                   )
        # Successful command execution is implied if no exception is raised.

        # Step 18: Verify failsafe disabled
        self.step(18)
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
                                   )
        # Successful command execution is implied if no exception is raised.

        # Step 19: Verify network remains removed
        self.step(19)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Incorrect network count")
        for network in networks:
            asserts.assert_not_equal(network.networkID, thread_network_id_bytes,
                                     "Network still present after removal")

        # Step 20: (Cleanup) Add the network back.
        logging.info("Adding network back as cleanup step.")
        self.step(20)

        # Retrieve the operational dataset provided via command line
        operational_dataset = self.matter_test_config.thread_operational_dataset

        # Need to re-arm failsafe to add network
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
                                   )

        # Use AddOrUpdateThreadNetwork with the dataset
        add_resp = await self.send_single_cmd(
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=operational_dataset,
                breadcrumb=1  # Use a new breadcrumb for this operation
            )
        )
        asserts.assert_equal(add_resp.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failed to add/update Thread network during cleanup")

        # Commit the change by completing commissioning
        await self.send_single_cmd(endpoint=0,
                                   cmd=gen_comm.Commands.CommissioningComplete()
                                   )

        # Verify network added and is the one we intended to add
        networks_after_add = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        # Check count increases back to original count
        asserts.assert_equal(len(networks_after_add), num_networks, "Network count incorrect after re-adding")
        found = False
        for network in networks_after_add:
            # We compare against the NetworkID extracted from the operational dataset
            if network.networkID == thread_network_id_bytes:
                found = True
                # Check if connected status is True, although this might take time
                # asserts.assert_true(network.connected, "Re-added network is not connected")
                logging.info(f"Network {network.networkID.hex()} found. Connected: {network.connected}")
                break
        asserts.assert_true(
            found, "Added network (matching dataset-extracted Extended PAN ID) not found in Networks list after cleanup")


if __name__ == "__main__":
    default_matter_test_main()
