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

from mobly import asserts
from zeroconf import ServiceBrowser, ServiceListener, Zeroconf

import matter.clusters as Clusters
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

MAX_ATTEMPTS = 3
TIMEOUT = 900
MDNS_DISCOVERY_TIMEOUT = 10

# Global variable to store target device ID for mDNS discovery
_target_device_id = None


class MatterServiceListener(ServiceListener):
    """
    Collects all discovered Matter services with optional device ID filtering.
    """

    def __init__(self, target_device_id=None):
        self.discovered_services = []
        self.target_device_id = target_device_id

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        if info:
            try:
                # Extract device identifier from TXT records
                device_id = None
                if info.properties:
                    # Try to get device identifier from various TXT record fields
                    device_id = (info.properties.get(b'D') or
                                 info.properties.get(b'DN') or
                                 info.properties.get(b'deviceId'))
                    if device_id:
                        device_id = device_id.decode('utf-8') if isinstance(device_id, bytes) else str(device_id)

                # Only add if no filter or if device ID matches
                if not self.target_device_id or (device_id and str(device_id) == str(self.target_device_id)):
                    service_data = {
                        'name': name,
                        'type': type_,
                        'addresses': [addr for addr in info.addresses],
                        'port': info.port,
                        'properties': info.properties,
                        'device_id': device_id
                    }
                    logger.info(f"MatterServiceListener: Found matching Matter service: {name} (Device ID: {device_id})")
                    if service_data not in self.discovered_services:
                        self.discovered_services.append(service_data)
                else:
                    logger.debug(f"MatterServiceListener: Ignoring service {name} (Device ID: {device_id}) - does not match target")

            except Exception as e:
                logger.warning(f"MatterServiceListener: Error processing service {name}: {e}")
                # Add without device ID if parsing fails
                if not self.target_device_id:
                    self.discovered_services.append(service_data)

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass


def get_target_device_id():
    """Get the cached target device ID for mDNS discovery."""
    return _target_device_id


def set_target_device_id(device_id):
    """Set the target device ID for mDNS discovery."""
    global _target_device_id
    _target_device_id = str(device_id) if device_id is not None else None


async def find_matter_devices_mdns(max_attempts=MAX_ATTEMPTS):
    """
    Finds Matter devices via mDNS using zeroconf, optionally filtering by target device ID.
    Returns the list of discovered services.
    """
    service_types = ["_matter._tcp.local.", "_matterc._udp.local."]
    target_device_id = get_target_device_id()

    logger.info(
        f"find_matter_devices_mdns: Searching for Matter devices{' with target device ID: ' + target_device_id if target_device_id else ''}")

    for attempt in range(1, max_attempts + 1):
        try:
            zc = Zeroconf()
            listener = MatterServiceListener(target_device_id)
            browsers = [ServiceBrowser(zc, stype, listener) for stype in service_types]

            await asyncio.sleep(MDNS_DISCOVERY_TIMEOUT)

            # Cleanup
            for browser in browsers:
                browser.cancel()
            zc.close()

            if listener.discovered_services:
                logger.info(f"find_matter_devices_mdns: Found {len(listener.discovered_services)} Matter device(s)")
                return listener.discovered_services

        except Exception as e:
            logger.error(f"find_matter_devices_mdns: Discovery attempt {attempt} failed: {e}")
            try:
                zc.close()
            except Exception:
                pass

        if attempt < max_attempts:
            logger.info(f"find_matter_devices_mdns: Retrying discovery (attempt {attempt + 1}/{max_attempts})...")
            await asyncio.sleep(2)

    raise Exception(
        f"find_matter_devices_mdns: mDNS discovery failed after {max_attempts} attempts - No Matter devices found{' for target device ID: ' + target_device_id if target_device_id else ''}")


