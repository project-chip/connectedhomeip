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
#   run:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --passcode 20202021 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method ble-wifi
#       --qr-code MT:-24J042C00KA0648G00
#       --wifi-ssid 47524C50726976617465
#       --wifi-passphrase VH8F6wRSKs
#       --string-arg PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:"hex:7A6967626565686F6D65"
#       --string-arg PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS:"hex:70617373776f7264313233"
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, run_if_endpoint_matches, has_feature
from mobly import asserts

from controller.python.chip import ChipDeviceCtrl
from python_testing.matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_int32

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class TC_CNET_4_11(MatterBaseTest):

    def steps_TC_CNET_4_11(self):
        return [
            TestStep("precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(2, "TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'."),
            TestStep(3, "TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'Userwifi_netidx'"),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"),
            TestStep(5, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1"),
            TestStep(6, "TH reads Networks attribute from the DUT"),
            TestStep(7, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2"),
            # TestStep(8, "TH changes its WiFi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(9, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network"),
            TestStep(10, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT"),
            TestStep(11, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0."),
            # TestStep(12, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"),
            TestStep(13, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network"),
            TestStep(14, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(15, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"),
            TestStep(16, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1"),
            TestStep(17, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3"),
            # TestStep(18, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(19, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network"),
            TestStep(20, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT"),
            TestStep(21, "TH sends the CommissioningComplete command to the DUT"),
            TestStep(22, "TH reads Networks attribute from the DUT"),
        ]

    def desc_TC_CNET_4_11(self):
        return "[TC-CNET-4.11] [Wi-Fi] Verification for ConnectNetwork Command [DUT-Server]"

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_11(self):

        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:<hex:7A6967626565686F6D65>")
        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS:<hex:70617373776f7264313233>")

        pixit_wifi_1st_ap_ssid = self.matter_test_config.wifi_ssid
        pixit_wifi_2nd_ap_ssid = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"]
        pixit_wifi_2nd_ap_credentials = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS"]
        wifi_1st_ap_ssid = bytes.fromhex(pixit_wifi_1st_ap_ssid)
        wifi_2nd_ap_ssid = bytes.fromhex(pixit_wifi_2nd_ap_ssid)
        wifi_2nd_ap_credentials = bytes.fromhex(pixit_wifi_2nd_ap_credentials)

        cgen = Clusters.GeneralCommissioning
        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        # Commissioning is already done
        self.step("precondition")

        # Precondition: TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint
        feature_map = await self.read_single_attribute_check_success(
            cluster=cnet, attribute=attr.FeatureMap
        )
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info('Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900)
        )
        # Successful command execution is implied if no exception is raised.

        # TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'
        self.step(2)

        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        num_networks = len(networks)
        logger.info(f" --- Got NumNetworks: {num_networks}")

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        # TODO: move following (step 3) to step 2 in Spec
        # Verify that the Networks attribute list has an entry with the following fields:
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        userwifi_netidx = None
        for idx, network in enumerate(networks):
            if network.networkID == wifi_1st_ap_ssid:
                userwifi_netidx = idx
                logger.info(f" --- NetworkID 1st SSID: {network.networkID.hex()}")
                asserts.assert_true(network.connected, f"Wifi network {wifi_1st_ap_ssid.hex()} is not connected.")
                break
        asserts.assert_true(userwifi_netidx is not None, f"Wifi network not found for 1st SSID: {wifi_1st_ap_ssid.hex()}")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid,
                breadcrumb=1
            )
        )
        # Verify that DUT sends NetworkConfigResponse to command with the following fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            f"Expected response to be of type NetworkConfigResponse but got: {type(response)}")
        # 1. NetworkingStatus is Success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex matches previously saved 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network
        logger.info("--- Adding second wifi test network")
        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid, credentials=wifi_2nd_ap_credentials, breadcrumb=1)
        response = await self.send_single_cmd(cmd=cmd)
        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        asserts.assert_equal(type(response.debugText), str,
                             f"Expected debugText to be of type string, but got: {type(response.debugText)}")
        asserts.assert_less_equal(len(response.debugText), 512,
                                  f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH reads Networks attribute from the DUT
        self.step(6)

        # Verify that the Networks attribute list has an entry with the following fields:
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is FALSE
        userwifi_2nd_netidx = None
        for idx, network in enumerate(networks):
            if network.networkID == wifi_2nd_ap_ssid:
                logger.info(f" --- NetworkID 2nd SSID: {network.networkID.hex()}")
                userwifi_2nd_netidx = idx
                asserts.assert_false(network.connected, f"Wifi network {wifi_2nd_ap_ssid.hex()} is not connected.")
                break
        asserts.assert_true(userwifi_2nd_netidx is not None, f"Wifi network not found for 2nd SSID: {wifi_2nd_ap_ssid.hex()}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(7)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=wifi_2nd_ap_ssid,
                breadcrumb=2
            )
        )
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")

        # TODO: step 8 is manual step, need to remove from here and from test spec
        # TH changes its WiFi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # self.step(8)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(9)

        # Verify that the TH successfully connects to the DUT
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        for idx, network in enumerate(networks):
            if network.networkID == wifi_2nd_ap_ssid:
                asserts.assert_true(network.connected, f"Wifi network {wifi_2nd_ap_ssid.hex()} is not connected.")
                logger.info(f" --- Connected to 2nd SSID: {wifi_2nd_ap_ssid.hex()}")
                break

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(10)

        # Verify that the breadcrumb value is set to 2
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )
        asserts.assert_equal(breadcrumb, 2, f"Expected breadcrumb to be 2, but got: {breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0.
        # This forcibly disarms the fail-safe and is expected to cause the changes of
        # configuration to NetworkCommissioning cluster done so far to be reverted.
        self.step(11)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0)
        )
        # Successful command execution is implied if no exception is raised.

        # TODO: step 12 is manual step, need to remove from here and from test spec
        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # How does it change? is it automatic or need to use ConnectNetwork? if so, look at step 7
        # self.step(12)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(13)

        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        # Verify that the TH successfully connects to the DUT
        for idx, network in enumerate(networks):
            if network.networkID == wifi_1st_ap_ssid:
                asserts.assert_true(network.connected, f"Wifi network {wifi_1st_ap_ssid.hex()} is not connected.")
                logger.info(f" --- Connected to 1st SSID: {wifi_1st_ap_ssid.hex()}")
                break

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(14)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900)
        )
        # Successful command execution is implied if no exception is raised.

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(15)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid,
                breadcrumb=1
            )
        )
        # Verify that DUT sends NetworkConfigResponse to command with the following response fields:
        # 1. NetworkingStatus is success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex is 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(16)

        logger.info("--- Adding second wifi test network")
        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid, credentials=wifi_2nd_ap_credentials, breadcrumb=1)
        response = await self.send_single_cmd(cmd=cmd)
        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        asserts.assert_equal(type(response.debugText), str,
                             f"Expected debugText to be of type string, but got: {type(response.debugText)}")
        asserts.assert_less_equal(len(response.debugText), 512,
                                  f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3
        self.step(17)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.ConnectNetwork(
                networkID=wifi_2nd_ap_ssid,
                breadcrumb=3
            )
        )
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")

        # TODO: same as step 8, remove from here and from spec
        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # self.step(18)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(19)

        # Verify that the TH successfully connects to the DUT
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        for idx, network in enumerate(networks):
            if network.networkID == wifi_2nd_ap_ssid:
                asserts.assert_true(network.connected, f"Wifi network {wifi_2nd_ap_ssid.hex()} is not connected.")
                logger.info(f" --- Connected to 2nd SSID: {wifi_2nd_ap_ssid.hex()}")
                break

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(20)

        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )
        # Verify that the breadcrumb value is set to 3
        asserts.assert_equal(breadcrumb, 3, f"Expected breadcrumb to be 3, but got: {breadcrumb}")

        # TH sends the CommissioningComplete command to the DUT
        self.step(21)

        # Disarm the failsafe
        logger.info(" --- Disarming the failsafe")
        cmd = cgen.Commands.CommissioningComplete()
        response = await self.send_single_cmd(cmd=cmd)
        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse), "Got wrong response type")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse to be 0, but got {response.errorCode}")

        # TH reads Networks attribute from the DUT
        self.step(22)

        # Verify that the Networks attribute list has an entry with the following fields:
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        for idx, network in enumerate(networks):
            if network.networkID == wifi_2nd_ap_ssid:
                userwifi_2nd_netidx = idx
                asserts.assert_true(network.connected, f"Wifi network {wifi_2nd_ap_ssid.hex()} is not connected.")
                logger.info(f" --- Connected to 2nd SSID: {wifi_2nd_ap_ssid.hex()}")
                break
        asserts.assert_true(userwifi_2nd_netidx is not None, "Wifi network not found")


if __name__ == "__main__":
    default_matter_test_main()
