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
# test-runner-runs:
#   run1:
#     app: examples/fabric-admin/scripts/fabric-sync-app.py
#     app-args: --app-admin=${FABRIC_ADMIN_APP} --app-bridge=${FABRIC_BRIDGE_APP} --discriminator=1234
#     app-ready-pattern: "Successfully opened pairing window on the device"
#     app-stdin-pipe: dut-fsa-stdin
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --string-arg th_icd_server_app_path:${LIT_ICD_APP} dut_fsa_stdin_pipe:dut-fsa-stdin
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

# This test requires a TH_ICD_SERVER application. Please specify with --string-arg th_icd_server_app_path:<path_to_app>
# TH_ICD_SERVER must support following arguments: --secured-device-port --discriminator --passcode --KVS
# E.g: python3 src/python_testing/TC_BRBINFO_4_1.py --commissioning-method on-network --qr-code MT:-24J042C00KA0648G00 \
#      --string-arg th_icd_server_app_path:out/linux-x64-lit-icd/lit-icd-app

import asyncio
import logging
import os
import queue
import random
import tempfile

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError, Status
from chip.testing.apps import IcdAppServerSubprocess
from chip.testing.event_attribute_reporting import SimpleEventCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)
_ROOT_ENDPOINT_ID = 0


class TC_BRBINFO_4_1(MatterBaseTest):

    # This test has some manual steps and also multiple sleeps >= 30 seconds. Test typically runs under 3 mins,
    # so 6 minutes is more than enough.
    @property
    def default_timeout(self) -> int:
        return 6*60

    def desc_TC_BRBINFO_4_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_BRBINFO_4_1] Verification of KeepActive Command [DUT-Server]"

    def steps_TC_BRBINFO_4_1(self) -> list[TestStep]:
        return [
            TestStep("0",  "DUT commissioned and preconditions", is_commissioning=True),
            TestStep("1", "TH reads from the ICD the A_IDLE_MODE_DURATION, A_ACTIVE_MODE_DURATION, and ACTIVE_MODE_THRESHOLD attributes"),
            TestStep("2", "Setting up subscribe to ActiveChange event"),
            TestStep("3", "Check TimeoutMs too low fails"),
            TestStep("4", "Check TimeoutMs too high fails"),
            TestStep("5", "Check KeepActive successful with valid command parameters lowest possible TimeoutMs"),
            TestStep("6", "Validate previous command results in ActiveChanged event shortly after ICD device checks-in"),
            TestStep("7", "Check KeepActive successful with valid command parameters highest possible TimeoutMs"),
            TestStep("8", "Validate previous command results in ActiveChanged event shortly after ICD device checks-in"),
            TestStep("9", "Send multiple KeepActive commands during window where ICD device will not check in"),
            TestStep("10", "Validate previous command results in single ActiveChanged event shortly after ICD device checks-in"),
            TestStep("11", "Validate we received no additional ActiveChanged event after subsequent ICD check in"),
            TestStep("12", "Send KeepActive command with shortest TimeoutMs value while TH_ICD is prevented from sending check-ins"),
            TestStep("13", "TH allows TH_ICD to resume sending check-ins after timeout should have expired"),
            TestStep("14", "Wait for TH_ICD to check into TH twice, then confirm we have had no new ActiveChanged events reported from DUT"),
            TestStep("15", "Send KeepActive command with shortest TimeoutMs value while TH_ICD is prevented from sending check-ins"),
            TestStep("16", "Wait 15 seconds then send second KeepActive command with double the TimeoutMs value of the previous step"),
            TestStep("17", "TH allows TH_ICD to resume sending check-ins after timeout from step 15 expired but before second timeout from step 16 still valid"),
            TestStep("18", "Wait for TH_ICD to check into TH, then confirm we have received new event from DUT"),
        ]

    def _ask_for_vendor_commissioning_ux_operation(self, discriminator, setupPinCode, setupManualCode, setupQRCode):
        self.wait_for_user_input(
            prompt_msg=f"Using the DUT vendor's provided interface, commission the ICD device using the following parameters:\n"
            f"- discriminator: {discriminator}\n"
            f"- setupPinCode: {setupPinCode}\n"
            f"- setupQRCode: {setupQRCode}\n"
            f"- setupManualCode: {setupManualCode}\n"
            f"If using FabricSync Admin test app, you may type:\n"
            f">>> pairing onnetwork 111 {setupPinCode} --icd-registration true")

    async def _send_keep_active_command(self, stay_active_duration_ms, timeout_ms, endpoint_id) -> int:
        logging.info("Sending keep active command")
        keep_active = await self.default_controller.SendCommand(nodeid=self.dut_node_id, endpoint=endpoint_id, payload=Clusters.Objects.BridgedDeviceBasicInformation.Commands.KeepActive(stayActiveDuration=stay_active_duration_ms, timeoutMs=timeout_ms))
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
        super().setup_class()

        # These steps are not explicitly, but they help identify the dynamically added endpoint
        # The second part of this process happens on _get_dynamic_endpoint()
        root_part_list = await self.read_single_attribute_check_success(
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList,
            endpoint=_ROOT_ENDPOINT_ID)
        self.set_of_dut_endpoints_before_adding_device = set(root_part_list)

        self._active_change_event_subscription = None
        self.th_icd_server = None
        self.storage = None

        th_icd_server_app = self.user_params.get("th_icd_server_app_path", None)
        if not th_icd_server_app:
            asserts.fail('This test requires a TH_ICD_SERVER app. Specify app path with --string-arg th_icd_server_app_path:<path_to_app>')
        if not os.path.exists(th_icd_server_app):
            asserts.fail(f'The path {th_icd_server_app} does not exist')

        # Create a temporary storage directory for keeping KVS files.
        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)
        logging.info("Temporary storage directory: %s", self.storage.name)

        if self.is_pics_sdk_ci_only:
            # Get the named pipe path for the DUT_FSA app input from the user params.
            dut_fsa_stdin_pipe = self.user_params.get("dut_fsa_stdin_pipe")
            if not dut_fsa_stdin_pipe:
                asserts.fail("CI setup requires --string-arg dut_fsa_stdin_pipe:<path_to_pipe>")
            self.dut_fsa_stdin = open(dut_fsa_stdin_pipe, "w")

        self.th_icd_server_port = 5543
        self.th_icd_server_discriminator = random.randint(0, 4095)
        self.th_icd_server_passcode = 20202021

        # Start the TH_ICD_SERVER app.
        self.th_icd_server = IcdAppServerSubprocess(
            th_icd_server_app,
            storage_dir=self.storage.name,
            port=self.th_icd_server_port,
            discriminator=self.th_icd_server_discriminator,
            passcode=self.th_icd_server_passcode)
        self.th_icd_server.start(
            expected_output="Server initialization complete",
            timeout=30)

        logging.info("Commissioning of ICD to fabric one (TH)")
        self.icd_nodeid = 1111

        self.default_controller.EnableICDRegistration(self.default_controller.GenerateICDRegistrationParameters())
        await self.default_controller.CommissionOnNetwork(
            nodeId=self.icd_nodeid,
            setupPinCode=self.th_icd_server_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.th_icd_server_discriminator)

        logging.info("Commissioning of ICD to fabric two (DUT)")
        params = await self.open_commissioning_window(dev_ctrl=self.default_controller, node_id=self.icd_nodeid)

        if not self.is_pics_sdk_ci_only:
            self._ask_for_vendor_commissioning_ux_operation(
                params.randomDiscriminator,
                params.commissioningParameters.setupPinCode,
                params.commissioningParameters.setupManualCode,
                params.commissioningParameters.setupQRCode)
        else:
            self.dut_fsa_stdin.write(
                f"pairing onnetwork 2 {params.commissioningParameters.setupPinCode} --icd-registration true\n")
            self.dut_fsa_stdin.flush()
            # Wait for the commissioning to complete.
            await asyncio.sleep(5)

    def teardown_class(self):
        if self._active_change_event_subscription is not None:
            self._active_change_event_subscription.Shutdown()
            self._active_change_event_subscription = None
        if self.th_icd_server is not None:
            self.th_icd_server.terminate()
        if self.storage is not None:
            self.storage.cleanup()
        super().teardown_class()

    #
    # BRBINFO 4.1 Test Body
    #

    @async_test_body
    async def test_TC_BRBINFO_4_1(self):
        icdm_cluster = Clusters.Objects.IcdManagement
        icdm_attributes = icdm_cluster.Attributes
        brb_info_cluster = Clusters.Objects.BridgedDeviceBasicInformation
        basic_info_cluster = Clusters.Objects.BasicInformation
        basic_info_attributes = basic_info_cluster.Attributes

        dynamic_endpoint_id = await self._get_dynamic_endpoint()
        logging.info(f"Dynamic endpoint is {dynamic_endpoint_id}")

        self.step("0")
        logging.info("Ensuring DUT is commissioned to TH")

        # Confirms commissioning of DUT on TH as it reads its feature map
        await self.read_single_attribute_check_success(
            endpoint=_ROOT_ENDPOINT_ID,
            cluster=basic_info_cluster,
            attribute=basic_info_attributes.FeatureMap,
            node_id=self.dut_node_id,
        )

        logging.info("Ensuring ICD is commissioned to TH")

        self.step("1")

        idle_mode_duration_s = await self.read_single_attribute_check_success(
            endpoint=_ROOT_ENDPOINT_ID,
            cluster=icdm_cluster,
            attribute=icdm_attributes.IdleModeDuration,
            node_id=self.icd_nodeid,
        )
        logging.info(f"IdleModeDurationS: {idle_mode_duration_s}")

        active_mode_duration_ms = await self.read_single_attribute_check_success(
            endpoint=_ROOT_ENDPOINT_ID,
            cluster=icdm_cluster,
            attribute=icdm_attributes.ActiveModeDuration,
            node_id=self.icd_nodeid,
        )
        logging.info(f"ActiveModeDurationMs: {active_mode_duration_ms}")

        active_mode_threshold_ms = await self.read_single_attribute_check_success(
            endpoint=_ROOT_ENDPOINT_ID,
            cluster=icdm_cluster,
            attribute=icdm_attributes.ActiveModeThreshold,
            node_id=self.icd_nodeid,
        )
        logging.info(f"ActiveModeThresholdMs: {active_mode_threshold_ms}")

        self.step("2")
        event = brb_info_cluster.Events.ActiveChanged
        self.q = queue.Queue()
        urgent = 1
        cb = SimpleEventCallback("ActiveChanged", event.cluster_id, event.event_id, self.q)
        self._active_change_event_subscription = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=[(dynamic_endpoint_id, event, urgent)], reportInterval=[1, 3])
        self._active_change_event_subscription.SetEventUpdateCallback(callback=cb)

        self.step("3")
        stay_active_duration_ms = 1000
        keep_active_timeout_ms = 29999
        try:
            await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)
            asserts.fail("KeepActive with invalid TimeoutMs was expected to fail")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "DUT sent back an unexpected error, we were expecting ConstraintError")

        self.step("4")
        keep_active_timeout_ms = 3600001
        try:
            await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)
            asserts.fail("KeepActive with invalid TimeoutMs was expected to fail")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "DUT sent back an unexpected error, we were expecting ConstraintError")

        self.step("5")
        keep_active_timeout_ms = 30000
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("6")
        wait_for_icd_checkin_timeout_s = idle_mode_duration_s + max(active_mode_duration_ms, stay_active_duration_ms)/1000
        wait_for_dut_event_subscription_s = 5
        # This will throw exception if timeout is exceeded.
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        promised_active_duration_ms = await self._wait_for_active_changed_event(timeout_s=wait_for_dut_event_subscription_s)
        asserts.assert_greater_equal(promised_active_duration_ms, stay_active_duration_ms,
                                     "PromisedActiveDuration < StayActiveDuration")
        asserts.assert_equal(self.q.qsize(), 0, "Unexpected event received from DUT")

        self.step("7")
        keep_active_timeout_ms = 3600000
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("8")
        # This will throw exception if timeout is exceeded.
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        promised_active_duration_ms = await self._wait_for_active_changed_event(timeout_s=wait_for_dut_event_subscription_s)
        asserts.assert_greater_equal(promised_active_duration_ms, stay_active_duration_ms,
                                     "PromisedActiveDuration < StayActiveDuration")
        asserts.assert_equal(self.q.qsize(), 0, "Unexpected event received from DUT")

        self.step("9")
        self.th_icd_server.pause()
        # sends 3x keep active commands
        stay_active_duration_ms = 2000
        keep_active_timeout_ms = 60000
        logging.info(f"Sending first KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)
        logging.info(f"Sending second KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)
        logging.info(f"Sending third KeepActiveCommand({stay_active_duration_ms})")
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("10")
        self.th_icd_server.resume()
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        promised_active_duration_ms = await self._wait_for_active_changed_event(timeout_s=wait_for_dut_event_subscription_s)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")

        self.step("11")
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")

        self.step("12")
        self.th_icd_server.pause()
        stay_active_duration_ms = 2000
        keep_active_timeout_ms = 30000
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("13")
        await asyncio.sleep(30)
        self.th_icd_server.resume()

        self.step("14")
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        asserts.assert_equal(self.q.qsize(), 0, "Unexpected event received from DUT")

        self.step("15")
        self.th_icd_server.pause()
        stay_active_duration_ms = 2000
        keep_active_timeout_ms = 30000
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("16")
        await asyncio.sleep(15)
        stay_active_duration_ms = 2000
        keep_active_timeout_ms = 60000
        await self._send_keep_active_command(stay_active_duration_ms, keep_active_timeout_ms, dynamic_endpoint_id)

        self.step("17")
        await asyncio.sleep(15)
        self.th_icd_server.resume()

        self.step("18")
        await self.default_controller.WaitForActive(self.icd_nodeid, timeoutSeconds=wait_for_icd_checkin_timeout_s, stayActiveDurationMs=5000)
        promised_active_duration_ms = await self._wait_for_active_changed_event(timeout_s=wait_for_dut_event_subscription_s)
        asserts.assert_equal(self.q.qsize(), 0, "More than one event received from DUT")


if __name__ == "__main__":
    default_matter_test_main()
