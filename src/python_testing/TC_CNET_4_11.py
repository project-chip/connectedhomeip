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
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --commissioning-method ble-wifi
#       --int-arg PIXIT.CNET.ENDPOINT_WIFI:0
#       --wifi-ssid 47524C50726976617465
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches, run_if_endpoint_matches, has_feature
from mobly import asserts

from controller.python.chip import ChipDeviceCtrl
from python_testing.matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_int32

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


async def arm_failsafe(commissioner: ChipDeviceCtrl.ChipDeviceController,
                       cluster: Clusters.GeneralCommissioning,
                       nodeid,
                       endpoint,
                       expiryLengthSeconds,
                       breadcrumb,
                       expectedErrorCode=0) -> None:
    logger.info("Arming the failsafe")
    req = cluster.Commands.ArmFailSafe(expiryLengthSeconds=expiryLengthSeconds, breadcrumb=breadcrumb)
    res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
    logger.info(f"Received response: {res}")
    # Verify that DUT sends ArmFailSafeResponse command to the TH
    asserts.assert_true(type_matches(res, cluster.Commands.ArmFailSafeResponse),
                        "Unexpected value returned from ArmFailSafe")
    asserts.assert_equal(res.errorCode, expectedErrorCode,
                         f"Expected Error code to be {expectedErrorCode}, but got: {res.errorCode}")


async def read_network_list(commissioner: ChipDeviceCtrl.ChipDeviceController,
                            cluster: Clusters.NetworkCommissioning,
                            nodeid,
                            endpoint):
    logger.info("Reading Networks")
    res = await commissioner.ReadAttribute(nodeid=nodeid, attributes=[(endpoint, cluster.Attributes.Networks)], returnClusterObject=True)
    logger.info(f"Received response: {res}")
    return res[endpoint][cluster].networks


async def connect_to_network(commissioner: ChipDeviceCtrl.ChipDeviceController,
                             cluster: Clusters.NetworkCommissioning,
                             nodeid,
                             endpoint,
                             ssid,
                             breadcrumb):
    logger.info(f"Connect to {ssid}")
    req = cluster.Commands.ConnectNetwork(
        networkID=ssid.encode(), breadcrumb=breadcrumb)
    interactionTimeoutMs = commissioner.ComputeRoundTripTimeout(nodeid, upperLayerProcessingTimeoutMs=5000)
    res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req, interactionTimeoutMs=interactionTimeoutMs)
    logger.info(f"Received response: {res}")
    asserts.assert_equal(res.networkingStatus, cluster.Enums.NetworkCommissioningStatusEnum.kSuccess,
                         f"Expected NetworkingStatus to be success but got: {res.networkingStatus}")
    assert_valid_int32(res.errorValue, "ErrorValue")


async def add_second_network(commissioner: ChipDeviceCtrl.ChipDeviceController,
                             cluster: Clusters.NetworkCommissioning,
                             nodeid,
                             endpoint,
                             ssid,
                             credentials,
                             breadcrumb=1):
    logger.info("Adding second wifi test network")
    req = cluster.Commands.AddOrUpdateWiFiNetwork(
        ssid=ssid.encode(), credentials=credentials.encode(), breadcrumb=breadcrumb)
    res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
    logger.info(f"Received response: {res}")
    # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
    # 1. NetworkingStatus is success which is "0"
    asserts.assert_true(type_matches(res, cluster.Commands.NetworkConfigResponse),
                        "Unexpected value returned from AddOrUpdateWiFiNetwork")
    asserts.assert_equal(res.networkingStatus, cluster.Enums.NetworkCommissioningStatusEnum.kSuccess,
                         f"Expected 0 (Success), but got: {res.networkingStatus}")
    asserts.assert_equal(res.networkIndex, 0, f"Expected NetworkIndex to be 0, but got: {res.networkIndex}")
    # 2. DebugText is of type string with max length 512 or empty
    asserts.assert_equal(type(res.debugText), str, f"Expected debugText to be of type string, but got: {type(res.debugText)}")
    asserts.assert_less_equal(len(res.debugText), 512,
                              f"Expected length of debugText to be less than or equal to 512, but got: {len(res.debugText)}")


