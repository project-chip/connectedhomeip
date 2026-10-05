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


import asyncio
import logging
import struct
import subprocess
import tempfile
import time
from os import path
from time import monotonic, sleep

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.apps import OtaImagePath, OTAProviderSubprocess
from matter.testing.matter_testing import MatterBaseTest
from matter.tlv import TLVReader

# Type aliases for AccessControl cluster types
AccessControlCluster = Clusters.AccessControl
AccessControlEntryStruct = AccessControlCluster.Structs.AccessControlEntryStruct
AccessControlTargetStruct = AccessControlCluster.Structs.AccessControlTargetStruct
AccessControlEntryPrivilegeEnum = AccessControlCluster.Enums.AccessControlEntryPrivilegeEnum
AccessControlEntryAuthModeEnum = AccessControlCluster.Enums.AccessControlEntryAuthModeEnum

log = logging.getLogger(__name__)

# Upper bound for establishing a subscription to the DUT. On a reachable DUT this takes well
# under a second; the bound exists because AttributeSubscriptionHandler.start() goes through
# ReadAttribute(), whose autoResubscribe parameter defaults to True, so a DUT that has dropped
# off the network makes it retry establishment indefinitely and the step stalls for the rest of
# the test budget. Every step subscribes only after the DUT has already answered the controller,
# so a minute is generous for the establishment itself.
SUBSCRIPTION_START_TIMEOUT_SEC = 60
# Bound for the reachability probe that runs when a subscription fails to establish. Only needs
# to answer "does the DUT still talk to this controller at all", so it is kept short.
SUBSCRIPTION_PROBE_TIMEOUT_MS = 5000


