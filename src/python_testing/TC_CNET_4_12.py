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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
# N/A - Test will not run on CI

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CNET_4_12(MatterBaseTest):
    """
    This test verifies the DUT connectivity to the Thread network using the ConnectNetwork Command, 
    including validation for two Thread operational datasets.

    Requires the following PIXITs:
    - PIXIT.CNET.ENDPOINT_THREAD: Required endpoint for the Thread network.
    - PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET: First operational dataset for the Thread network.
    - PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET: Second operational dataset for the Thread network.

    If any of the above PIXITs are missing, the test will raise an error.
    """

    CLUSTER_CNET = Clusters.NetworkCommissioning
    CLUSTER_DESC = Clusters.Descriptor
    CLUSTER_CGEN = Clusters.GeneralCommissioning
    failsafe_expiration_seconds = 900

    async def validate_thread_dataset(self, dataset_bytes, dataset_name):
        """
        Validates the structure of a Thread operational dataset (formatted).

        Args:
            dataset_bytes: The dataset to validate, expected to be in byte format.
            dataset_name: The name of the dataset.
        """
        logger.info(f"Validating {dataset_name}")

        # Validate that the dataset contains valid data (not empty)
        asserts.assert_true(len(dataset_bytes) > 0, f"PIXIT.CNET.{dataset_name} must be supplied.")

        # Validate that the ExtPANID TLV exists in the dataset
        ext_pan_id_marker = b'\x02\x08'
        marker_index = dataset_bytes.find(ext_pan_id_marker)

        asserts.assert_true(
            marker_index != -1 and len(dataset_bytes) >= marker_index + len(ext_pan_id_marker) + 8,
            f"Could not find ExtPANID marker in PIXIT dataset: {dataset_name} or dataset too short in PIXIT bytes."
        )

        ext_pan_id_start = marker_index + len(ext_pan_id_marker)
        thread_network_id_bytes = dataset_bytes[ext_pan_id_start: ext_pan_id_start + 8]

        asserts.assert_equal(len(thread_network_id_bytes), 8, f"Extracted ExtPANID from {dataset_name} must be 8 bytes long.")

        logger.info(f"Extracted Network ID (ExtPANID) from {dataset_name}: {thread_network_id_bytes.hex()}")

        return thread_network_id_bytes

    def def_TC_CNET_4_12(self):
        return '[TC-CNET-4.12] [Thread] Verification for ConnectNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_12(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "CNET.S"
        ]
        return pics

    def steps_TC_CNET_4_12(self) -> list[TestStep]:
        steps = [
            TestStep('precondition-1', 'DUT supports CNET.S.F01(TH)'),
            TestStep('precondition-2', 'DUT has a Network Commissioning cluster on endpoint PIXIT.CNET.ENDPOINT_THREAD with FeatureMap attribute of 2', is_commissioning=True),
            TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(2, 'TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks'),
            TestStep(3, 'TH saves the index of the Networks list entry from step 2 as Userth_netidx'),
            TestStep(4, '''TH sends RemoveNetwork Command to the DUT 
                     with NetworkID field set to th_xpan 
                     and Breadcrumb field set to 1'''),
            TestStep(5, '''TH sends AddOrUpdateThreadNetwork command to the DUT 
                     with operational dataset field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET 
                     and Breadcrumb field set to 1'''),
            TestStep(6, 'TH reads Networks attribute from the DUT'),
            TestStep(7, '''TH sends ConnectNetwork command to the DUT 
                     with NetworkID field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET 
                     and Breadcrumb field set to 2'''),
            TestStep(8, 'TH discovers and connects to DUT on the PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET operational network'),
            TestStep(9, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            TestStep(10, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0'),
            TestStep(11, 'TH ensures it can communicate on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET'),
            TestStep(12, 'TH discovers and connects to DUT on the PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET operational network'),
            TestStep(13, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(14, '''TH sends RemoveNetwork Command to the DUT with NetworkID field set to th_xpan 
                     and Breadcrumb field set to 1'''),
            TestStep(15, '''TH sends AddOrUpdateThreadNetwork command to the DUT 
                     with the OperationalDataset field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET 
                     and Breadcrumb field set to 1'''),
            TestStep(16, '''TH sends ConnectNetwork command to the DUT 
                     with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET 
                     and Breadcrumb field set to 3'''),
            TestStep(17, 'TH discovers and connects to DUT on the PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET operational network'),
            TestStep(18, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            TestStep(19, 'TH sends the CommissioningComplete command to the DUT'),
            TestStep(20, 'TH reads Networks attribute from the DUT')
        ]
        return steps

    @async_test_body
    async def test_TC_CNET_4_12(self):

        if self.is_pics_sdk_ci_only:
            logger.info('Test is not running in CI.')
            self.skip_all_remaining_steps('precondition-1')
            return

        # Pre-Conditions
        self.step('precondition-1')
        logger.info('Precondition 1: DUT supports CNET.S.F01(TH)')

        self.step('precondition-2')
        # By running this test from the terminal, it commissions the device.
        logger.info('Precondition 2: DUT has a Network Commissioning cluster on the correct endpoint.')

        # Assign required PIXITs
        endpoint = self.user_params.get('PIXIT.CNET.ENDPOINT_THREAD')
        thread_dataset_1 = self.user_params.get('PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET')
        thread_dataset_2 = self.user_params.get('PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET')

        # Validate required PIXIT
        asserts.assert_true(endpoint is not None, "Missing required PIXIT: PIXIT.CNET.ENDPOINT_THREAD")
        asserts.assert_true(thread_dataset_1 is not None, "Missing required PIXIT: PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET")
        asserts.assert_true(thread_dataset_2 is not None, "Missing required PIXIT: PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET")

        # All required PIXITs are present and assigned,  Thread dataset as str
        logger.info('Precondition 2: All required PIXITs are present and assigned, Thread dataset as str: '
                    f'PIXIT.CNET.ENDPOINT_THREAD = {endpoint}, '
                    f'PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET = {thread_dataset_1}, '
                    f'PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET = {thread_dataset_2}')

        thread_dataset_1_bytes = bytes.fromhex(thread_dataset_1)
        thread_dataset_2_bytes = bytes.fromhex(thread_dataset_2)

        # All required PIXITs are present and assigned,  Thread dataset as bytes
        logger.info('Precondition 2: All required PIXITs are present and assigned, Thread dataset as bytes: '
                    f'PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET = {thread_dataset_1_bytes}, '
                    f'PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET = {thread_dataset_2_bytes}')

        # Validate the operational dataset structure (for both datasets)
        logger.info("Precondition 2: Validating THREAD operational datasets")

        thread_network_id_bytes_th1 = await self.validate_thread_dataset(thread_dataset_1_bytes, "THREAD_1ST_OPERATIONALDATASET")
        thread_network_id_bytes_th2 = await self.validate_thread_dataset(thread_dataset_2_bytes, "THREAD_2ND_OPERATIONALDATASET")

        # The FeatureMap attribute value is 2
        feature_map = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_CNET,
            attribute=self.CLUSTER_CNET.Attributes.FeatureMap)
        asserts.assert_true(feature_map == 2,
                            msg="Verify that feature_map is equal to 1")
        logger.info(f'Pre-Conditions #3: The FeatureMap attribute value is: {feature_map}')

        # Steps

        self.step(1)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=self.failsafe_expiration_seconds, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #1 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(2)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )

        logger.info(f'Step #2: Networks attribute: {networks}')
        logger.info(f'Step #2: Networks attribute - networkID: ({networks.networkID})')
        logger.info(f'Step #2: Networks attribute - connected: ({networks.connected})')

        num_networks = len(networks)
        logger.info(f'Step #2: Number of Networks entries (NumNetworks): {num_networks}')
        asserts.assert_true(num_networks > 0, "Error: No networks found")

        self.step(3)
        # Find network index
        userth_netidx = None
        for idx, network in enumerate(networks):
            if network.networkID == thread_network_id_bytes_th1:
                userth_netidx = idx
                asserts.assert_true(network.connected, "Thread network not connected")
                break
        asserts.assert_true(userth_netidx is not None, "Thread network not found")
        logger.info(f'Step #3: Networks attribute: {network.networkID}')
        logger.info(f'Step #3: Networks attribute: {thread_network_id_bytes_th1}')
        logger.info(f'Step #3: Networks attribute: {userth_netidx}')

        self.step(4)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=thread_network_id_bytes_th1, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #4: RemoveNetwor response ({vars(resp)})')
        logger.info(f'Step #4: RemoveNetwork Status is success ({resp.networkingStatus})')
        logger.info(f'Step #4: RemoveNetwork NetworkIndex: ({resp.networkIndex})')

        # Verify that the DUT responds with Remove Network with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Network was not removed")
        asserts.assert_equal(resp.networkIndex, userth_netidx, "The network index is not as expected.")

        self.step(5)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=thread_dataset_2_bytes, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #5: AddOrUpdateThreadNetwork response ({vars(resp)})')
        logger.info(f'Step #5: AddOrUpdateThreadNetwork Status is success ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNetwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from AddThreadNetwork")
        debug_text = resp.debugText
        # TODO: Check if None is part of the validation
        # asserts.assert_true(debug_text is None or debug_text == '' or len(debug_text) <= 512,
        #                     "debugText must be None, empty or have a maximum length of 512 characters.")

        self.step(6)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #6: Networks attribute: {networks}')
        # TODO; Implement the Verify that the Networks attribute list has an entry NetworkID=th_xpan, Connected=FALSE
        # TODO: Why th_xpan? should be th_xpan_1

        self.step(7)
        network_name = b"OpenThread-55dc"

        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=thread_network_id_bytes_th2, breadcrumb=2)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #7: ConnectNetwork resp VARS ({vars(resp)})')
        logger.info(f'Step #7: ConnectNetwork Status is success ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNConnectNetworketwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ConnectNetwork")

        self.step(8)
        # TODO: Verify that the TH successfully connects to the DUT from previous step

        self.step(9)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #9:  Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")

        self.step(10)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #10 - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(11)
        # TODO: Verify that TH can communicate on th_xpan

        self.step(12)
        # TODO: Verify that TH can discover and connect on th_xpan

        self.step(13)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=self.failsafe_expiration_seconds)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #1b - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(14)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=th_xpan, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        network_index = resp.networkIndex
        logger.info(f'Step #14: RemoveNetwork Status is success ({resp.networkingStatus})')
        logger.info(f'Step #14: Network index: ({network_index})')

        # Verify that the DUT responds with Remove Network with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ReordeRemove Network")
        asserts.assert_equal(network_index, 0, "The network index is not as expected.")

        self.step(15)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(operationalDataset=th_xpan_1, breadcrumb=3)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #15: AddOrUpdateThreadNetwork Status is success ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNetwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from AddThreadNetwork")
        debug_text = resp.debugText
        # TODO: Check if None is part of the validation
        # asserts.assert_true(debug_text is None or debug_text == '' or len(debug_text) <= 512,
        #                     "debugText must be None, empty or have a maximum length of 512 characters.")

        self.step(16)
        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(operationalDataset=th_xpan_1, breadcrumb=3)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #16: ConnectNetwork Status is success ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNConnectNetworketwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ConnectNetwork")

        self.step(17)
        # TODO: Verify that the TH successfully connects to the DUT from previous step

        self.step(18)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #18:  Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 3,
                             "The Breadcrumb attribute is not 3")

        self.step(19)
        # TODO: Implement TH sends the CommissioningComplete and CommissioningCompleteResponse with the ErrorCode OK (0)

        self.step(20)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #20: Networks attribute: {networks}')

        # TODO: Implement proper validation to verify the the Networks attribute "NetworkID" and "Connected"
        # for cnet in networks:
        #     if cnet.networkID.decode('utf-8') == th_xpan_1 and cnet.connected:
        #         network_found = True
        #         break
        # logger.info(f'Step #2: Found network with ID {th_xpan_1} and connected={network_found}.')
        # asserts.assert_true(
        #     network_found, f"Error: Network with ID {th_xpan_1} and connected=True not found.")


if __name__ == "__main__":
    default_matter_test_main()
