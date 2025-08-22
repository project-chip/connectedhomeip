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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --secured-device-port 5541
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --vendor-id 65521
#       --product-id 32769
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import asyncio
import threading     # For Step #1
import time          # For Step #2
import subprocess
import os
import signal
import psutil
import socket


import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, AttributeChangeCallback
from chip.testing.matter_testing import ClusterAttributeChangeAccumulator, AttributeMatcher

from mobly import asserts

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_2(MatterBaseTest):
    cluster_otap = Clusters.OtaSoftwareUpdateProvider
    cluster_otar = Clusters.OtaSoftwareUpdateRequestor

    async def launch_ota_provider(self, ota_file: str, provider_discriminator: int,
                                  provider_setupPinCode: int, secured_device_port: int,
                                  queue: str = None, timeout: int = None):
        """
        Launches the OTA Provider in a new macOS Terminal window.
        Returns the subprocess.Popen object.
        """
        queue_cmd = f"-q {queue}" if queue else ""
        timeout_cmd = f"-t {timeout}" if timeout else ""
        cmd = f'''
        osascript -e 'tell application "Terminal"
            do script "cd /Users/jdelgado/connectedhomeip && \
        source scripts/activate.sh && \
        ./out/debug/chip-ota-provider-app --filepath {ota_file} \
        --discriminator {provider_discriminator} \
        --passcode {provider_setupPinCode} \
        --secured-device-port {secured_device_port} {queue_cmd} {timeout_cmd}; exit"
        end tell'
        '''

        proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        await asyncio.sleep(10)  # give the provider time to initialize
        logger.info(f'Launched OTA Provider with command: {cmd}')
        return proc

    async def kill_ota_provider_process(self, process_name="chip-ota-provider-app", timeout=10):
        """
        Kills the OTA Provider process by name.
        """
        for proc in psutil.process_iter(['pid', 'name']):
            if process_name in proc.info['name']:
                logger.info(f'Killing OTA Provider process: {proc.info}')
                proc.terminate()

    async def kill_ota_requestor_process(self, process_name="ota.update", timeout=10):
        """
        Kills the OTA Provider process by name.
        """
        for proc in psutil.process_iter(['pid', 'name']):
            if process_name in proc.info['name']:
                logger.info(f'Killing OTA Provider process: {proc.info}')
                proc.terminate()

    async def launch_ota_requestor(self, discriminator: int, passcode: int, secured_device_port: int = 5541):
        cmd = f'''
        osascript -e 'tell application "Terminal"
            do script "cd /Users/jdelgado/connectedhomeip && \
        source scripts/activate.sh && \
        ./out/debug/chip-ota-requestor-app \
            --discriminator {discriminator} \
            --passcode {passcode} \
            --secured-device-port {secured_device_port} \
            --autoApplyImage \
            --KVS /tmp/chip_kvs_requestor; exit"
        end tell'
        '''
        proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        await asyncio.sleep(10)
        logger.info(f'Launched OTA Requestor with command: {cmd}')
        return proc

    async def set_ota_acls_for_provider(self, controller, requestor_node: int, provider_node: int, fabric_index: int):
        """
        Set ACLs for OTA interaction between a Requestor and Provider.
        Preserves existing ACLs to avoid overwriting.
        """
        # -------------------------------
        # Prerequisite #3.1: ACLs on Requestor to allow Provider to interact with OTA Requestor
        # -------------------------------

        # Read existing ACLs on Requestor
        acl_original_requestor = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=requestor_node,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )
        logger.info(f'Prerequisite #3.1 - ACLs current privileges on Requestor (DUT): {acl_original_requestor}')

        # ACLs on Requestor to allow Provider interaction
        acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[provider_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id)]
        )
        acl_view_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[provider_node],
            targets=[]
        )
        acl_admin_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[provider_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id)]
        )

        # Combine original + new ACLs on Requestor
        # This avoids overwriting existing entries when adding new ACLs
        acls_requestor = acl_original_requestor + [acl_admin_provider, acl_view_provider, acl_operate_provider]
        resp = await self.write_acl(controller, requestor_node, acls_requestor)
        logger.info(
            f'Prerequisite #3.1 - Response of wrote combined ACLs on Requestor to allow Provider with node {provider_node}: {resp}')

        # Read new ACLs on Requestor (after combined)
        acl_current_requestor = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=requestor_node,  # DUT
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )
        logger.info(f'Prerequisite #3.1 - ACLs combined current + new privileges on Requestor (DUT): {acl_current_requestor}')

        # -------------------------------
        # Prerequisite #3.1: ACLs on Provider to allow Requestor to interact with OTA Provider
        # -------------------------------

        # Read existing ACLs on Provider
        acl_original_provider = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=provider_node,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )
        logger.info(f'Prerequisite #3.2 - ACLs current privileges on Provider with node {provider_node}: {acl_original_requestor}')

        # ACLs on Provider to allow Requestor interaction
        acl_operate_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id)]
        )
        acl_view_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node],
            targets=[]
        )
        acl_admin_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id)]
        )

        # Combine original + new ACLs on Provider
        # This avoids overwriting existing entries when adding new ACLs
        acls_provider = acl_original_provider + [acl_admin_requestor, acl_view_requestor, acl_operate_requestor]
        resp = await self.write_acl(controller, provider_node, acls_provider)
        logger.info(
            f'Prerequisite #3.2 - Response of wrote combined ACLs on Provider with node {provider_node} to allow access from Requestor: {resp}')

        # Read new ACLs on Provider (after combined)
        acl_current_provider = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=provider_node,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )
        logger.info(f'Prerequisite #3.2 - ACLs current privileges on Provider with node {provider_node}: {acl_current_provider}')

    async def add_single_ota_provider(self, controller, requestor_node_id: int, provider_node_id: int):
        """
        Adds a single OTA provider to the Requestor's DefaultOTAProviders attribute.
        Preserves existing providers and writes only the new one.
        """
        # Read existing DefaultOTAProviders on the Requestor
        current_providers = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=self.cluster_otar,
            attribute=self.cluster_otar.Attributes.DefaultOTAProviders
        )
        logger.info(f'Prerequisite #4.0 - Current DefaultOTAProviders on Requestor: {current_providers}')

        # Create a ProviderLocation for the new provider
        provider_location = self.cluster_otar.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=0,
            fabricIndex=controller.fabricId
        )
        logger.info(f'Prerequisite #4.0 - ProviderLocation to add: {provider_location}')

        # Combine with existing providers (preserving previous ones)
        updated_providers = current_providers + [provider_location]

        # Write the updated DefaultOTAProviders list back to the Requestor
        attr = self.cluster_otar.Attributes.DefaultOTAProviders(value=updated_providers)
        resp = await controller.WriteAttribute(
            attributes=[(0, attr)],
            nodeid=requestor_node_id
        )
        logger.info(f'Prerequisite #4.0 - Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

    async def is_port_free(self, port):
        """Check if a TCP port is free"""
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            return s.connect_ex(('127.0.0.1', port)) != 0

    async def write_acl(self, controller, node_id, acl):
        """
        Writes the Access Control List (ACL) to the DUT device using the specified controller.

        Args:
            controller: The Matter controller (e.g., th1, th4) that will perform the write operation.
            acl (list): List of AccessControlEntryStruct objects defining the ACL permissions to write.
            node_id:

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """
        result = await controller.WriteAttribute(
            node_id,
            [(0, Clusters.AccessControl.Attributes.Acl(acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "ACL write failed")
        return True

    async def run_ota_step(
        self,
        step_number,
        controller,
        requestor_node_id,
        requestor_discriminator,
        requestor_passcode,
        requestor_port,
        provider_node_id,
        provider_discriminator,
        provider_setupPinCode,
        provider_port,
        provider_ota_file,
        fabric_id,
        provider_queue="Busy",   # Optional
        provider_timeout=120         # Optional
    ):
        """
        Run a full OTA step: launch Requestor, Provider, Commissioning, ACLs, AnnounceOTAProvider.
        """

        # ------------------------------------------------------------------------------------
        # Prerequisites: All the steps to prepare the Requestor and Provider for OTA update
        # ------------------------------------------------------------------------------------

        # Prerequisite #1.1 - Launch Requestor (DUT) from the test
        logger.info(f'{step_number}: Prerequisite #1.1 - Launching Requestor (DUT) from test')
        requestor_proc = await self.launch_ota_requestor(
            discriminator=requestor_discriminator,
            passcode=requestor_passcode,
            secured_device_port=requestor_port
        )
        await asyncio.sleep(5)

        # Prerequisite #1.2 - Commission the Requestor using controller
        logger.info(f'{step_number}: Prerequisite #1.2 - Commissioning Requestor (NodeID={requestor_node_id})')

        await controller.CommissionOnNetwork(
            nodeId=requestor_node_id,
            setupPinCode=requestor_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=requestor_discriminator
        )

        # Prerequisite #2.0 - Launch Provider
        logger.info(f'{step_number}: Prerequisite #2.0 - Launch Provider')

        if step_number == "[STEP_1]":
            provider_proc = await self.launch_ota_provider(
                ota_file=provider_ota_file,
                provider_discriminator=provider_discriminator,
                provider_setupPinCode=provider_setupPinCode,
                secured_device_port=provider_port
            )
        else:
            provider_proc = await self.launch_ota_provider(
                ota_file=provider_ota_file,
                provider_discriminator=provider_discriminator,
                provider_setupPinCode=provider_setupPinCode,
                secured_device_port=provider_port,
                queue=provider_queue,       # STEP_2
                timeout=provider_timeout    # STEP_2
            )

        # Prerequisite #2.1 - Open commissioning window on DUT
        logger.info(f'{step_number}: Prerequisite #2.1 - Commissioning window opened on Requestor (DUT)')

        params = await self.open_commissioning_window(controller, requestor_node_id)
        setup_pin_code = params.commissioningParameters.setupPinCode
        long_discriminator = params.randomDiscriminator
        logging.info(f'{step_number}: Prerequisite #2.1 - Commissioning window opened: {vars(params)}')

        # Prerequisite #2.2 - Commission Provider
        logging.info(f'{step_number}: Prerequisite #2.2 - Commissioning DUT with Provider')

        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'{step_number}: Prerequisite #2.2 - Provider Commissioning response: {resp}')

        # Prerequisite #3.0 - Setting ACLs
        logger.info(f'{step_number}: Prerequisite #3.0 - Setting ACLs under FabricIndex {fabric_id}')
        logger.info(f'{step_number}: Prerequisite #3.0 - Requestor (DUT), NodeID: {requestor_node_id}')
        logger.info(f'{step_number}: Prerequisite #3.0 - Provider, NodeID: {provider_node_id}')

        # Set ACLs for OTA update:
        # - On Requestor to allow specified Provider to interact with OTA Requestor cluster
        # - On Provider to allow Requestor to interact with OTA Provider cluster
        await self.set_ota_acls_for_provider(
            controller,
            requestor_node=requestor_node_id,
            provider_node=provider_node_id,
            fabric_index=fabric_id
        )

        # Prerequisite #4.0 - Add OTA Provider to the Requestor
        logger.info(f'{step_number}: Prerequisite #4.0 - Add Provider to Requestor(DUT) DefaultOTAProviders')

        await self.add_single_ota_provider(controller, requestor_node_id, provider_node_id)

        return requestor_proc, provider_proc

    def desc_TC_SU_2_2(self) -> str:
        return "[TC-SU-2.2] Handling Different QueryImageResponse Scenarios on Requestor"

    def pics_TC_SU_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.S",      # Pics
        ]
        return pics

    def steps_TC_SU_2_2(self) -> list[TestStep]:
        # TODO: In progress
        steps = [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider) from Test.",
                     is_commissioning=False),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "Set ImageURI to the location where the image is located..",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'Busy', DelayedActionTime is set to 60 seconds.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            # TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to 'NotAvailable'.",
            #          "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec " \
            #          "which is 2 minutes (120 seconds) from the last QueryImage command."),
            # TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to Busy, Set DelayedActionTime to 3 minutes. On the subsequent QueryImage command, " \
            #          "TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'.",
            #          "Verify that the DUT waits for at least the time mentioned in the DelayedActionTime (3 minutes) before issuing another QueryImage command to the TH/OTA-P."
            #          "Verify that there is a transfer of the software image after the second QueryImageResponse with UpdateAvailable status from the TH/OTA-P to the DUT."),
            # TestStep(5, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
            #          "ImageURI should have the https url from where the image can be downloaded.",
            #          "Verify that the DUT queries the https url and downloads the software image."),
            # TestStep(6, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
            #          "Software Version should be set to the same or an older (numerically lower) version.",
            #          "Verify that the DUT does not start transferring the software image."),
            # TestStep(7, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. " \
            #          "QueryStatus is set to 'UpdateAvailable', ImageURI field contains an invalid BDX ImageURI.",
            #          "Verify that the DUT does not start transferring the software image."),
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_2(self):
        # TODO: In progress
        # ------------------------------------------------------------------------------------
        # Run script
        # ------------------------------------------------------------------------------------
        # Run Python test with commission Provisioner/Requestor from Terminal:
        #   python3 src/python_testing/TC_SU_2_2.py
        #
        # ------------------------------------------------------------------------------------

        self.step(0)
        # ------------------------------------------------------------------------------------
        # Prerequisite #1.0 - Setup Requestor as DUT
        # ------------------------------------------------------------------------------------
        controller = self.default_controller
        fabric_id = controller.fabricId
        provider_ota_file_all_test = "firmware_requestor_v2min.ota"

        step_number_s1 = "[STEP_1]"
        requestor_node_id_s1 = self.dut_node_id  # Assigned on execution time
        requestor_discriminator_s1 = 1234
        requestor_passcode_s1 = 20202021
        requestor_port_s1 = 5541
        provider_node_id_s1 = 1
        provider_discriminator_s1 = 1111
        provider_setupPinCode_s1 = 20202021
        provider_port_s1 = 5540
        fabric_id = controller.fabricId

        logger.info(f'Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id_s1}, FabricId: {fabric_id}')

        # --- Step 1 ---
        requestor_proc_1, provider_proc_1 = await self.run_ota_step(
            step_number=step_number_s1,
            controller=self.default_controller,
            requestor_node_id=requestor_node_id_s1,
            requestor_discriminator=requestor_discriminator_s1,
            requestor_passcode=requestor_passcode_s1,
            requestor_port=requestor_port_s1,
            provider_node_id=provider_node_id_s1,
            provider_discriminator=provider_discriminator_s1,
            provider_setupPinCode=provider_setupPinCode_s1,
            provider_port=provider_port_s1,
            provider_ota_file=provider_ota_file_all_test,
            fabric_id=controller.fabricId
        )

        self.step(1)
        # ------------------------------------------------------------------------------------
        # Step #1.0 - Provider sends AnnounceOTAProvider command to Requestor
        # ------------------------------------------------------------------------------------

        logger.info("Step #1.0 - Provider_S1 sends AnnounceOTAProvider command to DUT")
        cmd_announce = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=1,  # Provider
            vendorID=0xFFF1,
            announcementReason=Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kSimpleAnnouncement,
            metadataForNode=None,
            endpoint=0
        )
        logger.info(f'Step #1.0 - cmd AnnounceOTAProvider: {cmd_announce}')

        resp_announce = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id_s1,  # DUT NodeID
            cmd=cmd_announce
        )
        logging.info(f'Step #1.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # Step #1.1 - Matcher for OTA records logs
        # Step #1.1.1 - UpdateState matcher: track "Downloading > Applying > Idle"
        # Step #1.1.2 - UpdateStateProgress matcher: Track progress reaching 99%
        # ------------------------------------------------------------------------------------

        logger.info("Step #1.1.1 - Create an accumulator for the UpdateState attribute")
        # UpdateState Accumulator
        accumulator_state = ClusterAttributeChangeAccumulator(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        logger.info("Step #1.1.2 - Create an accumulator for the UpdateStateProgress attribute")
        # UpdateProgress Accumulator
        accumulator_progress = ClusterAttributeChangeAccumulator(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress
        )

        # Start subscriptions for both accumulators in parallel
        await asyncio.gather(
            accumulator_state.start(
                dev_ctrl=controller,
                node_id=requestor_node_id_s1,
                endpoint=0,
                fabric_filtered=False,
                min_interval_sec=0.5,
                max_interval_sec=1,
                keepSubscriptions=True
            ),
            accumulator_progress.start(
                dev_ctrl=controller,
                node_id=requestor_node_id_s1,
                endpoint=0,
                fabric_filtered=False,
                min_interval_sec=2,
                max_interval_sec=3,
                keepSubscriptions=True
            )
        )

        # Track OTA UpdateState: observed states only once per type, and final idle
        observed_states = set()
        state_sequence = []  # Full OTA state flow
        final_idle_seen = False

        # Track OTA UpdateStateProgress
        progress_values = []

        def matcher_update_state(report):
            """
            Step #1.1.1 matcher function to track OTA UpdateState.
            Records each state (Downloading / Applying) once, then validates at Idle.
            """
            nonlocal observed_states, final_idle_seen, state_sequence
            val = report.value
            if val is None:
                return False

            # Only track Downloading (4) or Applying (5) once
            if val in [Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,  # 4
                       Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying]:  # 5
                if val not in observed_states:
                    observed_states.add(val)
                    state_sequence.append(val)
                    logger.info(f'1.1.1 - UpdateState recorded: {val}')
            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:  # 1
                if not final_idle_seen:  # log only once
                    final_idle_seen = True
                    state_sequence.append(val)
                    logger.info("1.1.1 - OTA UpdateState sequence complete, final state is Idle")
            # Return True only when Idle is reached
            return final_idle_seen

        # Create matcher object fro UpdateState
        matcher_update_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState transitions: Downloading > Applying > Idle",
            matcher=matcher_update_state
        )

        def matcher_progress(report):
            """
            Step #1.1.2 matcher function to track OTA progress ≥90%.
            Stores values in a set to avoid duplicates.
            """
            nonlocal progress_values
            val = getattr(report.value, "value", report.value)  # unwrap Nullable if needed

            # Normalize Nulls
            if val is None:
                val = "Null"

            # Only add if not already in list
            if val not in progress_values:
                progress_values.append(val)
                # logger.info(f'Step #1.1.2 - UpdateStateProgress recorded: {progress_values}')

            # Check UpdateStateProgress 99 progress
            if val == 99:
                logger.info("Step #1.1.2 - UpdateStateProgress reached 99")
                return True
            return False

        # Create matcher object for UpdateStateProgress
        matcher_progress_obj = AttributeMatcher.from_callable(
            description="Track OTA progress",
            matcher=matcher_progress
        )

        # ------------------------------------------------------------------------------------
        # Step #1.2 - Start a task to collect progress updates and validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (20 min)

            thread_state = threading.Thread(
                target=lambda: accumulator_state.await_all_expected_report_matches([matcher_update_state_obj], timeout_sec=1200.0)
            )
            thread_progress = threading.Thread(
                target=lambda: accumulator_progress.await_all_expected_report_matches([matcher_progress_obj], timeout_sec=1200.0)
            )

            # Start both threads
            thread_state.start()
            thread_progress.start()

            # Wait for both threads to complete
            thread_state.join()
            thread_progress.join()

            logger.info("Step #1.2 - Both UpdateState (1.1.1) and UpdateStateProgress (1.1.2) matchers have completed.")
        except Exception as e:
            logger.warning(f'OTA update encountered an error or timeout: {e}')
        finally:
            # Cancel both subscriptions and task
            await accumulator_state.cancel()
            await accumulator_progress.cancel()

        # ------------------------------------------------------------------------------------
        # Step #1.3 - Verify image transfer from TH/OTA-P to DUT is successful
        # ------------------------------------------------------------------------------------

        # Log the full sequence
        logger.info(f"Step #1.3 - Full OTA state sequence observed: {state_sequence}")
        logger.info(f"Step #1.3 - Progress values observed: {progress_values}")

        # Expected OTA flow
        expected_flow = [
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        ]

        # Assert the observed sequence matches expected
        msg = f"Observed OTA flow: {state_sequence}, Expected: {expected_flow}"
        asserts.assert_equal(state_sequence, expected_flow, msg=msg)

        # ------------------------------------------------------------------------------------
        # Step #1.4 - Close Provider_S1 Process  (CLEANUP!!!!)
        # ------------------------------------------------------------------------------------
        logger.info(f"Step #1.4 - Close Provider_S1 Process")
        await self.kill_ota_provider_process()
        await self.kill_ota_requestor_process()
        subprocess.run("rm -rf /tmp/chip_kvs*", shell=True)
        await asyncio.sleep(3)

        # Evict sessions
        controller.MarkSessionForEviction(provider_node_id_s1)
        await asyncio.sleep(2)
        controller.MarkSessionForEviction(requestor_node_id_s1)
        await asyncio.sleep(2)
        controller.DeleteAllSessionResumptionStorage()

        # Close BLE if open
        controller.CloseBLEConnection()
        await asyncio.sleep(2)

        self.step(2)
        # ------------------------------------------------------------------------------------
        # Prerequisite Step #2.0 - Setup Requestor_S2 and Provider_S2 as DUT
        # ------------------------------------------------------------------------------------
        # controller = self.default_controller
        # fabric_id = controller.fabricId
        # provider_ota_file_all_test = "firmware_requestor_v2min.ota"

        step_number_s2 = "[STEP_2]"
        requestor_node_id_s2 = self.dut_node_id     # Assigned on execution time
        requestor_discriminator_s2 = 1234
        requestor_passcode_s2 = 20202021
        requestor_port_s2 = 5542                    # Changed in each step
        provider_node_id_s2 = 3                     # Changed in each step
        provider_discriminator_s2 = 2222            # Changed in each step
        provider_setupPinCode_s2 = 20202021
        provider_port_s2 = 5543                     # Changed in each step
        fabric_id = controller.fabricId

        logger.info(f'Prerequisite #2.0 - Requestor_S2 (DUT), NodeID: {requestor_node_id_s2}, FabricId: {fabric_id}')

        # --- Step 2 ---
        requestor_proc_2, provider_proc_2 = await self.run_ota_step(
            step_number=step_number_s2,
            controller=self.default_controller,
            requestor_node_id=requestor_node_id_s2,
            requestor_discriminator=requestor_discriminator_s2,
            requestor_passcode=requestor_passcode_s2,
            requestor_port=requestor_port_s2,
            provider_node_id=provider_node_id_s2,
            provider_discriminator=provider_discriminator_s2,
            provider_setupPinCode=provider_setupPinCode_s2,
            provider_port=provider_port_s2,
            provider_ota_file=provider_ota_file_all_test,
            fabric_id=controller.fabricId,
            provider_queue="busy",   # optional
            provider_timeout=60         # optional
        )

        # # ------------------------------------------------------------------------------------
        # # Step #2.0 - Provider sends AnnounceOTAProvider command to Requestor
        # # ------------------------------------------------------------------------------------

        logger.info("Step #2.0 - Provider_S2 sends AnnounceOTAProvider command to DUT")
        cmd_announce = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id_s2,  # Provider_S2
            vendorID=0xFFF1,
            announcementReason=Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kSimpleAnnouncement,
            metadataForNode=None,
            endpoint=0
        )
        logger.info(f"Step #2.0 - cmd AnnounceOTAProvider: {cmd_announce}")

        resp_announce = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id_s2,  # DUT NodeID
            cmd=cmd_announce
        )
        logging.info(f"Step #2.0 - cmd AnnounceOTAProvider response: {resp_announce}.")

        # ------------------------------------------------------------------------------------
        # Step # 2.1 - Matcher for OTA records logs
        # ------------------------------------------------------------------------------------
        logger.info("Step #2.1 - Validate DUT respects minimum QueryImage interval after Busy response")
        t_start_query = time.time()

        logger.info("Step #2.1 - Create an accumulator for the UpdateState attribute")
        # UpdateState Accumulator
        accumulator_state = ClusterAttributeChangeAccumulator(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await accumulator_state.start(
            dev_ctrl=controller,
            node_id=requestor_node_id_s2,
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=True
        )

        # Track OTA UpdateState: observed states only once per type, and final idle
        observed_states = set()
        state_sequence = []  # Full OTA state flow
        final_downloading_seen = False

        def matcher_busy_state(report):
            """
            Step #2.1 matcher function to track OTA UpdateState.
            Records state Querying and after 120 seconds Downloading.
            """
            nonlocal observed_states, final_downloading_seen, state_sequence
            global t_start_downloading
            val = report.value

            if val is None:
                return False

            # Only track kQuerying (2) once
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery:  #
                if val not in observed_states:
                    observed_states.add(val)
                    state_sequence.append(val)
                    logger.info(f'Step #2.1 - UpdateState recorded: {val}')

            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:  # 4
                if not final_downloading_seen:  # log only once
                    final_downloading_seen = True
                    state_sequence.append(val)
                    t_start_downloading = time.time()
                    logger.info("Step #2.1 - OTA UpdateState sequence From Busy (Querying) to Downloading....")
            # Return True only when Downloading is reached
            return final_downloading_seen

        # Create matcher object fro UpdateState
        matcher_busy_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA Busy UpdateState transitions: Querying > Downloading",
            matcher=matcher_busy_state
        )

        # ------------------------------------------------------------------------------------
        # Step #2.2 - Start a task to collect progress updates and validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (20 min)
            await accumulator_state.await_all_expected_report_matches([matcher_busy_state_obj], timeout_sec=300.0)
            logger.info("Step #2.2 - UpdateState Busy matchers have completed.")
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel both subscriptions and task
            accumulator_state.cancel()

        # ------------------------------------------------------------------------------------
        # Step # 2.3 - Verify image transfer from TH/OTA-P to DUT is Busy
        # ------------------------------------------------------------------------------------
        logger.info(f"Step #2.3 - Full OTA state sequence observed: {state_sequence}")
        logger.info(f"Step #2.3 - Time Start as Busy: {t_start_query}, Time Ends as Busy {t_start_downloading}")
        delayed_action_time = t_start_downloading - t_start_query
        logger.info(f"Step #2.3 - Delay between Querying and Downloading: {delayed_action_time:.2f} s")

        asserts.assert_true(delayed_action_time >= 120,
                            f"Expected delay >= 120 seconds, but got {delayed_action_time:.2f} seconds")

        # ------------------------------------------------------------------------------------
        # Step # 2.4 - Close Requestor S_2 and Provider_S2 Process  (CLEANUP!!!!)
        # ------------------------------------------------------------------------------------
        logger.info(f"Step #2.4 - Close Requestor_S2 and Provider_S2 Process")
        await self.kill_ota_provider_process()
        await self.kill_ota_requestor_process(process_name="chip-ota-requestor-app")
        subprocess.run("rm -rf /tmp/chip_kvs*", shell=True)
        await asyncio.sleep(3)

        # Evict sessions
        controller.MarkSessionForEviction(provider_node_id_s2)
        await asyncio.sleep(2)
        controller.MarkSessionForEviction(requestor_node_id_s2)
        await asyncio.sleep(2)
        controller.DeleteAllSessionResumptionStorage()

        # Close BLE if open
        controller.CloseBLEConnection()
        await asyncio.sleep(2)


if __name__ == "__main__":
    default_matter_test_main()
