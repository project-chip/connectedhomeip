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
from os import path
from typing import Optional

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)


class SoftwareUpdateBaseTest(MatterBaseTest):
    """This is the base test class for SoftwareUpdate Test Cases"""

    async def announce_ota_provider(self, controller: ChipDeviceCtrl, vendor_id: int, provider_node_id: int,  requestor_node_id: int, endpoint: int = 0, reason: Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable):
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

    def verfy_state_transition_event(self, event_report: Clusters.OtaSoftwareUpdateRequestor.Events.StateTransition, previous_state, new_state, target_version: Optional[int] = None, reason: Optional[int] = None):
        logger.info(f"Verifying the event {event_report}")

        asserts.assert_equal(event_report.previousState, previous_state, f"Previous state was not {previous_state}")
        asserts.assert_equal(event_report.newState,  new_state, f"New state is not {new_state}")
        if target_version is not None:
            asserts.assert_equal(event_report.targetSoftwareVersion,  target_version, f"Target version is not {target_version}")
        if reason is not None:
            asserts.assert_equal(event_report.reason,  reason, f"Reason is not {reason}")
