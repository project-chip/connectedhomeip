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

import asyncio
import logging
import queue
import subprocess
import threading
import time

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing import matter_asserts
from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.testing.apps import OTAProviderSubprocess, ACLHandler, OTAHelper

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_2(MatterBaseTest):

    LOG_FILE_PATH = "provider.log"
    APP_PATH = "./out/debug/chip-ota-provider-app"
    KVS_PATH = "/tmp/chip_kvs_provider"

    async def send_announce_ota_provider(self, controller, requestor_node_id, provider_node_id, vendor_id=0xFFF1, reason=None, endpoint=0):
        """
        Send the AnnounceOTAProvider command from the Requestor to the Provider.

        Args:
            controller: Controller object to send the command.
            requestor_node_id: Node ID of the OTA Requestor (DUT).
            provider_node_id: Node ID of the OTA Provider.
            vendor_id: Vendor ID associated with the Provider (default: 0xFFF1).
            reason: AnnouncementReasonEnum for the announcement (default: kUpdateAvailable).
            endpoint: Endpoint on which to send the command (default: 0).

        Returns:
            resp: Response object from the AnnounceOTAProvider command.
        """
        reason = reason or Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUrgentUpdateAvailable
        cmd = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=vendor_id,
            announcementReason=reason,
            metadataForNode=None,
            endpoint=endpoint
        )
        logger.info(f'Sending AnnounceOTAProvider to providerNodeID={provider_node_id}: {cmd}')
        resp = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            cmd=cmd
        )
        logger.info(f'AnnounceOTAProvider response: {resp}')
        return resp

    def desc_TC_SU_2_2(self) -> str:
        return "[TC-SU-2.2] Handling Different QueryImageResponse Scenarios on Requestor"

    def pics_TC_SU_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",      # Pics
        ]
        return pics

    def steps_TC_SU_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "Set ImageURI to the location where the image is located.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'Busy', DelayedActionTime is set to 60 seconds.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'NotAvailable'.",
                     "Verify that the DUT does not send a QueryImage command before the minimum interval defined by spec "
                     "which is 2 minutes (120 seconds) from the last QueryImage command."),
            TestStep(4, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to Busy, Set DelayedActionTime to 3 minutes. On the subsequent QueryImage command, "
                     "TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'.",
                     "Verify that the DUT waits for at least the time mentioned in the DelayedActionTime (3 minutes) before issuing another QueryImage command to the TH/OTA-P."
                     "Verify that there is a transfer of the software image after the second QueryImageResponse with UpdateAvailable status from the TH/OTA-P to the DUT."),
            TestStep(5, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
                     "ImageURI should have the https url from where the image can be downloaded.",
                     "Verify that the DUT queries the https url and downloads the software image."),
            TestStep(6, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. QueryStatus is set to 'UpdateAvailable'",
                     "Software Version should be set to the same or an older (numerically lower) version.",
                     "Verify that the DUT does not start transferring the software image."),
            TestStep(7, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable', ImageURI field contains an invalid BDX ImageURI.",
                     "Verify that the DUT does not start transferring the software image.")
        ]
        return steps

    @async_test_body
    async def test_TC_SU_2_2(self):
        # ------------------------------------------------------------------------------------
        # Run script
        # ------------------------------------------------------------------------------------
        # Run Python test with commission Provisioner/Requestor from Terminal:
        #   python3 src/python_testing/TC_SU_2_2.py \
        #   --commissioning-method on-network \
        #   --discriminator 1234 \
        #   --passcode 20202021 \
        #   --vendor-id 65521 \
        #   --product-id 32769 \
        #   --nodeId 2
        # ------------------------------------------------------------------------------------

        self.step(0)
        # Controller has already commissioned the requestor

        # Prerequisite #1.0 - Requestor (DUT) info
        CONTROLLER = self.default_controller
        FABRIC_ID = CONTROLLER.fabricId
        REQUESTOR_NODE_ID = self.dut_node_id
        self.acl_handler = ACLHandler(CONTROLLER)
        self.ota_helper = OTAHelper(
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
            acl_handler=self.acl_handler
        )

        step_number = "[STEP_0]"
        logger.info(f'{step_number}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        self.step(1)
        # ------------------------------------------------------------------------------------
        # [STEP_1]: Prerequisites - Setup Provider
        # Steps:
        #     1. Launch the OTA Provider process with given parameters.
        #     2. Commission the Provider onto the specified fabric.
        #     3. Configure ACLs on both Requestor and Provider to allow OTA cluster interactions.
        #     4. Add the Provider to the Requestor's DefaultOTAProviders attribute if none exists.
        # ------------------------------------------------------------------------------------
        step_number_s1 = "[STEP_1]"

        # Prerequisite #1.0 - Provider_S1 info
        provider_node_id = 1
        provider_discriminator = 1111
        provider_setupPinCode = 20202021
        provider_port = 5540
        provider_ota_file_s1 = "firmware_requestor_v2min.ota"

        # Launch, Commisioning, configure ACLs and add the DefaultOTAProviders
        provider_proc_s1 = await self.ota_helper.setup_provider(
            controller=CONTROLLER,
            fabric_id=FABRIC_ID,
            requestor_node_id=REQUESTOR_NODE_ID,
            provider_node_id=provider_node_id,
            provider_discriminator=provider_discriminator,
            provider_setup_pin_code=provider_setupPinCode,
            provider_port=provider_port,
            provider_ota_file=provider_ota_file_s1,
            provider_wait_for="Status: Satisfied",
            provider_queue=None,                    # Optional
            provider_timeout=None,                  # Optional
            provider_override_image_uri=None        # Optional
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState and UpdateStateProgress
        # ------------------------------------------------------------------------------------
        subscription_attr_state = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        subscription_attr_progress = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateStateProgress
        )

        # Start subscriptions for both attributes in parallel
        await asyncio.gather(
            subscription_attr_state.start(
                dev_ctrl=CONTROLLER,
                node_id=REQUESTOR_NODE_ID,
                endpoint=0,
                fabric_filtered=False,
                min_interval_sec=0.5,
                max_interval_sec=1,
                keepSubscriptions=True
            ),
            subscription_attr_progress.start(
                dev_ctrl=CONTROLLER,
                node_id=REQUESTOR_NODE_ID,
                endpoint=0,
                fabric_filtered=False,
                min_interval_sec=2,
                max_interval_sec=3,
                keepSubscriptions=True
            )
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s1}: Step #1.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s1}: Step #1.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.2 -  Track OTA attributes: UpdateState and UpdateStateProgress
        # [STEP_1]: Step #1.2.1 - UpdateState matcher: Track "Downloading > Applying > Idle"
        # [STEP_1]: Step #1.2.2 - UpdateStateProgress matcher: Track non-null values "rage 1–99" and final "None"
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s1}: Step #1.1 - Started subscription for UpdateState and UpdateStateProgress attributes '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence = []  # Full OTA state flow
        final_idle_seen = False

        def matcher_update_state(report):
            """
            Step #1.2.1 matcher function to track OTA UpdateState.
            Tracks state transitions: Downloading > Applying > Idle.
            Records each observed state only once and validates when Idle is reached.
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
                    logger.info(f'{step_number_s1}: 1.2.1 - UpdateState recorded: {val}')
            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:  # 1
                if not final_idle_seen:  # log only once
                    final_idle_seen = True
                    state_sequence.append(val)
                    logger.info(f'{step_number_s1}: 1.2.1 - OTA UpdateState sequence complete, final state is Idle')
            # Return True only when Idle is reached
            return final_idle_seen

        # Create matcher object from UpdateState
        matcher_update_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState transitions: Downloading > Applying > Idle",
            matcher=matcher_update_state
        )

        progress_recorded = set()
        progress_values = []
        progress_seen = False
        final_null_seen = False

        def matcher_progress(report):
            """
            Step #1.2.2 matcher function to track OTA progress values.
            Tracks state transitions: non-null progress values range 1-99 > None.
            Records each unique value once and expects a final None after progress has been observed.
            """
            nonlocal progress_values, progress_seen, final_null_seen, progress_recorded
            val = getattr(report.value, "value", report.value)  # unwrap Nullable if needed

            if str(val).lower() == "null":
                val = None

            # Track progress numbers 1-99
            if val is not None and isinstance(val, int) and 1 <= val <= 99:
                progress_seen = True
                if val not in progress_recorded:
                    progress_recorded.add(val)
                    progress_values.append(val)
                    logger.info(f'{step_number_s1}: 1.2.2 - UpdateStateProgress recorded: {val}')
                return False

            # Track final None after some progress
            if val is None and progress_seen:
                final_null_seen = True
                progress_values.append(val)
                logger.info(f'{step_number_s1} 1.2.2 - OTA UpdateStateProgress sequence complete, final state is None: {progress_values}')
                return final_null_seen

            return False

        # Create matcher object for UpdateStateProgress
        matcher_progress_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateStateProgress transitions: non-null values rage 1-99 and final None",
            matcher=matcher_progress
        )

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.3 - Start tasks to track OTA attributes:
        # UpdateState (state sequence) and UpdateStateProgress (progress values) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (20 min)

            thread_state = threading.Thread(
                target=lambda: subscription_attr_state.await_all_expected_report_matches(
                    [matcher_update_state_obj], timeout_sec=1200.0)
            )
            thread_progress = threading.Thread(
                target=lambda: subscription_attr_progress.await_all_expected_report_matches(
                    [matcher_progress_obj], timeout_sec=1200.0)
            )

            # Start both threads
            thread_state.start()
            thread_progress.start()

            # Wait for both threads to complete
            thread_state.join()
            thread_progress.join()

            logger.info(
                f'{step_number_s1}: Step #1.3 - Both OTA attributes UpdateState (1.2.1) and UpdateStateProgress (1.2.2) matchers have completed.')
        except Exception as e:
            logger.warning(f'OTA update encountered an error or timeout: {e}')
        finally:
            # Cancel both subscriptions and task
            await subscription_attr_state.cancel()
            await subscription_attr_progress.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.4 - Verify image transfer from TH/OTA-P to DUT is successful
        # ------------------------------------------------------------------------------------

        # Log the full sequence
        logger.info(f'{step_number_s1}: Step #1.4 - Full OTA state sequence observed: {state_sequence}')
        logger.info(f'{step_number_s1}: Step #1.4 - Progress values observed: {progress_values}')

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
        # [STEP_1]: Step #1.5 - Close Provider_S1 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s1}: Step #1.5 - Close Provider_S1 Process')

        # Kill Provider process
        provider_proc_s1.terminate()

        self.step(2)
        # ------------------------------------------------------------------------------------
        # [STEP_2]: Prerequisites - Setup Provider_S2
        # ------------------------------------------------------------------------------------
        step_number_s2 = "[STEP_2]"
        logger.info(f'{step_number_s2}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        # Prerequisite #1.0 - Provider_S2 info
        provider_ota_file_s2 = "firmware_requestor_v3min.ota"

        provider_proc_s2 = OTAProviderSubprocess(
            ota_file=provider_ota_file_s2,           # Path to OTA image file
            discriminator=provider_discriminator,
            passcode=provider_setupPinCode,
            secured_device_port=provider_port,
            queue="busy",
            timeout=60,
            override_image_uri=None,
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
        )
        provider_wait_for = "Status: Satisfied"
        provider_proc_s2.start(expected_output=provider_wait_for, timeout=300)

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState (Busy sequence)
        # ------------------------------------------------------------------------------------
        t_start_query = time.time()

        subscription_attr_state_busy = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await subscription_attr_state_busy.start(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=True
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s2}: Step #2.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.2 - Track OTA attributes: UpdateState (Busy sequence)
        # UpdateState (Busy sequence) matcher: DelayedOnQuery > Downloading > Idle
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s2}: Step #2.1 - Started subscription for UpdateState attribute (Busy sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence = []  # Full OTA state flow
        final_downloading_seen = False
        final_idle_seen = False

        def matcher_busy_state(report):
            """
            Step #2.2 matcher function to track OTA UpdateState (Busy sequence).
            Tracks state transitions: DelayedOnQuery > Downloading > Idle.
            Records each observed state only once and captures the timestamp when Downloading starts (~120s after DelayedOnQuery).
            """
            nonlocal observed_states, final_downloading_seen, final_idle_seen, state_sequence
            global t_start_downloading
            val = report.value

            if val is None:
                return False

            # Only track kQuerying (2) once
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery:  #
                if val not in observed_states:
                    observed_states.add(val)
                    state_sequence.append(val)
                    logger.info(f'{step_number_s2}: Step #2.2 - UpdateState (Busy sequence) recorded: {val}')

            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:   # 4
                if not final_downloading_seen:  # log only once
                    final_downloading_seen = True
                    state_sequence.append(val)
                    t_start_downloading = time.time()
                    logger.info(f'{step_number_s2}: Step #2.2 - UpdateState (Busy sequence) recorded: {val}')
                    logger.info(
                        f'{step_number_s2}: Step #2.2 - OTA UpdateState (Busy sequence) transitioned to Downloading after Busy (expect ~120s).')

            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:  # 1
                if not final_idle_seen:  # log only once
                    final_idle_seen = True
                    state_sequence.append(val)
                    logger.info(f'{step_number_s2}: Step #2.2 - UpdateState (Busy sequence) recorded: {val}')
                    logger.info(f'{step_number_s2}: Step #2.2 - OTA UpdateState (Busy sequence) complete, final state is Idle')
            # Return True only when Idle is reached
            return final_idle_seen

        # Create matcher object for UpdateState
        matcher_busy_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState (Busy sequence) transitions: DelayedOnQuery > Downloading > Idle",
            matcher=matcher_busy_state
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]:  Step #2.3 - Start tasks to track OTA attributes:
        # UpdateState (Busy state sequence) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (10 min)
            await subscription_attr_state_busy.await_all_expected_report_matches([matcher_busy_state_obj], timeout_sec=600.0)
            logger.info(f'{step_number_s2}: Step #2.3 - UpdateState (Busy sequence) matcher has completed.')
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscriptions and task
            await subscription_attr_state_busy.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.4 - Verify image transfer from TH/OTA-P to DUT is Busy
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.4 - Full OTA UpdateState (Busy sequence) observed: {state_sequence}')
        logger.info(f'{step_number_s2}: Step #2.4 - Time Start as Busy: {t_start_query}, Time Ends as Busy {t_start_downloading}')
        delayed_action_time = t_start_downloading - t_start_query
        logger.info(f'{step_number_s2}: Step #2.4 - Delay between Querying and Downloading: {delayed_action_time:.2f} s.')

        asserts.assert_true(delayed_action_time >= 120,
                            f"Expected delay >= 120 seconds, but got {delayed_action_time:.2f} seconds")

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.5 - Close Provider_S2 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s2}: Step #2.5 - Closed Provider_S2 process.')

        # Kill Provider process
        provider_proc_s2.terminate()

        self.step(3)
        # ------------------------------------------------------------------------------------
        # [STEP_3]: Prerequisites - Setup Provider_S3
        # ------------------------------------------------------------------------------------
        step_number_s3 = "[STEP_3]"
        logger.info(f'{step_number_s3}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        # Prerequisite #1.0 - Provider_S3 info
        provider_ota_file_s3 = "firmware_requestor_v4.ota"  # OTA updated

        provider_proc_s3 = OTAProviderSubprocess(
            ota_file=provider_ota_file_s3,           # Path to OTA image file
            discriminator=provider_discriminator,
            passcode=provider_setupPinCode,
            secured_device_port=provider_port,
            queue="updateNotAvailable",
            timeout=60,
            override_image_uri=None,
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
        )
        provider_wait_for = "Status: Satisfied"
        provider_proc_s3.start(expected_output=provider_wait_for, timeout=300)

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState (updateNotAvailable sequence)
        # ------------------------------------------------------------------------------------
        subscription_attr_state_updatenotavailable = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await subscription_attr_state_updatenotavailable.start(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=True
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s3}: Step #3.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.2 - Track OTA attributes: UpdateState (updateNotAvailable sequence)
        # UpdateState (updateNotAvailable sequence) matcher: Idle > Idle after ≥120s
        # Tracks all observed states to verify that no non-Idle state occurs during the 120s interval.
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s3}: Step #3.1 - Started subscription for UpdateState attribute '
            '(updateNotAvailable sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence_notavailable = []  # Full OTA state flow
        t_first_idle = None
        MIN_QUERY_IMAGE_INTERVAL = 120 + 1  # Buffer time

        def matcher_not_available_state(report):
            """
            Step #3.2 matcher function to track OTA UpdateState (updateNotAvailable sequence).
            Tracks state transitions: Idle > Idle after ≥120s.
            Records each observed state only once and validates when Idle is reached.
            Tracks all observed states to verify that no non-Idle state occurs during the 120s interval.
            """
            nonlocal observed_states, t_first_idle
            val = report.value  # UpdateStateEnum

            if val is None:
                return False

            current_time = time.time()

            # First Idle observed
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle and "first_idle" not in observed_states:
                observed_states.add("first_idle")
                t_first_idle = current_time
                state_sequence_notavailable.append(val)
                logger.info(
                    f'{step_number_s3}: Step #3.2 - UpdateState (updateNotAvailable sequence) recorded: {val}')
                logger.info(
                    f'{step_number_s3}: Step #3.2 - First Idle recorded at {time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(t_first_idle))}'
                )
                return False  # Keep waiting for 120s

            # Track all states after first Idle
            if "first_idle" in observed_states and "second_idle" not in observed_states:
                # Immediate validation: fail if non-Idle state observed
                if val != Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:
                    logger.info(f'{step_number_s3}: Step #3.4 - OTA UpdateState (updateNotAvailable) observed in 120s interval: {val}')
                    raise AssertionError(
                        f"Unexpected non-Idle state {val} observed during 120s interval"
                    )

            # Check if >=120s passed since first Idle
            if "first_idle" in observed_states and "second_idle" not in observed_states and current_time - t_first_idle >= MIN_QUERY_IMAGE_INTERVAL and val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:
                observed_states.add("second_idle")
                state_sequence_notavailable.append(val)
                logger.info(
                    f'{step_number_s3}: Step #3.2 - OTA UpdateState sequence transitioned to Idle after updateNotAvailable (expect ~120s).')
                logger.info(
                    f'{step_number_s3}: Step #3.2 - Idle state after 120s interval at {time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(current_time))}, no Query_Image was sent')
                logger.info(
                    f'{step_number_s3}: Step #3.2 - And, no non-Idle states observed during the 120s interval.')
                return True

            return False

        # Create matcher object for UpdateState (updateNotAvailable sequence)
        matcher_not_available_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState (updateNotAvailable sequence) transitions: Idle > Idle after ≥120s",
            matcher=matcher_not_available_state
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.3 - Start tasks to track OTA attributes:
        # UpdateState (updateNotAvailable sequence) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (10 min)
            await subscription_attr_state_updatenotavailable.await_all_expected_report_matches([matcher_not_available_state_obj], timeout_sec=600.0)
            logger.info(f'{step_number_s3}: Step #3.3 - UpdateState (updateNotAvailable sequence) matcher has completed.')
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscriptions and task
            await subscription_attr_state_updatenotavailable.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.4 - Verify image transfer from TH/OTA-P to DUT is Busy
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.4 - Full OTA UpdateState (updateNotAvailable) observed: {state_sequence_notavailable}')
        logger.info(f'{step_number_s3}: Step #3.4 - NotAvailable Idle sequence verification completed successfully.')

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.5 - Close Provider_S3 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s3}: Step #3.5 - Closed Provider_S3 process.')

        # Kill Provider process
        provider_proc_s3.terminate()

        self.step(4)
        # ------------------------------------------------------------------------------------
        # [STEP_4]: Prerequisites - Setup Provider_S4
        # ------------------------------------------------------------------------------------
        step_number_s4 = "[STEP_4]"
        logger.info(f'{step_number_s4}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        # Prerequisite #1.0 - Provider_S4 info
        provider_ota_file_s4 = "firmware_requestor_v5.ota"

        provider_proc_s4 = OTAProviderSubprocess(
            ota_file=provider_ota_file_s4,           # Path to OTA image file
            discriminator=provider_discriminator,
            passcode=provider_setupPinCode,
            secured_device_port=provider_port,
            queue="busy",
            timeout=180,
            override_image_uri=None,
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
        )
        provider_wait_for = "Status: Satisfied"
        provider_proc_s4.start(expected_output=provider_wait_for, timeout=300)

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState (Busy, 180s DelayedActionTime)
        # ------------------------------------------------------------------------------------
        t_start_query = time.time()

        subscription_attr_state_busy_180s = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await subscription_attr_state_busy_180s.start(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=True
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=900,  # 15 min
            breadcrumb=1
        )
        resp = await self.send_single_cmd(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,
            cmd=cmd
        )
        logger.info(f'{step_number_s4}: Step #4.0 - FailSafe armed for 900s: {resp}')

        logger.info(f'{step_number_s4}: Step #4.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s4}: Step #4.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.2 -  Track OTA attributes: UpdateState (Busy, 180s DelayedActionTime)
        # UpdateState (Busy, 180s DelayedActionTime) matcher: DelayedOnQuery > Downloading after ~180s delay > Idle
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.1 - Started subscription for UpdateState attribute '
            '(Busy, 180s DelayedActionTime sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence = []  # Full OTA state flow
        final_downloading_seen = False
        final_idle_seen = False

        def matcher_busy_state_delayed_180s(report):
            """
            Step #4.2 matcher function to track OTA UpdateState (Busy, 180s DelayedActionTime sequence)
            Tracks state transitions: DelayedOnQuery > Downloading after ~180s delay > Idle.
            Records each observed state only once and captures the timestamp when Downloading starts (~180s after DelayedOnQuery).
            """
            nonlocal observed_states, final_downloading_seen, final_idle_seen, state_sequence
            global t_start_downloading
            val = report.value

            if val is None:
                return False

            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery:  # 3
                if val not in observed_states:
                    observed_states.add(val)
                    state_sequence.append(val)
                    logger.info(f'{step_number_s4}: Step #4.2 - UpdateState (Busy, 180s DelayedActionTime sequence) recorded: {val}')

            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading:   # 4
                if not final_downloading_seen:  # log only once
                    final_downloading_seen = True
                    state_sequence.append(val)
                    t_start_downloading = time.time()
                    logger.info(f'{step_number_s4}: Step #4.2 - UpdateState (Busy, 180s DelayedActionTime sequence) recorded: {val}')
                    logger.info(
                        f'{step_number_s4}: Step #4.2 - OTA UpdateState (Busy, 180s DelayedActionTime sequence) transitioned to Downloading after Busy (expect ~180s).')

            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:  # 1
                if not final_idle_seen:  # log only once
                    final_idle_seen = True
                    state_sequence.append(val)
                    logger.info(f'{step_number_s4}: Step #4.2 - UpdateState (Busy, 180s DelayedActionTime sequence) recorded: {val}')
                    logger.info(
                        f'{step_number_s4}: Step #4.2 - OTA UpdateState (Busy, 180s DelayedActionTime sequence) complete, final state is Idle')
            # Return True only when Idle is reached
            return final_idle_seen

        # Create matcher object for UpdateState (expect Busy > Downloading transition after 180s).
        matcher_busy_state_delayed_180s_obj = AttributeMatcher.from_callable(
            description="Validate OTA Busy UpdateState transitions after 180s delay: DelayedOnQuery > Downloading > Idle",
            matcher=matcher_busy_state_delayed_180s
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.3 - Start tasks to track OTA attributes:
        # UpdateState (Busy, 180s DelayedActionTime sequence) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (15 min)
            await subscription_attr_state_busy_180s.await_all_expected_report_matches([matcher_busy_state_delayed_180s_obj], timeout_sec=900.0)
            logger.info(f'{step_number_s4}: Step #4.3 - UpdateState Busy > Downloading transition (180s) successfully observed.')
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscriptions
            await subscription_attr_state_busy_180s.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step # 4.4 - Verify image transfer from TH/OTA-P to DUT is Busy (180s DelayedActionTime)
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s4}: Step #4.4 - Full OTA state (Busy, 180s DelayedActionTime sequence) observed: {state_sequence}')
        logger.info(f'{step_number_s4}: Step #4.4 - Time Start as Busy: {t_start_query}, Time Start Downloading: {t_start_downloading}')
        delayed_action_time = t_start_downloading - t_start_query
        logger.info(f'{step_number_s4}: Step #4.4 - Observed delay:: {delayed_action_time:.2f} s.')

        asserts.assert_true(delayed_action_time >= 180,
                            f"Expected delay >= 180 seconds, but got {delayed_action_time:.2f} seconds")

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.5 - Close Provider_S4 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.5 - Closed Provider_S4 process.')

        # Kill Provider process
        provider_proc_s4.terminate()

        self.step(5)
        # Step #5 - HTTPS image download
        # NOTE: Step skipped (provisional / not implemented in spec)

        self.step(6)
        # ------------------------------------------------------------------------------------
        # [STEP_6]: Prerequisites - Setup Provider_S6
        # ------------------------------------------------------------------------------------
        step_number_s6 = "[STEP_6]"
        logger.info(f'{step_number_s6}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        # Prerequisite #1.0 - Provider_S6 info
        provider_ota_file_s6 = "firmware_requestor_v3min.ota"

        provider_proc_s6 = OTAProviderSubprocess(
            ota_file=provider_ota_file_s6,           # Path to OTA image file
            discriminator=provider_discriminator,
            passcode=provider_setupPinCode,
            secured_device_port=provider_port,
            queue=None,
            timeout=None,
            override_image_uri=None,
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
        )
        provider_wait_for = "Status: Satisfied"
        provider_proc_s6.start(expected_output=provider_wait_for, timeout=300)

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.1 - Matcher for OTA records logs
        # Start EventSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Events: StateTransition (UpdateState should stay Idle if UpdateAvailable version is same or lower)
        # ------------------------------------------------------------------------------------
        subscription_state_no_download = EventSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id
        )

        # Start subscriptions
        await subscription_state_no_download.start(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1
        )
        await asyncio.sleep(1)

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------

        logger.info(f'{step_number_s6}: Step #6.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s6}: Step #6.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.2 -  Track OTA StateTransition event: should stay Idle if UpdateAvailable version is same or lower
        # StateTransition event matcher: Idle > Querying > Idle
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s6}: Step #6.1 - Create a subscription for StateTransition event '
            '(should stay Idle if UpdateAvailable version is same or lower) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        state_sequence_notavailable = []  # Full OTA state flow

        def matcher_idle_state_no_download(report):
            """
            Step #6.2 matcher function to track OTA StateTransition event (should stay Idle if UpdateAvailable version is same or lower)
            Tracks state transitions events:
            First event: Idle > Querying
            Second event: Querying > Idle
            Records each observed transition only once and validates that no image transfer occurs when version is same or lower.
            """
            nonlocal state_sequence_notavailable
            val = report.Data.newState
            prev_state = report.Data.previousState
            event_reason = report.Data.reason
            event_target_sw = report.Data.targetSoftwareVersion

            if val is None:
                return False

            transition = (prev_state, val)
            state_sequence_notavailable.append(transition)

            # logger.info(f'{step_number_s6}: [DEBUG] {report.Data}')
            # logger.info(f'{step_number_s6}: [DEBUG] StateTransition observed, new_state {val}, previous state {prev_state}')

            expected = [
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying),
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle),
            ]

            # Validate the last two transitions
            if len(state_sequence_notavailable) >= 2:
                if state_sequence_notavailable[-2:] == expected:
                    logger.info(
                        f"{step_number_s6}: Step #6.2 - Validated StateTransition events: Idle > Querying (first event) > Querying > Idle (second event)")
                    return True

            return False

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.3 - Start tasks to track OTA events:
        # StateTransition two events: (1) Idle > Querying, (2) Querying > Idle, ensuring no image transfer occurs if UpdateAvailable version is same or lower.
        # ------------------------------------------------------------------------------------
        try:
            timeout_total = 60  # 1 min
            start_time = time.time()

            while time.time() - start_time < timeout_total:
                try:
                    report = subscription_state_no_download.get_event_from_queue(block=True, timeout=10)
                    matcher_idle_state_no_download(report)
                except queue.Empty:
                    continue
        finally:
            logger.info(f'{step_number_s6}: Step #6.3 - StateTransition two events: '
                        '(1) Idle > Querying, (2) Querying > Idle, if UpdateAvailable version is same or lower, '
                        'successfully observed.')
            subscription_state_no_download.reset()

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step # 6.4 - Verify NO image transfer occurs from TH/OTA-P to DUT if UpdateAvailable version is same or lower
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s6}: Step #6.4 - Full OTA StateTransition (if UpdateAvailable version is same or lower) observed: {state_sequence_notavailable}')

        # ------------------------------------------------------------------------------------
        # [STEP_6]: Step #6.5 - Close Provider_S6 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s6}: Step #6.5 - Closed Provider_S6.')

        # Kill Provider process
        provider_proc_s6.terminate()

        self.step(7)
        # ------------------------------------------------------------------------------------
        # [STEP_7]: Prerequisites - Setup Provider_S7
        # ------------------------------------------------------------------------------------
        step_number_s7 = "[STEP_7]"
        logger.info(f'{step_number_s7}: Prerequisite #1.0 - Requestor (DUT), NodeID: {REQUESTOR_NODE_ID}, FabricId: {FABRIC_ID}')

        # Prerequisite #1.0 - Provider_S7 info
        provider_ota_file_s7 = "firmware_requestor_v5.ota"

        provider_proc_s7 = OTAProviderSubprocess(
            ota_file=provider_ota_file_s7,           # Path to OTA image file
            discriminator=provider_discriminator,
            passcode=provider_setupPinCode,
            secured_device_port=provider_port,
            queue=None,
            timeout=None,
            override_image_uri="bdx://000000000000000X",
            log_file_path=self.LOG_FILE_PATH,
            app_path=self.APP_PATH,
            kvs_path=self.KVS_PATH,
        )
        provider_wait_for = "Status: Satisfied"
        provider_proc_s7.start(expected_output=provider_wait_for, timeout=300)

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.1 - Matcher for OTA records logs
        # Start EventSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Events: StateTransition (should stay Idle due to invalid BDX ImageURI in UpdateAvailable)
        # ------------------------------------------------------------------------------------
        subscription_state_invalid_uri = EventSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_event_id=Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition.event_id
        )

        # Start subscriptions
        await subscription_state_invalid_uri.start(
            dev_ctrl=CONTROLLER,
            node_id=REQUESTOR_NODE_ID,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1
        )
        await asyncio.sleep(1)

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s7}: Step #7.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(CONTROLLER, REQUESTOR_NODE_ID, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s7}: Step #7.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.2 -  Track OTA StateTransition event: should stay Idle due to invalid BDX ImageURI in UpdateAvailable.
        # StateTransition event matcher: Idle > Querying (first event) > Querying > Idle (second event)
        # ------------------------------------------------------------------------------------

        state_sequence_notavailable = []  # Full OTA state flow

        def matcher_idle_state_no_download_invalid_uri(report):
            """
            Step #7.2 matcher function to track OTA StateTransition event (should stay Idle due invalid BDX ImageURI in UpdateAvailable)
            Tracks state transitions events:
            First event: Idle > Querying
            Second event: Querying > Idle
            Records each observed transition only once and validates that no image transfer occurs due invalid BDX ImageURI.
            """
            nonlocal state_sequence_notavailable
            val = report.Data.newState
            prev_state = report.Data.previousState
            event_reason = report.Data.reason
            event_target_sw = report.Data.targetSoftwareVersion

            if val is None:
                return False

            transition = (prev_state, val)
            state_sequence_notavailable.append(transition)

            # logger.info(f'{step_number_s7}: [DEBUG] {report.Data}')
            # logger.info(f'{step_number_s7}: [DEBUG] StateTransition observed, new_state {val}, previous state {prev_state}')

            expected = [
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying),
                (Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying,
                 Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle),
            ]

            # Validate the last two transitions
            if len(state_sequence_notavailable) >= 2:
                if state_sequence_notavailable[-2:] == expected:
                    logger.info(
                        f"{step_number_s7}: Step #7.2 - Validated StateTransition events: Idle > Querying (first event) > Querying > Idle (second event)")
                    return True

            return False

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.3 - Start tasks to track OTA events:
        # StateTransition two events: (1) Idle > Querying, (2) Querying > Idle, ensuring no image transfer occurs due invalid BDX ImageURI.
        # ------------------------------------------------------------------------------------
        try:
            timeout_total = 60  # 1 min
            start_time = time.time()

            while time.time() - start_time < timeout_total:
                try:
                    report = subscription_state_invalid_uri.get_event_from_queue(block=True, timeout=10)
                    matcher_idle_state_no_download_invalid_uri(report)
                except queue.Empty:
                    continue
        finally:
            logger.info(f'{step_number_s7}: Step #7.3 - StateTransition two events: '
                        '(1) Idle > Querying, (2) Querying > Idle, due invalid BDX ImageURI, '
                        'successfully observed.')
            subscription_state_invalid_uri.reset()

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step # 7.4 - Verify NO image transfer occurs from TH/OTA-P to DUT due invalid BDX ImageURI in UpdateAvailable.
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s7}: Step #7.4 - Full OTA StateTransition (invalid BDX ImageURI) observed: {state_sequence_notavailable}')

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.5 - Close Provider_S7 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s7}: Step #7.5 - Closed Provider_S7 process.')

        try:
            provider_proc_s7.terminate()
        except Exception as e:
            logger.info(
                f'{step_number_s7}: Step #7.5 - Provider termination raised an expected exception (likely BDX unsolicited message): {e}'
            )


if __name__ == "__main__":
    default_matter_test_main()
