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
from datetime import datetime
from os import environ, getcwd, path
from typing import Optional, Union

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

    current_provider_app_proc: Union[OTAProviderSubprocess, None] = None
    current_requestor_app_proc: Union[OTAProviderSubprocess, None] = None

    def start_provider(self,
                       version: int = 2,
                       setup_pincode: int = 20202021,
                       discriminator: int = 1234,
                       port: int = 5541,
                       storage_dir='/tmp',
                       extra_args: list = [],
                       kvs_path: Optional[str] = None,
                       log_file: Optional[str] = None, expected_output: str = "Status: Satisfied",
                       timeout: int = 10):
        """Start the provider process using the provided configuration.

        Args:
            version (int, optional): Provider app version to load. Defaults to 2.
            setup_pincode (int, optional): Setup pincode for the provider process. Defaults to 20202021.
            discriminator (int, optional): Discriminator for the provider process. Defaults to 1234.
            port (int, optional): Port for the provider process. Defaults to 5541.
            storage_dir (str, optional): Storage dir for the provider proccess. Defaults to '/tmp'.
            extra_args (list, optional): Extra args to send to the provider process. Defaults to [].
            kvs_path(str): Str of the path for the kvs path, if not will use temp file.
            log_file (Optional[str], optional): Destination for the app process logs. Defaults to None.
            expected_output (str): Expected string to see after a default timeout. Defaults to "Status: Satisfied".
            timeout (int): Timeout to wait for the expected output. Defaults to 10 seconds
        """
        logger.info(f"Launching provider app with version {version}")
        # Image to launch
        ota_app = ""
        if environ.get("OTA_PROVIDER_APP") is not None:
            ota_app = environ.get("OTA_PROVIDER_APP")
        else:
            ota_app = f"{getcwd()}/out/debug/chip-ota-provider-app"

        # Ota image
        ota_image_path = OtaImagePath(path=self.get_ota_image_path(version=version))
        # Ideally we send the logs to a fixed location to avoid conflicts

        if log_file is None:
            now = datetime.now()
            ts = int(now.timestamp())
            log_file = f'/tmp/provider_{ts}.log'
        logger.info(f"Writing Provider logs at : {log_file}")
        # Launch the Provider subprocess using the Wrapper
        proc = OTAProviderSubprocess(
            ota_app,
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

    def get_ota_image_path(self, version: int = 2) -> str:
        """Get the path ota image used for the provider.

        Args:
            version (int): App version . Defaults to 2.

        Returns:
            str: Path of the ota image
        """

        ota_image = ""
        if environ.get(f"SU_OTA_REQUESTOR_V{version}") is not None:
            ota_image = environ.get(f"SU_OTA_REQUESTOR_V{version}")
        else:
            ota_image = f"{getcwd()}/chip-ota-requestor-app_v{version}.min.ota"

        return ota_image

    async def announce_ota_provider(self,
                                    controller: ChipDeviceCtrl,
                                    provider_node_id: int,
                                    requestor_node_id: int,
                                    reason: Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable,
                                    vendor_id: int = 0xFFF1,
                                    endpoint: int = 0):
        """Announce the OTA provider that a software update is requested.

        Args:
            controller (ChipDeviceCtrl): Controller for DUT
            provider_node_id (int): Node id for the provider
            requestor_node_id (int): Node id for the requestor
            reason (Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum, optional): Update Reason. Defaults to Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable.
            vendor_id (int, optional): Vendor id. Defaults to 0xFFF1.
            endpoint (int, optional): Endpoint id. Defaults to 0.

        Returns:
            _type_: _description_
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

    async def write_ota_providers(self, controller: ChipDeviceCtrl, provider_node_id: int, endpoint: int = 0):
        """Write the provider list in the requestor to initiate the SU.

        Args:
            controller (ChipDeviceCtrl): Controller to write the providers.
            provider_node_id (int): Node where the provider is localted.
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
            nodeid=self.dut_node_id,
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
                                     previous_state,
                                     new_state,
                                     target_version: Optional[int] = None,
                                     reason: Optional[int] = None):
        """Verify the values of the StateTransitionEvent from the EventHandler given the provided arguments.

        Args:
            event_report (Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition): StateTransition Event report to verify.
            previous_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the previous state.
            new_state (UpdateStateEnum:int): Int or UpdateStateEnum value for the new or current state.
            target_version (Optional[int], optional): Software version to verify if not provided ignore this check.. Defaults to None.
            reason (Optional[int], optional): UpdateStateEnum reason of the event, if not provided ignore. Defaults to None.
        """
        logger.info(f"Verifying the event {event_report}")
        asserts.assert_equal(event_report.previousState, previous_state, f"Previous state was not {previous_state}")
        asserts.assert_equal(event_report.newState,  new_state, f"New state is not {new_state}")
        if target_version is not None:
            asserts.assert_equal(event_report.targetSoftwareVersion,  target_version, f"Target version is not {target_version}")
        if reason is not None:
            asserts.assert_equal(event_report.reason,  reason, f"Reason is not {reason}")

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
            nodeid=provider_node_id,
            attributes=[(0, acl_attribute)]
        )


