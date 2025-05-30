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
import random
import string

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_asserts import (assert_int_in_range, assert_string_length, assert_valid_uint8, assert_valid_uint16,
                                         assert_valid_uint64)
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_CNET_4_22(MatterBaseTest):
    """
    [TC-CNET-4.22] [Thread] Verification for ScanNetworks command [DUT-Server].
    Example Usage:
        To run the test case, use the following command:
        ```bash
        python src/python_testing/TC_CNET_4_22.py --commissioning-method ble-thread -discriminator <discriminator> -passcode <passcode> \
               --endpoint <endpoint_value> --thread-dataset-hex <dataset_value>
        ```
        Where `<endpoint_value>` should be replaced with the actual endpoint
        number for the Network Commissioning cluster on the DUT, and
        `<dataset_value>` should be replaced with the Thread Operational Dataset
        in hexadecimal format.
    """

    def steps_TC_CNET_4_22(self):
        return [
            TestStep('Precondition', 'TH is commissioned', is_commissioning=True),
            TestStep(1, 'TH sends ScanNetworks command to the DUT with the SSID field omitted and the Breadcrumb field set to 1'),
            TestStep(2, 'TH reads Breadcrumb attribute from the General Commissioning Cluster'),
            TestStep(3, 'TH sends ScanNetworks command to the DUT with the SSID field set to null and the Breadcrumb field set to 2'),
            TestStep(4, 'TH reads Breadcrumb attribute from the General Commissioning Cluster'),
            TestStep(5, 'TH sends ScanNetworks command to the DUT with the SSID field set to a random string of ASCII characters with a size of between 1 and 31 characters and the Breadcrumb field set to 3'),
            TestStep(6, 'TH reads Breadcrumb attribute from the General Commissioning Cluster')
        ]

    def def_TC_CNET_4_22(self):
        return '[TC-CNET-4.22] [Thread] Verification for ScanNetworks command [DUT-Server]'

    def pics_TC_CNET_4_22(self):
        return ['CNET.S.F01']

    def scan_network_response_thread_scan_results(self, thread_interfaces: list[str]):

        # Verify Thread interfaces are not None and length equal to 0
        asserts.assert_true(thread_interfaces is not None, "There are not any Thread interfaces available")
        asserts.assert_true(len(thread_interfaces) > 0, "Expected to find at least one Thread network")

        logging.info(f"Threads: {thread_interfaces}")

        # Each element in the ThreadScanResults list will have the following fields:
        for thread_interface in thread_interfaces:

            logging.info(f"Thread interface: {thread_interface}")

            # PanId with a range of 0 to 65534 (2**16-2)
            assert_int_in_range(thread_interface.panId, 0, 65534, "PanId")

            # ExtendedPanId
            assert_valid_uint64(thread_interface.extendedPanId, "Extended PanId")

            # NetworkName is a string with a size of 1 to 16 bytes
            assert_string_length(thread_interface.networkName, "NetworkName", 1, 16)

            # Channel is of type uint16 with a range 0 to 65535 (2**16-1)
            assert_valid_uint16(thread_interface.channel, "Channel")

            # Version is a uint8
            assert_valid_uint8(thread_interface.version, "Version")

            # ExtendedAddress is a hwaddr with a size of 8 bytes
            expected_len_bytes_extended_address = 8
            asserts.assert_equal(len(thread_interface.extendedAddress), expected_len_bytes_extended_address,
                                 f"The hwaddr value is {len(thread_interface.extendedAddress)} bytes long instead of {expected_len_bytes_extended_address}")

            asserts.assert_true(isinstance(thread_interface.extendedAddress, bytes),
                                "ExtendedAddress is not a hwadr instance")

            # RSSI is an of type int8 with a range of -120 to 0
            assert_int_in_range(thread_interface.rssi, -120, 0, "RSSI")

            # LQI is a uint8
            assert_valid_uint8(thread_interface.lqi, "LQI")

    async def read_and_check_breadcrumb(self, expected_breadcrumb):

        # TH reads Breadcrumb attribute from the General Commissioning Cluster
        breadcrumb = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.Breadcrumb)

        # Verify that the Breadcrumb attribute is set to 'expected_breadcrumb'
        asserts.assert_equal(breadcrumb, expected_breadcrumb,
                             f"Breadcrumb value is {breadcrumb} and it should be equal to {expected_breadcrumb}")

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_22(self):

        enum = Clusters.NetworkCommissioning.Enums.NetworkCommissioningStatusEnum

        # Commissioning is already done
        self.step('Precondition')

        # TH sends ScanNetworks command to the DUT with the SSID field omitted and the Breadcrumb field set to 1
        self.step(1)
        empty_string = ""
        empty_octstr = empty_string.encode('utf-8')
        cmd = Clusters.NetworkCommissioning.Commands.ScanNetworks(ssid=empty_octstr, breadcrumb=1)
        scan_network_response = await self.send_single_cmd(cmd=cmd)

        logging.info(f"Scan network response: {scan_network_response}")

        # Verify that DUT sends ScanNetworksResponse command to the TH with the following fields:
        # NetworkingStatus field value is Success
        asserts.assert_equal(scan_network_response.networkingStatus, enum.kSuccess,
                             "NetworkingStatus is not Success")

        # DebugText is of type string with max length 512 or absent
        if scan_network_response.debugText:
            debug_text_len = len(scan_network_response.debugText)
            asserts.assert_less_equal(debug_text_len, 512, f"DebugText length {debug_text_len} was out of range")

        # Verify WiFiScanResults is None
        asserts.assert_true(scan_network_response.wiFiScanResults is None,
                            "WiFi network was found and it was not supposed to be found")

        self.scan_network_response_thread_scan_results(scan_network_response.threadScanResults)

        # TH reads Breadcrumb attribute from the General Commissioning Cluster
        self.step(2)
        self.read_and_check_breadcrumb(expected_breadcrumb=1)

        # TH sends ScanNetworks command to the DUT with the SSID field set to null and the Breadcrumb field set to 2
        self.step(3)
        cmd = Clusters.NetworkCommissioning.Commands.ScanNetworks(ssid=NullValue, breadcrumb=2)
        scan_network_response = await self.send_single_cmd(cmd=cmd)

        logging.info(f"Scan network response: {scan_network_response}")

        # Verify that DUT sends ScanNetworksResponse command to the TH with the following fields:
        # NetworkingStatus field value is Success
        asserts.assert_equal(scan_network_response.networkingStatus, enum.kSuccess,
                             "NetworkingStatus is not Success")

        # DebugText is of type string with max length 512 or absent
        if scan_network_response.debugText:
            debug_text_len = len(scan_network_response.debugText)
            asserts.assert_less_equal(debug_text_len, 512, f"DebugText length {debug_text_len} was out of range")

        # Verify WiFiScanResults is None
        asserts.assert_true(scan_network_response.wiFiScanResults is None,
                            "WiFi network was found and it was not supposed to be found")

        self.scan_network_response_thread_scan_results(scan_network_response.threadScanResults)

        # TH reads Breadcrumb attribute from the General Commissioning Cluster
        self.step(4)
        self.read_and_check_breadcrumb(expected_breadcrumb=2)

        # TH sends ScanNetworks command to the DUT with the SSID field set to a random string of ASCII characters with a size of between 1 and 31 characters and the Breadcrumb field set to 3
        self.step(5)
        random_ASCII = ''.join(random.choices(string.ascii_letters + string.digits +
                               string.punctuation, k=random.randint(1, 31))).encode('utf-8')
        cmd = Clusters.NetworkCommissioning.Commands.ScanNetworks(ssid=random_ASCII, breadcrumb=3)
        scan_network_response = await self.send_single_cmd(cmd=cmd)

        logging.info(f"Scan network response: {scan_network_response}")

        # Verify that DUT sends ScanNetworksResponse command to the TH with the following fields:
        # NetworkingStatus field value is Success
        asserts.assert_equal(scan_network_response.networkingStatus, enum.kSuccess,
                             "NetworkingStatus is not Success")

        # DebugText is of type string with max length 512 or absent
        if scan_network_response.debugText:
            debug_text_len = len(scan_network_response.debugText)
            asserts.assert_less_equal(debug_text_len, 512, f"DebugText length {debug_text_len} was out of range")

        # Verify WiFiScanResults is None
        asserts.assert_true(scan_network_response.wiFiScanResults is None,
                            "WiFi network was found and it was not supposed to be found")

        self.scan_network_response_thread_scan_results(scan_network_response.threadScanResults)

        # TH reads Breadcrumb attribute from the General Commissioning Cluster
        self.step(6)
        self.read_and_check_breadcrumb(expected_breadcrumb=3)


if __name__ == "__main__":
    default_matter_test_main()
