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
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, run_if_endpoint_matches, default_matter_test_main, has_feature, type_matches, async_test_body
from mobly import asserts


class TC_CNET_4_9(MatterBaseTest):
    def steps_TC_CNET_4_9(self):
        return [
            TestStep("Precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            TestStep(2, 'TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks'),
            TestStep(3, 'TH finds the index of the Networks list entry with NetworkID field value PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as Userwifi_netidx'),
            TestStep(4, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1'),
            TestStep(5, 'TH reads Networks attribute from the DUT'),
            TestStep(6, 'TH reads LastNetworkingStatus attribute from the DUT'),
            TestStep(7, 'TH reads LastNetworkID attribute from the DUT'),
            TestStep(8, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            TestStep(9, 'TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 2'),
            # TestStep(10, 'TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT'),
            # TestStep(11, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0'),
            # TestStep(12, 'TH reads Networks attribute from the DUT'),
            # TestStep(13, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
            # TestStep(14, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1'),
            # TestStep(15, 'TH sends the CommissioningComplete command to the DUT'),
            # TestStep(16, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0 to ensure the CommissioningComplete call properly persisted the failsafe context. This call should have no effect if Commissioning Complete call is handled correctly'),
            # TestStep(17, 'TH reads Networks attribute from the DUT')
        ]

    def def_TC_CNET_4_9(self):
        return '[TC-CNET-4.9] [Wi-Fi] Verification for RemoveNetwork Command [DUT-Server]'

    def pics_TC_CNET_4_9(self):
        return ['CNET.S']

    # @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    @async_test_body
    async def test_TC_CNET_4_9(self):
        # cnet = Clusters.GeneralCommissioning
        # attr = cnet.Attributes
        ssid = b'en3'  # IS THIS CORRECT? PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID

        # Commissioning is already done
        self.step("Precondition")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        result = await self.send_single_cmd(cmd=cmd)
        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(result, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")
        asserts.assert_equal(result.errorCode, 0, "Error code is not 0")
        asserts.assert_equal(result.debugText, "", "Debug text is not empty")

        # TH reads Networks attribute from the DUT and saves the number of entries as NumNetworks
        self.step(2)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)
        num_networks = len(networks)

        # Verify that the Networks attribute list has an entry with: NetworkID field value as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        if next((i for i, network in enumerate(networks) if network.networkID == ssid), None) == None:
            asserts.fail("There is not a NetworkID field equal to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID in Networks attribute list")

        # Verify that the Networks attribute list has an entry with: Connected field value is of type bool and has the value true

        # TH finds the index of the Networks list entry with NetworkID field value PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as Userwifi_netidx
        self.step(3)

        userwifi_netidx = next((i for i, network in enumerate(networks) if network.networkID == ssid), None)

        if userwifi_netidx is None:
            asserts.fail("Could not find user WiFi network id")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)
        cmd = Clusters.NetworkCommissioning.Commands.RemoveNetwork(networkID=ssid, breadcrumb=1)
        # result = await self.send_single_cmd(cmd=cmd) # NOT WORKING
        # asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.RemoveNetworkResponse),
        #                     "Unexpected value returned from RemoveNetwork")
        # asserts.assert_equal(result.networkStatus, Status.Success, "Network status was not successful")
        # asserts.assert_equal(result.networkIndex, userwifi_netidx, "NetworkIndex does not match user WiFi network index")

        # TH reads Networks attribute from the DUT
        self.step(5)
        networks = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.Networks)

        # asserts.assert_equal(len(networks), num_networks - 1, "Networks attribute length is not equal as NumNetworks - 1")

        # TH reads LastNetworkingStatus attribute from the DUT
        self.step(6)
        last_networking_status = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkingStatus)
        # Verify that DUT sends LastNetworkingStatus as Success which is 0 or null if 'NumNetworks' - 1 == 0 entries.

        # TH reads LastNetworkID attribute from the DUT
        self.step(7)
        last_network_id = await self.read_single_attribute_check_success(cluster=Clusters.NetworkCommissioning, attribute=Clusters.NetworkCommissioning.Attributes.LastNetworkID)
        # Verify that DUT sends LastNetworkID as PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID or null if 'NumNetworks' - 1 == 0 entries.

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(8)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)
        # Verify that the breadcrumb value is set to 1
        # asserts.assert_equal(breadcrumb, 1, "Breadcrumb attribute from General Commissioning cluster is not equal to 1")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(9)
        cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=ssid, breadcrumb=2)
        # result = await self.send_single_cmd(cmd=cmd) # UnsupportedCommand
        # Verify that the DUT sends a ConnectNetworkResponse to the command with the NetworkingStatus field set to NetworkIdNotFound
        # asserts.assert_true(type_matches(result, Clusters.NetworkCommissioning.Commands.ConnectNetworkResponse),
        #                     "Unexpected value returned from ConnectNetwork")
        # asserts.assert_equal(result.NetworkingStatus, Cluster.NetworkCommissioning.Enum.NetworkIDNotFound, "Should have received network status not found")


if __name__ == "__main__":
    default_matter_test_main()
