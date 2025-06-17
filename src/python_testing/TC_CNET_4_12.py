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

import asyncio
import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CNET_4_12(MatterBaseTest):
    """
    This test verifies the DUT connectivity to the Thread network using the ConnectNetwork Command, 
    including validation for two Thread operational datasets.

    Requires the following arguments:
    - --endpoint: Required endpoint for the Thread network.
    - --thread-dataset-hex: First operational dataset for the Thread network.

    Requires the following PIXITs:
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

    async def verify_thread_network_connected(self, networks, expected_thread_network_id, step_number, thread_network_name):
        """
        Verify that the expected Thread network is present in the list and is connected.

        Args:
            networks (list): List of network objects with 'networkID' and 'connected' attributes.
            expected_thread_network_id (bytes): The expected Thread networkID to search for.
            step_number (str): Step number for logging purposes starting with # (e.g., "#2").
            thread_network_name (str): Descriptive name of the Thread network (e.g., "THREAD_1ST").
        """
        for network in networks:
            if network.networkID == expected_thread_network_id:
                asserts.assert_true(network.connected,
                                    f"{thread_network_name} Thread network is not connected.")
                logger.info(f"Step {step_number}: Thread network '{thread_network_name}' was found and is connected.")
                break
        else:
            asserts.fail(f"{thread_network_name} Thread network not found.")

    async def verify_thread_network_response(self, resp):
        """
        Validate the thread network response from the device under test (DUT).

        This function performs the following checks:
        1. Ensures that the NetworkingStatus is 'Success' (0).
        2. Verifies that debugText is either None or an empty string if the status is success.
        3. If debugText contains any text, confirms that its length does not exceed 512 characters.

        Args:
            resp: The response object from the DUT containing networkingStatus and debugText attributes.

        Raises:
            AssertionError: If any of the validation conditions fail.
        """
        debug_text = resp.debugText

        # Validate that NetworkingStatus is 'Success' (0)
        asserts.assert_equal(
            resp.networkingStatus,
            Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
            "Expected NetworkingStatus to be 'Success' (0)"
        )

        # Validate that debugText is None or empty if status is success
        asserts.assert_true(
            debug_text is None or debug_text == '',
            "debugText must be None or empty if status is success."
        )

        # If debugText is present and not empty, validate len does not exceed 512 char
        if debug_text:
            asserts.assert_true(
                len(debug_text) <= 512,
                "debugText must be a string with a max length of 512 characters."
            )

    @property
    def default_timeout(self) -> int:
        return 900

    def desc_TC_CNET_4_12(self):
        return '[TC-CNET-4.12] [Thread] Verification for ConnectNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_12(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "CNET.S.F01"
        ]
        return pics

    def steps_TC_CNET_4_12(self) -> list[TestStep]:
        steps = [
            TestStep("precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT responds with ArmFailSafeResponse to the TH"),
            TestStep(2, "TH reads Networks attribute from the DUT and saves the number of entries as `NumNetworks`",
                     "* Verify that the Networks attribute list has an entry with the following fields:\n"
                     ". NetworkID is `th_xpan_1`\n"
                     ". Connected is of type bool and is TRUE"),
            TestStep(3, "TH saves the index of the Networks list entry from step 2 as `Userth_netidx`",
                     ""),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to `th_xpan_1` and Breadcrumb field set to 1",
                     "* Verify that DUT sends NetworkConfigResponse to command with the following response fields:\n"
                     ". NetworkingStatus is success\n"
                     ". NetworkIndex is `Userth_netidx`"),
            TestStep(5, "TH sends AddOrUpdateThreadNetwork command to the DUT with operational dataset field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET and Breadcrumb field set to 1",
                     "* Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields:\n"
                     ". NetworkingStatus is success which is '0'\n"
                     ". DebugText is of type string with max length 512 or empty"),
            TestStep(6, "TH reads Networks attribute from the DUT",
                     "* Verify that the Networks attribute list has an entry with the following fields:\n"
                     ". NetworkID is `th_xpan_2`\n"
                     ". Connected is of type bool and is FALSE"),
            TestStep(7, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET and Breadcrumb field set to 2",
                     "Wait for device to connect to the Thread network. A wait time of `connect_max_time_seconds + fudge_factor_seconds` is applied to allow Thread network connection and SRP record propagation"),
            TestStep(8, "TH discovers and connects to DUT on the PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(9, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 2"),
            TestStep(10, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(11, "TH ensures it can communicate with THREAD_1ST_OPERATIONALDATASET operational network (from --thread-dataset-hex)",
                     ""),
            TestStep(12, "TH discovers and connects to DUT on the THREAD_1ST_OPERATIONALDATASET operational network (from --thread-dataset-hex)",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(14, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to 'th_xpan_1' and Breadcrumb field set to 1",
                     "* Verify that DUT sends NetworkConfigResponse to command with the following fields:"
                     ". NetworkingStatus is success"
                     ". NetworkIndex is `Userth_netidx`"),
            TestStep(15, "TH sends AddOrUpdateThreadNetwork command to the DUT with the OperationalDataset field set to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET and Breadcrumb field set to 1",
                     "* Verify that DUT sends the NetworkConfigResponse command to the TH with the following fields:\n"
                     " . NetworkingStatus is success which is '0'\n"
                     ". DebugText is of type string with max length 512 or empty"),
            TestStep(16, "TH sends ConnectNetwork command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET and Breadcrumb field set to 3",
                     "Wait for device to connect to the Thread network. A wait time of `connect_max_time_seconds + fudge_factor_seconds` is applied to allow Thread network connection and SRP record propagation"),
            TestStep(17, "TH discovers and connects to DUT on the PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(18, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 3. Note: Wait for device to connect to the Thread network. "
                     "A wait time of `connect_max_time_seconds + fudge_factor_seconds` is applied to allow Thread network connection and SRP record propagation"),
            TestStep(19, "TH sends the CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)"),
            TestStep(20, "TH reads Networks attribute from the DUT",
                     "* Verify that the Networks attribute list has an entry with the following values:\n"
                     ". NetworkID field value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET\n"
                     ". Connected field value is of type bool and is TRUE"),
            TestStep(21, "TH switches back to THREAD_1ST_OPERATIONALDATASET operational network (from --thread-dataset-hex)",
                     "* Verify that the Networks attribute list has an entry with the following values:\n"
                     "  . NetworkID field value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET\n"
                     "  . Connected field value is of type bool and is TRUE")
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_12(self):

        # Pre-Conditions
        # Commissioning is already done
        self.step("precondition")

        # Assign required endpoint and Threads dataset
        endpoint = self.get_endpoint()
        thread_dataset_1 = self.matter_test_config.thread_operational_dataset
        thread_dataset_2 = self.user_params.get('PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET')

        # Validate required endpoint and Threads dataset
        asserts.assert_true(endpoint is not None, "Missing required endpoint")
        asserts.assert_true(thread_dataset_1 is not None, "Missing required THREAD_1ST: THREAD_1ST_OPERATIONALDATASET")
        asserts.assert_true(thread_dataset_2 is not None, "Missing required THREAD_2ND: PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET")

        # All required endpoint and Threads dataset are set and assigned, Thread dataset as str
        logger.info('Precondition: All required arguments are set and assigned, Thread dataset as str: '
                    f'THREADS_ENDPOINT = {endpoint}, '
                    f'THREAD_1ST_OPERATIONALDATASET = {thread_dataset_1}, '
                    f'PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET = {thread_dataset_2}')
        # thread_dataset_1_bytes = bytes.fromhex(thread_dataset_1)
        thread_dataset_1_bytes = thread_dataset_1
        thread_dataset_2_bytes = bytes.fromhex(thread_dataset_2)

        # All required arguments are set and assigned, Thread dataset as bytes
        logger.info('Precondition: All required arguments are set and assigned, Thread dataset as bytes: '
                    f'THREAD_1ST_OPERATIONALDATASET = {thread_dataset_1_bytes}, '
                    f'PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET = {thread_dataset_2_bytes}')

        # Validate the operational dataset structure (for both datasets)
        logger.info("Precondition: Validating THREAD operational datasets")

        # TODO: Consider centralizing this function (maybe in matter_testing.py)
        # Once CNET Thread tests are merged, Follow-up list of tests that might use this function:
        # CNET 4.10, ... (add other tests here)
        thread_network_id_bytes_th1 = await self.validate_thread_dataset(thread_dataset_1_bytes, "THREAD_1ST_OPERATIONALDATASET")
        thread_network_id_bytes_th2 = await self.validate_thread_dataset(thread_dataset_2_bytes, "THREAD_2ND_OPERATIONALDATASET")
        logger.info('Precondition: NetworkID : '
                    f'NetworkID_THREAD_1ST_OPERATIONALDATASET = {thread_network_id_bytes_th1}, '
                    f'NetworkID_THREAD_2ND_OPERATIONALDATASET = {thread_network_id_bytes_th2}')

        # Read the ConnectMaxTimeSeconds attribute after attempting to connect
        connect_max_time_seconds = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.ConnectMaxTimeSeconds
        )
        logger.info(f'Precondition: ConnectMaxTimeSeconds value: {connect_max_time_seconds}')

        # Fudge factor to ensure the Thread networks are fully initialized before switching.
        # TODO: Opened follow-up issue TC-CNET-4.12 Follow up - reconsider fudge factor, link: https://github.com/project-chip/matter-test-scripts/issues/583
        fudge_factor_seconds = 60
        logger.info(f'Precondition: fudge_factors_seconds value: {fudge_factor_seconds}')

        # Steps
        self.step(1)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=self.failsafe_expiration_seconds, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        logger.info(f'Step #1: ArmFailSafeResponse with ErrorCode as OK: ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(2)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #2: Networks attribute: {networks}')

        num_networks = len(networks)
        logger.info(f'Step #2: Number of Networks entries (NumNetworks): {num_networks}')
        asserts.assert_true(num_networks > 0, "Error: No networks found")

        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th1, "THREAD_1ST", "#2")

        self.step(3)
        # TODO: This test currently uses a single endpoint for both threads.
        # Issue #39069<https://github.com/project-chip/connectedhomeip/issues/39069> created to track follow-ups on problems with secondary networks,
        # since secondary network support is not implemented yet.

        # Find network index
        userth_netidx = None
        for idx, network in enumerate(networks):
            if network.networkID == thread_network_id_bytes_th1:
                userth_netidx = idx
                asserts.assert_true(network.connected, "Thread network not connected")
                break
        logger.info(f'Step #3: NetworkID from Networks attribute: {network.networkID}')
        logger.info(f'Step #3: Expected NetworkID for THREAD_1ST: {thread_network_id_bytes_th1}')
        logger.info(f'Step #3: Index of THREAD_1ST in Networks list (Userth_netidx): {userth_netidx}')
        asserts.assert_true(userth_netidx is not None, "Thread network not found in Networks list")

        self.step(4)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=thread_network_id_bytes_th1, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #4: RemoveNetwork response for THREAD_1ST: ({vars(resp)})')
        logger.info(f'Step #4: RemoveNetwork Status for THREAD_1ST is success: ({resp.networkingStatus})')
        logger.info(f'Step #4: RemoveNetwork NetworkIndex for THREAD_1ST: ({resp.networkIndex})')

        # Verify that the DUT responds with Remove Network with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Thread network was not removed")
        asserts.assert_equal(resp.networkIndex, userth_netidx, "Thread network index is not as expected.")

        self.step(5)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=thread_dataset_2_bytes,
            breadcrumb=1
        )
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #5: AddOrUpdateThreadNetwork response for THREAD_2ND: ({vars(resp)})')
        logger.info(f'Step #5: AddOrUpdateThreadNetwork Status for THREAD_2ND is success: ({resp.networkingStatus})')

        await self.verify_thread_network_response(resp)

        self.step(6)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #6: Networks attribute: {networks}')

        network_ids = [n.networkID for n in networks]
        logger.info(f"Step #6: Found NetworkIDs: {network_ids}")
        asserts.assert_in(
            thread_network_id_bytes_th2,
            network_ids,
            f"Expected networkID for THREAD_2ND {thread_network_id_bytes_th2} not found in Thread networks."
        )

        self.step(7)
        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=thread_network_id_bytes_th2, breadcrumb=2)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #7: ConnectNetwork resp for THREAD_2ND: ({vars(resp)})')
        logger.info(f'Step #7: ConnectNetwork Status for THREAD_2ND is success: ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNConnectNetworketwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ConnectNetwork")

        # TODO: Consider replacing the sleep (connect_max_time + fudge_factor) with dns-sd adverts check as improvement.
        # Wait for the device to establish connection with the new Thread network
        # Includes a fudge factor for SRP record propagation.
        await asyncio.sleep(connect_max_time_seconds + fudge_factor_seconds)
        logger.info("Step #7: Sleep completed for Thread network connection and SRP record propagation")

        self.step(8)
        # Verify that the TH successfully connects to the DUT
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #8: Networks attribute: {networks}')

        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th2, "THREAD_2ND", "#8")

        self.step(9)
        # Expired session is re-established the new session reading attribute (Breadcrum)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #9:  Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 2,
                             "The Breadcrumb attribute is not 2")

        self.step(10)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        logger.info(f'Step #10: ArmFailSafeResponse with ErrorCode as OK: ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(11)
        # Step 11: When the failsafe is disarmed, the device should automatically return to THREAD_1ST.
        # This means the THREAD_2ND network will be removed,
        # and the device will reconnect to Thread 1 without further intervention.
        logger.info('Step #11: DUT automatically return to THREAD_1ST')

        self.step(12)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #12: Networks attribute: {networks}')

        # Session expired and re-establish the new session reading attribute (Breadcrum)
        await asyncio.sleep(connect_max_time_seconds + 5)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #12: Breadcrumb attribute: {breadcrumb_info}')

        await asyncio.sleep(connect_max_time_seconds + 5)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #12: Networks attribute after read atribute: {networks}')
        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th1, "THREAD_1ST", "#12")

        self.step(13)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=self.failsafe_expiration_seconds
        )
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        logger.info(f'Step #13: ArmFailSafeResponse with ErrorCode as OK: ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        self.step(14)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=thread_network_id_bytes_th1, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        network_index = resp.networkIndex
        logger.info(f'Step #14: RemoveNetwork Status for THREAD_1ST is success: ({resp.networkingStatus})')
        logger.info(f'Step #14: Network index for THREAD_1ST: ({network_index})')

        # Verify that the DUT responds with Remove Network with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ReordeRemove Network")
        asserts.assert_equal(network_index, 0, "The network index is not as expected.")

        self.step(15)
        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=thread_dataset_2_bytes,
            breadcrumb=1
        )
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #15: AddOrUpdateThreadNetwork response for THREAD_2ND: ({vars(resp)})')
        logger.info(f'Step #15: AddOrUpdateThreadNetwork Status for THREAD_2ND is success: ({resp.networkingStatus})')

        await self.verify_thread_network_response(resp)

        self.step(16)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #16: Networks attribute: {networks}')

        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=thread_network_id_bytes_th2, breadcrumb=3)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #16: ConnectNetwork resp for THREAD_2ND: ({vars(resp)})')
        logger.info(f'Step #16: ConnectNetwork Status for THREAD_2ND is success ({resp.networkingStatus})')
        # Verify that the DUT responds with AddThreadNConnectNetworketwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ConnectNetwork")

        # Wait for the device to establish connection with the new Thread network
        # Includes a fudge factor for SRP record propagation.
        await asyncio.sleep(connect_max_time_seconds + fudge_factor_seconds)
        logger.info("Step #16: Sleep completed for Thread network connection and SRP record propagation")

        self.step(17)
        # THREAD_2ND Successfully connects to the DUT from previous step
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #17: Networks attribute: {networks}')

        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th2, "THREAD_2ND", "#17")

        self.step(18)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #18:  Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 3,
                             "The Breadcrumb attribute is not 3")

        # Wait for the device to establish connection with the new Thread network
        # Includes a fudge factor for SRP record propagation.
        await asyncio.sleep(connect_max_time_seconds + fudge_factor_seconds)
        logger.info("Step #18: Sleep completed for Thread network connection and SRP record propagation")

        self.step(19)
        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #19: CommissioningComplete response: ({vars(resp)})')
        logger.info(f'Step #19: CommissioningComplete Status is success: ({resp.errorCode})')
        # Verify that the DUT responds with CommissioningComplete with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from CommissioningComplete")

        self.step(20)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #20: Networks attribute: {networks}')

        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th2, "THREAD_2ND", "#20")

        self.step(21)
        # Back to THREAD_1ST
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=self.failsafe_expiration_seconds
        )
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        logger.info(f'Step #21: ArmFailSafeResponse with ErrorCode as OK: ({resp.errorCode})')
        asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")

        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=thread_network_id_bytes_th2)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        network_index = resp.networkIndex
        logger.info(f'Step #21: RemoveNetwork Status for THREAD_2ND is success: ({resp.networkingStatus})')
        logger.info(f'Step #21: Network index for THREAD_2nd: ({network_index})')

        # Verify that the DUT responds with Remove Network with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ReordeRemove Network")
        asserts.assert_equal(network_index, 0, "The network index is not as expected.")

        cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=thread_dataset_1_bytes
        )
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #21: AddOrUpdateThreadNetwork response for THREAD_1ST: ({vars(resp)})')
        logger.info(f'Step #21: AddOrUpdateThreadNetwork Status for THREAD_1ST is success: ({resp.networkingStatus})')

        await self.verify_thread_network_response(resp)

        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(
            networkID=thread_network_id_bytes_th1
        )

        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #21: ConnectNetwork resp for for THREAD_1ST: ({vars(resp)})')
        # Verify that the DUT responds with ConnectNetwork with NetworkingStatus as 'Success'(0)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failure status returned from ConnectNetwork")

        # TODO: Consider replacing the sleep (connect_max_time + fudge_factor) with dns-sd adverts check as improvement.
        # Wait for the device to establish connection with the new Thread network
        # Includes a fudge factor for SRP record propagation.
        await asyncio.sleep(connect_max_time_seconds + fudge_factor_seconds)
        logger.info("Step #21: Sleep completed for Thread network connection and SRP record propagation")

        # THREAD_1ST Successfully connects to the DUT from previous step
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #21: Networks attribute: {networks}')

        await self.verify_thread_network_connected(networks, thread_network_id_bytes_th1, "THREAD_1ST", "#21")


if __name__ == "__main__":
    default_matter_test_main()
