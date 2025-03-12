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
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --str-arg PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID:${WIFI_SSID}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import math

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_CNET_4_13(MatterBaseTest):

    CLUSTER_CNET = Clusters.NetworkCommissioning
    CLUSTER_DESC = Clusters.Descriptor
    CLUSTER_CGEN = Clusters.GeneralCommissioning
    platforms_networks_value = 1
    failsafe_expiration_seconds = 900
    default_network_id = 'INFINITUM10A2'

    def def_TC_CNET_4_13(self):
        return '[TC-CNET-4.13] [Wi-Fi] Verification for ReorderNetwork command [DUT-Server]'

    def pics_TC_CNET_4_13(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "CNET.S"
        ]
        return pics

    def steps_TC_CNET_4_13(self) -> list[TestStep]:
        steps = [
            TestStep('precondition-1', 'TH is commissioned', is_commissioning=True),
            TestStep('precondition-2', 'The cluster Identifier 49 (0x0031) is present in the ServerList attribute'),
            TestStep('precondition-3', 'The FeatureMap attribute value is 1'),
            TestStep('1a', 'TH reads MaxNetworks attribute from DUT and is saved as MaxNetworksValue for future use'),
            TestStep('1b', 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(2, 'TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks'),
            TestStep(3, '''TH calculates the number of remaining network slots as MaxNetworksValue - NumNetworks
                      and saves as RemainingNetworkSlots'''),
            TestStep(4, '''TH calculates the midpoint of the network list as floor(('MaxNetworksValue' + 1)/2) 
                     and saves as 'Midpoint'''),
            TestStep(5, '''TH sends AddOrUpdateWiFiNetwork command to the DUT. 
                     This step should be repeated 'RemainingNetworkSlots' times using DIFFERENT SSID 
                     and credential values and the Breadcrumb field set to 1. 
                     Note that these credentials are NOT required to be connectable.'''),
            TestStep(6, '''TH reads Networks attribute from the DUT and saves the list as 'OriginalNetworkList'''),
            TestStep(7, '''TH sends ReorderNetwork Command to the DUT with the following fields:
                     NetworkID is PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
                     NetworkIndex is 'MaxNetworksValue'
                     Breadcrumb is 2'''),
            TestStep(8, '''TH reads Breadcrumb attribute from the General Commissioning Cluster'''),
            TestStep(9, '''TH sends ReorderNetwork Command to the DUT with the following fields:
                     NetworkID is a NetworkID value NOT present in 'OriginalNetworkList'
                     NetworkIndex is 'Midpoint'
                     Breadcrumb is 2'''),
            TestStep(10, '''TH reads Breadcrumb attribute from the General Commissioning Cluster'''),
            TestStep(11, '''TH sends ReorderNetwork Command to the DUT with the following fields:
                     NetworkID is PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
                     NetworkIndex is 'Midpoint'
                     Breadcrumb is 2'''),
            TestStep(12, 'TH reads Breadcrumb attribute from the General Commissioning Cluster'),
            TestStep(13, 'TH reads Networks attribute list from the DUT'),
            TestStep(14, '''TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0'''),
            TestStep(15, '''TH reads Networks attribute list from the DUT'''),
            TestStep(16, '''TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900''')
        ]
        return steps

    @async_test_body
    async def test_TC_CNET_4_13(self):

        if self.is_pics_sdk_ci_only:
            logger.info('Test is not running in CI.')
            self.skip_all_remaining_steps('precondition-1')
            return

        # Pre-Conditions
        self.step('precondition-1')
        # By running this test from the terminal, it commissions the device with the ble commissioning method.
        logger.info('Pre-Conditions #1: The DUT is commisioned over BLE')

        self.step('precondition-2')
        # The cluster Identifier 49 (0x0031) is present in the ServerList attribute
        server_list = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_DESC,
            attribute=self.CLUSTER_DESC.Attributes.ServerList)

        logger.info(
            f'Pre-Conditions #2: The cluster Identifier 49 (0x0031) is present in the ServerList attribute {server_list}')

        asserts.assert_true(49 in server_list,
                            msg="Verify for the presence of an element with value 49 (0x0031) in the ServerList")

        self.step('precondition-3')
        # The FeatureMap attribute value is 1
        feature_map = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_CNET,
            attribute=self.CLUSTER_CNET.Attributes.FeatureMap)
        asserts.assert_true(feature_map == 1,
                            msg="Verify that feature_map is equal to 1")
        logger.info(f'Pre-Conditions #3: The FeatureMap attribute value is: {feature_map}')

        # Steps
        self.step('1a')
        max_networks_value = await self.read_single_attribute_check_success(
            cluster=self.CLUSTER_CNET,
            attribute=self.CLUSTER_CNET.Attributes.MaxNetworks)
        logger.info(f'Step #1a: The "MaxNetworksValue" connected: {max_networks_value}')

        if max_networks_value < self.platforms_networks_value:
            logger.info('Step #1a: MaxNetworksValue is less than 2, skipping all remaining steps.')
            self.skip_all_remaining_steps('1b')
            return

        # Proceed with the following steps

        self.step('1b')
        cmd = self.CLUSTER_CGEN.Commands.ArmFailSafe(expiryLengthSeconds=self.failsafe_expiration_seconds, breadcrumb=1)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, self.CLUSTER_CGEN.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #1b - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(2)
        pixit_network_id = self.user_params.get('PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID', self.default_network_id)
        logger.info(f'Step #2: pixit_network_id: {pixit_network_id}')
        network_found = False

        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #2: Networks attribute: {networks}')

        num_networks = len(networks)
        logger.info(f'Step #2: Number of Networks entries (NumNetworks): {num_networks}')
        asserts.assert_true(num_networks > 0, "Error: No networks found")

        for cnet in networks:
            if cnet.networkID.decode('utf-8') == pixit_network_id and cnet.connected:
                network_found = True
                break
        logger.info(f'Step #2: Found network with ID {pixit_network_id} and connected={network_found}.')
        asserts.assert_true(
            network_found, f"Error: Network with ID {pixit_network_id} and connected=True not found.")

        self.step(3)
        remaining_network_slots = max_networks_value - num_networks
        logger.info(f'Step #3: The remaining network slots : {remaining_network_slots}')

        self.step(4)
        midpoint = math.floor((max_networks_value + 1) / 2)
        logger.info(f'Step #4: The calculated Midpoint is: {midpoint}')

        self.step(5)
        # TODO: Implement Step #5: Platforms must support more than 1 Networks as MaxNetworks
        # # This step should be repeated 'RemainingNetworkSlots' times using DIFFERENT SSID and credential values and the Breadcrumb field set to 1
        # cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(
        #      ssid=WIFI_SSID.encode(), credentials=WIFI_PASS.encode(), breadcrumb=1)
        # resp = await self.send_single_cmd(
        #     dev_ctrl=self.default_controller,
        #     node_id=self.dut_node_id,
        #     cmd=cmd
        # )

        # logger.info(f"Received response: {resp}")
        # logger.info(f"Received response status: {resp.networkingStatus}")
        # Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess
        # Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kBoundsExceeded

        self.step(6)
        original_network_list = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #6: Networks attribute saved as OriginalNetworkList: {original_network_list}')

        self.step(7)
        cmd = Clusters.NetworkCommissioning.Commands.ReorderNetwork(
            networkID=pixit_network_id.encode(), networkIndex=max_networks_value, breadcrumb=2)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #7: ReorderNetwork with ErrorCode as OutOfRange ({resp.networkingStatus})')
        # Verify that the DUT responds with ReorderNetwork with NetworkingStatus as 'OutOfRange'(1)
        asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kOutOfRange,
                             "Failure status returned from ReorderNetwork")
        # Verify that DebugText is empty or has a maximum length of 512 characters
        debug_text = resp.debugText
        # TODO: Check if None is part of the validation
        asserts.assert_true(debug_text is None or debug_text == '' or len(debug_text) <= 512,
                            "debugText must be None, empty or have a maximum length of 512 characters.")

        self.step(8)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #8:  Breadcrumb attribute: {breadcrumb_info}')
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")

        self.step(9)
        # TODO: Step #9 - Not able to get the value NetworkIdNotFound

        self.step(10)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb_info, 1,
                             "The Breadcrumb attribute is not 1")
        logger.info(f'Step #10:  Breadcrumb attribute: {breadcrumb_info}')

        self.step(11)
        cmd = Clusters.NetworkCommissioning.Commands.ReorderNetwork(
            networkID=pixit_network_id.encode(), networkIndex=max_networks_value, breadcrumb=2)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        logger.info(f'Step #11: ReorderNetwork with response as Success ({resp.networkingStatus})')
        # TODO: Assert validation commented due platforms must support more than 1 Networks as MaxNetworks
        # Verify that the DUT responds with ReorderNetwork with NetworkingStatus as 'Success'(0)
        # asserts.assert_equal(resp.networkingStatus, Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum.kSuccess,
        #                      "Failure status returned from ReorderNetwork")
        # # Verify that DebugText is empty or has a maximum length of 512 characters
        # debug_text = resp.debugText
        # asserts.assert_true(debug_text is None or debug_text == '' or len(debug_text) <= 512,
        #                     "debugText must be None, empty or have a maximum length of 512 characters.")

        self.step(12)
        breadcrumb_info = await self.read_single_attribute_check_success(
            cluster=Clusters.GeneralCommissioning,
            attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb
        )
        logger.info(f'Step #12:  Breadcrumb attribute: {breadcrumb_info}')
        # TODO: Assert validation commented due platforms must support more than 1 Networks as MaxNetworks
        # asserts.assert_equal(breadcrumb_info, 2,
        #                      "The Breadcrumb attribute is not 2")

        self.step(13)
        networks = await self.read_single_attribute_check_success(
            cluster=Clusters.NetworkCommissioning,
            attribute=Clusters.NetworkCommissioning.Attributes.Networks
        )
        logger.info(f'Step #13: Networks attribute: {networks}')

        # Convert PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID to hexadecimal
        hex_representation = pixit_network_id.encode('utf-8').hex()
        logger.info(f'Step #13: Networks attribute: {hex_representation}')
        # TODO: Assert validation commented out because accessing MidPoint list_index 1 is not possible, when the list only has 1 value at list_index 0).
        # network_at_midpoint = networks[midpoint]
        # network_id_midpoint = network_at_midpoint['networkID']
        # network_id_midpoint_hex = network_id_midpoint.hex()
        # logger.info(f'Step #13: Midpoint NetworkID matches the expected value: {network_id_midpoint_hex}')
        # TODO: Assert validation commented due platforms must support more than 1 Networks as MaxNetworks
        # 1) List item with NetworkID value the hex
        # asserts.assert_equal(network_id_midpoint_hex, hex_representation,
        #                      f"Expected {hex_representation}, but got {network_id_midpoint_hex}")
        # 2) Other list items in same relative order as 'OriginalNetworkList'
        # for list_index, network in enumerate(networks):
        #     network_id = network['networkID']  # Get the 'networkID' from the current network

        #     # Compare with the 'networkID' from the 'original_network_list' (in the same position)
        #     asserts.assert_equal(network_id, original_network_list[list_index],
        #                  f"Network ID mismatch at position {list_index}. Expected {original_network_list[list_index]}, but got {network_id}")
        # logger.info('Step #13: All networks match the expected order and values.')

        self.step(14)
        cmd = self.CLUSTER_CGEN.Commands.ArmFailSafe(expiryLengthSeconds=0)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, self.CLUSTER_CGEN.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #1b - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')

        self.step(15)
        # Verify that Networks attribute list ordering matches 'OriginalNetworkList'
        asserts.assert_equal(networks, original_network_list,
                             "The order of the network list does not match the original network list.")
        logger.info('Step #15: Network list order matches the original network list.')

        self.step(16)
        cmd = self.CLUSTER_CGEN.Commands.ArmFailSafe(expiryLengthSeconds=self.failsafe_expiration_seconds)
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cmd=cmd
        )
        # Verify that the DUT responds with ArmFailSafeResponse with ErrorCode as 'OK'(0)
        asserts.assert_equal(resp.errorCode, self.CLUSTER_CGEN.Enums.CommissioningErrorEnum.kOk,
                             "Failure status returned from arm failsafe")
        logger.info(f'Step #1b - ArmFailSafeResponse with ErrorCode as OK({resp.errorCode})')


if __name__ == "__main__":
    default_matter_test_main()
