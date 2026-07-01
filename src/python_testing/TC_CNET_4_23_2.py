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
# TC_CNET_4_23_2.py — continuation of TC-CNET-4.23: Network Commissioning on a second
# Wi-Fi profile over an operational (CASE) session. Maps to test plan steps 17–22. Intended to follow TC_CNET_4_23; with default
# harness behaviour the DUT is commissioned before this test runs when
# commissioning_method is configured.
#

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

TIMED_REQUEST_TIMEOUT_MS = 5000
CONNECT_NETWORK_TIMEOUT_MS = 30000
NETWORK_STATUS_UPDATE_DELAY = 3
TIMEOUT = 180

cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning


class TC_CNET_4_23_2(MatterBaseTest):

    async def _validate_network_config_response(
        self,
        response: cnet.Commands.NetworkConfigResponse,
        expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None
    ) -> None:
        expected_status = expected_status or cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected response type from NetworkConfig command")
        asserts.assert_equal(response.networkingStatus, expected_status,
                             f"Expected NetworkingStatus {expected_status}, got {response.networkingStatus}")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _validate_connect_network_response(
        self,
        response: cnet.Commands.ConnectNetworkResponse,
        expect_success: bool = True
    ) -> None:
        asserts.assert_true(isinstance(response, cnet.Commands.ConnectNetworkResponse),
                            "Unexpected response type from ConnectNetwork command")
        if expect_success:
            asserts.assert_equal(response.networkingStatus,
                                 cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                 f"Expected success, got {response.networkingStatus}")
        else:
            asserts.assert_not_equal(response.networkingStatus,
                                     cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                                     "Expected failure, got success")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _read_last_networking_status(
        self,
        endpoint: int,
        expected_status: cnet.Enums.NetworkCommissioningStatusEnum = None,
        valid_statuses: list = None
    ) -> cnet.Enums.NetworkCommissioningStatusEnum:
        status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus,
        )
        logger.info(f" --- LastNetworkingStatus = {status}")
        if expected_status is not None:
            asserts.assert_equal(status, expected_status,
                                 f"Expected {expected_status}, got {status}")
        elif valid_statuses is not None:
            asserts.assert_in(status, valid_statuses,
                              f"Expected one of {valid_statuses}, got {status}")
        return status

    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_23_2(self):
        return [
            TestStep(1, "ArmFailSafe in operational to allow NC commands", ""),
            TestStep(2, "AddOrUpdateWiFiNetwork with WRONG password", ""),
            TestStep(3, "ConnectNetwork should FAIL", ""),
            TestStep(4, "check LastNetworkingStatus again", ""),
            TestStep(5, "AddOrUpdateWiFiNetwork with CORRECT password", ""),
            TestStep(6, "ConnectNetwork should SUCCEED", ""),
        ]

    def desc_TC_CNET_4_23_2(self):
        return ("[TC-CNET-4.23] [Wi-Fi] Second SSID commissioning (harness steps 1–6; "
                "test plan 17–22, CASE) [DUT-Server]")

    @async_test_body
    async def test_TC_CNET_4_23_2(self):
        endpoint = ROOT_ENDPOINT_ID

        second_ssid = self.user_params.get("second_ssid")
        second_password = self.user_params.get("second_password")
        asserts.assert_is_not_none(second_ssid, "user_params.second_ssid is required")
        asserts.assert_is_not_none(second_password, "user_params.second_password is required")

        correct_ssid_case = second_ssid.encode("utf-8")
        correct_password_case = second_password.encode("utf-8")
        incorrect_password = b"IncorrectPassword123"

        feature_map = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.FeatureMap,
        )
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info(" --- Device does not support WiFi on endpoint 0, skipping remaining steps")
            self.mark_all_remaining_steps_skipped(1)
            return

        self.step(1)
        logger.info(" --- CASE phase: arming failsafe for operational network commands...")
        afs = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=120, breadcrumb=100),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_true(isinstance(afs, cgen.Commands.ArmFailSafeResponse),
                            "Expected ArmFailSafeResponse in CASE phase")
        asserts.assert_equal(afs.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"ArmFailSafe (arm) failed with errorCode={afs.errorCode}")

        self.step(2)
        logger.info(" --- CASE phase: AddOrUpdateWiFiNetwork with WRONG password")
        resp = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid_case,
                credentials=incorrect_password,
                breadcrumb=101
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(resp)

        self.step(3)
        logger.info(" --- CASE phase: ConnectNetwork with WRONG password (expect fail)")
        resp = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=correct_ssid_case, breadcrumb=102),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
        )
        await self._validate_connect_network_response(resp, expect_success=False)

        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        self.step(4)
        await self._read_last_networking_status(
            endpoint,
            valid_statuses=[
                cnet.Enums.NetworkCommissioningStatusEnum.kAuthFailure,
                cnet.Enums.NetworkCommissioningStatusEnum.kOtherConnectionFailure
            ]
        )

        self.step(5)
        logger.info(" --- CASE phase: AddOrUpdateWiFiNetwork with CORRECT password")
        resp = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateWiFiNetwork(
                ssid=correct_ssid_case,
                credentials=correct_password_case,
                breadcrumb=103
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        await self._validate_network_config_response(resp)

        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        self.step(6)
        logger.info(" --- CASE phase: ConnectNetwork with CORRECT password (expect success)")
        resp = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.ConnectNetwork(networkID=correct_ssid_case, breadcrumb=104),
            timedRequestTimeoutMs=CONNECT_NETWORK_TIMEOUT_MS
        )
        await self._validate_connect_network_response(resp, expect_success=True)

        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        await self._read_last_networking_status(
            endpoint,
            expected_status=cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        )

        logger.info(" --- CASE phase: disarming failsafe")
        afs_disarm = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=105),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        asserts.assert_true(isinstance(afs_disarm, cgen.Commands.ArmFailSafeResponse),
                            "Expected ArmFailSafeResponse when disarming fail-safe")
        asserts.assert_equal(afs_disarm.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"ArmFailSafe (disarm) failed with errorCode={afs_disarm.errorCode}")


if __name__ == "__main__":
    default_matter_test_main()
