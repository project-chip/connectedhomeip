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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import os
import queue
import pathlib
import uuid
import subprocess
import time
import signal


import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, SimpleEventCallback
from mobly import asserts

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin

logger = logging.getLogger(__name__)
kRootEndpointId = 0
kICDMEndpointId = 0
kICDBridgedEndpointId = 2
kMaxUserActiveModeBitmap = 0x1FFFF
kMaxUserActiveModeTriggerInstructionByteLength = 128

#uat = cluster.Bitmaps.UserActiveModeTriggerBitmap
#modes = cluster.Enums.OperatingModeEnum
#features = cluster.Bitmaps.Feature

class TC_BRBINFO_4_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    async def _read_attribute_expect_success(self, endpoint, cluster, attribute, node_id):
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute, node_id=node_id)

    #
    # Test Harness Helpers
    #

    def desc_TC_BRBINFO_4_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_BRBINFO_4_1] Verification of KeepActive Command [DUT-Server]"

    def steps_TC_BRBINFO_4_1(self) -> list[TestStep]:
        steps = [
            TestStep("0", "Preconditions"),
            TestStep("1a", "TH reads from the ICD the A_IDLE_MODE_DURATION, A_ACTIVE_MODE_DURATION, and ACTIVE_MODE_THRESHOLD attributes"),
            TestStep("1b", "Simple KeepActive command w/ subscription. ActiveChanged event received by TH contains PromisedActiveDuration"),
            TestStep("2", "Sends 3x KeepActive commands w/ subscription. ActiveChanged event received ONCE and contains PromisedActiveDuration"),
            TestStep("3", "KeepActive not returned after 60 minutes of offline ICD"),
        ]
        return steps


    def pics_TC_BRBINFO_4_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "BRBINFO.C.80",
        ]
        return pics

    def _ask_for_vendor_commissioniong_ux_operation(self, discriminator, setupPinCode):
        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the ICD device using the following parameters:\n"
                        f"- discriminator: {discriminator}\n"
                        f"- setupPinCode: {setupPinCode}\n"
                        f"If using FabricSync Admin, you may type:\n"
                        f">>> pairing onnetwork 111 {setupPinCode}")

    def _display_message_and_wait_for_user_input(self, message):
        self.wait_for_user_input(prompt_msg=message)

    async def _send_keep_active_command(self, duration) -> int:
        logging.info("Mock sending keep active command")
        keep_active = await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=kICDBridgedEndpointId, payload=Clusters.Objects.BridgedDeviceBasicInformation.Commands.KeepActive(stayActiveDuration=duration))
        return keep_active

    async def _wait_for_active_changed_event(self, timeout) -> int:
        try:
            promised_active_duration = self.q.get(block=True, timeout=timeout)
            logging.info(f"PromisedActiveDuration: {promised_active_duration}")
            return promised_active_duration
        except queue.Empty:
            asserts.fail("Timeout on event")


    @async_test_body
    async def setup_class(self):
        super().setup_class()
        app = os.path.join(pathlib.Path(__file__).resolve().parent, '..','..','out', 'linux-x64-lit-icd', 'lit-icd-app')

        self.kvs = f'kvs_{str(uuid.uuid4())}'
        self.port = 5543
        discriminator = 3850
        passcode = 20202021
        app_args = f'--secured-device-port {self.port} --discriminator {discriminator} --passcode {passcode} --KVS {self.kvs} ' + \
          '--secured-device-port 5545'
        cmd = f'{app} {app_args}'

        logging.info("Starting ICD Server App")
        self.app_process = subprocess.Popen(cmd, bufsize=0, shell=True)
        logging.info("ICD started")
        time.sleep(3)

        logging.info("Commissioning of ICD to fabric one (TH)")
        self.icd_nodeid = 1111

        await self.default_controller.CommissionOnNetwork(nodeId=self.icd_nodeid, setupPinCode=passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

        logging.info("Commissioning of ICD to fabric two (DUT)")
        params = await self.openCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.icd_nodeid)

        self._ask_for_vendor_commissioniong_ux_operation(params.randomDiscriminator, params.commissioningParameters.setupPinCode)

    def teardown_class(self):
        logging.warning("Stopping app with SIGTERM")
        self.app_process.send_signal(signal.SIGTERM.value)
        self.app_process.wait()
        os.remove(self.kvs)
        super().teardown_class()

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

        # Preconditions
        self.step("0")

        logging.info("Ensuring DUT is commissioned to TH")

        # Confirms commissioning of DUT on TH as it reads basic info
        await self._read_attribute_expect_success(
          kRootEndpointId,
          basic_info_cluster,
          basic_info_attributes.FeatureMap,
          self.dut_node_id
        )

        logging.info("Ensuring ICD is commissioned to TH")

        # Confirms commissioning of ICD on TH as it reads basic info
        await self._read_attribute_expect_success(
          kRootEndpointId,
          basic_info_cluster,
          basic_info_attributes.FeatureMap,
          self.icd_nodeid
        )

        self.step("1a")

        idle_mode_duration = await self._read_attribute_expect_success(
          kICDMEndpointId,
          icdm_cluster,
          icdm_attributes.IdleModeDuration,
          self.icd_nodeid
        )
        logging.info(f"IdleModeDuration: {idle_mode_duration}")

        active_mode_duration = await self._read_attribute_expect_success(
          kICDMEndpointId,
          icdm_cluster,
          icdm_attributes.ActiveModeDuration,
          self.icd_nodeid
        )
        logging.info(f"ActiveModeDuration: {active_mode_duration}")

        self.step("1b")

        # Subscription to ActiveChanged
        event = brb_info_cluster.Events.ActiveChanged
        self.q = queue.Queue()
        urgent = 1
        cb = SimpleEventCallback("ActiveChanged", event.cluster_id, event.event_id, self.q)
        subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(kICDBridgedEndpointId, event, urgent)], reportInterval=[1, 3])
        subscription.SetEventUpdateCallback(callback=cb)

        stay_active_duration = 1000
        logging.info(f"Sending KeepActiveCommand({stay_active_duration}ms)")
        self._send_keep_active_command(stay_active_duration)

        logging.info("Waiting for ActiveChanged from DUT...")
        promised_active_duration = await self._wait_for_active_changed_event((idle_mode_duration + max(active_mode_duration, stay_active_duration))/1000)

        asserts.assert_greater_equal(promised_active_duration, stay_active_duration, "PromisedActiveDuration < StayActiveDuration")

        self.step("2")

        stay_active_duration = 1500
        logging.info(f"Sending KeepActiveCommand({stay_active_duration}ms)")
        self._send_keep_active_command(stay_active_duration)

        logging.info("Waiting for ActiveChanged from DUT...")
        promised_active_duration = await self._wait_for_active_changed_event((idle_mode_duration + max(active_mode_duration, stay_active_duration))/1000)

        # wait for active time duration
        time.sleep(max(stay_active_duration/1000, active_mode_duration))
        # ICD now should be in idle mode

        # sends 3x keep active commands
        logging.info(f"Sending KeepActiveCommand({stay_active_duration})")
        self._send_keep_active_command(stay_active_duration)
        time.sleep(100)
        logging.info(f"Sending KeepActiveCommand({stay_active_duration})")
        self._send_keep_active_command(stay_active_duration)
        time.sleep(100)
        logging.info(f"Sending KeepActiveCommand({stay_active_duration})")
        self._send_keep_active_command(stay_active_duration)
        time.sleep(100)

        logging.info("Waiting for ActiveChanged from DUT...")
        promised_active_duration = await self._wait_for_active_changed_event((idle_mode_duration + max(active_mode_duration, stay_active_duration))/1000)

        asserts.assert_true(self.q.qSize() == 0, "More than one event received from DUT")

        self.step("3")

        stay_active_duration = 10000
        logging.info(f"Sending KeepActiveCommand({stay_active_duration})")
        self._send_keep_active_command(stay_active_duration)

        ## halt the ICD process
        self.app_process.send_signal(signal.SIGSTOP.value)

        if not self.is_ci:
            logging.info("Waiting for 60 minutes")
            time.sleep(60*60)

        ## resume the ICD
        self.app_process.send_signal(signal.SIGCONT.value)

        # wait for active changed event, expect no event will be sent
        event_timeout = (idle_mode_duration + max(active_mode_duration, stay_active_duration))/1000
        try:
            promised_active_duration = self.q.get(block=True, timeout=event_timeout)
        finally:
            asserts.assert_true(queue.Empty(), "ActiveChanged event received when not expected")

if __name__ == "__main__":
    default_matter_test_main()
