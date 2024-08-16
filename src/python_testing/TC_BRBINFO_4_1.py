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

# This test requires a TH_ICD_SERVER application. Please specify with --string-arg th_icd_server_app_path:<path_to_app>
# TH_ICD_SERVER must support following arguments: --secured-device-port --discriminator --passcode --KVS
# E.g: python3 src/python_testing/TC_BRBINFO_4_1.py --commissioning-method on-network --qr-code MT:-24J042C00KA0648G00 \
#      --string-arg th_icd_server_app_path:out/linux-x64-lit-icd/lit-icd-app

import logging
import os
import queue
import signal
import subprocess
import time
import uuid

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from matter_testing_support import MatterBaseTest, SimpleEventCallback, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)
_ROOT_ENDPOINT_ID = 0


class TC_BRBINFO_4_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_attribute_expect_success(self, endpoint, cluster, attribute, node_id):
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute, node_id=node_id)

    # Override default timeout to support a 60 min wait
    @property
    def default_timeout(self) -> int:
        return 63*60

    def desc_TC_BRBINFO_4_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_BRBINFO_4_1] Verification of KeepActive Command [DUT-Server]"

    def steps_TC_BRBINFO_4_1(self) -> list[TestStep]:
        steps = [
            TestStep("0",  "DUT commissioned", is_commissioning=True),
            TestStep("0a", "Preconditions"),
            TestStep("1a", "TH reads from the ICD the A_IDLE_MODE_DURATION, A_ACTIVE_MODE_DURATION, and ACTIVE_MODE_THRESHOLD attributes"),
            TestStep("1b", "Simple KeepActive command w/ subscription. ActiveChanged event received by TH contains PromisedActiveDuration"),
            TestStep("2", "Sends 3x KeepActive commands w/ subscription. ActiveChanged event received ONCE and contains PromisedActiveDuration"),
            TestStep("3", "TH waits for check-in from TH_ICD to confirm no additional ActiveChanged events are recieved"),
            TestStep("4", "KeepActive not returned after 60 minutes of offline ICD"),
        ]
        return steps

    def _ask_for_vendor_commissioniong_ux_operation(self, discriminator, setupPinCode, setupManualCode, setupQRCode):
        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the ICD device using the following parameters:\n"
            f"- discriminator: {discriminator}\n"
            f"- setupPinCode: {setupPinCode}\n"
            f"- setupQRCode: {setupQRCode}\n"
            f"- setupManualcode: {setupManualCode}\n"
            f"If using FabricSync Admin test app, you may type:\n"
            f">>> pairing onnetwork 111 {setupPinCode} --icd-registration true")

    async def _send_keep_active_command(self, duration, endpoint_id) -> int:
        logging.info("Sending keep active command")
        keep_active = await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=endpoint_id, payload=Clusters.Objects.BridgedDeviceBasicInformation.Commands.KeepActive(stayActiveDuration=duration))
        return keep_active

    async def _wait_for_active_changed_event(self, timeout_s) -> int:
        try:
            promised_active_duration_event = self.q.get(block=True, timeout=timeout_s)
            logging.info(f"PromisedActiveDurationEvent: {promised_active_duration_event}")
            promised_active_duration_ms = promised_active_duration_event.Data.promisedActiveDuration
            return promised_active_duration_ms
        except queue.Empty:
            asserts.fail("Timeout on event ActiveChanged")

    async def _get_dynamic_endpoint(self) -> int:
        root_part_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList, endpoint=_ROOT_ENDPOINT_ID)
        set_of_endpoints_after_adding_device = set(root_part_list)

        asserts.assert_true(set_of_endpoints_after_adding_device.issuperset(
            self.set_of_dut_endpoints_before_adding_device), "Expected only new endpoints to be added")
        unique_endpoints_set = set_of_endpoints_after_adding_device - self.set_of_dut_endpoints_before_adding_device
        asserts.assert_equal(len(unique_endpoints_set), 1, "Expected only one new endpoint")
        newly_added_endpoint = list(unique_endpoints_set)[0]
        return newly_added_endpoint

    @async_test_body
    async def setup_class(self):
        # These steps are not explicitly, but they help identify the dynamically added endpoint
        # The second part of this process happens on _get_dynamic_endpoint()
        root_part_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.PartsList, endpoint=_ROOT_ENDPOINT_ID)
        self.set_of_dut_endpoints_before_adding_device = set(root_part_list)

        super().setup_class()
        self.app_process = None
        self.app_process_paused = False
        app = self.user_params.get("th_icd_server_app_path", None)
        if not app:
            asserts.fail('This test requires a TH_ICD_SERVER app. Specify app path with --string-arg th_icd_server_app_path:<path_to_app>')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = 3850
        passcode = 20202021
        app_args = f'--secured-device-port {self.port} --discriminator {discriminator} --passcode {passcode} --KVS {self.kvs} '
        cmd = f'{app} {app_args}'

        logging.info("Starting ICD Server App")
        self.app_process = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info("ICD started")
        time.sleep(3)

        logging.info("Commissioning of ICD to fabric one (TH)")
        self.icd_nodeid = 1111

        self.default_controller.EnableICDRegistration(self.default_controller.GenerateICDRegistrationParameters())
        await self.default_controller.CommissionOnNetwork(nodeId=self.icd_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

        logging.info("Commissioning of ICD to fabric two (DUT)")
        params = await self.openCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.icd_nodeid)

        self._ask_for_vendor_commissioniong_ux_operation(params.randomDiscriminator, params.commissioningParameters.setupPinCode,
                                                         params.commissioningParameters.setupManualCode, params.commissioningParameters.setupQRCode)

    def teardown_class(self):
        # In case the th_icd_server_app_path does not exist, then we failed the test
        # and there is nothing to remove
        if self.app_process is not None:
            self.resume_th_icd_server(check_state=False)
            logging.warning("Stopping app with SIGTERM")
            self.app_process.send_signal(signal.SIGTERM.value)
            self.app_process.wait()

            if os.path.exists(self.kvs):
                os.remove(self.kvs)

        super().teardown_class()

    def pause_th_icd_server(self, check_state):
        if check_state:
            asserts.assert_false(self.app_process_paused, "ICD TH Server unexpectedly is already paused")
        if self.app_process_paused:
            return
        # stops (halts) the ICD server process by sending a SIGTOP signal
        self.app_process.send_signal(signal.SIGSTOP.value)
        self.app_process_paused = True

    def resume_th_icd_server(self, check_state):
        if check_state:
            asserts.assert_true(self.app_process_paused, "ICD TH Server unexpectedly is already running")
        if not self.app_process_paused:
            return
        # resumes (continues) the ICD server process by sending a SIGCONT signal
        self.app_process.send_signal(signal.SIGCONT.value)
        self.app_process_paused = False

    #
    # BRBINFO 4.1 Test Body
    #

    @async_test_body
    async def test_TC_BRBINFO_4_1(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        icdm_cluster = Clusters.Objects.IcdManagement
        icdm_attributes = icdm_cluster.Attributes
        brb_info_cluster = Clusters.Objects.BridgedDeviceBasicInformation
        basic_info_cluster = Clusters.Objects.BasicInformation
        basic_info_attributes = basic_info_cluster.Attributes

        dynamic_endpoint_id = await self._get_dynamic_endpoint()
        logging.info(f"Dynamic endpoint is {dynamic_endpoint_id}")

        self.step("0")

        # Preconditions
        self.step("0a")

        logging.info("Ensuring DUT is commissioned to TH")

        # Confirms commissioning of DUT on TH as it reads its fature map
        await self._read_attribute_expect_success(
            _ROOT_ENDPOINT_ID,
            basic_info_cluster,
            basic_info_attributes.FeatureMap,
            self.dut_node_id
        )

        logging.info("Ensuring ICD is commissioned to TH")

        # Confirms commissioning of ICD on TH as it reads its feature map
        await self._read_attribute_expect_success(
            _ROOT_ENDPOINT_ID,
            basic_info_cluster,
            basic_info_attributes.FeatureMap,
            self.icd_nodeid
        )

        self.step("1a")

        idle_mode_duration_s = await self._read_attribute_expect_success(
            _ROOT_ENDPOINT_ID,
            icdm_cluster,
            icdm_attributes.IdleModeDuration,
            self.icd_nodeid
        )
        logging.info(f"IdleModeDurationS: {idle_mode_duration_s}")

        active_mode_duration_ms = await self._read_attribute_expect_success(
            _ROOT_ENDPOINT_ID,
            icdm_cluster,
            icdm_attributes.ActiveModeDuration,
            self.icd_nodeid
        )
        logging.info(f"ActiveModeDurationMs: {active_mode_duration_ms}")

        self.step("1b")

        # Subscription to ActiveChanged
        event = brb_info_cluster.Events.ActiveChanged
        self.q = queue.Queue()
        urgent = 1
        cb = SimpleEventCallback("ActiveChanged", event.cluster_id, event.event_id, self.q)
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(dynamic_endpoint_id, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        stay_active_duration_ms = 1000
        logging.info(f"Sending KeepActiveCommand({stay_active_duration_ms}ms)")
        await self._send_keep_active_command(stay_active_duration_ms, dynamic_endpoint_id)

        logging.info("Waiting for ActiveChanged from DUT...")
        timeout_s = idle_mode_duration_s + max(active_mode_duration_ms, stay_active_duration_ms)/1000
        promised_active_duration_ms = await self._wait_for_active_changed_event(timeout_s)

        asserts.assert_greater_equal(promised_active_duration_ms, stay_active_duration_ms,
                                     "PromisedActiveDuration < StayActiveDuration")

        self.step("2")

        # Prevent icd app from sending any check-in messages.
        self.pause_th_icd_server(check_state=True)
        # sends 3x keep active commands
        stay_active_duration_ms = 2000
        logging.info(f"Sending first KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, dynamic_endpoint_id)
        logging.info(f"Sending second KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, dynamic_endpoint_id)
        logging.info(f"Sending third KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, dynamic_endpoint_id)
        self.resume_th_icd_server(check_state=True)

        logging.info("Waiting for ActiveChanged from DUT...")
        promised_active_duration_ms = await self._wait_for_active_changed_event((idle_mode_duration_s + max(active_mode_duration_ms, stay_active_duration_ms))/1000)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")

        self.step("3")
        await self.default_controller.WaitForActive(self.icd_nodeid, stayActiveDurationMs=5000)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")

        self.step("4")

        logging.info("TH waiting for checkin from TH_ICD...")
        await self.default_controller.WaitForActive(self.icd_nodeid, stayActiveDurationMs=10000)
        stay_active_duration_ms = 10000
        logging.info(f"Sending KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, dynamic_endpoint_id)

        self.pause_th_icd_server(check_state=True)
        # If we are seeing assertion below fail test assumption is likely incorrect.
        # Test assumes after TH waits for check-in from TH_ICD it has enough time to
        # call the KeepActive command and pause the app to prevent it from checking in
        # after DUT recieved the KeepActive command. Should this assumption be incorrect
        # we could look into using existing ICDTestEventTriggerEvent, or adding test
        # event trigger that will help suppress check-ins from the TH_ICD_SERVER.
        asserts.assert_equal(self.q.qsize(), 0, "")

        if not self.is_ci:
            logging.info("Waiting for 60 minutes")
            time.sleep(60*60)

        self.resume_th_icd_server(check_state=True)

        logging.info("TH waiting for first checkin from TH_ICD...")
        await self.default_controller.WaitForActive(self.icd_nodeid, stayActiveDurationMs=10000)
        logging.info("TH waiting for second checkin from TH_ICD...")
        await self.default_controller.WaitForActive(self.icd_nodeid, stayActiveDurationMs=10000)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")


if __name__ == "__main__":
    default_matter_test_main()
