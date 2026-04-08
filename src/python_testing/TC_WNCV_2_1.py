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
from matter.clusters.Types import NullValue
from matter.interaction_model import Status as IMStatus
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import assert_valid_uint8, assert_valid_uint16
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


def _combined_bitmap_mask(bitmap_enum) -> int:
    """Return an integer mask of all defined bits in a bitmap enum."""
    mask = 0
    for bitmask in bitmap_enum:
        mask |= int(bitmask)
    return mask


def _field_value(value: int, mask: int) -> int:
    """Extract and right-shift a multi-bit field from value using mask."""
    shift = (mask & -mask).bit_length() - 1
    return (value & mask) >> shift


# Mandatory attribute IDs defined by the WindowCovering cluster spec (section 5.3.5).
# These must always be present in the DUT's AttributeList regardless of feature flags.
MANDATORY_ATTRIBUTE_IDS = {
    Clusters.WindowCovering.Attributes.Type.attribute_id,
    Clusters.WindowCovering.Attributes.ConfigStatus.attribute_id,
    Clusters.WindowCovering.Attributes.OperationalStatus.attribute_id,
    Clusters.WindowCovering.Attributes.EndProductType.attribute_id,
    Clusters.WindowCovering.Attributes.Mode.attribute_id,
}


class TC_WNCV_2_1(MatterBaseTest):

    def desc_TC_WNCV_2_1(self) -> str:
        return "[TC-WNCV-2.1] Attributes with DUT as Server"

    def pics_TC_WNCV_2_1(self) -> list[str]:
        return ["WNCV.S"]

    def steps_TC_WNCV_2_1(self) -> list[TestStep]:
        return [
            TestStep(1,    "Commissioning, already done", is_commissioning=True),
            # Mandatory Attributes
            TestStep("1a", "TH reads from the DUT the (0) Type attribute"),
            TestStep("1b", "TH reads from the DUT the (7) ConfigStatus attribute"),
            TestStep("1c", "TH reads from the DUT the (10) OperationalStatus attribute"),
            TestStep("1d", "TH reads from the DUT the (13) EndProductType attribute"),
            TestStep("1e", "TH reads from the DUT the (23) Mode attribute"),
            TestStep("1f", "TH writes value 0 to the (23) Mode attribute and verifies success, then reads back to confirm"),
            # Conditionally Mandatory Attributes
            TestStep("2a", "TH reads from the DUT the (11) TargetPositionLiftPercent100ths attribute, if supported"),
            TestStep("2b", "TH reads from the DUT the (12) TargetPositionTiltPercent100ths attribute, if supported"),
            TestStep("2c", "TH reads from the DUT the (14) CurrentPositionLiftPercent100ths attribute, if supported"),
            TestStep("2d", "TH reads from the DUT the (15) CurrentPositionTiltPercent100ths attribute, if supported"),
            # Conditionally and Purely Optional Attributes
            TestStep("3a", "TH reads from the DUT the (26) SafetyStatus attribute, if supported"),
            TestStep("3b", "TH reads from the DUT the (5) NumberOfActuationsLift attribute, if supported"),
            TestStep("3c", "TH reads from the DUT the (6) NumberOfActuationsTilt attribute, if supported"),
            TestStep("3d", "TH reads from the DUT the (8) CurrentPositionLiftPercentage attribute, if supported"),
            TestStep("3e", "TH reads from the DUT the (9) CurrentPositionTiltPercentage attribute, if supported"),
        ]

    # WindowCovering cluster lives on endpoint 1 on most devices.
    # get_endpoint() returns the --endpoint CLI arg, falling back to this value.
    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_WNCV_2_1(self):

        self.endpoint = self.get_endpoint()
        cluster = Clusters.WindowCovering
        attributes = cluster.Attributes
        enums = cluster.Enums
        bitmaps = cluster.Bitmaps

        # ------------------------------------------------------------------ #
        # Step 1 — Commissioning (already done)
        # ------------------------------------------------------------------ #
        self.step(1)

        # --- AttributeList read & validation --------------------------------
        # Read once upfront. Used throughout for conditional attribute checks.
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.AttributeList
        )

        # Validate AttributeList is a non-empty list of integers (attribute IDs).
        asserts.assert_is_instance(attribute_list, list,
                                   "AttributeList must be a list")
        asserts.assert_greater(len(attribute_list), 0,
                               "AttributeList must not be empty")
        for attr_id in attribute_list:
            asserts.assert_is_instance(attr_id, int,
                                       f"Every entry in AttributeList must be an int, got {type(attr_id)}")

        # Verify every mandatory attribute ID is reported by the DUT.
        # A DUT that omits a mandatory attribute is non-conformant.
        for mandatory_id in MANDATORY_ATTRIBUTE_IDS:
            asserts.assert_in(
                mandatory_id, attribute_list,
                f"Mandatory attribute ID 0x{mandatory_id:04X} is missing from AttributeList"
            )
        log.info(f"AttributeList contains {len(attribute_list)} entries, all mandatory IDs present")

        # ================================================================== #
        # MANDATORY ATTRIBUTES
        # ================================================================== #

        # Step 1a — Type (enum8)
        self.step("1a")
        type_dut = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.Type
        )
        # Validate Type against generated enum entries from Objects.py.
        # Exclude kUnknownEnumValue because it is an internal SDK helper marker.
        valid_type_values = [
            member for name, member in enums.Type.__members__.items()
            if name != "kUnknownEnumValue"
        ]
        asserts.assert_in(
            type_dut, valid_type_values,
            f"Type value {type_dut} is not a valid Type enum entry"
        )
        log.info(f"Type: {type_dut} (valid)")

        # Step 1b — ConfigStatus (bitmap8)
        self.step("1b")
        config_status_dut = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.ConfigStatus
        )
        assert_valid_uint8(int(config_status_dut), "ConfigStatus should be a valid bitmap8 value")
        # Use _combined_bitmap_mask to derive reserved-bit mask from the SDK-generated
        # ConfigStatus bitmap enum — avoids hardcoding bit positions that could drift
        # from the spec if the cluster definition is updated.
        config_status_reserved_bits = int(config_status_dut) & ~_combined_bitmap_mask(bitmaps.ConfigStatus)
        asserts.assert_equal(
            config_status_reserved_bits, 0,
            f"ConfigStatus has reserved bits set: 0x{int(config_status_dut):02X}"
        )
        log.info(f"ConfigStatus: 0x{int(config_status_dut):02X} (valid, no reserved bits set)")

        # Step 1c — OperationalStatus (bitmap8)
        self.step("1c")
        operational_status_dut = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.OperationalStatus
        )
        assert_valid_uint8(int(operational_status_dut), "OperationalStatus should be a valid bitmap8 value")
        # Check reserved bits using the SDK-generated bitmap mask.
        op_status_reserved_bits = int(operational_status_dut) & ~_combined_bitmap_mask(bitmaps.OperationalStatus)
        asserts.assert_equal(
            op_status_reserved_bits, 0,
            f"OperationalStatus has reserved bits set: 0x{int(operational_status_dut):02X}"
        )
        # Each 2-bit movement field (Global / Lift / Tilt) must not be 0b11 (undefined).
        # _field_value extracts and right-shifts the field using its SDK-defined mask.
        for field_name, field_mask in [
            ("Global", int(bitmaps.OperationalStatus.kGlobal)),
            ("Lift",   int(bitmaps.OperationalStatus.kLift)),
            ("Tilt",   int(bitmaps.OperationalStatus.kTilt)),
        ]:
            field_val = _field_value(int(operational_status_dut), field_mask)
            asserts.assert_not_equal(
                field_val, _field_value(field_mask, field_mask),
                f"OperationalStatus {field_name} field has undefined value 0b11"
            )
        log.info(f"OperationalStatus: 0x{int(operational_status_dut):02X} (valid)")

        # Step 1d — EndProductType (enum8)
        self.step("1d")
        end_product_type_dut = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.EndProductType
        )
        # Validate EndProductType against generated enum entries from Objects.py.
        # Exclude kUnknownEnumValue because it is an internal SDK helper marker.
        valid_end_product_types = [
            member for name, member in enums.EndProductType.__members__.items()
            if name != "kUnknownEnumValue"
        ]
        asserts.assert_in(
            end_product_type_dut, valid_end_product_types,
            f"EndProductType value {end_product_type_dut} is not a valid EndProductType enum entry"
        )
        log.info(f"EndProductType: {end_product_type_dut} (valid)")

        # Step 1e — Mode (bitmap8)
        self.step("1e")
        mode_dut = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.Mode
        )
        assert_valid_uint8(int(mode_dut), "Mode should be a valid bitmap8 value")
        # Check reserved bits using the SDK-generated Mode bitmap mask.
        mode_reserved_bits = int(mode_dut) & ~_combined_bitmap_mask(bitmaps.Mode)
        asserts.assert_equal(
            mode_reserved_bits, 0,
            f"Mode has reserved bits set: 0x{int(mode_dut):02X}"
        )
        log.info(f"Mode before write: 0x{int(mode_dut):02X} (valid, no reserved bits set)")

        # Step 1f — Write 0 to Mode, validate response, then read back
        self.step("1f")

        # --- Write --------------------------------------------------------
        # expect_success=False so the framework does NOT assert internally —
        # we assert manually below to provide a descriptive error message.
        write_status = await self.write_single_attribute(
            attribute_value=attributes.Mode(0),
            endpoint_id=self.endpoint,
            expect_success=False,
        )
        asserts.assert_equal(
            write_status, IMStatus.Success,
            f"Write to Mode attribute failed — expected Status.Success, got {write_status}"
        )
        log.info(f"Write to Mode succeeded, status: {write_status}")

        # --- Read-back ----------------------------------------------------
        # After a successful write we read the attribute back and confirm
        # the DUT actually stored the value 0. This catches DUTs that ACK
        # the write but silently discard it.
        mode_after_write = await self.read_single_attribute_check_success(
            endpoint=self.endpoint,
            cluster=cluster,
            attribute=attributes.Mode
        )
        asserts.assert_equal(
            int(mode_after_write), 0,
            f"Mode read-back after write expected 0, got {mode_after_write}"
        )
        log.info(f"Mode read-back after write: {mode_after_write} (confirmed 0)")

        # ================================================================== #
        # CONDITIONALLY MANDATORY ATTRIBUTES
        # ================================================================== #

        # Helper used for Percent100ths nullable uint16 attributes (steps 2a–2d).
        # The SDK represents a null attribute value using the NullValue sentinel
        # (matter.clusters.Types.NullValue), NOT Python None. Checking against
        # None would silently pass a null through to assert_valid_uint16, causing
        # a spurious failure. We check for NullValue explicitly instead.
        # When a concrete value is present it must be within [0, 10000]
        # (representing 0.00% – 100.00%).
        def validate_percent100ths(value, attr_name: str):
            # The SDK represents null attribute values using NullValue sentinel,
            # not Python None — guard against it explicitly to avoid passing
            # NullValue through to assert_valid_uint16 and causing a spurious failure.
            if value is NullValue:
                log.info(f"{attr_name}: NullValue (position unknown — permitted by spec)")
                return
            asserts.assert_is_instance(
                value, int,
                f"{attr_name} expected int or NullValue, got {type(value)}"
            )
            assert_valid_uint16(value, f"{attr_name} should be a valid uint16 value")
            asserts.assert_greater_equal(value, 0,    f"{attr_name} is below minimum (0)")
            asserts.assert_less_equal(value, 10000,   f"{attr_name} is above maximum (10000)")
            log.info(f"{attr_name}: {value} (valid Percent100ths)")

        # Step 2a — TargetPositionLiftPercent100ths
        if attributes.TargetPositionLiftPercent100ths.attribute_id in attribute_list:
            self.step("2a")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.TargetPositionLiftPercent100ths
            )
            validate_percent100ths(value, "TargetPositionLiftPercent100ths")
        else:
            self.skip_step("2a")

        # Step 2b — TargetPositionTiltPercent100ths
        if attributes.TargetPositionTiltPercent100ths.attribute_id in attribute_list:
            self.step("2b")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.TargetPositionTiltPercent100ths
            )
            validate_percent100ths(value, "TargetPositionTiltPercent100ths")
        else:
            self.skip_step("2b")

        # Step 2c — CurrentPositionLiftPercent100ths
        if attributes.CurrentPositionLiftPercent100ths.attribute_id in attribute_list:
            self.step("2c")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.CurrentPositionLiftPercent100ths
            )
            validate_percent100ths(value, "CurrentPositionLiftPercent100ths")
        else:
            self.skip_step("2c")

        # Step 2d — CurrentPositionTiltPercent100ths
        if attributes.CurrentPositionTiltPercent100ths.attribute_id in attribute_list:
            self.step("2d")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.CurrentPositionTiltPercent100ths
            )
            validate_percent100ths(value, "CurrentPositionTiltPercent100ths")
        else:
            self.skip_step("2d")

        # ================================================================== #
        # CONDITIONALLY AND PURELY OPTIONAL ATTRIBUTES
        # ================================================================== #

        # Helper used for nullable Percent uint8 attributes (steps 3d–3e).
        # The SDK represents a null attribute value using the NullValue sentinel
        # (matter.clusters.Types.NullValue), NOT Python None. Same reasoning as
        # validate_percent100ths above — we guard against NullValue explicitly.
        # Concrete values must be within [0, 100] (representing 0% – 100%).
        def validate_percent(value, attr_name: str):
            # Same NullValue sentinel handling as validate_percent100ths above.
            if value is NullValue:
                log.info(f"{attr_name}: NullValue (position unknown — permitted by spec)")
                return
            asserts.assert_is_instance(
                value, int,
                f"{attr_name} expected int or NullValue, got {type(value)}"
            )
            assert_valid_uint8(value, f"{attr_name} should be a valid uint8 value")
            asserts.assert_greater_equal(value, 0,   f"{attr_name} is below minimum (0)")
            asserts.assert_less_equal(value, 100,    f"{attr_name} is above maximum (100)")
            log.info(f"{attr_name}: {value} (valid Percent)")

        # Step 3a — SafetyStatus (bitmap16)
        if attributes.SafetyStatus.attribute_id in attribute_list:
            self.step("3a")
            safety_status_dut = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.SafetyStatus
            )
            assert_valid_uint16(int(safety_status_dut), "SafetyStatus should be a valid bitmap16 value")
            log.info(f"SafetyStatus: {safety_status_dut}")
        else:
            self.skip_step("3a")

        # Step 3b — NumberOfActuationsLift (uint16)
        if attributes.NumberOfActuationsLift.attribute_id in attribute_list:
            self.step("3b")
            num_lift_dut = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.NumberOfActuationsLift
            )
            assert_valid_uint16(num_lift_dut, "NumberOfActuationsLift should be a valid uint16 value")
            log.info(f"NumberOfActuationsLift: {num_lift_dut}")
        else:
            self.skip_step("3b")

        # Step 3c — NumberOfActuationsTilt (uint16)
        if attributes.NumberOfActuationsTilt.attribute_id in attribute_list:
            self.step("3c")
            num_tilt_dut = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.NumberOfActuationsTilt
            )
            assert_valid_uint16(num_tilt_dut, "NumberOfActuationsTilt should be a valid uint16 value")
            log.info(f"NumberOfActuationsTilt: {num_tilt_dut}")
        else:
            self.skip_step("3c")

        # Step 3d — CurrentPositionLiftPercentage (nullable Percent, uint8, [0..100])
        if attributes.CurrentPositionLiftPercentage.attribute_id in attribute_list:
            self.step("3d")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.CurrentPositionLiftPercentage
            )
            validate_percent(value, "CurrentPositionLiftPercentage")
        else:
            self.skip_step("3d")

        # Step 3e — CurrentPositionTiltPercentage (nullable Percent, uint8, [0..100])
        if attributes.CurrentPositionTiltPercentage.attribute_id in attribute_list:
            self.step("3e")
            value = await self.read_single_attribute_check_success(
                endpoint=self.endpoint, cluster=cluster,
                attribute=attributes.CurrentPositionTiltPercentage
            )
            validate_percent(value, "CurrentPositionTiltPercentage")
        else:
            self.skip_step("3e")


if __name__ == "__main__":
    default_matter_test_main()
    