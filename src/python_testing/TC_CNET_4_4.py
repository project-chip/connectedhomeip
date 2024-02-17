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

import logging
import random
import string
from typing import Optional

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts


class TC_CNET_4_4(MatterBaseTest):
    def steps_TC_CNET_4_4(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH reads from the DUT the Network Commissioning Cluster FeatureMap. If the FeatureMap does not include the WI flag (bit 0), skip the remaining steps in this test case'),
                TestStep(2, 'TH reads from the DUT the SupportedWifiBands attribute and saves as supported_wifi_bands'),
                TestStep(3, 'TH reads from the DUT the Networks attribute.'),
                TestStep(4, 'TH sends ScanNetworks command to the DUT with the SSID field set to null and Breadcrumb field set to 1'),
                TestStep(5, 'TH reads from the DUT the Breadcrumb attribute from the General Commissioning Cluster'),
                TestStep(6, 'TH sends ScanNetworks Command to the DUT with SSID field set to known_ssid and Breadcrumb field set to 2'),
                TestStep(7, 'TH reads Breadcrumb attribute from the General Commissioning Cluster'),
                TestStep(8, 'TH sends ScanNetworks Command to the DUT with SSID field set to a string of 31 random alphabetical characters and Breadcrumb field set to 2')]

    def def_TC_CNET_4_4(self):
        return '[TC-CNET-4.4] [Wi-Fi] Verification for ScanNetworks command [DUT-Server]'

    def pics_TC_CNET_4_4(self):
        return ['CNET.S']

    @async_test_body
    async def test_TC_CNET_4_4(self):
        # Commissioning is already done
        self.step("precondition")

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        self.step(1)
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.FeatureMap)
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logging.info('Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(2)
            return

        self.step(2)
        supported_wifi_bands = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.SupportedWiFiBands)

        self.step(3)
        networks = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.Networks)
        connected = [network for network in networks if network.connected is True]
        asserts.assert_greater_equal(len(connected), 1, "Did not find any connected networks on a commissioned device")
        known_ssid = connected[0].networkID

        async def scan_and_check(ssid_to_scan: Optional[bytes], breadcrumb: int, expect_results: bool = True):
            all_security = 0
            for security_bitmask in cnet.Bitmaps.WiFiSecurityBitmap:
                all_security |= security_bitmask

            ssid = ssid_to_scan if ssid_to_scan is not None else NullValue
            cmd = cnet.Commands.ScanNetworks(ssid=ssid, breadcrumb=breadcrumb)
            scan_results = await self.send_single_cmd(cmd=cmd)
            asserts.assert_true(type_matches(scan_results, cnet.Commands.ScanNetworksResponse),
                                "Unexpected value returned from scan network")
            logging.info(f"Scan results: {scan_results}")

            if scan_results.debugText:
                debug_text_len = len(scan_results.debug_text)
                asserts.assert_less_equal(debug_text_len, 512, f"DebugText length {debug_text_len} was out of range")

            if expect_results:
                asserts.assert_equal(scan_results.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                     f"ScanNetworks was expected to have succeeded, got {scan_results.networkingStatus} instead")
                asserts.assert_greater_equal(len(scan_results.wiFiScanResults), 1, "No responses returned from ScanNetwork command")
            else:
                asserts.assert_equal(scan_results.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound,
                                     f"ScanNetworks was expected to received NetworkNotFound(5), got {scan_results.networkingStatus} instead")
                return

            for network in scan_results.wiFiScanResults:
                asserts.assert_true((network.security & ~all_security) == 0, "Unexpected bitmap in the security field")
                asserts.assert_less_equal(len(network.ssid), 32, f"Returned SSID {network.ssid} is too long")
                if ssid_to_scan is not None:
                    asserts.assert_equal(network.ssid, ssid_to_scan, "Unexpected SSID returned in directed scan")
                asserts.assert_true(type_matches(network.bssid, bytes), "Incorrect type for BSSID")
                asserts.assert_equal(len(network.bssid), 6, "Unexpected length of BSSID")
                # TODO: this is inherited from the old test plan, but we should match the channel to the supported band. This range is unreasonably large.
                asserts.assert_less_equal(network.channel, 65535, "Unexpected channel value")
                if network.wiFiBand:
                    asserts.assert_true(network.wiFiBand in supported_wifi_bands,
                                        "Listed wiFiBand is not in supported_wifi_bands")
                if network.rssi:
                    asserts.assert_greater_equal(network.rssi, -120, "RSSI out of range")
                    asserts.assert_less_equal(network.rssi, 0, "RSSI out of range")

        self.step(4)
        await scan_and_check(ssid_to_scan=None, breadcrumb=1, expect_results=True)

        self.step(5)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb, endpoint=0)
        asserts.assert_equal(breadcrumb, 1, "Incorrect breadcrumb value")

        self.step(6)
        await scan_and_check(ssid_to_scan=known_ssid, breadcrumb=2, expect_results=True)

        self.step(7)
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb, endpoint=0)
        asserts.assert_equal(breadcrumb, 2, "Incorrect breadcrumb value")

        self.step(8)
        random_ssid = ''.join(random.choice(string.ascii_letters) for _ in range(31)).encode("utf-8")
        await scan_and_check(ssid_to_scan=random_ssid, breadcrumb=2, expect_results=False)


if __name__ == "__main__":
    default_matter_test_main()