async def remove_network(commissioner: ChipDeviceCtrl.ChipDeviceController,
                         cluster: Clusters.NetworkCommissioning,
                         nodeid,
                         endpoint,
                         ssid,
                         breadcrumb,
                         userwifi_netidx):
    logger.info("Sending RemoveNetwork command")
    req = cluster.Commands.RemoveNetwork(networkID=ssid, breadcrumb=breadcrumb)
    res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
    logger.info(f"Received response: {res}")
    # Verify that DUT sends NetworkConfigResponse to command with the following fields: NetworkingStatus is success
    # asserts.assert_is_instance(res, cnet.Commands.NetworkConfigResponse.response_type)
    asserts.assert_true(type_matches(res, cluster.Commands.NetworkConfigResponse),
                        "Unexpected value returned from RemoveNetwork")
    asserts.assert_equal(res.networkingStatus, cluster.Enums.NetworkCommissioningStatusEnum.kSuccess,
                         f"Expected kNetworkIDNotFound but got: {res.networkingStatus}")
    # Verify NetworkIndex matches previously saved 'userwifi_netidx'
    asserts.assert_equal(res.networkIndex, userwifi_netidx,
                         f"Expected NetworkIndex to be: {userwifi_netidx}, but got: {res.networkIndex}")


def verify_ssid_connected(networkList, ssid, connected: bool = True):
    isPresent = False
    isConnected = False
    for network in networkList:
        if network.networkID == ssid:
            isPresent = True
            if network.connected:
                isConnected = True
            break
    asserts.assert_true(isPresent, f"NetworkID: {ssid} is not in NetworkList")
    asserts.assert_true(isConnected == connected, f"Expected connected to be {connected}, but got {isConnected}")
    return isConnected


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

    def def_TC_CNET_4_11(self):
        return '[TC-CNET-4.11] [Wi-Fi] Verification for ConnectNetwork Command [DUT-Server]'

    # @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    @async_test_body
    async def test_TC_CNET_4_11(self):

        asserts.assert_true('PIXIT.CNET.ENDPOINT_WIFI' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.ENDPOINT_WIFI must be included on the command line in "
                            "the --int-arg flag as PIXIT.CNET.ENDPOINT_WIFI:<endpoint>")
        endpoint = self.matter_test_config.global_test_params['PIXIT.CNET.ENDPOINT_WIFI']
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        nodeid = self.matter_test_config.controller_node_id

        cgen = Clusters.GeneralCommissioning
        cnet = Clusters.NetworkCommissioning

        PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID = bytes.fromhex("47524C50726976617465")
        PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID = bytes.fromhex("7A6967626565686F6D65")
        PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS = bytes.fromhex("70617373776f7264313233")

        # Commissioning is already done
        self.step("precondition")

        # Precondition: TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, attribute=cnet.Attributes.FeatureMap)
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info('Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        await arm_failsafe(commissioner=commissioner,
                           cluster=cgen,
                           nodeid=nodeid,
                           endpoint=endpoint,
                           expiryLengthSeconds=900,
                           breadcrumb=0)

        # TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'
        self.step(2)

        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        numNetworks = len(networkList)
        # Verify that the Networks attribute list has an entry with the following fields:
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID)

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        idx = -1
        for idx, network in enumerate(networkList):
            if network.networkID == PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID:
                userwifi_netidx = idx
                break
        asserts.assert_greater_equal(userwifi_netidx == 0, f"Expected idx to be greater or equal to 0, but got: {idx}")

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)

        await remove_network(commissioner=commissioner,
                             cluster=cnet,
                             nodeid=nodeid,
                             endpoint=endpoint,
                             ssid=PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID,
                             breadcrumb=1,
                             userwifi_netidx=userwifi_netidx)

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network
        await add_second_network(commissioner=commissioner,
                                 cluster=cnet,
                                 nodeid=nodeid,
                                 endpoint=endpoint,
                                 ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID,
                                 credentials=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS)

        # TH reads Networks attribute from the DUT
        self.step(6)

        # Verify that the Networks attribute list has an entry with the following fields:
        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is FALSE
        verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID, connected=False)

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(7)

        await connect_to_network(commissioner=commissioner,
                                 cluster=cnet,
                                 nodeid=nodeid,
                                 endpoint=endpoint,
                                 ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID,
                                 breadcrumb=2)

        # TODO: step 8 is manual step, need to remove from here and from test spec
        # TH changes its WiFi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # self.step(8)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(9)

        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        # Verify that the TH successfully connects to the DUT
        isConnected = verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID)
        if isConnected:
            logger.info("Device connected to a network.")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(10)

        # Verify that the breadcrumb value is set to 2
        res = await commissioner.ReadAttribute(nodeid=nodeid, attributes=[(endpoint, cgen.Attributes.Breadcrumb)], returnClusterObject=True)
        breadcrumb = res[0][cgen].breadcrumb
        asserts.assert_equal(breadcrumb, 2, f"Expected breadcrumb to be 2, but got: {breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0.
        # This forcibly disarms the fail-safe and is expected to cause the changes of
        # configuration to NetworkCommissioning cluster done so far to be reverted.
        self.step(11)

        await arm_failsafe(commissioner=commissioner,
                           cluster=cgen,
                           nodeid=nodeid,
                           endpoint=endpoint,
                           expiryLengthSeconds=0,
                           breadcrumb=0)

        # TODO: step 12 is manual step, need to remove from here and from test spec
        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # How does it change? is it automatic or need to use ConnectNetwork? if so, look at step 7
        # self.step(12)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(13)

        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        # Verify that the TH successfully connects to the DUT
        isConnected = verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID)
        if isConnected:
            logger.info("Device connected to a network.")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(14)

        await arm_failsafe(commissioner=commissioner,
                           cluster=cgen,
                           nodeid=nodeid,
                           endpoint=endpoint,
                           expiryLengthSeconds=900,
                           breadcrumb=0)

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(15)

        await remove_network(commissioner=commissioner,
                             cluster=cnet,
                             nodeid=nodeid,
                             endpoint=endpoint,
                             ssid=PIXIT_CNET_WIFI_1ST_ACCESSPOINT_SSID,
                             breadcrumb=1,
                             userwifi_netidx=userwifi_netidx)

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(16)

        await add_second_network(commissioner=commissioner,
                                 cluster=cnet,
                                 nodeid=nodeid,
                                 endpoint=endpoint,
                                 ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID,
                                 credentials=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_CREDENTIALS)

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3
        self.step(17)

        await connect_to_network(commissioner=commissioner,
                                 cluster=cnet,
                                 nodeid=nodeid,
                                 endpoint=endpoint,
                                 ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID,
                                 breadcrumb=3)

        # TODO: same as step 8, remove from here and from spec
        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # self.step(18)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(19)

        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        # Verify that the TH successfully connects to the DUT
        isConnected = verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID)
        if isConnected:
            logger.info("Device connected to a network.")

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(20)

        # Verify that the breadcrumb value is set to 3
        res = await commissioner.ReadAttribute(nodeid=nodeid, attributes=[(endpoint, cgen.Attributes.Breadcrumb)], returnClusterObject=True)
        breadcrumb = res[0][cgen].breadcrumb
        asserts.assert_equal(breadcrumb, 3, f"Expected breadcrumb to be 2, but got: {breadcrumb}")

        # TH sends the CommissioningComplete command to the DUT
        self.step(21)

        # Disarm the failsafe
        logger.info("Disarming the failsafe")
        req = cgen.Commands.CommissioningComplete()
        res = await commissioner.SendCommand(nodeid=nodeid, endpoint=endpoint, payload=req)
        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        logger.info(f"Received response: {res}")
        asserts.assert_equal(res.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse to be 0, but got {res.errorCode}")

        # TH reads Networks attribute from the DUT
        self.step(22)

        # Verify that the Networks attribute list has an entry with the following fields:
        networkList = await read_network_list(commissioner=commissioner, cluster=cnet, nodeid=nodeid, endpoint=endpoint)
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        verify_ssid_connected(networkList=networkList, ssid=PIXIT_CNET_WIFI_2ND_ACCESSPOINT_SSID)


if __name__ == "__main__":
    default_matter_test_main()
