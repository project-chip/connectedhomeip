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

_CHIP_ERROR_INTERNAL = 172
_CHIP_ERROR_MISSING_SECURE_SESSION = 0x77
_DEFAULT_FAILSAFE_EXPIRY_SECONDS = 60
_TIMING_TOLERANCE_SECONDS = 20


class TC_DD_3_27(MatterTestCommissioner):
    disable_wildcard_subscription = True

    def desc_TC_DD_3_27(self) -> str:
        return "[TC-DD-3.27] NFC-based commissioning - Fail-Safe Timer Behavior (Powered) [DUT as Commissionee]"

    def steps_TC_DD_3_27(self) -> list[TestStep]:
        return [
            TestStep(1, "Detecting the NFC Tag and reading the Payload", is_commissioning=False),
            TestStep(2, "Initiate commissioning through NFC (NTL) and establish a PASE session "
                     "using the Discriminator and Passcode found in the onboarding data"),
            TestStep(3, "Power up the DUT for the second phase of the commissioning. Once the operational CASE "
                     "session is established, halt for ExpiryLengthSeconds so the fail-safe timer expires on the "
                     "Commissionee, then resume. The commissioning SHALL fail on the CommissioningComplete step"),
            TestStep(4, "Switch off the DUT and restart the NFC Commissioning process to complete the full "
                     "commissioning"),
        ]

    def setup_test(self):
        super().setup_test()

        # Booleans to detect some commissioner stages
        self.unpowered_phase_complete_seen: bool = False
        self.find_operational_seen: bool = False
        self.send_complete_seen: bool = False

        # Filled at runtime
        self.commissionee_node_id = 0

        # Seconds to block in OnCommissioningStageStart("SendComplete"). None disables the halt.
        self.halt_before_send_complete_seconds: float | None = None

        def _stage_start_listener(node_id: int, stage):
            # Normalize stage to string
            if isinstance(stage, bytes):
                stage = stage.decode("utf-8", errors="replace")

            log.info("[_stage_start_listener] node=0x%X, stage=%s", node_id, stage)

            self.commissionee_node_id = node_id

            if stage == "UnpoweredPhaseComplete":
                log.info("Detected 'UnpoweredPhaseComplete' commissioning stage")
                self.unpowered_phase_complete_seen = True

            if stage == "FindOperationalForCommissioningComplete":
                log.info("Detected 'FindOperationalForCommissioningComplete' commissioning stage")
                self.find_operational_seen = True

            if stage == "SendComplete":
                log.info("Detected 'SendComplete' commissioning stage")
                self.send_complete_seen = True

                halt = self.halt_before_send_complete_seconds
                if halt is not None:
                    self.halt_before_send_complete_seconds = None  # one-shot
                    log.info("Halting %.1fs before CommissioningComplete so the DUT fail-safe expires", halt)
                    time.sleep(halt)
                    log.info("Halt complete, resuming commissioning")

        self._commissioning_stage_start_callback = _DevicePairingDelegate_OnCommissioningStageStartFunct(
            _stage_start_listener
        )
        self.default_controller.setCommissioningStageStartCallback(self._commissioning_stage_start_callback)

    @async_test_body
    async def test_TC_DD_3_27(self):

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

        commissioning_success = await self.commission_ntl_device(payload)
        asserts.assert_true(commissioning_success, "Device Commissioning using nfc transport has failed")
        asserts.assert_true(self.unpowered_phase_complete_seen, "Stage 'UnpoweredPhaseComplete' was not seen!")

        # Step 3: Power the DUT up and run the second phase, halting after the operational CASE session
        # is established so the DUT's fail-safe expires before CommissioningComplete is sent.
        self.step(3)

        expiry_length_seconds = self.user_params.get(
            "PIXIT.DD.FailsafeExpiryLengthSeconds", _DEFAULT_FAILSAFE_EXPIRY_SECONDS)

        self.wait_for_user_input(prompt_msg="Power ON the device")

        asserts.assert_not_equal(
            self.commissionee_node_id, 0,
            "commissionee_node_id was not set before calling ContinueCommissioningAfterConnectNetworkRequest"
        )
        log.info("commissionee_node_id : 0x%X", self.commissionee_node_id)

        self.find_operational_seen = False
        self.send_complete_seen = False
        self.halt_before_send_complete_seconds = expiry_length_seconds + _TIMING_TOLERANCE_SECONDS

        started_at = time.monotonic()
        with asserts.assert_raises(ChipStackError) as failure:  # chipstack-ok
            await self.default_controller.ContinueCommissioningAfterConnectNetworkRequest(
                self.commissionee_node_id
            )
        elapsed = time.monotonic() - started_at
        log.info("Second commissioning phase failed after %.2fs with err=%s", elapsed, failure.exception.err)

        # The CASE session must have been established: kSendComplete is only reached once
        # kFindOperationalForCommissioningComplete has produced an operational device proxy.
        asserts.assert_true(
            self.find_operational_seen,
            "Stage 'FindOperationalForCommissioningComplete' was not seen: the operational CASE session "
            "was never established, so the failure did not happen where this test expects it to"
        )
        asserts.assert_true(
            self.send_complete_seen,
            "Stage 'SendComplete' was not seen: commissioning failed before reaching CommissioningComplete"
        )
        asserts.assert_true(
            elapsed >= expiry_length_seconds,
            f"Second phase failed after only {elapsed:.2f}s, before the {expiry_length_seconds}s "
            f"fail-safe could have expired on the Commissionee"
        )
        asserts.assert_equal(
            failure.exception.err, _CHIP_ERROR_INTERNAL,
            f"Expected CHIP_ERROR_INTERNAL({_CHIP_ERROR_INTERNAL}), which is how a rejected "
            f"CommissioningCompleteResponse surfaces, got {failure.exception.err}"
        )

        with asserts.assert_raises(ChipStackError) as invoke_failure:  # chipstack-ok
            await self.send_single_cmd(
                cmd=Clusters.GeneralCommissioning.Commands.CommissioningComplete(),
                node_id=self.dut_node_id, endpoint=0)

        log.info("CommissioningComplete after fail-safe expiry failed with err=%s", invoke_failure.exception.err)

        asserts.assert_equal(
            invoke_failure.exception.err, _CHIP_ERROR_MISSING_SECURE_SESSION,
            f"Expected CHIP_ERROR_MISSING_SECURE_SESSION(0x{_CHIP_ERROR_MISSING_SECURE_SESSION:02X}) when re-sending "
            f"CommissioningComplete, which is how the fail-safe expiry tearing down the operational session "
            f"surfaces, got {invoke_failure.exception.err}"
        )

        self.default_controller.ExpireSessions(self.dut_node_id)

        # Step 4: Restart NFC commissioning from scratch and take it all the way through.
        self.step(4)

        self.unpowered_phase_complete_seen = False
        self.find_operational_seen = False
        self.send_complete_seen = False
        self.halt_before_send_complete_seconds = None

        self.wait_for_user_input(prompt_msg="Power OFF the DUT, then bring its NFC interface close to the NFC"
                                 " reader again to restart commissioning")

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

        self.wait_for_user_input(prompt_msg="Power ON the device")

        effective_node_id = await self.default_controller.ContinueCommissioningAfterConnectNetworkRequest(
            self.commissionee_node_id
        )
        asserts.assert_equal(
            effective_node_id,
            self.commissionee_node_id,
            "Effective node ID returned by ContinueCommissioningAfterConnectNetworkRequest "
            "does not match commissionee_node_id"
        )
        asserts.assert_true(self.send_complete_seen, "Stage 'SendComplete' was not seen on restart!")


if __name__ == "__main__":
    default_matter_test_main()
