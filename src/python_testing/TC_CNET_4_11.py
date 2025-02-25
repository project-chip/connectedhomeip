#
#    Copyright (c) 2024 Project CHIP Authors
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
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --int-arg: PIXIT.CNET.ENDPOINT_THREAD:0
#       --commissioning-method ble-wifi
#       --int-arg PIXIT.CNET.ENDPOINT_WIFI:0
#       --wifi-ssid 47524C50726976617465
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging
from typing import Optional

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches, run_if_endpoint_matches, has_feature
from mobly import asserts

from controller.python.chip import ChipDeviceCtrl

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class TC_CNET_4_11(MatterBaseTest):
    def steps_TC_CNET_4_11(self):
        return [
            TestStep("precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            TestStep(2, "TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'"),
            TestStep(3, "TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'Userwifi_netidx'"),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"),
            TestStep(5, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1"),
            TestStep(6, "TH reads Networks attribute from the DUT"),
            # TestStep(7, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2"),
            # TestStep(8, "TH changes its WiFi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            # TestStep(9, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network"),
            # TestStep(10, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT"),
            # TestStep(11, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0."),
            # TestStep(12, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"),
            # TestStep(13, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network"),
            # TestStep(14, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900"),
            # TestStep(15, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"),
            # TestStep(16, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1"),
            # TestStep(17, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3"),
            # TestStep(18, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            # TestStep(19, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network"),
            # TestStep(20, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT"),
            # TestStep(21, "TH sends the CommissioningComplete command to the DUT"),
            # TestStep(22, "TH reads Networks attribute from the DUT"),
        ]

    def def_TC_CNET_4_11(self):
        return '[TC-CNET-4.11] [Wi-Fi] Verification for ConnectNetwork Command [DUT-Server]'

    # def pics_TC_CNET_4_11(self):
    #     return ['CNET.S']

    # @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    @async_test_body
    async def test_TC_CNET_4_11(self):

        asserts.assert_true('PIXIT.CNET.ENDPOINT_WIFI' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.ENDPOINT_WIFI must be included on the command line in "
                            "the --int-arg flag as PIXIT.CNET.ENDPOINT_WIFI:<endpoint>")
        endpoint = self.matter_test_config.global_test_params['PIXIT.CNET.ENDPOINT_WIFI']
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        nodeid = self.matter_test_config.controller_node_id

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID = bytes.fromhex("47524C50726976617465")
        PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID = bytes.fromhex("7A6967626565686F6D65")
        PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS = bytes.fromhex("70617373776f7264313233")

        # Commissioning is already done
        self.step("precondition")

        # Precondition: TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.FeatureMap)
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info('Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        logger.info("Arming the failsafe")
        req = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=0)
        res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
        logger.info(f"Received response: {res}")
        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_true(type_matches(res, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
                            "Unexpected value returned from ArmFailSafe")
        asserts.assert_equal(res.errorCode, 0, f"Expected Error code to be 0, but got: {res.errorCode}")
        asserts.assert_equal(res.debugText, "", f"Exected Debug text to be empty, but got: {res.debugText}")

        # TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'
        self.step(2)

        logger.info("Reading Networks")
        res = await commissioner.ReadAttribute(nodeid=nodeid, attributes=[(endpoint, attr.Networks)], returnClusterObject=True)
        logger.info(f"Received response: {res}")
        networkList = res[endpoint][cnet].networks
        numNetworks = len(networkList)
        asserts.assert_in(PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID, networkList,
                          f"NetworkID: {PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID} is not in NetworkList")

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        idx = -1
        for idx, network in enumerate(networkList):
            if network.networkId == PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID:
                userwifi_netidx = idx
                break
        asserts.assert_greater_equal(userwifi_netidx == 0, f"Expected idx to be greater or equal to 0, but got: {idx}")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)

        logger.info("Sending RemoveNetwork command")
        req = cnet.Commands.RemoveNetwork(networkID=PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID, breadcrumb=1)
        res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
        logger.info(f"Received response: {res}")
        # Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is success
        asserts.assert_is_instance(res, cnet.Commands.NetworkConfigResponse.response_type)
        asserts.assert_equal(res.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected kNetworkIDNotFound but got: {res.networkingStatus}")
        # Verify NetworkIndex matches previously saved 'userwifi_netidx'
        asserts.assert_equal(res.networkIndex == userwifi_netidx,
                             f"Expected NetworkIndex to be: {userwifi_netidx}, but got: {res.networkIndex}")

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network
        logger.info("Adding second wifi test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateWiFiNetwork(
            ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID.encode(), credentials=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS.encode(), breadcrumb=1)
        res = await commissioner.SendCommand(nodeid=self._nodeid, endpoint=endpoint, payload=req)
        logger.info(f"Received response: {res}")
        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_is_instance(res, cnet.Commands.NetworkConfigResponse.response_type)
        asserts.assert_equal(res.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected Success, but got: {res.networkingStatus}")
        asserts.assert_equal(res.networkIndex, 0, f"Expected NetworkIndex to be 0, but got: {res.networkIndex}")
        # 2. DebugText is of type string with max length 512 or empty
        asserts.assert_true(type(res.debugText), str, f"Expected debugText to be of type string, but got: {type(res.debugText)}")
        asserts.assert_less_equal(len(res.debugText), 512,
                                  f"Expected length of debugText to be less than or equal to 512, but got: {len(res.debugText)}")

        # TH reads Networks attribute from the DUT
        self.step(6)

        # Verify that the Networks attribute list has an entry with the following fields:
        logger.info("Reading Networks")
        res = await commissioner.ReadAttribute(nodeid=nodeid, attributes=[(endpoint, attr.Networks)], returnClusterObject=True)
        logger.info(f"Received response: {res}")
        networkList = res[endpoint][cnet].networks
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        isPresent, isConnected = False, False
        for network in networkList:
            if network.networkId == PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS:
                isPresent = True
                if network.connected:
                    isConnected = True
                break
        asserts.assert_true(isPresent, "Expected NetworkID to be in NetworkList but got False")
        # 2. Connected is of type bool and is FALSE
        asserts.assert_true(not isConnected, "Expected isConnected to be False, but got True")

        #
        # self.step(7)
        # breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb, endpoint=0)
        # asserts.assert_equal(breadcrumb, 2, "Incorrect breadcrumb value")

        # await scan_and_check(ssid_to_scan=known_ssid, breadcrumb=2, expect_results=True)

        # breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb, endpoint=0)
        # asserts.assert_equal(breadcrumb, 1, "Incorrect breadcrumb value")

        # logger.info(f"NumNetworks: {numNetworks}, Networks: {networkList}")
        # logger.info(f"NetworkID: {networks[0].networkID}, Connected: {networks[0].connected}")
        # asserts.assert_true(PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID ==
        #                     networks[0].networkID, f"Expected NetworkID to be: {PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID}, but got: {networks[0].networkID}")
        # asserts.assert_true(networks[0].connected, f"network {networks[0].networkID} IS NOT connected")

        # supported_wifi_bands = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.SupportedWiFiBands)
        # print(f"---------------- supported_wifi_bands: {supported_wifi_bands}")
        # networks = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.Networks)
        # print(f"---------------- networks: {networks}")
        # connected = [network for network in networks if network.connected is True]
        # asserts.assert_greater_equal(len(connected), 1, "Did not find any connected networks on a commissioned device")
        # known_ssid = connected[0].networkID

        # async def scan_and_check(ssid_to_scan: Optional[bytes], breadcrumb: int, expect_results: bool = True):
        #     all_security = 0
        #     for security_bitmask in cnet.Bitmaps.WiFiSecurityBitmap:
        #         all_security |= security_bitmask

        #     ssid = ssid_to_scan if ssid_to_scan is not None else NullValue
        #     print(f"---------------- ssid: {ssid}")
        #     cmd = cnet.Commands.ScanNetworks(ssid=ssid, breadcrumb=breadcrumb)
        #     scan_results = await self.send_single_cmd(cmd=cmd)
        #     print(f"---------------- scan_results: {scan_results}")
        #     asserts.assert_true(type_matches(scan_results, cnet.Commands.ScanNetworksResponse),
        #                         "Unexpected value returned from scan network")
        #     logging.info(f"Scan results: {scan_results}")

        #     if scan_results.debugText:
        #         debug_text_len = len(scan_results.debug_text)
        #         asserts.assert_less_equal(debug_text_len, 512, f"DebugText length {debug_text_len} was out of range")

        #     if expect_results:
        #         asserts.assert_equal(scan_results.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
        #                              f"ScanNetworks was expected to have succeeded, got {scan_results.networkingStatus} instead")
        #         asserts.assert_greater_equal(len(scan_results.wiFiScanResults), 1, "No responses returned from ScanNetwork command")
        #     else:
        #         asserts.assert_equal(scan_results.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound,
        #                              f"ScanNetworks was expected to received NetworkNotFound(5), got {scan_results.networkingStatus} instead")
        #         return

        #     print(f"---------------- scan_results.wiFiScanResults: {scan_results.wiFiScanResults}")
        #     for network in scan_results.wiFiScanResults:
        #         print(f"---------------- network: {network}")
        #         asserts.assert_true((network.security & ~all_security) == 0, "Unexpected bitmap in the security field")
        #         asserts.assert_less_equal(len(network.ssid), 32, f"Returned SSID {network.ssid} is too long")
        #         if ssid_to_scan is not None:
        #             print(f"---------------- ssid_to_scan: {ssid_to_scan}")
        #             asserts.assert_equal(network.ssid, ssid_to_scan, "Unexpected SSID returned in directed scan")
        #         asserts.assert_true(type_matches(network.bssid, bytes), "Incorrect type for BSSID")
        #         asserts.assert_equal(len(network.bssid), 6, "Unexpected length of BSSID")
        #         # TODO: this is inherited from the old test plan, but we should match the channel to the supported band. This range is unreasonably large.
        #         asserts.assert_less_equal(network.channel, 65535, "Unexpected channel value")
        #         if network.wiFiBand:
        #             asserts.assert_true(network.wiFiBand in supported_wifi_bands,
        #                                 "Listed wiFiBand is not in supported_wifi_bands")
        #         if network.rssi:
        #             asserts.assert_greater_equal(network.rssi, -120, "RSSI out of range")
        #             asserts.assert_less_equal(network.rssi, 0, "RSSI out of range")

        # self.step(8)
        # random_ssid = ''.join(random.choice(string.ascii_letters) for _ in range(31)).encode("utf-8")
        # await scan_and_check(ssid_to_scan=random_ssid, breadcrumb=2, expect_results=False)

        # self.step(9)
        # self.step(10)
        # self.step(11)
        # self.step(12)
        # self.step(13)
        # self.step(14)
        # self.step(15)
        # self.step(16)
        # self.step(17)
        # self.step(18)
        # self.step(19)
        # self.step(20)
        # self.step(21)
        # self.step(22)


if __name__ == "__main__":
    default_matter_test_main()