class SoftwareUpdateBaseTest(MatterBaseTest):
    """This is the base test class for SoftwareUpdate Test Cases"""
    current_provider_app_proc: OTAProviderSubprocess | None = None
    provider_app_path: str | None = None
    _test_budget_deadline: float | None = None

    def start_test_budget_clock(self, safety_margin_sec: float = 30.0) -> None:
        """Record the overall time budget of the running test body.

        Call at the top of the test body.  The budget is the same value the framework
        passes to ``asyncio.wait_for`` around the test body (``--timeout`` from the
        command line, or ``default_timeout``), minus ``safety_margin_sec`` so that waits
        sized via :meth:`remaining_test_budget_sec` fail with a clear assertion inside
        the step instead of being cancelled opaquely by ``asyncio.wait_for``.

        Together with :meth:`remaining_test_budget_sec` this lets device-dependent waits
        (OTA download, firmware apply, BDX recovery) share a single operator-set budget
        instead of hard-coded per-step timeouts that cannot be known for a real DUT.
        """
        total_sec = self.matter_test_config.timeout or self.default_timeout
        self._test_budget_deadline = monotonic() + total_sec - safety_margin_sec
        log.info("Test budget clock started: %.0fs total, %.0fs safety margin", total_sec, safety_margin_sec)

    def remaining_test_budget_sec(self, reserve_sec: float = 0.0, minimum_sec: float = 30.0) -> float:
        """Return the remaining test time budget available for the next wait.

        Args:
            reserve_sec: Time to keep aside for waits that are still ahead and whose
                duration is known up front (e.g. spec-mandated minimum query intervals).
            minimum_sec: Floor for the returned value so a wait never receives a
                zero/negative timeout — exhausting the budget then fails inside the wait
                with a descriptive message rather than instantly.
        """
        if self._test_budget_deadline is None:
            asserts.fail("remaining_test_budget_sec() called before start_test_budget_clock()")
        return max(minimum_sec, self._test_budget_deadline - monotonic() - reserve_sec)
    
    async def _start_subscription_bounded(self, subscription, step_name: str, **start_kwargs) -> None:
        """Start ``subscription``, failing the step if it is not established in time.

        Subscription establishment has no timeout of its own: ReadAttribute() defaults to
        autoResubscribe=True, so a DUT that has dropped off the network is retried forever and
        the step blocks until the whole test budget is gone — reported, if at all, as an opaque
        overall timeout hours later.

        On timeout the DUT is probed with a plain GetConnectedDevice before the step is failed,
        because "no subscription" has two causes that call for opposite reactions: a DUT that has
        left the network, versus a DUT that still answers while only the subscription fails (an
        earlier subscription of this run retrying in the background competes for CASE sessions to
        the same node, and cannot be cancelled once the DUT stops acknowledging). Reporting which
        one it is keeps the failure from being read as a verdict on the DUT.
        """
        try:
            await asyncio.wait_for(subscription.start(**start_kwargs), timeout=SUBSCRIPTION_START_TIMEOUT_SEC)
            return
        except TimeoutError:
            log.info('%s: no subscription after %ss; probing whether the DUT answers at all.',
                        step_name, SUBSCRIPTION_START_TIMEOUT_SEC)

        controller = start_kwargs.get('dev_ctrl', self.default_controller)
        node_id = start_kwargs.get('node_id', self.dut_node_id)
        probe_error = None
        try:
            # Bounded twice: GetConnectedDevice honours timeoutMs, and wait_for keeps a wedged
            # probe from stalling the very failure it exists to explain.
            await asyncio.wait_for(
                controller.GetConnectedDevice(node_id, allowPASE=False, timeoutMs=SUBSCRIPTION_PROBE_TIMEOUT_MS),
                timeout=SUBSCRIPTION_PROBE_TIMEOUT_MS / 1000 + 5)
        except (TimeoutError, ChipDeviceCtrl.ChipStackError) as e:
            probe_error = e

        preamble = (f"{step_name}: could not establish a subscription to the DUT within "
                    f"{SUBSCRIPTION_START_TIMEOUT_SEC}s")
        no_verdict = ("No verdict: without a subscription this run observed nothing, so the DUT's OTA "
                      "behaviour was neither proved nor disproved.")
        if probe_error is None:
            asserts.fail(
                f"{preamble}, yet the DUT still answers the controller (GetConnectedDevice succeeded).\n"
                f"{no_verdict} The DUT is not the suspect here — the controller's subscription machinery "
                "is, most likely an earlier subscription of this run still retrying in the background.\n"
                "ACTION: re-run the test. If this recurs at the same step, investigate the controller "
                "side rather than the DUT.")
        asserts.fail(
            f"{preamble}, and the DUT does not answer the controller at all "
            f"(GetConnectedDevice failed: {probe_error}).\n"
            f"{no_verdict}\n"
            "ACTION: check that the DUT is powered and on the network. A one-off drop-out is an "
            "environment problem — re-run the test. A DUT that repeatedly stops answering at this "
            "point is itself the defect and must not be retried away.")

    async def _announce_until_provider_queried(self, controller, provider_node_id: int, requestor_node_id: int,
                                               timeout_sec: float, step_name: str,
                                               retry_interval_sec: float = 60.0) -> None:
        """Announce the provider, repeating until the provider reports receiving a QueryImage.

        One AnnounceOTAProvider does not guarantee the DUT queries this provider. The requestor
        drops an announce that arrives while UpdateState is not kIdle, and a query it does send
        can still die in a CASE session left over from the previous provider process — every step
        replaces that process — costing a message-layer timeout before the DUT tries again.
        Repeating the announce keeps the step moving in both cases instead of leaving it at the
        mercy of the DUT's own retry policy: DefaultOTARequestorDriver grants a single automatic
        retry per invalid session (kMaxInvalidSessionRetries) and the spec mandates none at all.

        Waiting on the provider's own report of the query, rather than on a DUT state change, is
        what makes the loop terminate for the right reason: it is the only signal that separates a
        query this provider answered from one that never arrived.

        The caller must arm the provider's matcher with PROVIDER_QUERY_RECEIVED_LOG right after
        starting it and before calling this, so a receipt landing between the announce and the
        wait cannot be missed.

        Announces stop as soon as the provider reports the query, so this never perturbs a
        timing guard that follows: any announce it sends is either dropped by a busy DUT or
        answered by the query that ends the loop.
        """
        proc = self.current_provider_app_proc
        t_start = time.time()
        attempt = 0

        while True:
            remaining = timeout_sec - (time.time() - t_start)
            if remaining <= 0:
                asserts.fail(f"{step_name}: the provider received no QueryImage within {timeout_sec:.0f}s "
                             f"of the first announce ({attempt} sent); the DUT never reached it.")
            attempt += 1
            try:
                await self.announce_ota_provider(
                    controller, provider_node_id=provider_node_id, requestor_node_id=requestor_node_id)
                log.info('%s: AnnounceOTAProvider sent (attempt %d).', step_name, attempt)
            except (TimeoutError, ChipDeviceCtrl.ChipStackError) as e:
                # Expected while the DUT is recovering from an aborted transfer: its session to
                # the controller can drop (e.g. under Wi-Fi power-save). Retry on the next pass.
                log.info('%s: AnnounceOTAProvider failed (DUT transiently unreachable): %s; will retry.',
                            step_name, e)

            if proc.wait_for_output(timeout=min(retry_interval_sec, remaining)):
                log.info('%s: provider received a QueryImage %.0fs after the first announce.',
                            step_name, time.time() - t_start)
                return

            log.info('%s: no QueryImage reached the provider in %.0fs (elapsed %.0fs / %.0fs); re-announcing.',
                        step_name, retry_interval_sec, time.time() - t_start, timeout_sec)

    async def _wait_until_idle_before_announce(self, controller, requestor_node_id: int, subscription,
                                               timeout_sec: float, step_name: str) -> None:
        """Block until the DUT's UpdateState is kIdle, so the AnnounceOTAProvider that follows is
        acted on instead of dropped.

        The requestor silently ignores an AnnounceOTAProvider that arrives while UpdateState is
        not kIdle ("State is not kIdle, ignoring the AnnounceOTAProviders"), which would leave the
        step depending on whatever retry the DUT runs on its own rather than on the announce.

        The current value is READ rather than awaited from ``subscription``:
        AttributeSubscriptionHandler.start() registers its callback only after ReadAttribute() has
        already consumed the priming report, so nothing is enqueued for a DUT that is idle when the
        subscription starts — and an unchanged attribute is never reported again, so a pure wait
        would block until it times out. The subscription is used only for the case where the DUT
        still has to transition, which does produce a report.
        """
        kIdle = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum.kIdle
        state = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=0,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState)
        if state == kIdle:
            log.info('%s: DUT is already idle — the announce will be acted on.', step_name)
            return

        log.info('%s: DUT is in %s; waiting up to %.0fs for it to reach kIdle before announcing.',
                    step_name, state, timeout_sec)
        subscription.await_first_value_asserting_no_forbidden(
            target_value=kIdle,
            forbidden_values=set(),
            timeout_sec=timeout_sec,
            expected_attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState,
        )
        log.info('%s: DUT reached kIdle — the announce will be acted on.', step_name)


    def start_provider(self,
                       provider_app_path: str = "",
                       ota_image_path: str = "",
                       setup_pincode: int = 20202021,
                       discriminator: int = 1234,
                       port: int = 5541,
                       storage_dir='/tmp',
                       extra_args: list = [],
                       kvs_path: str | None = None,
                       log_file: str | None = None, expected_output: str = "Server initialization complete",
                       timeout: int = 30):
        """Start the provider process using the provided configuration.

        Args:
            provider_app_path (str): Path of Requestor app to load.
            ota_image_path (str): Ota image to load within the provider.
            setup_pincode (int, optional): Setup pincode for the provider process. Defaults to 20202021.
            discriminator (int, optional): Discriminator for the provider process. Defaults to 1234.
            port (int, optional): Port for the provider process. Defaults to 5541.
            storage_dir (str, optional): Storage dir for the provider process. Defaults to '/tmp'.
            extra_args (list, optional): Extra args to send to the provider process. Defaults to [].
            kvs_path(str): Str of the path for the kvs path, if not will use temp file.
            log_file (Optional[str], optional): Destination for the app process logs. Defaults to None.
            expected_output (str): Expected string to see after a default timeout. Defaults to "Server initialization complete".
            timeout (int): Timeout to wait for the expected output. Defaults to 10 seconds
        """
        log.info("Launching provider app with with ota image %s", ota_image_path)
        # Image to launch
        self.provider_app_path = provider_app_path
        if not path.exists(provider_app_path):
            raise FileNotFoundError(f"Provider app not found {provider_app_path}")

        if not path.exists(ota_image_path):
            raise FileNotFoundError(f"Ota image provided does not exists {ota_image_path}")
        ota_image_path = OtaImagePath(path=ota_image_path)
        # Ideally we send the logs to a fixed location to avoid conflicts

        if log_file is None:
            # Assign the file descriptor to log_file
            log_file = tempfile.NamedTemporaryFile(  # noqa: SIM115
                dir=storage_dir, prefix='provider_', suffix='.log', mode='ab')
            log.info("Writing Provider logs at :%s", log_file.name)
        else:
            log.info("Writing Provider logs at : %s", log_file)
        # Launch the Provider subprocess using the Wrapper
        proc = OTAProviderSubprocess(
            provider_app_path,
            storage_dir=storage_dir,
            port=port,
            discriminator=discriminator,
            passcode=setup_pincode,
            ota_source=ota_image_path,
            extra_args=extra_args,
            kvs_path=kvs_path,
            log_file=log_file,
            err_log_file=log_file)
        proc.start(
            expected_output=expected_output,
            timeout=timeout)

        self.current_provider_app_proc = proc
        log.info("Provider started with PID:  %s", self.current_provider_app_proc.get_pid())

    def terminate_provider(self):
        if hasattr(self, "current_provider_app_proc") and self.current_provider_app_proc is not None:
            log.info("Terminating existing OTA Provider")
            self.current_provider_app_proc.terminate()
            self.current_provider_app_proc = None
        else:
            log.warning("Provider process not found. Unable to terminate.")

    async def announce_ota_provider(self,
                                    controller: ChipDeviceCtrl.ChipDeviceController,
                                    provider_node_id: int,
                                    requestor_node_id: int,
                                    reason: Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable,
                                    vendor_id: int = 0xFFF1,
                                    endpoint: int = 0):
        """ Launch the requestor.AnnounceOTAProvider method with the specific configuration.
            Starts the communication from the requestor to the provider to start a software update.
        Args:
            controller (ChipDeviceCtrl): Controller for DUT
            provider_node_id (int): Node id for the provider
            requestor_node_id (int): Node id for the requestor
            reason (Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum, optional): Update Reason. Defaults to Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable.
            vendor_id (int, optional): Vendor id. Defaults to 0xFFF1.
            endpoint (int, optional): Endpoint id. Defaults to 0.

        Returns:
            object: Return the data from the OtaSoftwareUpdateRequestor.AnnounceOTAProvider command.
        """
        cmd_announce_ota_provider = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=vendor_id,
            announcementReason=reason,
            metadataForNode=None,
            endpoint=endpoint
        )
        log.info("Sending AnnounceOTA Provider Command")
        cmd_resp = await self.send_single_cmd(
            cmd=cmd_announce_ota_provider,
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=endpoint,
        )
        log.info("AnnounceOTA command sent")
        return cmd_resp

    async def set_default_ota_providers_list(self, controller: ChipDeviceCtrl.ChipDeviceController, provider_node_id: int, requestor_node_id: int, endpoint: int = 0):
        """Write the provider list in the requestor to initiate the Software Update.

        Args:
            controller (ChipDeviceCtrl): Controller to write the providers.
            provider_node_id (int): Node where the provider is located.
            requestor_node_id (int): Node of the requestor to write the providers.
            endpoint (int, optional): Endpoint to write the providers. Defaults to 0.
        """

        current_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        log.info("OTA Providers: %s", current_otap_info)

        # Create Provider Location into Requestor
        provider_location_struct = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation(
            providerNodeID=provider_node_id,
            endpoint=endpoint,
            fabricIndex=controller.fabricId
        )

        # Create the OTA Provider Attribute
        ota_providers_attr = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[provider_location_struct])

        # Write the Attribute
        resp = await controller.WriteAttribute(
            attributes=[(endpoint, ota_providers_attr)],
            nodeId=requestor_node_id,
        )
        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to write Default OTA Providers Attribute")

        # Read Updated OTAProviders
        after_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        log.info("OTA Providers List: %s", after_otap_info)

    async def verify_version_applied_basic_information(self, controller: ChipDeviceCtrl.ChipDeviceController, node_id: int, target_version: int) -> int:
        """Verify the version from the BasicInformationCluster and compares against the provider target version.

        Args:
            controller (ChipDeviceCtrl): Controller
            node_id (int): Node to request
            target_version (int): Version to compare
        """

        basicinfo_softwareversion = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=node_id)
        asserts.assert_equal(basicinfo_softwareversion, target_version,
                             f"Version from basic info cluster is not {target_version}, current cluster version is {basicinfo_softwareversion}")
        return int(basicinfo_softwareversion)

    def get_downloaded_ota_image_info(self, ota_path='/tmp/test.bin') -> dict:
        """Return the data of the downloaded image from the provider.

        Args:
            ota_path (str, optional): _description_. Defaults to '/tmp/test.bin'.

        Returns:
            dict: Dict with the image info.
        """
        ota_image_info = {
            "path": ota_path,
            "exists": False,
            "size": 0,
        }
        try:
            ota_image_info['size'] = path.getsize(ota_path)
            ota_image_info['exists'] = True
        except OSError:
            log.info("OTA IMAGE at %s does not exists", ota_path)
            return ota_image_info

        return ota_image_info

    def verify_state_transition_event(self,
                                      event_report: Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
                                      expected_previous_state,
                                      expected_new_state,
                                      expected_target_version: int | None = None,
                                      expected_reason: int | None = None):
        """Verify the values of the StateTransitionEvent from the EventHandler given the provided arguments.

        Args:
            event_report (Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition): StateTransition Event report to verify.
            previous_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the previous state.
            new_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the new or current state.
            target_version (Optional[int], optional): Software version to verify if not provided ignore this check.. Defaults to None.
            reason (Optional[int], optional): UpdateStateEnum reason of the event, if not provided ignore. Defaults to None.
        """
        log.info("Verifying the event %s", event_report)
        asserts.assert_equal(event_report.previousState, expected_previous_state,
                             f"Previous state was not {expected_previous_state}")
        asserts.assert_equal(event_report.newState,  expected_new_state, f"New state is not {expected_new_state}")
        if expected_target_version is not None:
            asserts.assert_equal(event_report.targetSoftwareVersion,  expected_target_version,
                                 f"Target version is not {expected_target_version}")
        if expected_reason is not None:
            asserts.assert_equal(event_report.reason,  expected_reason, f"Reason is not {expected_reason}")

    def create_acl_entry(self,
                         dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
                         provider_node_id: int,
                         requestor_node_id: int | None = None,
                         acl_entries: list[AccessControlEntryStruct] | None = None,
                         ):
        """Create ACL entries to allow OTA requestors to access the provider.

        Args:
            dev_ctrl: Device controller for sending commands
            provider_node_id: Node ID of the OTA provider
            requestor_node_id: Optional specific requestor node ID for targeted access
            acl_entries: Optional[list[AccessControlEntryStruct]]. ACL list to write into the requestor.

        Returns:
            Result of the ACL write operation
        """
        # Standard ACL entry for OTA Provider cluster
        admin_node_id = dev_ctrl.nodeId if hasattr(dev_ctrl, 'nodeId') else self.DEFAULT_ADMIN_NODE_ID
        requestor_subjects = [requestor_node_id] if requestor_node_id else NullValue

        if acl_entries is None:
            # If there are not ACL entries using proper struct constructors create the default.
            acl_entries = [
                # Admin entry
                AccessControlEntryStruct(
                    privilege=AccessControlEntryPrivilegeEnum.kAdminister,
                    authMode=AccessControlEntryAuthModeEnum.kCase,
                    subjects=[admin_node_id],
                    targets=NullValue
                ),
                # Operate entry
                AccessControlEntryStruct(
                    privilege=AccessControlEntryPrivilegeEnum.kOperate,
                    authMode=AccessControlEntryAuthModeEnum.kCase,
                    subjects=requestor_subjects,
                    targets=[
                        AccessControlTargetStruct(
                            cluster=Clusters.OtaSoftwareUpdateProvider.id,
                            endpoint=NullValue,
                            deviceType=NullValue
                        )
                    ],
                )
            ]

        # Create the attribute descriptor for the ACL attribute
        acl_attribute = AccessControlCluster.Attributes.Acl(acl_entries)

        return dev_ctrl.WriteAttribute(
            nodeId=provider_node_id,
            attributes=[(0, acl_attribute)]
        )

    def restart_requestor(self, restore: bool = False):
        """This method Reboots or Restore the DUT."""
        restart_flag_file = self.get_restart_flag_file()
        log.info("RESTART FILE at %s", restart_flag_file)
        if not restart_flag_file:
            action_str = "Reboot"
            prompt_message = "Reboot the DUT. Press Enter when ready.\n"
            if restore:
                action_str = "Restore"
                prompt_message = "Manually restore the DUT to it's original version. Please type Enter when its ready.\n"
            log.info("Restart file not found. Entering Manual %s.", action_str)
            # No restart flag file: ask user to manually reboot. For this test will be needed to wipe or
            # restore to the previous software version.
            self.controller.ExpireSessions(self.requestor_node_id)
            self.wait_for_user_input(prompt_msg=prompt_message)
            # After manual reboot, expire previous sessions so that we can re-establish connections
            # In manual reboot or device no sleep is added as the user notify until the Device is ready.
            log.info("Manual device %s completed", action_str)
        else:
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write("restart")
                log.info("Created restart flag file to signal app restart")
                # This sleep allows the app start while waiting for app ready pattern. If not in some cases connections will Drop.
                sleep(1)
                # Expire sessions and re-establish connections
                log.info("Expiring sessions after manual device reboot")
                self.controller.ExpireSessions(self.requestor_node_id)
                log.info("App restart completed successfully")

            except Exception as e:
                asserts.fail(f"Requestor restart failed: {e}")

    async def clear_ota_providers(self, controller: ChipDeviceCtrl.ChipDeviceController, requestor_node_id: int):
        """
        Clears the DefaultOTAProviders attribute on the Requestor, leaving it empty.
        Args:
            controller (ChipDeviceCtrl): The controller to use for writing attributes.
            requestor_node_id (int): Node ID of the Requestor device.

        Returns:
            None
        """
        # Set DefaultOTAProviders to empty list
        attr_clear = Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value=[])
        resp = await controller.WriteAttribute(
            attributes=[(0, attr_clear)],
            nodeId=requestor_node_id
        )
        log.info('Cleanup - DefaultOTAProviders cleared')

        asserts.assert_equal(resp[0].Status, Status.Success, "Failed to clear DefaultOTAProviders")

    def clear_kvs(self, kvs_path_prefix: str = "/tmp/chip_kvs"):
        """
        Remove all temporary KVS files created.

        OTA Provider/Requestor use "/tmp/chip_kvs" as the default KVS location when no --KVS is provided.
        Tests may also specify custom prefixes such as "/tmp/chip_kvs_provider".

        Args:
            kvs_path_prefix (str, optional): Prefix of KVS files/folders to remove.
            Defaults to "/tmp/chip_kvs", which removes all temporary chip KVS files.
        """
        # Do not allow relative paths or paths outside of /tmp/
        real_kvs_path_prefix = path.realpath(kvs_path_prefix)
        # on some darwin devices /tmp/ folder is an alias of /private/tmp/
        if not (real_kvs_path_prefix.startswith('/tmp/') or real_kvs_path_prefix.startswith('/private/tmp/')):
            raise ValueError(
                f"kvs_path_prefix must be an absolute path starting with /tmp/ or /private/tmp/, but was: {real_kvs_path_prefix}")
        subprocess.run(['rm', '-rf', f'{real_kvs_path_prefix}*'])
        log.info("Removed all KVS files/folders with prefix: %s", real_kvs_path_prefix)

    def get_ota_image_software_version(self, ota_image_path: str) -> int:
        """Parse the OTA image header and return the embedded software version.

        Args:
            ota_image_path (str): Path to the OTA image file to parse.

        Returns:
            int: Software version read from the OTA image header (TLV context tag 2).
        """
        # Format values taken from src/app/ota_image_tool.py
        FIXED_HEADER_FORMAT = '<IQI'
        HEADER_MAGIC = 0x1BEEF11E
        header_tlv = None
        version = 0
        with open(ota_image_path, 'rb') as file:
            fixed_header = file.read(struct.calcsize(FIXED_HEADER_FORMAT))
            magic, total_size, header_size = struct.unpack(
                FIXED_HEADER_FORMAT, fixed_header)
            if magic != HEADER_MAGIC:
                asserts.fail("Invalid Ota Image")
            header_tlv = TLVReader(file.read(header_size)).get()['Any']

        try:
            # Version has context tag 2
            version = header_tlv[2]
        except KeyError:
            asserts.fail("Unable to retrieve the Software Version from the ota image.")

        return version

    async def check_ota_image_version(self, controller: ChipDeviceCtrl.ChipDeviceController, requestor_node_id: int, ota_image_path: str) -> int:
        """Verify the OTA image version against the DUT's current software version.

        Reads the software version from the OTA image header and compares it to the
        SoftwareVersion attribute reported by the DUT, confirming the update can proceed.
        Fails the test if the OTA image version is not greater than the DUT's current version.

        Args:
            controller (ChipDeviceCtrl): Controller used to read the DUT's SoftwareVersion attribute.
            requestor_node_id (int): Node ID of the requestor (DUT) to check the version against.
            ota_image_path (str): Path to the OTA image file to verify.

        Returns:
            int: Software version contained in the OTA image, to use as the target update version.
        """

        ota_version = self.get_ota_image_software_version(ota_image_path=ota_image_path)
        basicinfo_softwareversion = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion,
            node_id=requestor_node_id)
        if ota_version <= basicinfo_softwareversion:
            asserts.fail(
                f"Invalid OTA Image with version: {ota_version} to update Device running with version {basicinfo_softwareversion}.")

        log.info("OTA Image version is %s to install on Device with version %s", ota_version, basicinfo_softwareversion)
        return ota_version
