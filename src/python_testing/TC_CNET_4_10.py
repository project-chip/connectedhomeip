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

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.decorators import has_feature, run_if_endpoint_matches
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main
from mobly import asserts


class TC_CNET_4_10(MatterBaseTest):
    """
    [TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]

    Pre-Conditions:
        1. DUT supports CNET.S.F01(TH).
        2. DUT has a Network Commissioning cluster on the endpoint specified
           in the --endpoint command-line argument, with a FeatureMap attribute of 2.
        3. DUT is commissioned on the operational network specified
           in the --thread-dataset-hex command-line argument.
        4. TH can communicate with the DUT on the operational network specified
           in the --thread-dataset-hex command-line argument.
        5. PIXIT.CNET.THREAD_1ST_EXTPANID is a valid 8-byte Thread Extended PAN ID
           (hex string) of the operational network provided via the --hex-arg command-line argument.

    Example usage: 
        To run the test case, use the following command:

        ```bash
        python src/python_testing/TC_CNET_4_10.py --commissioning-method ble-thread --discriminator 3840 --passcode 20202021 \ 
        --endpoint <endpoint_value> --thread-dataset-hex <dataset_value> --hex-arg PIXIT.CNET.THREAD_1ST_EXTPANID:<extpanid_value>
        ```

        Where `<endpoint_value>` should be replaced with the actual endpoint number 
        for the Network Commissioning cluster on the DUT,
        `dataset_value` should be replaced with the operational dataset of the DUT in hexadecimal format, and 
        `<extpanid_value>`should be replaced with the 8-byte Thread Extended PAN ID of the operational network 
        in hexadecimal format.
    """

    def desc_TC_CNET_4_10(self):
        return '[TC-CNET-4.10] [Thread] Verification for RemoveNetwork Command [DUT-Server]'

    def steps_TC_CNET_4_10(self):
        return [
            TestStep("preconditions", "TH is commissioned", "Commissioning is successful",  is_commissioning=True),
            TestStep(1, "TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters).",
                     "Verify that there is a single connected network across ALL network commissioning clusters. "),
            TestStep(2, "Skip remaining steps if the connected network is NOT on the cluster currently being verified."),
            TestStep(3, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900 and Breadcrumb set to 0",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(4, "TH reads Networks attribute from the DUT and save the number of entries as 'NumNetworks'",
                     "Verify that the Networks attribute list has an entry with the following values: "
                     "1. NetworkID field value set as the PIXIT.CNET.THREAD_1ST_EXTPANID"
                     "2. Connected field value is of type boolean and has the value True"),
            TestStep(5, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_EXTPANID and Breadcrumb field set to 1",
                     "Verify that DUT sends NetworkConfigResponse to command with the following fields: "
                     "1. NetworkingStatus is success "
                     "2. NetworkIndex is 'Userth_netidx'"),
            TestStep(6, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list has 'NumNetworks' - 1 entries"),
            TestStep(7, "TH reads LastNetworkingStatus attribute from the DUT",
                     "Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(8, "TH reads LastNetworkID attribute from the DUT",
                     "Verify that DUT sends LastNetworkID as PIXIT.CNET.THREAD_1ST_EXTPANID or null if 'NumNetworks' - 1 == 0 entries."),
            TestStep(9, "TH reads Breadcrumb attribute from the General Commissioning cluster",
                     "Verify that the breadcrumb value is set to 1"),
            TestStep(10, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_1ST_EXTPANID and Breadcrumb set to 2",
                     "Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound"),
            TestStep(11, "TH reads Breadcrumb attribute from the General Commissioning cluster",
                     "Verify that the breadcrumb value is set to 1"),
            TestStep(12, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(13, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list contains 'NumNetworks' entries and has an entry with the following values:"
                     "1. NetworkID field value set as the PIXIT.CNET.THREAD_1ST_EXTPANID"
                     "2. Connected field value is of type boolean and has the value True"),
            TestStep(14, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(15, "TH sends RemoveNetwork Command to the DUT with NetworkID set to PIXIT.CNET.THREAD_1ST_EXTPANID and Breadcrumb set to 1",
                     "Verify that the DUT sends NetworkConfigResponse to command with the following fields: "
                     "1. NetworkingStatus is success "
                     "2. NetworkIndex is 'Userth_netidx'"),
            TestStep(16, "TH sends CommissioningComplete command to the DUT",
                     "Verify that the DUT sends CommissioningCompleteResponse to the command with the ErrorCode field set to OK (0)"),
            TestStep(17, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 and Breadcrumb set to 0",
                     "Verify that the DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(18, "TH reads Networks attribute from the DUT",
                     "Verify that the Networks attribute list contains 'NumNetworks' -1 entries and does not contain the PIXIT.CNET.THREAD_1ST_EXTPANID"),
            TestStep(19, "(Cleanup) TH adds the Thread network back to the DUT.")
        ]

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_10(self):
        # Commissioning is already done
        self.step("preconditions")

        cnet = Clusters.NetworkCommissioning
        gen_comm = Clusters.GeneralCommissioning

        # Get NetworkID (Extended PAN ID) directly from PIXIT
        thread_network_id_bytes = self.matter_test_config.global_test_params.get('PIXIT.CNET.THREAD_1ST_EXTPANID')
        asserts.assert_is_not_none(thread_network_id_bytes, "PIXIT.CNET.THREAD_1ST_EXTPANID must be supplied via --hex-arg.")
        # Ensure it's bytes and the correct length (8 bytes)
        asserts.assert_is_instance(thread_network_id_bytes, bytes,
                                   "PIXIT.CNET.THREAD_1ST_EXTPANID should be delivered as bytes by --hex-arg.")
        asserts.assert_equal(len(thread_network_id_bytes), 8,
                             f"PIXIT.CNET.THREAD_1ST_EXTPANID must be 8 bytes (16 hex chars), received {len(thread_network_id_bytes)} bytes.")
        logging.info(f"Using Network ID (ExtPANID) from PIXIT: {thread_network_id_bytes.hex()}")

        # Step 1: Read Networks and verify thread network
        self.step(1)
        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        logging.info(f"Networks by endpoint: {networks_dict}")
        connected_network_count = {}
        for ep in networks_dict:
            connected_network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
        logging.info(f"Connected networks count by endpoint: {connected_network_count}")
        asserts.assert_equal(sum(connected_network_count.values()), 1,
                             "Verify that only one entry has connected status as TRUE across ALL endpoints")

        # Step 2: Skip remaining steps if the connected network is NOT on the cluster currently being verified.
        self.step(2)
        current_cluster_connected = connected_network_count[self.get_endpoint()] == 1
        if not current_cluster_connected:
            logging.info("Current cluster is not connected, skipping all remaining test steps")
            self.skip_all_remaining_steps()
            return

        # Step 3: Arm failsafe and verify response
        self.step(3)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 4: Read Networks and verify thread network
        self.step(4)
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

        # Step 5: Remove network
        self.step(5)
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

        # Step 6: Verify network count reduced and store the result
        self.step(6)
        networks_after_removal = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks_after_removal), num_networks - 1,
                             "Network count not reduced")

        # Step 7: Check LastNetworkingStatus
        self.step(7)
        status = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkingStatus
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(status, NullValue, "LastNetworkingStatus should be Null when Networks list is empty")
        else:
            expected_status = cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
            asserts.assert_equal(status, expected_status, "LastNetworkingStatus should be Success when Networks list is not empty")

        # Step 8: Check LastNetworkID
        self.step(8)
        last_network_id = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.LastNetworkID
        )
        if len(networks_after_removal) == 0:
            asserts.assert_equal(last_network_id, NullValue, "LastNetworkID should be Null when Networks list is empty")
        else:
            expected_network_id = thread_network_id_bytes
            asserts.assert_equal(last_network_id, expected_network_id, "LastNetworkID incorrect when Networks list is not empty")

        # Step 9: Verify breadcrumb
        self.step(9)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 10: Try to connect to removed network
        self.step(10)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound)

        # Step 11: Verify breadcrumb unchanged
        self.step(11)
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=gen_comm,
            attribute=gen_comm.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 1)

        # Step 12: Disable failsafe
        self.step(12)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 13: Verify network restored
        self.step(13)
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

        # Step 14: Re-arm failsafe
        self.step(14)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 15: Remove network again
        self.step(15)
        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=thread_network_id_bytes,
                breadcrumb=1
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
        asserts.assert_equal(response.networkIndex, userth_netidx)

        # Step 16: Complete commissioning
        self.step(16)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.CommissioningComplete()
        )
        # Successful command execution is implied if no exception is raised.

        # Step 17: Verify failsafe disabled
        self.step(17)
        await self.send_single_cmd(
            cmd=gen_comm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        )
        # Successful command execution is implied if no exception is raised.

        # Step 18: Verify network remains removed
        self.step(18)
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        asserts.assert_equal(len(networks), num_networks - 1,
                             "Incorrect network count")
        for network in networks:
            asserts.assert_not_equal(network.networkID, thread_network_id_bytes,
                                     "Network still present after removal")

        # Step 19: (Cleanup) Add the network back.
        logging.info("Adding network back as cleanup step.")
        self.step(19)

        # Retrieve the operational dataset provided via command line
        operational_dataset = self.matter_test_config.thread_operational_dataset

        # Need to re-arm failsafe to add network
        await self.send_single_cmd(
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
        await self.send_single_cmd(
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
            # We compare against the NetworkID extracted earlier from the PIXIT
            if network.networkID == thread_network_id_bytes:
                found = True
                # Check if connected status is True, although this might take time
                # asserts.assert_true(network.connected, "Re-added network is not connected")
                logging.info(f"Network {network.networkID.hex()} found. Connected: {network.connected}")
                break
        asserts.assert_true(found, "Added network (matching PIXIT NetworkID) not found in Networks list after cleanup")


if __name__ == "__main__":
    default_matter_test_main()
