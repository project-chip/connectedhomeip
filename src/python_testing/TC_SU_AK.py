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

import logging
import time

from typing import Optional
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from matter.testing.apps import OTAProviderSubprocess, OtaImagePath, OTARequestorProcess

logger = logging.getLogger(__name__)


class SU_BaseTest(MatterBaseTest):
    def setup_class(self):
        pass

    def teardown_test(self):
        if hasattr(self, "ota_provider") and self.ota_provider is not None:
            logger.info("Terminating existing OTA Provider")
            self.ota_provider.terminate()
            self.ota_provider = None
        if hasattr(self, "ota_requestor") and self.ota_requestor is not None:
            logger.info("Terminating existing OTA Requestor")
            self.ota_requestor.terminate()
            self.ota_requestor = None
        super().teardown_test()


class TC_SU_My_Test(SU_BaseTest):
    def start_requestor(self, extra_args: list[str], expected_output: Optional[str] = None, timeout: int = 30):
        logger.info("getting config")
        ota_r_app_filepath = self.user_params.get("ota-r-app-filepath", "./out/debug/chip-ota-requestor-app")
        ota_r_storage_dir = self.user_params.get("ota-r-storage-dir", "/tmp")
        ota_r_port = self.user_params.get("ota-r-port", 5540)
        ota_r_discriminator = self.user_params.get("ota-r-discriminator", 123)
        ota_r_passcode = self.user_params.get("ota-r-passcode", 2123)
        ota_r_kvs_path = self.user_params.get("ota-r-kvs-path", "/tmp/chip_kvs_requestor")

        logger.info("Starting OTA Requestor")
        self.ota_requestor = OTARequestorProcess(
            app=ota_r_app_filepath,
            storage_dir=ota_r_storage_dir,
            port=ota_r_port,
            discriminator=ota_r_discriminator,
            passcode=ota_r_passcode,
            extra_args=extra_args
            # kvs_path=ota_r_kvs_path
        )
        self.ota_requestor.start(expected_output=expected_output,
                                 timeout=timeout)

    def start_provider(self, extra_args: list[str], expected_output: Optional[str] = None, timeout: int = 30):
        logger.info("getting config")
        ota_p_app_filepath = self.user_params.get("ota-p-app-filepath", "./out/debug/chip-ota-provider-app")
        ota_p_storage_dir = self.user_params.get("ota-p-storage-dir", "/tmp")
        ota_p_port = self.user_params.get("ota-p-port", 5541)
        ota_p_discriminator = self.user_params.get("ota-p-discriminator", 321)
        ota_p_passcode = self.user_params.get("ota-p-passcode", 2321)
        ota_p_image_path = self.user_params.get("ota-p-image-path", OtaImagePath("firmware_requestor_v2.ota"))
        ota_p_kvs_path = self.user_params.get("ota-p-kvs-path", "/tmp/chip_kvs_provider")

        logger.info("Starting OTA Provider")
        self.ota_provider = OTAProviderSubprocess(
            app=ota_p_app_filepath,
            storage_dir=ota_p_storage_dir,
            port=ota_p_port,
            discriminator=ota_p_discriminator,
            passcode=ota_p_passcode,
            ota_source=ota_p_image_path,
            extra_args=extra_args
            # kvs_path=ota_p_kvs_path
        )
        self.ota_provider.start(expected_output=expected_output,
                                timeout=timeout)

    async def commission_provider(self):
        logger.info("Commissioning OTA Provider")

        dev_ctrl = self.default_controller
        await dev_ctrl.CommissionOnNetwork(
            nodeId=321,
            setupPinCode=2321,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=321
        )
        logger.info("OTA Provider commissioned")

    async def commission_requestor(self):
        logger.info("Commissioning OTA Requestor")

        dev_ctrl = self.default_controller
        await dev_ctrl.CommissionOnNetwork(
            nodeId=123,
            setupPinCode=2123,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=123
        )
        logger.info("OTA Requestor commissioned")

    async def announce_provider(self, dev_ctrl, requestor_node_id, provider_node_id, vendor_id=0xFFF1, reason=None, endpoint=0):
        """
        Send the AnnounceOTAProvider command from the Requestor to the Provider.

        Args:
            dev_ctrl: Controller object to send the command.
            requestor_node_id: Node ID of the OTA Requestor (DUT).
            provider_node_id: Node ID of the OTA Provider.
            vendor_id: Vendor ID associated with the Provider (default: 0xFFF1).
            reason: AnnouncementReasonEnum for the announcement (default: kUpdateAvailable).
            endpoint: Endpoint on which to send the command (default: 0).

        Returns:
            resp: Response object from the AnnounceOTAProvider command.
        """
        reason = reason or Clusters.OtaSoftwareUpdateRequestor.Enums.AnnouncementReasonEnum.kUpdateAvailable
        cmd = Clusters.OtaSoftwareUpdateRequestor.Commands.AnnounceOTAProvider(
            providerNodeID=provider_node_id,
            vendorID=vendor_id,
            announcementReason=reason,
            metadataForNode=None,
            endpoint=endpoint
        )
        logger.info(f'Sending AnnounceOTAProvider to providerNodeID={provider_node_id}: {cmd}')
        resp = await self.send_single_cmd(
            dev_ctrl=dev_ctrl,
            node_id=requestor_node_id,
            cmd=cmd
        )
        logger.info(f'AnnounceOTAProvider response: {resp}')
        return resp

    def steps_TC_SU_My_Test(self):
        return [
            TestStep(0, "Commissioning", "Device  is commissioned", is_commissioning=True),
            TestStep(1, "Start OTA Provider", "OTA provider app is running"),
            TestStep(2, "Do smth", "Something is done"),
        ]

    @async_test_body
    async def test_TC_SU_My_Test(self):
        self.step(0)
        logger.info("Commissioning done")

        self.step(1)
        logger.info("Starting OTA Requestor")
        self.start_requestor(extra_args=["--autoApplyImage"], expected_output="Server initialization complete",
                             timeout=3600)
        await self.commission_requestor()

        logger.info("Starting OTA Provider and initiating the software update")
        self.start_provider(extra_args=["-q", "updateAvailable"],
                            expected_output="Server initialization complete",
                            timeout=3600)
        await self.commission_provider()
        await self.ota_provider.create_acl_entry(
            dev_ctrl=self.default_controller,
            provider_node_id=321,
            requestor_node_id=123)
        await self.announce_provider(
            dev_ctrl=self.default_controller,
            requestor_node_id=123,
            provider_node_id=321)

        self.ota_provider.send(message="Expecting the update now",
                               expected_output="AckEOFReceived",
                               timeout=600)

        self.step(2)
        logger.info("Doing smth")
        time.sleep(5)
        asserts.assert_true(True, "Just a demo assert")
        logger.info("Done")

    def steps_Delayed(self):
        return [
            TestStep(0, "Commissioning", "Device  is commissioned", is_commissioning=True),
            TestStep(1, "Start OTA Provider", "OTA provider app is running"),
        ]

    @async_test_body
    async def test_Delayed(self):
        self.step(0)
        logger.info("Commissioning done")

        self.step(1)
        logger.info("Starting OTA Requestor")
        self.start_requestor(extra_args=["--autoApplyImage"], expected_output="Server initialization complete",
                             timeout=3600)
        await self.commission_requestor()

        logger.info("Starting OTA Provider and initiating the software update")
        self.start_provider(extra_args=["-q", "busy", "-t", "60"],
                            expected_output="Server initialization complete",
                            timeout=3600)
        await self.commission_provider()
        await self.ota_provider.create_acl_entry(
            dev_ctrl=self.default_controller,
            provider_node_id=321,
            requestor_node_id=123)
        await self.announce_provider(
            dev_ctrl=self.default_controller,
            requestor_node_id=123,
            provider_node_id=321)

        logger.info("Starting OTA Provider and initiating the software update")
        self.ota_requestor.send(message="Expecting the update now",
                                expected_output="Scheduling a retry",
                                timeout=300)


if __name__ == "__main__":
    default_matter_test_main()
