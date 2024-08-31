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

# TODO: add to CI. See https://github.com/project-chip/connectedhomeip/issues/34676

import hashlib
import logging
import os
import queue
import secrets
import signal
import struct
import subprocess
import time
import uuid
from dataclasses import dataclass

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from ecdsa.curves import NIST256p
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from mobly import asserts
from TC_SC_3_6 import AttributeChangeAccumulator

# Length of `w0s` and `w1s` elements
WS_LENGTH = NIST256p.baselen + 8


def _generate_verifier(passcode: int, salt: bytes, iterations: int) -> bytes:
    ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', passcode), salt, iterations, WS_LENGTH * 2)
    w0 = int.from_bytes(ws[:WS_LENGTH], byteorder='big') % NIST256p.order
    w1 = int.from_bytes(ws[WS_LENGTH:], byteorder='big') % NIST256p.order
    L = NIST256p.generator * w1

    return w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')


@dataclass
class _SetupParamters:
    setup_qr_code: str
    manual_code: int
    discriminator: int
    passcode: int


class TC_MCORE_FS_1_5(MatterBaseTest):
    @async_test_body
    async def setup_class(self):
        super().setup_class()
        self._app_th_server_process = None
        self._th_server_kvs = None

    def teardown_class(self):
        if self._app_th_server_process is not None:
            logging.warning("Stopping app with SIGTERM")
            self._app_th_server_process.send_signal(signal.SIGTERM.value)
            self._app_th_server_process.wait()

        if self._th_server_kvs is not None:
            os.remove(self._th_server_kvs)
        super().teardown_class()

    async def _create_th_server(self, port):
        # These are default testing values
        setup_params = _SetupParamters(setup_qr_code="MT:-24J0AFN00KA0648G00",
                                       manual_code=34970112332, discriminator=3840, passcode=20202021)
        kvs = f'kvs_{str(uuid.uuid4())}'

        cmd = [self._th_server_app_path]
        cmd.extend(['--secured-device-port', str(port)])
        cmd.extend(['--discriminator', str(setup_params.discriminator)])
        cmd.extend(['--passcode', str(setup_params.passcode)])
        cmd.extend(['--KVS', kvs])

        # TODO: Determine if we want these logs cooked or pushed to somewhere else
        logging.info("Starting TH_SERVER")
        self._app_th_server_process = subprocess.Popen(cmd)
        self._th_server_kvs = kvs
        logging.info("Started TH_SERVER")
        time.sleep(3)
        return setup_params

    def _ask_for_vendor_commissioning_ux_operation(self, setup_params: _SetupParamters):
        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the ICD device using the following parameters:\n"
            f"- discriminator: {setup_params.discriminator}\n"
            f"- setupPinCode: {setup_params.passcode}\n"
            f"- setupQRCode: {setup_params.setup_qr_code}\n"
            f"- setupManualcode: {setup_params.manual_code}\n"
            f"If using FabricSync Admin test app, you may type:\n"
            f">>> pairing onnetwork 111 {setup_params.passcode}")

    def steps_TC_MCORE_FS_1_5(self) -> list[TestStep]:
        steps = [TestStep(1, "TH subscribes to PartsList attribute of the Descriptor cluster of DUT_FSA endpoint 0."),
                 TestStep(2, "Follow manufacturer provided instructions to have DUT_FSA commission TH_SERVER"),
                 TestStep(3, "TH waits up to 30 seconds for subscription report from the PartsList attribute of the Descriptor to contain new endpoint"),
                 TestStep(4, "TH uses DUT to open commissioning window to TH_SERVER"),
                 TestStep(5, "TH commissions TH_SERVER"),
                 TestStep(6, "TH subscribes to AdministratorCommissioning attributes on DUT_FSA for the newly added endpoint identified in step 3"),
                 TestStep(7, "TH opens commissioning window to TH_SERVER directly (not using DUT)"),
                 TestStep(8, "TH reads CurrentFabricIndex attributes on OperationalCredentials cluster from TH_SERVER directly (not using DUT_FSA)"),
                 TestStep(9, "TH reads AdministratorCommissioning from TH_SERVER directly (not using DUT)"),
                 TestStep(10, "TH waits up to 10 seconds for subscription report from the AdministratorCommissioning attribute (from step 6) to reflect values from previous step")]
        return steps

    @property
    def default_timeout(self) -> int:
        return self.user_params.get("report_waiting_timeout_delay_sec", 10)*2 + 60

    @async_test_body
    async def test_TC_MCORE_FS_1_5(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')

        min_report_interval_sec = 0
        max_report_interval_sec = 30
        th_server_port = self.user_params.get("th_server_port", 5543)
        self._th_server_app_path = self.user_params.get("th_server_app_path", None)
        if not self._th_server_app_path:
            asserts.fail('This test requires a TH_SERVER app. Specify app path with --string-arg th_server_app_path:<path_to_app>')
        if not os.path.exists(self._th_server_app_path):
            asserts.fail(f'The path {self._th_server_app_path} does not exist')

        self.step(1)
        # Subscribe to the PartsList
        root_endpoint = 0
        parts_list_subscription_contents = [
            (root_endpoint, Clusters.Descriptor.Attributes.PartsList)
        ]
        parts_list_sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=parts_list_subscription_contents,
            reportInterval=(min_report_interval_sec, max_report_interval_sec),
            keepSubscriptions=False
        )

        parts_list_queue = queue.Queue()
        parts_list_attribute_handler = AttributeChangeAccumulator(
            name=self.default_controller.name, expected_attribute=Clusters.Descriptor.Attributes.PartsList, output=parts_list_queue)
        parts_list_sub.SetAttributeUpdateCallback(parts_list_attribute_handler)
        parts_list_cached_attributes = parts_list_sub.GetAttributes()
        step_1_dut_parts_list = parts_list_cached_attributes[root_endpoint][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList]

        asserts.assert_true(type_matches(step_1_dut_parts_list, list), "PartsList is expected to be a list")

        self.step(2)
        setup_params = await self._create_th_server(th_server_port)
        self._ask_for_vendor_commissioning_ux_operation(setup_params)

        self.step(3)
        report_waiting_timeout_delay_sec = 30
        logging.info("Waiting for update to PartsList.")
        start_time = time.time()
        elapsed = 0
        time_remaining = report_waiting_timeout_delay_sec

        parts_list_endpoint_count_from_step_1 = len(step_1_dut_parts_list)
        step_3_dut_parts_list = None
        while time_remaining > 0:
            try:
                item = parts_list_queue.get(block=True, timeout=time_remaining)
                endpoint, attribute, value = item['endpoint'], item['attribute'], item['value']

                # Record arrival of an expected subscription change when seen
                if endpoint == root_endpoint and attribute == Clusters.Descriptor.Attributes.PartsList and len(value) > parts_list_endpoint_count_from_step_1:
                    step_3_dut_parts_list = value
                    break

            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = report_waiting_timeout_delay_sec - elapsed

        asserts.assert_not_equal(step_3_dut_parts_list, None, "Timed out getting updated PartsList with new endpoint")
        set_of_step_1_parts_list_endpoint = set(step_1_dut_parts_list)
        set_of_step_3_parts_list_endpoint = set(step_3_dut_parts_list)
        unique_endpoints_set = set_of_step_3_parts_list_endpoint - set_of_step_1_parts_list_endpoint
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        newly_added_endpoint = list(unique_endpoints_set)[0]

        self.step(4)

        discriminator = 3840
        passcode = 20202021
        salt = secrets.token_bytes(16)
        iterations = 2000
        verifier = _generate_verifier(passcode, salt, iterations)

        # min commissioning timeout is 3*60 seconds
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=verifier,
                                                                                   discriminator=discriminator,
                                                                                   iterations=iterations,
                                                                                   salt=salt)
        await self.send_single_cmd(cmd, dev_ctrl=self.default_controller, node_id=self.dut_node_id, endpoint=newly_added_endpoint, timedRequestTimeoutMs=5000)

        self.step(5)
        self.th_server_local_nodeid = 1111
        await self.default_controller.CommissionOnNetwork(nodeId=self.th_server_local_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

        self.step(6)
        cadmin_subscription_contents = [
            (newly_added_endpoint, Clusters.AdministratorCommissioning)
        ]
        cadmin_sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=cadmin_subscription_contents,
            reportInterval=(min_report_interval_sec, max_report_interval_sec),
            keepSubscriptions=False
        )

        cadmin_queue = queue.Queue()
        # This AttributeChangeAccumulator is really just to let us know when new subscription came in
        cadmin_attribute_handler = AttributeChangeAccumulator(
            name=self.default_controller.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.WindowStatus, output=cadmin_queue)
        cadmin_sub.SetAttributeUpdateCallback(cadmin_attribute_handler)
        time.sleep(1)

        self.step(7)
        await self.default_controller.OpenCommissioningWindow(nodeid=self.th_server_local_nodeid, timeout=180, iteration=1000, discriminator=3840, option=1)

        self.step(8)
        current_fabric_index = await self.read_single_attribute_check_success(node_id=self.th_server_local_nodeid, cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)

        self.step(9)
        th_server_directly_read_result = await self.default_controller.ReadAttribute(self.th_server_local_nodeid, [(root_endpoint, Clusters.AdministratorCommissioning)])
        th_server_direct_cadmin = th_server_directly_read_result[root_endpoint][Clusters.AdministratorCommissioning]
        cadmin_attr = Clusters.AdministratorCommissioning.Attributes
        asserts.assert_equal(th_server_direct_cadmin[cadmin_attr.WindowStatus],
                             Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kEnhancedWindowOpen, "WindowStatus is expected to be EnhancedWindowOpen")
        asserts.assert_equal(th_server_direct_cadmin[cadmin_attr.AdminFabricIndex],
                             current_fabric_index, "AdminFabricIndex is unexpected")

        self.step(10)
        report_waiting_timeout_delay_sec = 10
        logging.info("Waiting for update to AdministratorCommissioning attributes.")
        start_time = time.time()
        elapsed = 0
        time_remaining = report_waiting_timeout_delay_sec

        cadmin_sub_new_data = False
        while time_remaining > 0:
            try:
                item = cadmin_queue.get(block=True, timeout=time_remaining)
                endpoint, attribute, value = item['endpoint'], item['attribute'], item['value']

                # Record arrival of an expected subscription change when seen
                if endpoint == newly_added_endpoint and attribute == Clusters.AdministratorCommissioning.Attributes.WindowStatus:
                    cadmin_sub_new_data = True
                    break

            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = report_waiting_timeout_delay_sec - elapsed

        asserts.assert_true(cadmin_sub_new_data,
                            "Timed out waiting for DUT to reflect AdministratorCommissioning attributes for bridged device")

        dut_read = await self.default_controller.ReadAttribute(self.dut_node_id, [(newly_added_endpoint, Clusters.AdministratorCommissioning)])
        dut_cadmin_for_th_server = dut_read[newly_added_endpoint][Clusters.AdministratorCommissioning]
        asserts.assert_equal(th_server_direct_cadmin[cadmin_attr.WindowStatus],
                             dut_cadmin_for_th_server[cadmin_attr.WindowStatus], "WindowStatus incorrectly reported by DUT")
        asserts.assert_equal(th_server_direct_cadmin[cadmin_attr.AdminFabricIndex],
                             dut_cadmin_for_th_server[cadmin_attr.AdminFabricIndex], "AdminFabricIndex incorrectly reported by DUT")
        asserts.assert_equal(th_server_direct_cadmin[cadmin_attr.AdminVendorId],
                             dut_cadmin_for_th_server[cadmin_attr.AdminVendorId], "AdminVendorId incorrectly reported by DUT")


if __name__ == "__main__":
    default_matter_test_main()
