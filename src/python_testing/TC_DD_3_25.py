#
#    Copyright (c) 2026 Project CHIP Authors
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
import time

from mobly import asserts

import matter.clusters as Clusters
import matter.testing.nfc
from matter.ChipDeviceCtrl import _DevicePairingDelegate_OnCommissioningStageStartFunct
from matter.exceptions import ChipStackError
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

_CHIP_TIMEOUT_ERROR = 50
_CHIP_ERROR_NOT_CONNECTED = 72


class TC_DD_3_25(MatterTestCommissioner):
    disable_wildcard_subscription = True

    def desc_TC_DD_3_25(self) -> str:
        return "[TC-DD-3.25] NFC-based commissioning - Fail-Safe Timer Behavior (Unpowered) [DUT as Commissionee]"

    def steps_TC_DD_3_25(self) -> list[TestStep]:
        return [
            TestStep(1, "Detecting the NFC Tag and reading the Payload", is_commissioning=False),
            TestStep(2, "Initiate commissioning through NFC (NTL) and establish a PASE session "
                     "using the Discriminator and Passcode found in the onboarding data"),
            TestStep(3, "Arm Fail-Safe on the Commissionee with ExpiryLengthSeconds field set to 2 seconds"),
            TestStep(4, "Send the Read Request command continuously for longer than 2 seconds so that the "
                     "fail-safe timer expires on the Commissionee while the NFC field remains active"),
            TestStep(5, "Restart the NFC Commissioning process by bringing the DUT close to the NFC tag again"),
        ]

    def setup_test(self):
        super().setup_test()

        # Boolean to detect the unpowered-phase commissioning stage
        self.unpowered_phase_complete_seen: bool = False

        def _stage_start_listener(node_id: int, stage):
            # Normalize stage to string
            if isinstance(stage, bytes):
                stage = stage.decode("utf-8", errors="replace")

            log.info("[_stage_start_listener] node=0x%X, stage=%s", node_id, stage)

            if stage == "UnpoweredPhaseComplete":
                log.info("Detected 'UnpoweredPhaseComplete' commissioning stage")
                self.unpowered_phase_complete_seen = True

        self._commissioning_stage_start_callback = _DevicePairingDelegate_OnCommissioningStageStartFunct(
            _stage_start_listener
        )
        self.default_controller.setCommissioningStageStartCallback(self._commissioning_stage_start_callback)

    @async_test_body
    async def test_TC_DD_3_25(self):

        self.wait_for_user_input(prompt_msg="Ensure the DUT is unpowered and in commissionable mode, then bring"
                                 " its NFC interface close to the NFC reader")

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

        payload = SetupPayload().ParseQrCode(nfc_tag_data)
        asserts.assert_true(
            payload.supports_nfc_commissioning,
            "NFC Transport Layer bit of Discovery Capabilities Bitmask is not set"
        )

        # Step 2: Establish a PASE-only session over NFC (NTL).
        self.step(2)

        commissioning_method = self.matter_test_config.in_test_commissioning_method
        asserts.assert_is_not_none(commissioning_method, "in_test_commissioning_method must not be None")
        asserts.assert_true(
            str(commissioning_method).startswith("nfc-"),
            f"Expected in_test_commissioning_method to start with 'nfc-', got: {commissioning_method}"
        )
        self.matter_test_config.commissioning_method = commissioning_method

        try:
            commissionee = await self.default_controller.FindOrEstablishPASESession(
                setupCode=nfc_tag_data, nodeId=self.dut_node_id
            )
        except ChipStackError as e:  # chipstack-ok
            asserts.fail(f"PASE session establishment over NFC failed: {e}")
        asserts.assert_is_not_none(commissionee, "Failed to find or establish PASE session over NFC")

        # Step 3: Arm the Fail-Safe with a short expiry directly on the just-established PASE session
        self.step(3)

        arm_cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=2)
        resp = await self.send_single_cmd(cmd=arm_cmd, node_id=self.dut_node_id, endpoint=0)
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafe failed with errorCode={resp.errorCode}"
        )
        fail_safe_armed_at = time.monotonic()

        # Step 4: Poll a Read Request until the Fail-Safe expires and the DUT tears down the PASE session,
        # at which point the Read Request is expected to fail with a TIMEOUT error.
        self.step(4)

        expiry_length_seconds = 2
        timing_tolerance_seconds = 5  # margin for MRP retries / NFC polling overhead; tune on real HW
        max_wait_seconds = expiry_length_seconds + timing_tolerance_seconds + 15  # hard safety bound
        poll_interval_seconds = 0.5

        breadcrumb_attr = Clusters.GeneralCommissioning.Attributes.Breadcrumb
        failure_seen = False
        failure_err = None
        elapsed_at_failure = None

        while time.monotonic() - fail_safe_armed_at <= max_wait_seconds:
            elapsed = time.monotonic() - fail_safe_armed_at
            try:
                await self.default_controller.ReadAttribute(nodeId=self.dut_node_id, attributes=[(0, breadcrumb_attr)])
                log.info("Read Request succeeded at t=%.2fs", elapsed)
            except ChipStackError as e:  # chipstack-ok
                failure_seen = True
                failure_err = e
                elapsed_at_failure = time.monotonic() - fail_safe_armed_at
                log.info("Read Request failed at t=%.2fs with err=%s", elapsed_at_failure, e.err)
                break
            await asyncio.sleep(poll_interval_seconds)

        asserts.assert_true(
            failure_seen,
            f"Read Request never failed within {max_wait_seconds}s of arming a {expiry_length_seconds}s Fail-Safe"
        )
        asserts.assert_true(
            elapsed_at_failure >= expiry_length_seconds,
            f"Read Request failed too early (t={elapsed_at_failure:.2f}s), before the "
            f"{expiry_length_seconds}s Fail-Safe could have expired"
        )
        asserts.assert_true(
            failure_err.err in (_CHIP_TIMEOUT_ERROR, _CHIP_ERROR_NOT_CONNECTED),
            f"Expected TIMEOUT({_CHIP_TIMEOUT_ERROR}) or NOT_CONNECTED({_CHIP_ERROR_NOT_CONNECTED}) reading "
            f"Breadcrumb over PASE after Fail-Safe expiry, got {failure_err.err}"
        )

        self.default_controller.ExpireSessions(self.dut_node_id)

        # Step 5: Restart NFC commissioning from scratch. Success here means the unpowered NFC phase completes
        self.step(5)

        self.unpowered_phase_complete_seen = False

        self.wait_for_user_input(prompt_msg="Bring the DUT's NFC interface close to the NFC reader again to"
                                 " restart commissioning (keep the DUT unpowered)")

        nfc_tag_data_2 = reader.read_nfc_tag_data()
        log.info("NFC Tag data (restart) : '%s'", nfc_tag_data_2)
        asserts.assert_true(
            reader.is_onboarding_data(nfc_tag_data_2),
            f"'{nfc_tag_data_2}' is not a valid Matter URI"
        )

        payload_2 = SetupPayload().ParseQrCode(nfc_tag_data_2)
        asserts.assert_true(payload_2.supports_nfc_commissioning, "Device does not Support NFC Commissioning")

        commissioning_success = await self.commission_ntl_device(payload_2)
        asserts.assert_true(commissioning_success, "Device Commissioning restart using nfc transport has failed")
        asserts.assert_true(self.unpowered_phase_complete_seen, "Stage 'UnpoweredPhaseComplete' was not seen on restart!")


if __name__ == "__main__":
    default_matter_test_main()
