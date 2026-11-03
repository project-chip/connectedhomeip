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

from mdns_discovery.mdns_discovery import DISCOVERY_TIMEOUT_SEC, MdnsDiscovery, MdnsServiceType
from mobly import asserts

import matter.testing.nfc
from matter.ChipDeviceCtrl import _DevicePairingDelegate_OnCommissioningStageStartFunct
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_3_24(MatterTestCommissioner):
    disable_wildcard_subscription = True

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
        """[TC-DD-3.24] NFC-based commissioning - DUT without power [DUT as Commissionee]"""

        self.wait_for_user_input(prompt_msg="Put the DUT in commissionable mode, bring its NFC interface close to the NFC reader"
                                 " and power OFF the DUT")

        # Step 1: Here we check if the Tag is connected to the Host machine and read the NFC Tag data
        self.step(1, "Detecting the NFC Tag and reading the Payload", is_commissioning=False)

        nfc_reader_index = self.user_params.get("NFC_Reader_index", 0)
        reader = matter.testing.nfc.NFCReader(nfc_reader_index)

        nfc_tag_data = reader.read_nfc_tag_data()
        asserts.assert_true(
            reader.is_onboarding_data(nfc_tag_data),
            f"'{nfc_tag_data}' is not a valid Matter URI"
        )
        self.matter_test_config.qr_code_content.append(nfc_tag_data)

        # Step 2: the NFC tag data is parsed and checked if the device supports NFC commissioning and commission begins
        self.step(2, "Validate the NFC bit in payload and perform the first phase of the commissioning, over NFC")
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

        # Force a commissioning over NTL
        commissioning_success = await self.commission_ntl_device(payload)
        asserts.assert_true(commissioning_success, "Device Commissioning using nfc transport has failed")
        asserts.assert_true(self.unpowered_phase_complete_seen, "Stage 'UnpoweredPhaseComplete' was not seen!")

        self.step(3, "DUT is powered ON.")
        self.wait_for_user_input(prompt_msg="Power ON the device")

        self.step(
            4,
            "Perform DNS-SD Discovery.",
            expectation="The DUT is discoverable over DNS-SD, advertises the “_IC” subtype, and includes TXT key `IC=1`"
        )

        asserts.assert_true(
            await self.check_operational_service_has_txt_ic(),
            'Incomplete commissioning mDNS advertisement not found (expected "_IC" subtype and TXT key IC=1)'
        )

        self.step(
            5,
            "Complete commissioning.",
            expectation="chip-tool application triggers the second phase of commissioning on the operational network. No error reported by Commissioner. Commissioner reports a successful commissioning."
        )

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

        self.step(
            6,
            "Perform DNS-SD Discovery.",
            expectation="The DUT is discoverable over DNS-SD, no longer advertises the “_IC” subtype, and the TXT key `IC` is absent or set to `0`"
        )

        step_6_conditions_met = False
        retry_query_timeout_sec = 5.0
        max_step_6_attempts = 3
        for attempt in range(max_step_6_attempts):
            step_6_conditions_met = await self.check_operational_service_no_longer_advertises_ic(
                retry_query_timeout_sec,
            )
            if step_6_conditions_met:
                break

            log.info('Attempt %d/%d: Step 6 conditions not met yet, retrying in 1 second',
                     attempt + 1, max_step_6_attempts)
            await asyncio.sleep(1)

        asserts.assert_true(
            step_6_conditions_met,
            'Step 6 conditions not met after 3 attempts: expected discoverable operational service, no "_IC" subtype, and IC absent or set to 0.'
        )

    def get_dut_instance_name(self, log_result: bool = False) -> str:
        """Return the operational mDNS instance name for the DUT.

        The value is "<compressed-fabric-id>-<node-id>" using uppercase, zero-padded 16-hex-digit fields.
        """
        node_id = self.dut_node_id
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'
        if log_result:
            log.info("DUT Instance Name: %s", instance_name)
        return instance_name

    async def check_operational_service_has_txt_ic(self, query_timeout_sec: float = DISCOVERY_TIMEOUT_SEC) -> bool:
        """Check whether the DUT operational mDNS service advertises TXT key "IC" as "1".

        Args:
            query_timeout_sec: Per-query timeout, in seconds, used for SRV and TXT lookups.

        Returns:
            True if the TXT record contains ``IC=1``. False if the TXT record is missing,
            has no TXT payload, or does not contain ``IC=1``.

        Raises:
            AssertionError: If the operational SRV record is not found, or TXT payload is not a dictionary.
        """
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
            query_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        if srv_record is None:
            asserts.fail(f"Operational mDNS service '{instance_qname}' was not found")

        ic_subtype = f"_IC._sub.{MdnsServiceType.OPERATIONAL.value}"
        ptr_records = await mdns.get_ptr_records(
            service_types=[ic_subtype],
            discovery_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        asserts.assert_true(
            any(record.instance_name == instance_name for record in ptr_records),
            f"Operational mDNS service '{instance_qname}' does not advertise subtype '{ic_subtype}'"
        )

        txt_record = await mdns.get_txt_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            query_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        if txt_record is None:
            log.info("Operational mDNS service '%s' has no TXT record", instance_qname)
            return False

        if not hasattr(txt_record, "txt"):
            log.info("Operational mDNS service '%s' does not contain TXT data", instance_qname)
            return False

        if txt_record.txt is None:
            log.info("Operational mDNS service '%s' has no TXT record", instance_qname)
            return False

        asserts.assert_true(
            isinstance(txt_record.txt, dict),
            f"Operational mDNS service '{instance_qname}' TXT data is not a dictionary: {txt_record.txt}"
        )

        log.info("Operational TXT record: %s", txt_record.txt)

        ic_value = txt_record.txt.get("IC")
        if isinstance(ic_value, bytes):
            ic_value = ic_value.decode("utf-8", errors="replace")
        return ic_value == "1"

    async def check_operational_service_no_longer_advertises_ic(
            self,
            query_timeout_sec: float = DISCOVERY_TIMEOUT_SEC,
    ) -> bool:
        """Check post-commissioning DNS-SD conditions for step 6.

        The expected state is:
        - The DUT remains discoverable over operational DNS-SD.
        - The DUT no longer advertises the ``_IC`` subtype.
        - The operational TXT key ``IC`` is absent or set to ``"0"``.

        Args:
            query_timeout_sec: Per-query timeout, in seconds, used for SRV/TXT/PTR lookups.

        Returns:
            True when all step 6 conditions are met, otherwise False.
            False when the operational SRV or TXT record is temporarily missing, when TXT payload
            is not yet available, or when the DUT still advertises ``_IC`` or ``IC=1``.
        """
        instance_name = self.get_dut_instance_name(log_result=True)
        instance_qname = f"{instance_name}.{MdnsServiceType.OPERATIONAL.value}"

        mdns = MdnsDiscovery()

        # Discoverable over operational DNS-SD.
        srv_record = await mdns.get_srv_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            query_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        if srv_record is None:
            log.info("Operational mDNS service '%s' was not found yet", instance_qname)
            return False

        # No longer advertising the _IC subtype.
        ic_subtype = f"_IC._sub.{MdnsServiceType.OPERATIONAL.value}"
        ptr_records = await mdns.get_ptr_records(
            service_types=[ic_subtype],
            discovery_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        advertises_ic_subtype = any(record.instance_name == instance_name for record in ptr_records)

        if advertises_ic_subtype:
            log.info("Operational mDNS service '%s' still advertises subtype '%s'", instance_qname, ic_subtype)

        txt_payload = None
        if hasattr(srv_record, "txt") and srv_record.txt is not None:
            asserts.assert_true(
                isinstance(srv_record.txt, dict),
                f"Operational mDNS service '{instance_qname}' SRV TXT data is not a dictionary: {srv_record.txt}"
            )
            txt_payload = srv_record.txt

        txt_record = await mdns.get_txt_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            query_timeout_sec=query_timeout_sec,
            log_output=True,
        )

        if txt_record is None:
            if txt_payload is None:
                log.info("Operational mDNS service '%s' TXT lookup returned no record", instance_qname)
                return False
            log.info(
                "Operational mDNS service '%s' TXT lookup returned no record; using TXT from SRV discovery data",
                instance_qname,
            )
        elif hasattr(txt_record, "txt") and txt_record.txt is not None:
            asserts.assert_true(
                isinstance(txt_record.txt, dict),
                f"Operational mDNS service '{instance_qname}' TXT data is not a dictionary: {txt_record.txt}"
            )
            txt_payload = txt_record.txt
        elif txt_payload is None:
            log.info("Operational mDNS service '%s' TXT payload not available yet", instance_qname)
            return False
        else:
            log.info(
                "Operational mDNS service '%s' TXT payload not available from TXT lookup; using TXT from SRV discovery data",
                instance_qname,
            )

        log.info("Operational TXT record used for IC check: %s", txt_payload)

        ic_value = txt_payload.get("IC")
        if isinstance(ic_value, bytes):
            ic_value = ic_value.decode("utf-8", errors="replace")
        ic_absent_or_zero = ic_value is None or ic_value == "0"

        if not ic_absent_or_zero:
            log.info("Operational TXT IC value is still '%s'", ic_value)

        return (not advertises_ic_subtype) and ic_absent_or_zero


if __name__ == "__main__":
    default_matter_test_main()
