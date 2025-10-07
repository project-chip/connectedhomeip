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

    async def commission_provider(self, **kwargs):
        """Launch provider, then comission provider and finally configure the provider and requestor to be ready for announce.
            Args:
            controller (required)
            version: Defaults 2
            setup_pincode: Defaults 202021
            discriminator : Defaults 1234
            port : Defaults 5541
            storage_dir : Defaults /tmp
            requestor_node_id: Required
            provider_node_id: Required
            endpoint: Defaults to 0
            extra_args: Optional defaults to []
            log_file : Defaults to /tmp/requrestor.log
        """
        provider_args_list = ['version', 'setup_pincode', 'discriminator', 'port', 'storage_dir', 'extra_args', 'log_file']
        provider_args: dict = {k: v for k, v in kwargs.items() if k in provider_args_list}
        self.launch_provider_app(**provider_args)
        controller = kwargs['controller']
        await controller.CommissionOnNetwork(
            nodeId=kwargs['provider_node_id'],
            setupPinCode=kwargs['setup_pincode'],
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=kwargs['discriminator']
        )
        await self.write_ota_providers(controller, kwargs['provider_node_id'], kwargs['endpoint'])
        await self.create_acl_entry(controller, kwargs['provider_node_id'], kwargs['requestor_node_id'])

    def launch_provider_app(self,
                            version: int = 2,
                            setup_pincode=20202021,
                            discriminator=1234,
                            port=5541,
                            storage_dir='/tmp',
                            extra_args: list = [],
                            log_file: Optional[str] = None, expected_output: str = "Status: Satisfied"):
        """Launch the provider app using the OTAProviderSubprocess.

        Args:
            version (int, optional): Version of the OTA image to load. Defaults to 2.
            extra_args (list, optional): List of arguments for ota provider. Defaults to [].
            log_file (str, optional): Destination of the output app logs. Defaults to "/tmp/provider.log".

        """
        logger.info(f"LAUNCHING PROVIDER APP WITH VERSION {version}")
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
        logger.info(f"Writing provider logs at : {log_file}")
        # Launch the subprocess
        proc = OTAProviderSubprocess(
            ota_app,
            storage_dir=storage_dir,
            port=port,
            discriminator=discriminator,
            passcode=setup_pincode,
            ota_source=ota_image_path,
            extra_args=extra_args,
            log_file=log_file,
            err_log_file=log_file)
        proc.start(
            expected_output=expected_output,
            timeout=10)

        self.current_provider_app_proc = proc
        logger.info(f"Provider stareted with  {self.current_provider_app_proc.get_pid()}")

    def get_ota_image_path(self, version=2) -> str:
        # Ota image
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
                                    endpoint=0):
        """ Announce the OTA provider that a software update is requested.

        Args:
            controller (ChipDeviceCtrl): _description_
            provider_node_id (int): Node id of the provider
            requestor_node_id (int): Node id of the requestor
            reason (Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum, optional): Reason of the announcement. Defaults to Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable.

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
            controller (ChipDeviceCtrl): _description_
            provider_node_id (int): _description_
            endpoint (int, optional): _description_. Defaults to 0.
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

    def verfiy_state_transition_event(self,
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
