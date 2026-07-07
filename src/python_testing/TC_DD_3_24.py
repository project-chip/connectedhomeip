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
import time

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mobly import asserts

import matter.testing.nfc
from matter.ChipDeviceCtrl import _DevicePairingDelegate_OnCommissioningStageStartFunct
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_3_24(MatterBaseTest):
    def desc_TC_DD_3_24(self) -> str:
        return "[TC-DD-3.24] NFC-based commissioning - DUT without power [DUT as Commissionee]"

    def steps_TC_DD_3_24(self) -> list[TestStep]:
        return [
            TestStep(1, "Detecting the NFC Tag and reading the Payload", is_commissioning=False),
            TestStep(2, "Validate the NFC bit in payload and perform the first phase of the commissioning, over NFC"),
            TestStep(3, "DUT is powered ON."),
            TestStep(4, "Perform DNS-SD Discovery and check the presence of a mDNS service with “_IC” subtype."),
            TestStep(5, "Commissioning is completed on the operational network."),
            TestStep(6, "Perform DNS-SD Discovery and check that the “_IC” subtype is no more present."),
        ]

    def setup_test(self):
        super().setup_test()

        # Booleans to detect some commissioner stages
        self.unpowered_phase_complete_seen: bool = False
        self.send_complete_seen: bool = False

        # Filled at runtime
        self.commissionee_node_id = 0

        def _stage_start_listener(node_id: int, stage):
            # Normalize stage to string
            if isinstance(stage, bytes):
                stage = stage.decode("utf-8", errors="replace")

            log.info("[_stage_start_listener] node=0x%X, stage=%s", node_id, stage)

            self.commissionee_node_id = node_id

            if stage == "UnpoweredPhaseComplete":
                log.info("Detected 'UnpoweredPhaseComplete' commissioning stage")
                self.unpowered_phase_complete_seen = True

            if stage == "SendComplete":
                log.info("Detected 'SendComplete' commissioning stage")
                self.send_complete_seen = True

        self._commissioning_stage_start_callback = _DevicePairingDelegate_OnCommissioningStageStartFunct(
            _stage_start_listener
        )
        self.default_controller.setCommissioningStageStartCallback(self._commissioning_stage_start_callback)

    @async_test_body
    async def test_TC_DD_3_24(self):

        self.wait_for_user_input(prompt_msg="Put the DUT in commissionable mode, bring its NFC interface close to the NFC reader"
                                 " and power OFF the DUT")

        # Step 1: Here we check if the Tag is connected to the Host machine and read the NFC Tag data
        self.step(1)

        nfc_reader_index = self.user_params.get("NFC_Reader_index", 0)
        reader = matter.testing.nfc.NFCReader(nfc_reader_index)

        nfc_tag_data = reader.read_nfc_tag_data()
        log.info("NFC Tag data : '%s'", nfc_tag_data)
        asserts.assert_true(
            reader.is_onboarding_data(nfc_tag_data),
            f"'{nfc_tag_data}' is not a valid Matter URI"
        )
        self.matter_test_config.qr_code_content.append(nfc_tag_data)

        # Step 2: the NFC tag data is parsed and checked if the device supports NFC commissioning and commission begins
        self.step(2)
        payload = SetupPayload().ParseQrCode(nfc_tag_data)
        asserts.assert_true(payload.supports_nfc_commissioning, "Device does not Support NFC Commissioning")

        commissioning_method = self.matter_test_config.in_test_commissioning_method
        asserts.assert_is_not_none(commissioning_method, "in_test_commissioning_method must not be None")
        asserts.assert_true(
            str(commissioning_method).startswith("nfc-"),
            f"Expected in_test_commissioning_method to start with 'nfc-', got: {commissioning_method}"
        )

        self.matter_test_config.commissioning_method = commissioning_method

        log.info("default_controller in test: %s (id=%s)",
                 getattr(self, "default_controller", None),
                 hex(id(self.default_controller)) if hasattr(self, "default_controller") else "N/A")

        commissioning_success = await self.commission_devices()
        asserts.assert_true(commissioning_success, "Device Commissioning using nfc transport has failed")
        asserts.assert_true(self.unpowered_phase_complete_seen, "Stage 'UnpoweredPhaseComplete' was not seen!")

        self.step(3)    # Power ON the DUT
        self.wait_for_user_input(prompt_msg="Power ON the device")

        self.step(4)    # Perform DNS-SD Discovery and check the presence of a mDNS service with “_IC” subtype.

        asserts.assert_true(
            await self.check_operational_service_has_txt_ic(),
            'TXT key "IC" was not found!'
        )

        self.step(5)    # Complete commissioning

        asserts.assert_not_equal(
            self.commissionee_node_id, 0,
            "commissionee_node_id was not set before calling ContinueCommissioningAfterConnectNetworkRequest"
        )

        log.info("commissionee_node_id : 0x%X", self.commissionee_node_id)

        effective_node_id = await self.default_controller.ContinueCommissioningAfterConnectNetworkRequest(
            self.commissionee_node_id
        )

        asserts.assert_equal(
            effective_node_id,
            self.commissionee_node_id,
            "Effective node ID returned by ContinueCommissioningAfterConnectNetworkRequest "
            "does not match commissionee_node_id"
        )

        asserts.assert_true(self.send_complete_seen, "Stage 'send_complete_seen' was not seen!")

        self.step(6)    # Perform DNS-SD Discovery and check that the “_IC” subtype is no more present.

        # Wait a bit that mDNS service gets updated and propagated
        time.sleep(1)

        asserts.assert_false(
            await self.check_operational_service_has_txt_ic(),
            'TXT key "IC" is still present!'
        )

    def get_dut_instance_name(self, log_result: bool = False) -> str:
        node_id = self.dut_node_id
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'
        if log_result:
            log.info("\n\n\tDUT Instance Name: %s\n", instance_name)
        return instance_name

    def get_operational_subtype(self, log_result: bool = False) -> str:
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        operational_subtype = f'_I{compressed_fabric_id:016X}._sub.{MdnsServiceType.OPERATIONAL.value}'
        if log_result:
            log.info("\n\n\tOperational Subtype: %s\n", operational_subtype)
        return operational_subtype

    async def check_operational_service_has_txt_ic(self) -> bool:
        # TH constructs the instance name for the DUT as the 64-bit compressed Fabric identifier, and the
        # assigned 64-bit Node identifier, each expressed as a fixed-length sixteen-character hexadecimal
        # string, encoded as ASCII (UTF-8) text using capital letters, separated by a hyphen.
        instance_name = self.get_dut_instance_name(log_result=True)
        log.info("instance_name: %s", instance_name)

        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"
        log.info("instance_qname: %s", instance_qname)

        mdns = MdnsDiscovery()
        srv_record = await mdns.get_srv_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            log_output=True
        )

        asserts.assert_true(
            srv_record is not None,
            f"Operational mDNS service '{instance_qname}' was not found"
        )

        asserts.assert_true(
            hasattr(srv_record, "txt"),
            f"Operational mDNS service '{instance_qname}' does not contain TXT data"
        )

        asserts.assert_is_not_none(
            srv_record.txt,
            f"Operational mDNS service '{instance_qname}' has no TXT record"
        )

        asserts.assert_true(
            isinstance(srv_record.txt, dict),
            f"Operational mDNS service '{instance_qname}' TXT data is not a dictionary: {srv_record.txt}"
        )

        log.info("Operational TXT record: %s", srv_record.txt)

        return "IC" in srv_record.txt and srv_record.txt["IC"] == "1"


if __name__ == "__main__":
    default_matter_test_main()
