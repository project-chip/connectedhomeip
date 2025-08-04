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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts

from python_testing.matter_testing_infrastructure.chip.testing.wifi import TIMEOUT, MAX_RETRIES, WIFI_WAIT_SECONDS, TIMED_REQUEST_TIMEOUT_MS, change_networks, connect_host_wifi


logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

cgen = Clusters.GeneralCommissioning
cnet = Clusters.NetworkCommissioning
attr = cnet.Attributes


class TC_CNET_4_11(MatterBaseTest):

    # Overrides default_timeout: Test includes several long waits, adjust timeout to accommodate.
    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    async def find_network_and_assert(self, networks, ssid, should_be_connected=True):
        asserts.assert_is_not_none(networks, "Could not read networks.")
        for idx, network in enumerate(networks):
            if network.networkID == ssid.encode():
                connection_state = "connected" if network.connected else "not connected"
                asserts.assert_equal(network.connected, should_be_connected, f"Wifi network {ssid} is {connection_state}.")
                return idx
        asserts.fail(f"Wifi network not found for SSID: {ssid}")

    async def verify_operational_network(self, ssid):
        networks = None
        retry = 1
        while retry <= MAX_RETRIES:
            logger.info(f" --- verify_operational_network: Trying to verify operational network: {retry}/{MAX_RETRIES}")
            try:
                networks = await asyncio.wait_for(
                    self.read_single_attribute_check_success(
                        cluster=cnet,
                        attribute=cnet.Attributes.Networks
                    ),
                    timeout=TIMEOUT
                )
            except Exception as e:
                logger.error(f" --- verify_operational_network: Exception reading networks: {e}")
                # Let's wait a couple of seconds to change networks
                await asyncio.sleep(WIFI_WAIT_SECONDS)
            finally:
                if networks and len(networks) > 0 and networks[0].connected:
                    logger.info(f" --- verify_operational_network: networks: {networks}")
                    break
                else:
                    retry += 1
        else:
            asserts.fail(f" --- verify_operational_network: Could not read networks after {MAX_RETRIES} retries.")

        userwifi_netidx = await self.find_network_and_assert(networks, ssid)
        if userwifi_netidx is not None:
            logger.info(f" --- verify_operational_network: DUT connected to SSID: {ssid}")

    def steps_TC_CNET_4_11(self):
        return [
            TestStep("precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(2, "TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'."),
            TestStep(3, "TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'Userwifi_netidx'",
                        "Verify that the Networks attribute list has an entry with the following fields:"
                        " 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"
                        " 2. Connected is of type bool and is TRUE"),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"
                        "Verify that DUT sends NetworkConfigResponse to command with the following fields:"
                        " 1. NetworkingStatus is Success"
                        " 2. NetworkIndex matches previously saved 'Userwifi_netidx'"),
            TestStep(5, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:"
                        " 1. NetworkingStatus is success which is '0'"
                        " 2. DebugText is of type string with max length 512 or empty"),
            TestStep(6, "TH reads Networks attribute from the DUT",
                        "Verify that the Networks attribute list has an entry with the following fields:"
                        "NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"
                        "Connected is of type bool and is FALSE"),
            TestStep(7, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2"
                        "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(8, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(9, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 2"),
            TestStep(10, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0."
                     "This forcibly disarms the fail-safe and is expected to cause the changes of configuration to NetworkCommissioning cluster done so far to be reverted.",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(11, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"),
            TestStep(12, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(14, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1",
                     "Verify that DUT sends NetworkConfigResponse to command with the following response fields:"
                     " 1. NetworkingStatus is success"
                     " 2. NetworkIndex is 'Userwifi_netidx'"),
            TestStep(15, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1",
                     "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:"
                     " 1. NetworkingStatus is success which is '0'"
                     " 2. DebugText is of type string with max length 512 or empty"),
            TestStep(16, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3"
                     "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(17, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(18, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 3"),
            TestStep(19, "TH sends the CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)"),
            TestStep(20, "TH reads Networks attribute from the DUT",
                     " Verify that the Networks attribute list has an entry with the following fields:"
                     " 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"
                     " 2. Connected is of type bool and is TRUE"),
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

        wifi_1st_ap_ssid = self.matter_test_config.wifi_ssid
        wifi_1st_ap_credentials = self.matter_test_config.wifi_passphrase
        wifi_2nd_ap_ssid = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"]
        wifi_2nd_ap_credentials = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS"]

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

        # TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters)
        connected_network_count = {}
        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        logger.info(f" --- Networks by endpoint: {networks_dict}")

        # Verify that there is a single connected network across ALL network commissioning clusters
        for ep in networks_dict:
            connected_network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
            logger.info(f" --- Connected networks count by endpoint: {connected_network_count}")
            asserts.assert_equal(sum(connected_network_count.values()), 1,
                                 "Verify that only one entry has connected status as TRUE across ALL endpoints")

        endpoint = self.get_endpoint()
        current_cluster_connected = connected_network_count[endpoint] == 1

        if not current_cluster_connected:
            logger.info("Current cluster is not connected, skipping all remaining test steps")
            self.skip_all_remaining_steps(1)
            return

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        # Successful command execution is implied if no exception is raised.

        # TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'
        self.step(2)

        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        num_networks = len(networks)
        logger.info(f" --- Step 2: num_networks: {num_networks}")

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        # Verify that the Networks attribute list has an entry with the following fields:
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        userwifi_netidx = await self.find_network_and_assert(networks, wifi_1st_ap_ssid)

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid.encode(),
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
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
        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid.encode(), credentials=wifi_2nd_ap_credentials.encode(), breadcrumb=1)
        response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
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
        await self.find_network_and_assert(networks, wifi_2nd_ap_ssid, False)

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(7)

        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        await asyncio.wait_for(
            change_networks(
                test=self,
                cluster=cnet,
                ssid=wifi_2nd_ap_ssid.encode(),
                password=wifi_2nd_ap_credentials.encode(),
                breadcrumb=2
            ),
            timeout=TIMEOUT
        )

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(8)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await self.verify_operational_network(wifi_2nd_ap_ssid)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(9)

        # Verify that the breadcrumb value is set to 2
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )
        logger.info(f" --- Step 9: Breadcrumb is: {breadcrumb}")
        asserts.assert_equal(breadcrumb, 2, f"Expected breadcrumb to be 2, but got: {breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0.
        # This forcibly disarms the fail-safe and is expected to cause the changes of
        # configuration to NetworkCommissioning cluster done so far to be reverted.
        self.step(10)

        response = await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_equal(
            response.errorCode,
            cgen.Enums.CommissioningErrorEnum.kOk,
            "ArmFailSafeResponse error code is not OK.",
        )

        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        self.step(11)

        await asyncio.wait_for(
            connect_host_wifi(wifi_1st_ap_ssid, wifi_1st_ap_credentials),
            timeout=TIMEOUT
        )
        # Let's wait a couple of seconds to change networks
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(12)

        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await self.verify_operational_network(wifi_1st_ap_ssid)

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(13)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        # Successful command execution is implied if no exception is raised.

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(14)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid.encode(),
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
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
        self.step(15)

        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid.encode(), credentials=wifi_2nd_ap_credentials.encode(), breadcrumb=1)
        response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3
        self.step(16)

        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        await change_networks(
            test=self,
            cluster=cnet,
            ssid=wifi_2nd_ap_ssid.encode(),
            password=wifi_2nd_ap_credentials.encode(),
            breadcrumb=3
        )

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(17)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await self.verify_operational_network(wifi_2nd_ap_ssid)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(18)

        breadcrumb = await asyncio.wait_for(
            self.read_single_attribute_check_success(
                cluster=cgen,
                attribute=cgen.Attributes.Breadcrumb
            ),
            timeout=TIMEOUT
        )
        logger.info(f" --- Step 18: Breadcrumb is: {breadcrumb}")
        # Verify that the breadcrumb value is set to 3
        asserts.assert_equal(breadcrumb, 3, f"Expected breadcrumb to be 3, but got: {breadcrumb}")

        # TH sends the CommissioningComplete command to the DUT
        self.step(19)

        # Disarm the failsafe
        cmd = cgen.Commands.CommissioningComplete()
        response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse), "Got wrong response type")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse to be 0, but got {response.errorCode}")

        # TH reads Networks attribute from the DUT
        self.step(20)

        # Verify that the Networks attribute list has an entry with the following fields:
        networks = await asyncio.wait_for(
            self.read_single_attribute_check_success(
                cluster=cnet,
                attribute=cnet.Attributes.Networks
            ),
            timeout=TIMEOUT
        )

        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        await self.find_network_and_assert(networks, wifi_2nd_ap_ssid)


if __name__ == "__main__":
    default_matter_test_main()
