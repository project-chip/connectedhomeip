#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import assert_valid_uint8
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


def _bit_position(mask: int) -> int:
    """Return the bit position for a single-bit mask."""
    return int(mask).bit_length() - 1


def _bit(value: int, mask: int) -> int:
    """Return 1 when the given bitmap mask is set, otherwise 0."""
    return 1 if value & int(mask) else 0


class TC_WNCV_2_2(MatterBaseTest):

    def desc_TC_WNCV_2_2(self) -> str:
        return "[TC-WNCV-2.2] ConfigStatus attribute with DUT as Server"

    def pics_TC_WNCV_2_2(self) -> list[str]:
        return ["WNCV.S", "WNCV.S.A0007"]

    def steps_TC_WNCV_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute to determine supported features"),
            TestStep(3, "TH reads ConfigStatus attribute from DUT and validates all bits"),
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WNCV_2_2(self):
        self.endpoint = self.get_endpoint()
        cluster = Clusters.WindowCovering
        attributes = cluster.Attributes
        config_status_bits = cluster.Bitmaps.ConfigStatus
        feature_bits = cluster.Bitmaps.Feature

        # ------------------------------------------------------------------ #
        # Step 1 — Commissioning (already done)
        # ------------------------------------------------------------------ #
        self.step(1)

        # ------------------------------------------------------------------ #
        # Step 2 — Read FeatureMap to know which features are active
        # ------------------------------------------------------------------ #
        self.step(2)
        feature_map = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.FeatureMap
        )
        feature_map = int(feature_map)

        # Derive the two compound feature flags referenced by the spec:
        # Use generated bitmap masks from WindowCovering.Objects.Bitmaps.Feature.
        has_lf = bool(feature_map & int(feature_bits.kLift))
        has_tl = bool(feature_map & int(feature_bits.kTilt))
        has_pa_lf = bool(feature_map & int(feature_bits.kPositionAwareLift))
        has_pa_tl = bool(feature_map & int(feature_bits.kPositionAwareTilt))

        # Compound conditions used in bit 3/4/5/6 validation
        lf_and_pa_lf = has_lf and has_pa_lf   # WNCV.S.F00 & WNCV.S.F02
        tl_and_pa_tl = has_tl and has_pa_tl   # WNCV.S.F01 & WNCV.S.F04

        log.info(
            f"FeatureMap: 0x{feature_map:02X} | "
            f"LF={has_lf} TL={has_tl} PA_LF={has_pa_lf} PA_TL={has_pa_tl} | "
            f"LF&PA_LF={lf_and_pa_lf} TL&PA_TL={tl_and_pa_tl}"
        )

        # ------------------------------------------------------------------ #
        # Step 3 — Read and validate ConfigStatus bit by bit
        # ------------------------------------------------------------------ #
        self.step(3)
        config_status = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.ConfigStatus
        )
        config_status = int(config_status)
        assert_valid_uint8(config_status, "ConfigStatus must be a valid uint8/bitmap8")
        log.info(f"ConfigStatus raw value: 0x{config_status:02X} (0b{config_status:08b})")

        known_config_status_mask = 0
        for bitmask in config_status_bits:
            known_config_status_mask |= int(bitmask)

        # ── Reserved bits: must always be 0 ────────────────────────────────
        asserts.assert_equal(
            config_status & ~known_config_status_mask, 0,
            f"ConfigStatus has reserved bits set: 0x{config_status:02X}"
        )
        log.info("Reserved ConfigStatus bits: 0 ✓")

        # ── Bit 0: Operational — SHALL always be 1 ────────────────────────
        # The spec states this bit SHALL always be set to 1b (Operational).
        asserts.assert_equal(
            _bit(config_status, config_status_bits.kOperational), 1,
            f"ConfigStatus bit 0 (Operational) SHALL always be 1, got 0x{config_status:02X}"
        )
        log.info(
            f"Bit {_bit_position(config_status_bits.kOperational)} (Operational): "
            f"{_bit(config_status, config_status_bits.kOperational)} ✓ (SHALL be 1)"
        )

        # ── Bit 1: OnlineReserved — reserved, just log it ─────────────────
        log.info(
            f"Bit {_bit_position(config_status_bits.kOnlineReserved)} (OnlineReserved): "
            f"{_bit(config_status, config_status_bits.kOnlineReserved)} (reserved — ignored per spec)"
        )

        # ── Bit 2: LiftMovementReversed — 0=normal, 1=reversed ────────────
        # No specific value requirement — just log the state.
        lift_reversed = _bit(config_status, config_status_bits.kLiftMovementReversed)
        log.info(
            f"Bit {_bit_position(config_status_bits.kLiftMovementReversed)} (LiftMovementReversed): {lift_reversed} "
            f"({'Reversed' if lift_reversed else 'Normal'})"
        )

        # ── Bit 3: LiftPositionAware ───────────────────────────────────────
        # If LF & PA_LF features are both present → MUST be 1
        # If either feature is absent              → MUST be 0
        lift_pos_aware = _bit(config_status, config_status_bits.kLiftPositionAware)
        if lf_and_pa_lf:
            asserts.assert_equal(
                lift_pos_aware, 1,
                f"ConfigStatus bit 3 (LiftPositionAware) MUST be 1 when LF & PA_LF features are present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kLiftPositionAware)} (LiftPositionAware): "
                f"{lift_pos_aware} ✓ (LF & PA_LF active → must be 1)"
            )
        else:
            asserts.assert_equal(
                lift_pos_aware, 0,
                f"ConfigStatus bit 3 (LiftPositionAware) MUST be 0 when LF & PA_LF features are not both present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kLiftPositionAware)} (LiftPositionAware): "
                f"{lift_pos_aware} ✓ (LF & PA_LF not both active → must be 0)"
            )

        # ── Bit 4: TiltPositionAware ───────────────────────────────────────
        # If TL & PA_TL features are both present → MUST be 1
        # If either feature is absent              → MUST be 0
        tilt_pos_aware = _bit(config_status, config_status_bits.kTiltPositionAware)
        if tl_and_pa_tl:
            asserts.assert_equal(
                tilt_pos_aware, 1,
                f"ConfigStatus bit 4 (TiltPositionAware) MUST be 1 when TL & PA_TL features are present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kTiltPositionAware)} (TiltPositionAware): "
                f"{tilt_pos_aware} ✓ (TL & PA_TL active → must be 1)"
            )
        else:
            asserts.assert_equal(
                tilt_pos_aware, 0,
                f"ConfigStatus bit 4 (TiltPositionAware) MUST be 0 when TL & PA_TL features are not both present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kTiltPositionAware)} (TiltPositionAware): "
                f"{tilt_pos_aware} ✓ (TL & PA_TL not both active → must be 0)"
            )

        # ── Bit 5: LiftEncoderControlled ───────────────────────────────────
        # Only verified when LF & PA_LF are both present.
        # 0 = Timer controlled, 1 = Encoder controlled — both are valid.
        # Just log if not applicable.
        lift_encoder = _bit(config_status, config_status_bits.kLiftEncoderControlled)
        if lf_and_pa_lf:
            log.info(
                f"Bit {_bit_position(config_status_bits.kLiftEncoderControlled)} (LiftEncoderControlled): {lift_encoder} "
                f"({'Encoder controlled' if lift_encoder else 'Timer controlled'}) "
                f"(LF & PA_LF active — both values valid)"
            )
        else:
            asserts.assert_equal(
                lift_encoder, 0,
                f"ConfigStatus bit 5 (LiftEncoderControlled) MUST be 0 when LF & PA_LF are not both present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kLiftEncoderControlled)} (LiftEncoderControlled): "
                f"{lift_encoder} ✓ (LF & PA_LF not active → must be 0)"
            )

        # ── Bit 6: TiltEncoderControlled ───────────────────────────────────
        # Only verified when TL & PA_TL are both present.
        # 0 = Timer controlled, 1 = Encoder controlled — both are valid.
        tilt_encoder = _bit(config_status, config_status_bits.kTiltEncoderControlled)
        if tl_and_pa_tl:
            log.info(
                f"Bit {_bit_position(config_status_bits.kTiltEncoderControlled)} (TiltEncoderControlled): {tilt_encoder} "
                f"({'Encoder controlled' if tilt_encoder else 'Timer controlled'}) "
                f"(TL & PA_TL active — both values valid)"
            )
        else:
            asserts.assert_equal(
                tilt_encoder, 0,
                f"ConfigStatus bit 6 (TiltEncoderControlled) MUST be 0 when TL & PA_TL are not both present, "
                f"got 0x{config_status:02X}"
            )
            log.info(
                f"Bit {_bit_position(config_status_bits.kTiltEncoderControlled)} (TiltEncoderControlled): "
                f"{tilt_encoder} ✓ (TL & PA_TL not active → must be 0)"
            )

        # ── Final summary log ──────────────────────────────────────────────
        log.info(
            f"ConfigStatus 0x{config_status:02X} summary:\n"
            f"  bit {_bit_position(config_status_bits.kOperational)} Operational          = {_bit(config_status, config_status_bits.kOperational)} (SHALL be 1)\n"
            f"  bit {_bit_position(config_status_bits.kOnlineReserved)} OnlineReserved       = {_bit(config_status, config_status_bits.kOnlineReserved)} (reserved)\n"
            f"  bit {_bit_position(config_status_bits.kLiftMovementReversed)} LiftMovementReversed = {_bit(config_status, config_status_bits.kLiftMovementReversed)}\n"
            f"  bit {_bit_position(config_status_bits.kLiftPositionAware)} LiftPositionAware    = {_bit(config_status, config_status_bits.kLiftPositionAware)}\n"
            f"  bit {_bit_position(config_status_bits.kTiltPositionAware)} TiltPositionAware    = {_bit(config_status, config_status_bits.kTiltPositionAware)}\n"
            f"  bit {_bit_position(config_status_bits.kLiftEncoderControlled)} LiftEncoderCtrl      = {_bit(config_status, config_status_bits.kLiftEncoderControlled)}\n"
            f"  bit {_bit_position(config_status_bits.kTiltEncoderControlled)} TiltEncoderCtrl      = {_bit(config_status, config_status_bits.kTiltEncoderControlled)}\n"
            f"  reserved bits             = 0x{config_status & ~known_config_status_mask:02X} (must be 0)"
        )


if __name__ == "__main__":
    default_matter_test_main()