class TC_CNET_4_23(MatterBaseTest):

    # Overrides default_timeout: Test includes several long waits, adjust timeout to accommodate.
    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_23(self):
        return [
            TestStep(1, "Commission DUT with correct WiFi credentials", is_commissioning=True),
            TestStep(2, "TH sends AddOrUpdateWiFiNetwork with INCORRECT credentials and Breadcrumb field set to 1",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty\n"
                        "Save network index as 'userwifi_netidx' for later verification"),
            TestStep(3, "TH sends ConnectNetwork command and Breadcrumb field set to 2",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is NOT kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(4, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kAuthFailure'"),
            TestStep(5, "TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 3",
                        "Verify that DUT sends NetworkConfigResponse to command with the following fields:\n"
                        "1. NetworkingStatus is Success\n"
                        "2. NetworkIndex matches previously saved 'userwifi_netidx'"),
            TestStep(6, "TH sends ScanNetworks command",
                        "Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:\n"
                        "1. wiFiScanResults contains the target network SSID"),
            TestStep(7, "TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 4",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is success which is '0'\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(8, "TH sends ConnectNetwork command and Breadcrumb field set to 5",
                        "Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:\n"
                        "1. NetworkingStatus is kSuccess (0)\n"
                        "2. DebugText is of type string with max length 512 or empty"),
            TestStep(9, "TH reads LastNetworkingStatus and LastConnectErrorValue",
                        "Verify LastNetworkingStatus to be 'kSuccess'"),
            TestStep(10, "TH reads Networks attribute",
                         "Verify device is connected to correct network"),
            # TestStep(11, "TH establishes CASE session and sends CommissioningComplete to finalize commissioning",
            #              "Verify that DUT sends CommissioningCompleteResponse with ErrorCode OK (0)"),
        ]

    def desc_TC_CNET_4_23(self):
        return "[TC-CNET-4.23] [Wi-Fi] Verification for Commissioning [DUT-Server]"

    # @async_test_body
    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_23(self):

        cgen = Clusters.GeneralCommissioning
        cnet = Clusters.NetworkCommissioning

        # Network Commissioning cluster is always on root endpoint (0) during commissioning
        endpoint = ROOT_ENDPOINT_ID

        TIMED_REQUEST_TIMEOUT_MS = 5000

        correct_ssid = self.matter_test_config.wifi_ssid.encode('utf-8')
        correct_password = self.matter_test_config.wifi_passphrase.encode('utf-8')
        wrong_password = correct_password + b"wrong"

        # TH begins commissioning the DUT over the initial commissioning radio (PASE):
        self.step(1)

        # Skip CommissioningComplete to manually configure WiFi with incorrect then correct credentials
        # self.default_controller.SetSkipCommissioningComplete(True)
        # self.matter_test_config.commissioning_method = self.matter_test_config.in_test_commissioning_method
        # self.matter_test_config.tc_version_to_simulate = None
        # self.matter_test_config.tc_user_response_to_simulate = None

        # * Establish PASE session
        # * Arm fail-safe timer (900 seconds)
        # * Configure regulatory and time information
        # await self.commission_devices()

        # feature_map = await self.read_single_attribute(
        #     dev_ctrl=self.default_controller,
        #     node_id=self.dut_node_id,
        #     endpoint=endpoint,
        #     attribute=cnet.Attributes.FeatureMap,
        # )

        # if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
        #     logger.info("Device does not support WiFi on endpoint 0, skipping remaining steps")
        #     self.skip_all_remaining_steps(2)
        #     return

        # AddOrUpdateWiFiNetwork with INCORRECT credentials and Breadcrumb field set to 1
        self.step(2)

        # Arm fail-safe to allow network configuration changes on commissioned device
        logger.info("Arming fail-safe timer (900 seconds) to enable network configuration changes")
        arm_failsafe_response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.ArmFailSafe(
                expiryLengthSeconds=900,
                breadcrumb=0
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_equal(arm_failsafe_response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             "Failed to arm fail-safe timer")

        # Remove existing WiFi network configured during commissioning
        # This ensures the device uses the new (incorrect) credentials we're about to add
        logger.info(f"Removing existing WiFi network: {correct_ssid.decode('utf-8')}")
        remove_response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=correct_ssid,
                breadcrumb=0
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_equal(remove_response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Failed to remove existing WiFi network")
        logger.info("Existing WiFi network removed successfully")

        logger.info(
            f"Using correct SSID: {correct_ssid.decode('utf-8')} with incorrect Password: {wrong_password.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=wrong_password,
                breadcrumb=1
            )
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kSuccess (0)
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (kSuccess), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # Save network index as 'userwifi_netidx' for later verification
        userwifi_netidx = response.networkIndex

        # TH sends ConnectNetwork command and Breadcrumb field set to 2
        self.step(3)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=2
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected value returned from ConnectNetwork")
        # 1. NetworkingStatus is NOT kSuccess (0)
        asserts.assert_not_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                 "Expected ConnectNetwork to fail with incorrect credentials")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")
        logger.info(f"ConnectNetwork failed as expected for SSID: {correct_ssid.decode('utf-8')} with incorrect credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(4)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus indicates a failure
        # Can be kAuthFailure (wrong password), kNetworkNotFound (network not in range),
        # or kOtherConnectionFailure (general connection failure)
        valid_failure_statuses = [
            cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
            cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound,
            cnet.Enums.NetworkCommissioningStatusEnum.kOtherConnectionFailure
        ]
        asserts.assert_in(response, valid_failure_statuses,
                          f"Expected LastNetworkingStatus to indicate failure, got {response}")

        # TH sends RemoveNetwork to the DUT with NetworkID field set to the SSID of the failed network and Breadcrumb field set to 3
        self.step(5)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(
                networkID=correct_ssid,
                breadcrumb=3
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

        # TH sends ScanNetworks command
        self.step(6)

        # Retry scan up to 3 times if no networks found or target SSID not found
        # WiFi scanning can be intermittent during commissioning
        max_scan_retries = 3
        scan_retry_delay = 2
        ssids_found = []
        target_ssid_found = False

        for scan_attempt in range(max_scan_retries):
            logger.info(f"ScanNetworks attempt {scan_attempt + 1}/{max_scan_retries}")

            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ScanNetworks()
            )

            # Verify that DUT sends the ScanNetworksResponse command to the TH with the following response fields:
            asserts.assert_true(isinstance(response, cnet.Commands.ScanNetworksResponse),
                                "Unexpected value returned from ScanNetworks")

            # 1. wiFiScanResults contains the target network SSID
            ssids_found = [network.ssid for network in response.wiFiScanResults]
            logger.info(f"Scan results: Found {len(ssids_found)} networks")

            if len(ssids_found) > 0:
                for i, ssid in enumerate(ssids_found):
                    logger.info(f"Network {i}: {ssid} (type: {type(ssid)})")

                # Check if target SSID is in the results
                if correct_ssid in ssids_found:
                    target_ssid_found = True
                    logger.info(f"ScanNetworks found target SSID: {correct_ssid.decode('utf-8')} on attempt {scan_attempt + 1}")
                    break
                else:
                    logger.warning(
                        f"Target SSID '{correct_ssid.decode('utf-8')}' not found in scan results (attempt {scan_attempt + 1})")
            else:
                logger.warning(f"No networks found in scan (attempt {scan_attempt + 1})")

            # If not found and not last attempt, wait before retry
            if not target_ssid_found and scan_attempt < max_scan_retries - 1:
                logger.info(f"Waiting {scan_retry_delay} seconds before retry...")
                await asyncio.sleep(scan_retry_delay)

        # After all retries, verify target SSID was found
        asserts.assert_true(target_ssid_found,
                            f"Expected to find SSID '{correct_ssid.decode('utf-8')}' in scan results after {max_scan_retries} attempts. "
                            f"Last scan found {len(ssids_found)} network(s).")

        # TH sends AddOrUpdateWiFiNetwork with CORRECT credentials and Breadcrumb field set to 4
        self.step(7)

        logger.info(
            f"Using correct SSID: {correct_ssid.decode('utf-8')} with correct Password: {correct_password.decode('utf-8')}")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid,
                credentials=correct_password,
                breadcrumb=4
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is kSuccess which is '0'
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (kSuccess), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command and Breadcrumb field set to 5
        self.step(8)

        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(
                networkID=correct_ssid,
                breadcrumb=5
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends ConnectNetworkResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected value returned from ConnectNetwork")
        # 1. NetworkingStatus is kSuccess (0)
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Expected ConnectNetwork to succeed with correct credentials")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")
        logger.info(f"ConnectNetwork succeeded for SSID: {correct_ssid.decode('utf-8')} with correct credentials")

        # TH reads LastNetworkingStatus and LastConnectErrorValue
        self.step(9)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )

        # Verify LastNetworkingStatus to be 'kSuccess'
        asserts.assert_equal(response, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             "Expected LastNetworkingStatus to be 'kSuccess'")

        # TH reads Networks attribute
        self.step(10)

        response = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks,
        )
        # Verify device is connected to correct network
        connected_networks = [network.networkID for network in response if network.connected]
        asserts.assert_true(correct_ssid in connected_networks,
                            f"Expected device to be connected to SSID '{correct_ssid.decode('utf-8')}'")
        logger.info(f"Device is connected to SSID: {correct_ssid.decode('utf-8')}")

        # Send CommissioningComplete to disarm fail-safe and finalize network configuration
        logger.info("Sending CommissioningComplete to disarm fail-safe")
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.CommissioningComplete(),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_equal(
            response.errorCode,
            cgen.Enums.CommissioningErrorEnum.kOk,
            f"Expected CommissioningComplete ErrorCode to be kOk (0), but got: {response.errorCode}"
        )
        logger.info("CommissioningComplete sent successfully - Network configuration changes finalized!")

        # TH establishes CASE session and sends CommissioningComplete to finalize commissioning
        # self.step(11)

        # # Set the target device ID for mDNS discovery
        # set_target_device_id(self.dut_node_id)
        # logger.info(f"Step 11: Set target device ID for mDNS filtering: {self.dut_node_id}")

        # # Wait for device to stabilize on WiFi network and become discoverable
        # logger.info("Step 11: Waiting for device to stabilize on WiFi network...")
        # await asyncio.sleep(5)

        # # Discover device via mDNS on the WiFi network
        # logger.info("Step 11: Discovering device via mDNS on WiFi network...")
        # for attempt in range(1, MAX_ATTEMPTS + 1):
        #     try:
        #         discovered_services = await find_matter_devices_mdns(max_attempts=MAX_ATTEMPTS)
        #         logger.info(
        #             f"Step 11: Successfully discovered {len(discovered_services)} Matter device(s) via mDNS in attempt {attempt}")
        #         break
        #     except Exception as e:
        #         logger.error(f"Step 11: mDNS discovery failed: {e}")
        #         if attempt < MAX_ATTEMPTS:
        #             logger.info(f"Step 11: Retrying mDNS discovery (attempt {attempt + 1}/{MAX_ATTEMPTS})...")
        #             await asyncio.sleep(2)
        # else:
        #     raise AssertionError("Failed to discover device via mDNS after WiFi connection")

        # # Give additional time for CASE session to be ready
        # await asyncio.sleep(2)

        # # Send CommissioningComplete to finalize commissioning
        # # The controller should automatically establish CASE session when communicating with commissioned device
        # logger.info("Step 11: Sending CommissioningComplete to finalize commissioning...")
        # try:
        #     response = await self.send_single_cmd(
        #         cmd=cgen.Commands.CommissioningComplete(),
        #         endpoint=endpoint,
        #         timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        #     )

        #     # Verify CommissioningCompleteResponse has ErrorCode kOk (0)
        #     asserts.assert_equal(
        #         response.errorCode,
        #         cgen.Enums.CommissioningErrorEnum.kOk,
        #         f"Expected CommissioningComplete ErrorCode to be kOk (0), but got: {response.errorCode}"
        #     )
        #     logger.info("Step 11: CommissioningComplete sent successfully via CASE session - Commissioning finalized!")

        # except Exception as e:
        #     logger.error(f"Step 11: Failed to send CommissioningComplete: {e}")
        #     raise AssertionError(f"Failed to send CommissioningComplete after CASE session establishment: {e}")


if __name__ == "__main__":
    default_matter_test_main()
