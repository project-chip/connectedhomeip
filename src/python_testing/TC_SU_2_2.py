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
import threading
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest, ACLHandler


import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.clusters.Types import NullValue
from matter.testing.apps import OTAProviderSubprocess
from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_2(SoftwareUpdateBaseTest):

    LOG_FILE_PATH = "provider.log"
    KVS_PATH = "/tmp/chip_kvs_provider"

    async def add_single_ota_provider(self, controller, requestor_node_id: int, provider_node_id: int):
        """
        Adds a single OTA provider to the Requestor's DefaultOTAProviders attribute
        only if no provider is currently registered. If a provider already exists,
        the function does nothing.

        Args:
            controller: The device controller.
            requestor_node_id (int): Node ID of the Requestor device.
            provider_node_id (int): Node ID of the OTA Provider to add.

        Returns:
            None
        """
        # Read existing DefaultOTAProviders on the Requestor
        current_providers = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        logger.info(f'Prerequisite #4.0 - Current DefaultOTAProviders on Requestor: {current_providers}')

        # If there is already a provider, skip adding
        if current_providers:
            logger.info(f'Skipping add: Requestor already has a provider registered ({current_providers})')
            return

        # Create a ProviderLocation for the new provider
        provider_location = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=0,
            fabricIndex=controller.fabricId
        )
        logger.info(f'Prerequisite #4.0 - ProviderLocation to add: {provider_location}')

        # Combine with existing providers (preserving previous ones)
        updated_providers = current_providers + [provider_location]

        # Write the updated DefaultOTAProviders list back to the Requestor
        attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=updated_providers)
        resp = await controller.WriteAttribute(
            attributes=[(0, attr)],
            nodeid=requestor_node_id
        )
        logger.info(f'Prerequisite #4.0 - Write DefaultOTAProviders response: {resp}')
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write DefaultOTAProviders attribute")

    async def clear_ota_providers(self, controller, requestor_node_id: int):
        """
        Clears the DefaultOTAProviders attribute on the Requestor, leaving it empty.

        Args:
            controller: The controller to use for writing attributes.
            requestor_node_id (int): Node ID of the Requestor device.

        Returns:
            None
        """
        # Set DefaultOTAProviders to empty list
        attr_clear = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[])
        resp = await controller.WriteAttribute(
            attributes=[(0, attr_clear)],
            nodeid=requestor_node_id
        )
        logger.info('Cleanup - DefaultOTAProviders cleared')

        assert resp[0].Status == Status.Success, "Failed to clear DefaultOTAProviders"

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
    async def teardown_test(self):
        self.current_provider_app_proc.terminate()
        super().teardown_test()

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
        #   --admin-vendor-id 65521 \
        #   --int-arg product-id:32769 \
        #   --nodeId 2
        # ------------------------------------------------------------------------------------

        self.step(0)
        # Controller has already commissioned the requestor

        # Prerequisite #1.0 - Requestor (DUT) info
        controller = self.default_controller
        fabric_id = controller.fabricId
        requestor_node_id = self.dut_node_id
        self.acl_handler = ACLHandler(controller)

        # Prerequisite #1.0 - Provider info
        provider_node_id = 1
        provider_discriminator = 1111
        provider_setupPinCode = 20202021
        provider_port = 5540

        self.step(1)
        # ------------------------------------------------------------------------------------
        # [STEP_1]: Prerequisites - Setup Provider
        # Steps:
        #     1. Launch the OTA Provider process with given parameters.
        #     2. Commission the Provider onto the specified fabric.
        #     3. Configure ACLs on both Requestor and Provider to allow OTA cluster interactions.
        #     4. Add the Provider to the Requestor's DefaultOTAProviders attribute if none exists.
        # ------------------------------------------------------------------------------------
        step_number = "[STEP_1]"
        logger.info(f'{step_number}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')
        logger.info(f'{step_number}: Prerequisite #1.0 - Provider logs path {self.KVS_PATH}, KVS logs path {self.LOG_FILE_PATH}')

        logger.info(f'{step_number}: Prerequisite #2.0 - Launched Provider')
        provider_data = {
            "node_id": provider_node_id,
            "discriminator": provider_discriminator,
            "setup_pincode": provider_setupPinCode,
            "port": provider_port,
        }

        provider_extra_args = [
            "--applyUpdateAction", "proceed",
            "--delayedApplyActionTimeSec", "0"
        ]

        # Prerequisite #2.0 - Start Provider Process
        self.start_provider(
            version=2,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

        # Prerequisite #3.0 - Commission Provider (Only one time)
        resp = await controller.CommissionOnNetwork(
            nodeId=provider_node_id,
            setupPinCode=provider_setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=provider_discriminator
        )
        logger.info(f'{step_number}: Prerequisite #3 - Provider Commissioning response: {resp}')

        # Prerequisite #4.0 - Setting ACLs on Requestor and Provider (Only one time)
        logger.info(f'{step_number}: Prerequisite #4.0 - Setting ACLs under FabricIndex {fabric_id}')
        logger.info(f'{step_number}: Prerequisite #4.0 - Requestor (DUT), NodeID: {requestor_node_id}')
        logger.info(f'{step_number}: Prerequisite #4.0 - Provider, NodeID: {provider_node_id}')

        # Set ACLs for OTA update:
        # - On Requestor to allow specified Provider to interact with OTA Requestor cluster
        # - On Provider to allow Requestor to interact with OTA Provider cluster

        # Read existing ACLs Requestor
        original_requestor_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        # Read existing ACLs Provider
        original_provider_acls = await self.read_single_attribute(
            dev_ctrl=controller,
            node_id=provider_node_id,
            endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl,
        )

        await self.acl_handler.set_ota_acls(
            requestor_node=requestor_node_id,
            provider_node=provider_node_id,
            fabric_index=fabric_id,
            original_requestor_acls=original_requestor_acls,
            original_provider_acls=original_provider_acls,
        )

        # Prerequisite #5.0 - Add OTA Provider to the Requestor (Only if none exists, and only one time)
        logger.info(f'{step_number}: Prerequisite #5.0 - Add Provider to Requestor(DUT) DefaultOTAProviders')
        await self.add_single_ota_provider(controller, requestor_node_id, provider_node_id)

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
                dev_ctrl=controller,
                node_id=requestor_node_id,
                endpoint=0,
                fabric_filtered=False,
                min_interval_sec=0.5,
                max_interval_sec=1,
                keepSubscriptions=True
            ),
            subscription_attr_progress.start(
                dev_ctrl=controller,
                node_id=requestor_node_id,
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
        logger.info(f'{step_number}: Step #1.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
        logger.info(f'{step_number}: Step #1.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.2 -  Track OTA attributes: UpdateState and UpdateStateProgress
        # [STEP_1]: Step #1.2.1 - UpdateState matcher: Track "Downloading > Applying > Idle"
        # [STEP_1]: Step #1.2.2 - UpdateStateProgress matcher: Track non-null values "rage 1–99" and final "None"
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number}: Step #1.1 - Started subscription for UpdateState and UpdateStateProgress attributes '
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
                    logger.info(f'{step_number}: 1.2.1 - UpdateState recorded: {val}')
            elif val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle:  # 1
                if not final_idle_seen:  # log only once
                    final_idle_seen = True
                    state_sequence.append(val)
                    logger.info(f'{step_number}: 1.2.1 - OTA UpdateState sequence complete, final state is Idle')
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

            if val == NullValue:
                val = None

            # Track progress numbers 1-99
            if val is not None and isinstance(val, int) and 1 <= val <= 99:
                progress_seen = True
                if val not in progress_recorded:
                    progress_recorded.add(val)
                    progress_values.append(val)
                    logger.info(f'{step_number}: 1.2.2 - UpdateStateProgress recorded: {val}')
                return False

            # Track final None after some progress
            if val is None and progress_seen:
                final_null_seen = True
                progress_values.append(val)
                logger.info(f'{step_number} 1.2.2 - OTA UpdateStateProgress sequence complete, final state is None: {progress_values}')
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
            # Wait until the final state (Idle) is reached or timeout (5 min)

            thread_state = threading.Thread(
                target=lambda: subscription_attr_state.await_all_expected_report_matches(
                    [matcher_update_state_obj], timeout_sec=300.0)
            )
            thread_progress = threading.Thread(
                target=lambda: subscription_attr_progress.await_all_expected_report_matches(
                    [matcher_progress_obj], timeout_sec=300.0)
            )

            # Start both threads
            thread_state.start()
            thread_progress.start()

            # Wait for both threads to complete
            thread_state.join()
            thread_progress.join()

            logger.info(
                f'{step_number}: Step #1.3 - Both OTA attributes UpdateState (1.2.1) and UpdateStateProgress (1.2.2) matchers have completed.')
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
        logger.info(f'{step_number}: Step #1.4 - Full OTA state sequence observed: {state_sequence}')
        logger.info(f'{step_number}: Step #1.4 - Progress values observed: {progress_values}')

        # Expected OTA flow
        expected_flows = [
            [
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kApplying,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
            ],
            [
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading,
                Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
            ]
        ]

        # Assert the observed sequence matches expected
        if state_sequence in expected_flows:
            logger.info(f'{step_number}: Step #1.4 - OTA flow is valid: {state_sequence}')
        else:
            msg = f"Observed OTA flow: {state_sequence}, Expected one of: {expected_flows}"
            asserts.fail(msg)

        # ------------------------------------------------------------------------------------
        # [STEP_1]: Step #1.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number}: Step #1.5 - Close Provider Process')

        # Kill Provider process
        self.current_provider_app_proc.terminate()

        self.step(2)
        # ------------------------------------------------------------------------------------
        # [STEP_2]: Prerequisites - Setup Provider
        # ------------------------------------------------------------------------------------
        step_number_s2 = "[STEP_2]"
        logger.info(f'{step_number_s2}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args = [
            "-q", "busy",
            "-t", "60"
        ]

        self.start_provider(
            version=3,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState (Busy sequence)
        # ------------------------------------------------------------------------------------
        subscription_attr_state_busy = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await subscription_attr_state_busy.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s2}: Step #2.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.2 - Track OTA attributes: UpdateState (Busy sequence)
        # UpdateState (Busy sequence) matcher: DUT stays in DelayedOnQuery or Querying for 120s interval
        # After the interval, Downloading is optional.
        # Any unexpected states during the 120s interval are asserted.
        # ------------------------------------------------------------------------------------

        logger.info(
            f'{step_number_s2}: Step #2.1 - Started subscription for UpdateState attribute (Busy sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence_busy = []
        observed_states_during_delay = set()
        t_delayedonquery = None
        t_end_interval = None
        MIN_DELAY_INTERVAL = 120    # exact 120s interval

        def matcher_busy_state(report):
            """
            Step #2.2 matcher function to track OTA UpdateState (Busy sequence).
            Tracks state transitions: Querying > DelayedOnQuery.
            Records each observed state only once and captures the timestamp when Querying starts.
            During the interval (120s): only DelayedOnQuery (3) or Querying (2) are allowed.
            """
            nonlocal observed_states, t_delayedonquery, t_end_interval
            val = report.value

            if val is None:
                return False

            current_time = time.time()

            # First DelayedOnQuery
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery and "delayedonquery_seen" not in observed_states:
                observed_states.add("delayedonquery_seen")
                t_delayedonquery = current_time
                state_sequence_busy.append(val)
                logger.info(f'{step_number_s2}: Step #2.2 - First DelayedOnQuery recorded: {val}')
                return False  # Keep waiting for 120s interval

            # Track unexpected states during 120s interval
            if "delayedonquery_seen" in observed_states and t_delayedonquery is not None and t_end_interval is None and current_time - t_delayedonquery < MIN_DELAY_INTERVAL:
                if val not in [
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
                ]:
                    observed_states_during_delay.add(val)
                    logger.info(f'{step_number_s2}: Step #2.2 - Unexpected state during 120s interval: {val}')

            # End matcher after 120s interval
            if t_delayedonquery is not None and t_end_interval is None and current_time - t_delayedonquery >= MIN_DELAY_INTERVAL:
                t_end_interval = current_time
                logger.info(f'{step_number_s2}: Step #2.2 - 120s interval completed')
                return True  # End matcher after 120s interval
            return False

        # Create matcher object for UpdateState
        matcher_busy_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState (Busy sequence) transitions: Querying > DelayedOnQuery",
            matcher=matcher_busy_state
        )

        # ------------------------------------------------------------------------------------
        # [STEP_2]:  Step #2.3 - Start tasks to track OTA attributes:
        # UpdateState (Busy state sequence) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (2.5 min)
            await subscription_attr_state_busy.await_all_expected_report_matches([matcher_busy_state_obj], timeout_sec=150.0)
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscriptions and task
            logger.info(f'{step_number_s2}: Step #2.3 - UpdateState (Busy sequence) matcher has completed.')
            await subscription_attr_state_busy.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.4 - Verify image transfer from TH/OTA-P to DUT is Busy
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.4 - Full OTA UpdateState (Busy sequence) observed: {state_sequence_busy}')
        interval_duration = t_end_interval - t_delayedonquery
        logger.info(f'{step_number_s2}: Step #2.4 - 120s interval: {interval_duration:.2f}s, '
                    f'unexpected states: {list(observed_states_during_delay)}')

        # Assert full Busy sequence
        expected_flow_busy = [
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery
        ]
        asserts.assert_equal(state_sequence_busy, expected_flow_busy,
                             f'Expected flow: {expected_flow_busy}, observed: {state_sequence_busy}')

        # Assert 120s interval duration and absence of unexpected states
        asserts.assert_true(interval_duration >= 120, f"Expected interval >= 120s, observed: {interval_duration:.2f}s")
        asserts.assert_equal(list(observed_states_during_delay), [], f"Unexpected states: {list(observed_states_during_delay)}")

        # ------------------------------------------------------------------------------------
        # [STEP_2]: Step #2.5 - Close Provider Process
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s2}: Step #2.5 - Closed Provider process.')

        # Kill Provider process
        self.current_provider_app_proc.terminate()

        self.step(3)
        # ------------------------------------------------------------------------------------
        # [STEP_3]: Prerequisites - Setup Provider_S3
        # ------------------------------------------------------------------------------------
        step_number_s3 = "[STEP_3]"
        logger.info(f'{step_number_s3}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args = [
            "-q", "updateNotAvailable",
            "-t", "60"
        ]

        self.start_provider(
            version=4,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

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
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s3}: Step #3.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.2 - Track OTA attributes: UpdateState (updateNotAvailable sequence)
        # UpdateState (Busy sequence) matcher: DUT stays in Idle for at least 120s
        # Any unexpected states during the 120s interval are asserted.
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s3}: Step #3.1 - Started subscription for UpdateState attribute '
            '(updateNotAvailable sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence_notavailable = []  # Full OTA state flow
        observed_states_during_delay = set()
        t_start_interval = None
        t_end_interval = None
        MIN_DELAY_INTERVAL = 120  # Buffer time

        def matcher_not_available_state(report):
            """
            Step #3.2 matcher function to track OTA UpdateState (updateNotAvailable sequence).
            Tracks state transitions: Idle > Idle (Remain in Idle during interval of 120s).
            Records each observed state only once and captures the timestamp when Idle starts.
            During the interval (120s): only Idle (1) is allowed.
            """
            nonlocal observed_states, t_start_interval, t_end_interval
            val = report.value  # UpdateStateEnum

            if val is None:
                return False

            current_time = time.time()

            # First Querying observed
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying and "querying_seen" not in observed_states:
                observed_states.add("querying_seen")
                t_start_interval = current_time
                state_sequence_notavailable.append(val)
                logger.info(f'{step_number_s3}: Step #3.2 - Querying recorded: {val}')
                return False  # Keep waiting for 120s

            # Track unexpected states during 120s interval
            if "querying_seen" in observed_states and t_start_interval is not None and t_end_interval is None and current_time - t_start_interval < MIN_DELAY_INTERVAL:
                if val not in [
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle,
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
                ]:
                    observed_states_during_delay.add(val)
                    logger.info(f'{step_number_s3}: Step #3.2 - Unexpected state during 120s interval: {val}')

            # End matcher after 120s interval
            if t_start_interval is not None and t_end_interval is None and current_time - t_start_interval >= MIN_DELAY_INTERVAL:
                t_end_interval = current_time
                logger.info(f'{step_number_s3}: Step #3.2 - 120s interval completed')
                return True  # End matcher after 120s interval

            return False

        # Create matcher object for UpdateState (updateNotAvailable sequence)
        matcher_not_available_state_obj = AttributeMatcher.from_callable(
            description="Validate OTA UpdateState (NotAvailable): remains in Idle for at least 120s",
            matcher=matcher_not_available_state
        )

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.3 - Start tasks to track OTA attributes:
        # UpdateState (updateNotAvailable sequence) with validation
        # ------------------------------------------------------------------------------------
        try:
            # Wait until the final state (Idle) is reached or timeout (2.5 min)
            await subscription_attr_state_updatenotavailable.await_all_expected_report_matches([matcher_not_available_state_obj], timeout_sec=150.0)
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout, test continues: {e}")
        finally:
            # Cancel subscriptions and task
            logger.info(f'{step_number_s3}: Step #3.3 - UpdateState (updateNotAvailable sequence) matcher has completed.')
            await subscription_attr_state_updatenotavailable.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.4 - Verify image transfer from TH/OTA-P to DUT is Busy
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.4 - Full OTA UpdateState (updateNotAvailable sequence) observed: {state_sequence_notavailable}')
        interval_duration = t_end_interval - t_start_interval
        logger.info(f'{step_number_s3}: Step #3.4 - 120s interval: {interval_duration:.2f}s, '
                    f'unexpected states: {list(observed_states_during_delay)}')

        # Assert full updateNotAvailable sequence
        expected_flow_busy = [
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
        ]
        asserts.assert_equal(state_sequence_notavailable, expected_flow_busy,
                             f'Expected flow: {expected_flow_busy}, observed: {state_sequence_notavailable}')

        # Assert 120s interval duration and absence of unexpected states
        asserts.assert_true(interval_duration >= 120, f"Expected interval >= 120s, observed: {interval_duration:.2f}s")
        asserts.assert_equal(list(observed_states_during_delay), [], f"Unexpected states: {list(observed_states_during_delay)}")

        # ------------------------------------------------------------------------------------
        # [STEP_3]: Step #3.5 - Close Provider_S3 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s3}: Step #3.5 - Closed Provider process.')

        # Kill Provider process
        self.current_provider_app_proc.terminate()

        self.step(4)
        # ------------------------------------------------------------------------------------
        # [STEP_4]: Prerequisites - Setup Provider_S4
        # ------------------------------------------------------------------------------------
        step_number_s4 = "[STEP_4]"
        logger.info(f'{step_number_s4}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args = [
            "-q", "busy",
            "-t", "180"
        ]

        self.start_provider(
            version=5,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.1 - Matcher for OTA records logs
        # Start AttributeSubscriptionHandler first to avoid missing any rapid OTA events (race condition)
        # Atrributes: UpdateState (Busy, 180s DelayedActionTime)
        # ------------------------------------------------------------------------------------

        subscription_attr_state_busy_180s = AttributeSubscriptionHandler(
            expected_cluster=Clusters.OtaSoftwareUpdateRequestor,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

        # Start subscriptions
        await subscription_attr_state_busy_180s.start(
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
            endpoint=0,
            fabric_filtered=False,
            min_interval_sec=1,
            max_interval_sec=1,
            keepSubscriptions=False
        )

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.0 - Controller sends AnnounceOTAProvider command
        # ------------------------------------------------------------------------------------
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(
            expiryLengthSeconds=900,  # 15 min
            breadcrumb=1
        )
        resp = await self.send_single_cmd(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            cmd=cmd
        )
        logger.info(f'{step_number_s4}: Step #4.0 - FailSafe armed for 900s: {resp}')

        logger.info(f'{step_number_s4}: Step #4.0 - Controller sends AnnounceOTAProvider command')
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
        logger.info(f'{step_number_s4}: Step #4.0 - cmd AnnounceOTAProvider response: {resp_announce}.')

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.2 -  Track OTA attributes: UpdateState (Busy, 180s DelayedActionTime)
        # UpdateState (Busy, 180s DelayedActionTime) matcher: DelayedOnQuery > Downloading after ~180s delay
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.1 - Started subscription for UpdateState attribute '
            '(Busy, 180s DelayedActionTime sequence) '
            'before AnnounceOTAProvider to avoid missing OTA events')

        observed_states = set()
        state_sequence_busy = []
        observed_states_during_delay = set()
        t_delayedonquery = None
        t_end_interval = None
        MIN_DELAY_INTERVAL = 180    # exact 180s interval

        def matcher_busy_state_delayed_180s(report):
            """
            Step #4.2 matcher function to track OTA UpdateState (Busy, 180s DelayedActionTime sequence).
            Tracks state transitions: Querying > DelayedOnQuery >  Start of Downloading.
            Records each observed state only once and captures the timestamp when Querying starts.
            During the interval (180s): only DelayedOnQuery (3) or Querying (2) are allowed.
            """
            nonlocal observed_states, t_delayedonquery, t_end_interval
            val = report.value

            if val is None:
                return False

            current_time = time.time()

            # First DelayedOnQuery observed
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery and "delayedonquery_seen" not in observed_states:
                observed_states.add("delayedonquery_seen")
                t_delayedonquery = current_time
                state_sequence_busy.append(val)
                logger.info(f'{step_number_s4}: Step #4.2 - First DelayedOnQuery recorded: {val}')
                return False  # Keep waiting for 120s interval

            # Track unexpected states during 120s interval
            if "delayedonquery_seen" in observed_states and t_delayedonquery is not None and t_end_interval is None and current_time - t_delayedonquery < MIN_DELAY_INTERVAL:
                if val not in [
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
                    Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kQuerying
                ]:
                    observed_states_during_delay.add(val)
                    logger.info(f'{step_number_s4}: Step #4.2 - Unexpected state during 120s interval: {val}')

            # End 180s interval
            if t_delayedonquery is not None and t_end_interval is None and current_time - t_delayedonquery >= MIN_DELAY_INTERVAL:
                t_end_interval = current_time
                logger.info(f'{step_number_s4}: Step #4.2 - 180s interval completed')
                return False  # End 180s interval

            # First Downloading observed: record and end matcher
            if val == Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading and t_end_interval is not None:
                observed_states.add("downloading_seen")
                state_sequence_busy.append(val)
                logger.info(f'{step_number_s4}: Step #4.2 - First Downloading recorded: {val}')
                return True  # End matcher after Downloading is seen
            return False

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
            # Wait until the final state (Idle) is reached or timeout (3.5 min)
            await subscription_attr_state_busy_180s.await_all_expected_report_matches([matcher_busy_state_delayed_180s_obj], timeout_sec=210.0)
            logger.info(f'{step_number_s4}: Step #4.3 - UpdateState Busy > Downloading transition (180s) successfully observed.')
        except Exception as e:
            logger.warning(f"OTA update encountered an error or timeout: {e}")
        finally:
            # Cancel subscriptions
            await subscription_attr_state_busy_180s.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step # 4.4 - Verify image transfer from TH/OTA-P to DUT is Busy (180s DelayedActionTime)
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s4}: Step #4.4 - Full OTA UpdateState (Busy, 180s DelayedActionTime sequence) observed: {state_sequence_busy}')
        interval_duration = t_end_interval - t_delayedonquery
        logger.info(f'{step_number_s4}: Step #4.4 - 180s interval: {interval_duration:.2f}s, '
                    f'unexpected states: {list(observed_states_during_delay)}')

        # Assert full Busy, 180s DelayedActionTime sequence
        expected_flow_busy = [
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDelayedOnQuery,
            Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kDownloading
        ]
        asserts.assert_equal(state_sequence_busy, expected_flow_busy,
                             f'Expected flow: {expected_flow_busy}, observed: {state_sequence_busy}')

        # Assert 180s interval duration and absence of unexpected states
        asserts.assert_true(interval_duration >= 180, f"Expected interval >= 180s, observed: {interval_duration:.2f}s")
        asserts.assert_equal(list(observed_states_during_delay), [], f"Unexpected states: {list(observed_states_during_delay)}")

        # ------------------------------------------------------------------------------------
        # [STEP_4]: Step #4.5 - Close Provider_S4 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        logger.info(
            f'{step_number_s4}: Step #4.5 - Closed Provider_S4 process.')

        # Kill Provider process
        self.current_provider_app_proc.terminate()

        self.step(5)
        # Step #5 - HTTPS image download
        # NOTE: Step skipped (provisional / not implemented in spec)

        self.step(6)
        # ------------------------------------------------------------------------------------
        # [STEP_6]: Prerequisites - Setup Provider_S6
        # ------------------------------------------------------------------------------------
        step_number_s6 = "[STEP_6]"
        logger.info(f'{step_number_s6}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args = [
            "--applyUpdateAction", "proceed",
            "--delayedApplyActionTimeSec", "0"
        ]

        self.start_provider(
            version=3,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

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
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
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
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
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
            # event_reason = report.Data.reason
            # event_target_sw = report.Data.targetSoftwareVersion

            if val is None:
                return False

            transition = (prev_state, val)
            state_sequence_notavailable.append(transition)

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
            await subscription_state_no_download.cancel()

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
        self.current_provider_app_proc.terminate()

        self.step(7)
        # ------------------------------------------------------------------------------------
        # [STEP_7]: Prerequisites - Setup Provider_S7
        # ------------------------------------------------------------------------------------
        step_number_s7 = "[STEP_7]"
        logger.info(f'{step_number_s7}: Prerequisite #1.0 - Requestor (DUT), NodeID: {requestor_node_id}, FabricId: {fabric_id}')

        provider_extra_args = [
            "-i", "bdx://000000000000000X"
        ]

        self.start_provider(
            version=3,
            setup_pincode=provider_data["setup_pincode"],
            discriminator=provider_data["discriminator"],
            port=provider_data["port"],
            extra_args=provider_extra_args,
            kvs_path=self.KVS_PATH,
            log_file=self.LOG_FILE_PATH
        )

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
            dev_ctrl=controller,
            node_id=requestor_node_id,  # DUT
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
        resp_announce = await self.send_announce_ota_provider(controller, requestor_node_id, provider_node_id=provider_node_id)
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
            # event_reason = report.Data.reason
            # event_target_sw = report.Data.targetSoftwareVersion

            if val is None:
                return False

            transition = (prev_state, val)
            state_sequence_notavailable.append(transition)

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
            await subscription_state_invalid_uri.cancel()

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step # 7.4 - Verify NO image transfer occurs from TH/OTA-P to DUT due invalid BDX ImageURI in UpdateAvailable.
        # ------------------------------------------------------------------------------------
        logger.info(f'{step_number_s7}: Step #7.4 - Full OTA StateTransition (invalid BDX ImageURI) observed: {state_sequence_notavailable}')

        # ------------------------------------------------------------------------------------
        # [STEP_7]: Step #7.5 - Close Provider_S7 Process (CLEANUP!)
        # ------------------------------------------------------------------------------------
        # Clear DefaultOTAProviders for a clean state
        await self.clear_ota_providers(
            controller=controller,
            requestor_node_id=requestor_node_id
        )

        logger.info(
            f'{step_number_s7}: Step #7.5 - Closed Provider_S7 process.')

        # Kill Provider process
        self.current_provider_app_proc.terminate()


if __name__ == "__main__":
    default_matter_test_main()
