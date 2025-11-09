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


import logging
import tempfile
from os import path
from time import sleep
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.apps import OtaImagePath, OTAProviderSubprocess
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)


class SoftwareUpdateBaseTest(MatterBaseTest):
    """This is the base test class for SoftwareUpdate Test Cases"""

    current_provider_app_proc: Optional[OTAProviderSubprocess] = None
    provider_app_path: Optional[str] = None

    def start_provider(self,
                       provider_app_path: str = "",
                       ota_image_path: str = "",
                       setup_pincode: int = 20202021,
                       discriminator: int = 1234,
                       port: int = 5541,
                       storage_dir='/tmp',
                       extra_args: list = [],
                       kvs_path: Optional[str] = None,
                       log_file: Optional[str] = None, expected_output: str = "Server initialization complete",
                       timeout: int = 10):
        """Start the provider process using the provided configuration.

        Args:
            provider_app_path (str): Path of Requestor app to load.
            ota_image_path (str): Ota image to load within the provider.
            setup_pincode (int, optional): Setup pincode for the provider process. Defaults to 20202021.
            discriminator (int, optional): Discriminator for the provider process. Defaults to 1234.
            port (int, optional): Port for the provider process. Defaults to 5541.
            storage_dir (str, optional): Storage dir for the provider proccess. Defaults to '/tmp'.
            extra_args (list, optional): Extra args to send to the provider process. Defaults to [].
            kvs_path(str): Str of the path for the kvs path, if not will use temp file.
            log_file (Optional[str], optional): Destination for the app process logs. Defaults to None.
            expected_output (str): Expected string to see after a default timeout. Defaults to "Server initialization complete".
            timeout (int): Timeout to wait for the expected output. Defaults to 10 seconds
        """
        logger.info(f"Launching provider app with with ota image {ota_image_path}")
        # Image to launch
        self.provider_app_path = provider_app_path
        if not path.exists(provider_app_path):
            raise FileNotFoundError(f"Provider app not found {provider_app_path}")

        if not path.exists(ota_image_path):
            raise FileNotFoundError(f"Ota image provided does not exists {ota_image_path}")

        # Ota image
        ota_image_path = OtaImagePath(path=ota_image_path)
        # Ideally we send the logs to a fixed location to avoid conflicts

        if log_file is None:
            # Assign the file descriptor to log_file
            log_file = tempfile.NamedTemporaryFile(
                dir=storage_dir, prefix='provider_', suffix='.log', mode='ab')
            logger.info(f"Writing Provider logs at :{log_file.name}")
        else:
            logger.info(f"Writing Provider logs at : {log_file}")
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
        logger.info(f"Provider started with PID:  {self.current_provider_app_proc.get_pid()}")

    def terminate_provider(self):
        if hasattr(self, "current_provider_app_proc") and self.current_provider_app_proc is not None:
            logger.info("Terminating existing OTA Provider")
            self.current_provider_app_proc.terminate()
            self.current_provider_app_proc = None

    async def announce_ota_provider(self,
                                    controller: ChipDeviceCtrl,
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
        logger.info("Sending AnnounceOTA Provider Command")
        cmd_resp = await self.send_single_cmd(
            cmd=cmd_announce_ota_provider,
            dev_ctrl=controller,
            node_id=requestor_node_id,
            endpoint=endpoint,
        )
        logger.info(f"Announce command sent {cmd_resp}")
        return cmd_resp

    async def set_default_ota_providers_list(self, controller: ChipDeviceCtrl, provider_node_id: int, requestor_node_id: int, endpoint: int = 0):
        """Write the provider list in the requestor to initiate the Software Update.

        Args:
            controller (ChipDeviceCtrl): Controller to write the providers.
            provider_node_id (int): Node where the provider is localted.
            requestor_node_id (int): Node of the requestor to write the providers.
            endpoint (int, optional): Endpoint to write the providerss. Defaults to 0.
        """

        current_otap_info = await self.read_single_attribute_check_success(
            dev_ctrl=controller,
            cluster=Clusters.OtaSoftwareUpdateRequestor,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders
        )
        logger.info(f"OTA Providers: {current_otap_info}")

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
        logger.info(f"OTA Providers List: {after_otap_info}")

    async def verify_version_applied_basic_information(self, controller: ChipDeviceCtrl, node_id: int, target_version: int):
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
            logger.info(f"OTA IMAGE at {ota_path} does not exists")
            return ota_image_info

        return ota_image_info

    def verify_state_transition_event(self,
                                      event_report: Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition,
                                      expected_previous_state,
                                      expected_new_state,
                                      expected_target_version: Optional[int] = None,
                                      expected_reason: Optional[int] = None):
        """Verify the values of the StateTransitionEvent from the EventHandler given the provided arguments.

        Args:
            event_report (Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition): StateTransition Event report to verify.
            previous_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the previous state.
            new_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the new or current state.
            target_version (Optional[int], optional): Software version to verify if not provided ignore this check.. Defaults to None.
            reason (Optional[int], optional): UpdateStateEnum reason of the event, if not provided ignore. Defaults to None.
        """
        logger.info(f"Verifying the event {event_report}")
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
                         requestor_node_id: Optional[int] = None,
                         acl_entries: Optional[list[Clusters.AccessControl.Structs.AccessControlEntryStruct]] = None,
                         ):
        """Create ACL entries to allow OTA requestors to access the provider.

        Args:
            dev_ctrl: Device controller for sending commands
            provider_node_id: Node ID of the OTA provider
            requestor_node_id: Optional specific requestor node ID for targeted access
            acl_entries: Optional[list[Clusters.AccessControl.Structs.AccessControlEntryStruct]]. ACL list to write ino the requestor.

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
                Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
                    privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,  # type: ignore
                    authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
                    subjects=[admin_node_id],  # type: ignore
                    targets=NullValue
                ),
                # Operate entry
                Clusters.AccessControl.Structs.AccessControlEntryStruct(  # type: ignore
                    privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,  # type: ignore
                    authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,  # type: ignore
                    subjects=requestor_subjects,  # type: ignore
                    targets=[
                        Clusters.AccessControl.Structs.AccessControlTargetStruct(  # type: ignore
                            cluster=Clusters.OtaSoftwareUpdateProvider.id,  # type: ignore
                            endpoint=NullValue,
                            deviceType=NullValue
                        )
                    ],
                )
            ]

        # Create the attribute descriptor for the ACL attribute
        acl_attribute = Clusters.AccessControl.Attributes.Acl(acl_entries)

        return dev_ctrl.WriteAttribute(
            nodeId=provider_node_id,
            attributes=[(0, acl_attribute)]
        )

    def restart_requestor(self, requestor_th):
        """This method restart the requestor so stops using updated requestor image.

        Args:
            controller (_type_): _description_
        """
        restart_flag_file = self.get_restart_flag_file()
        logger.info(f"RESTART FILE at {restart_flag_file}")
        if not restart_flag_file:
            # No restart flag file: ask user to manually reboot
            self.wait_for_user_input(prompt_msg="Reboot the DUT. Press Enter when ready.\n")

            # After manual reboot, expire previous sessions so that we can re-establish connections
            logging.info("Expiring sessions after manual device reboot")
            requestor_th.ExpireSessions(self.requestor_node_id)
            logging.info("Manual device reboot completed")

        else:
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write("restart")
                logging.info("Created restart flag file to signal app restart")

                # The test runner will automatically wait for the app-ready-pattern before continuing
                # Waiting 1 second after the app-ready-pattern is detected as we need to wait a tad longer for the app to be ready and stable, otherwise TH2 connection fails later on in test step 14.
                sleep(1)

                # Expire sessions and re-establish connections
                requestor_th.ExpireSessions(self.requestor_node_id)
                logging.info("App restart completed successfully")

            except Exception as e:
                logging.error(f"Failed to restart Requestor: {e}")
                asserts.fail(f"Requestor restart failed: {e}")