class ACLHandler:
    """
    Utility class to handle Access Control List (ACL) operations for OTA Provider and Requestor.
    """

    DEFAULT_ADMIN_NODE_ID = 112233

    def __init__(self, controller):
        """
        Initialize the ACL handler.

        Args:
            controller: The controller used to perform ACL write operations.
        """
        self.controller = controller

    async def write_acl(self, node_id: int, acl: list):
        """
        Writes the Access Control List (ACL) to the DUT device using the specified controller.

        Args:
            node_id: Node ID of the target device (provider or requestor).
            acl (list): List of AccessControlEntryStruct objects defining ACL permissions.

        Raises:
            AssertionError: If writing the ACL attribute fails (status is not Status.Success).
        """
        acl_attribute = Clusters.AccessControl.Attributes.Acl(acl)
        result = await self.controller.WriteAttribute(
            nodeid=node_id,
            attributes=[(0, acl_attribute)]
        )
        if result[0].Status != Status.Success:
            raise RuntimeError(f"ACL write failed for node {node_id}: {result[0].Status}")
        return True

    def create_acl_entry(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController, provider_node_id: int, requestor_node_id: Optional[int] = None):
        """Create ACL entries to allow OTA requestors to access the provider.

        Args:
            dev_ctrl: Device controller for sending commands
            provider_node_id: Node ID of the OTA provider
            requestor_node_id: Optional specific requestor node ID for targeted access

        Returns:
            Result of the ACL write operation
        """
        # Standard ACL entry for OTA Provider cluster
        admin_node_id = dev_ctrl.nodeId if hasattr(dev_ctrl, 'nodeId') else self.DEFAULT_ADMIN_NODE_ID
        requestor_subjects = [requestor_node_id] if requestor_node_id else NullValue

        # Create ACL entries using proper struct constructors
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
            nodeid=provider_node_id,
            attributes=[(0, acl_attribute)]
        )

    async def set_acl_for_requestor(
            self,
            requestor_node: int,
            provider_node: int,
            fabric_index: int,
            original_requestor_acls: list
    ):
        """
        Read existing ACLs on Requestor, add minimal ACL for Provider, and write back.

        Args:
            requestor_node: Node ID of the Requestor device.
            provider_node: Node ID of the Provider device.
            fabric_index: Fabric index for the ACL entry.
            original_requestor_acls: Existing ACLs for the Requestor.

        Returns:
            The original requestor ACLs (unchanged).
        """

        # Add minimal ACL for Provider
        acl_operate_provider = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[provider_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateRequestor.id
            )]
        )

        # Combine existing + new ACLs
        combined_acls = original_requestor_acls + [acl_operate_provider]
        await self.write_acl(requestor_node, combined_acls)

    async def set_acl_for_provider(
            self,
            provider_node: int,
            requestor_node: int,
            fabric_index: int,
            original_provider_acls: list
    ):
        """
        Read existing ACLs on Provider, add minimal ACL for Requestor, and write back.

        Args:
            provider_node: Node ID of the Provider device.
            requestor_node: Node ID of the Requestor device.
            fabric_index: Fabric index for the ACL entry.
            original_provider_acls: Existing ACLs for the Provider.

        Returns:
            The original provider ACLs (unchanged).
        """

        # Add minimal ACL for Requestor
        acl_operate_requestor = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            fabricIndex=fabric_index,
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[requestor_node],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                endpoint=0,
                cluster=Clusters.OtaSoftwareUpdateProvider.id
            )]
        )

        # Combine existing + new ACLs
        combined_acls = original_provider_acls + [acl_operate_requestor]
        await self.write_acl(provider_node, combined_acls)

    async def set_ota_acls(
            self,
            requestor_node: int,
            provider_node: int,
            fabric_index: int,
            original_requestor_acls: list,
            original_provider_acls: list
    ):
        """
        Set ACLs both ways and preserve originals.

        Args:
            requestor_node: Node ID of the Requestor.
            provider_node: Node ID of the Provider.
            fabric_index: Fabric index for ACL entries.
            original_requestor_acls: Existing ACLs on Requestor.
            original_provider_acls: Existing ACLs on Provider.

        Returns:
            original_requestor_acls, original_provider_acls
        """
        await self.set_acl_for_requestor(requestor_node, provider_node, fabric_index, original_requestor_acls)
        await self.set_acl_for_provider(provider_node, requestor_node, fabric_index, original_provider_acls)
