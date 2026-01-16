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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md"#defining-the-ci-test-arguments
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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import ClusterObjects
from matter.testing import matter_asserts
from matter.testing.conformance import is_provisional
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters
from matter.tlv import uint

logger = logging.getLogger(__name__)


class TC_DISHALM_2_1(MatterBaseTest):

    def desc_TC_DISHALM_2_1(self) -> str:
        return "198.2.1. [TC-DISHALM-2.1] Attributes with DUT as Server"

    def pics_TC_DISHALM_2_1(self):
        return [
            "DISHALM.S"
        ]

    def steps_TC_DISHALM_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the Mask attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(3, "TH reads from the DUT the Latch attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(4, "TH reads from the DUT the State attribute", "Verify that the DUT response contains a 32-bit value"),
            TestStep(5, "TH reads from the DUT the Supported attribute",
                     "Verify that the DUT response contains a 32-bit value. Verify that Supported, Mask, State, and Latch only contain valid alarm bits from the specification")
        ]

    def _get_valid_alarm_bitmap_mask(self, allow_provisional: bool = False) -> int:
        """
        Get the valid alarm bitmap mask from the DishwasherAlarm cluster specification.

        This method reads the AlarmBitmap definition from the data model XML files to
        determine which bits are valid and which are provisional.

        Args:
            allow_provisional: If True, include provisional alarm bits (for testing/verification)

        Returns:
            Bitmask with all valid alarm bits set (excluding provisional if allow_provisional is False)
        """
        # Get cluster ID for DishwasherAlarm (0x005D)
        dishwasher_alarm_cluster_id = uint(Clusters.DishwasherAlarm.id)

        # Build XML clusters from the latest data model
        xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_5)

        # Get the DishwasherAlarm cluster from parsed XML
        asserts.assert_in(dishwasher_alarm_cluster_id, xml_clusters,
                          f"DishwasherAlarm cluster (0x{dishwasher_alarm_cluster_id:04X}) not found in data model XML")
        xml_cluster = xml_clusters[dishwasher_alarm_cluster_id]

        # Get the AlarmBitmap from the cluster's bitmaps
        asserts.assert_in("AlarmBitmap", xml_cluster.bitmaps,
                          "AlarmBitmap not found in DishwasherAlarm cluster bitmaps")
        alarm_bitmap = xml_cluster.bitmaps["AlarmBitmap"]

        mask = 0
        provisional_bits = []
        non_provisional_bits = []

        # Iterate through all bitmap components (bitfields) and build the mask
        for bit_value, component in alarm_bitmap.components.items():
            bit_mask = 1 << bit_value
            if is_provisional(component.conformance):
                provisional_bits.append(component.name)
                if allow_provisional:
                    mask |= bit_mask
            else:
                non_provisional_bits.append(component.name)
                mask |= bit_mask

        logger.info(f"Non-provisional alarm bits: {non_provisional_bits}")
        logger.info(f"Provisional alarm bits: {provisional_bits}")
        logger.info(f"Valid alarm bitmap mask: 0x{mask:08X} (allow_provisional={allow_provisional})")

        asserts.assert_true(mask != 0, "No valid alarm bits found in AlarmBitmap specification")

        return mask

    def _validate_alarm_bitmap(self, attribute_name: str, bitmap_value: int,
                               valid_mask: int, supported_value: int = None):
        """
        Validate that an alarm bitmap only contains valid and supported bits.

        Performs two checks:
        1. All set bits are defined in the AlarmBitmap specification
        2. For Mask/State/Latch: all set bits are also set in Supported attribute

        Args:
            attribute_name: Name of the attribute being validated (for error messages)
            bitmap_value: The bitmap value to validate
            valid_mask: Mask of bits defined in the specification
            supported_value: Value of Supported attribute (None for Supported itself)

        Raises:
            AssertionError: If validation fails
        """
        # Check 1: Verify no undefined bits are set
        undefined_bits = bitmap_value & ~valid_mask
        if undefined_bits != 0:
            asserts_fail_msg = (
                f"{attribute_name} attribute contains undefined alarm bits. "
                f"Value: 0x{bitmap_value:08X}, Undefined bits: 0x{undefined_bits:08X}"
            )
            asserts.fail(asserts_fail_msg)

        # Check 2: For attributes other than Supported, verify bits are in Supported
        if supported_value is not None and attribute_name != "Supported":
            unsupported_bits = bitmap_value & ~supported_value
            if unsupported_bits != 0:
                asserts_fail_msg = (
                    f"{attribute_name} attribute contains bits not present in Supported attribute. "
                    f"Value: 0x{bitmap_value:08X}, Supported: 0x{supported_value:08X}, "
                    f"Unsupported bits: 0x{unsupported_bits:08X}"
                )
                asserts.fail(asserts_fail_msg)

        logger.info(f"{attribute_name} bitmap validation passed: 0x{bitmap_value:08X}")

    async def read_and_check_attributes_from_dishwasher_alarm(self, attribute: type[ClusterObjects.ClusterAttributeDescriptor]) -> int:
        resp = await self.read_single_attribute_check_success(
            cluster=self.cluster,
            attribute=attribute
        )

        matter_asserts.assert_valid_uint32(resp, attribute.__name__)

        logger.info(f"Reading attribute: {attribute}, response: {resp}")
        return int(resp)

    @run_if_endpoint_matches(has_cluster(Clusters.DishwasherAlarm))  # type: ignore[arg-type]
    async def test_TC_DISHALM_2_1(self):

        self.cluster = Clusters.DishwasherAlarm
        self.endpoint = self.get_endpoint()

        # Get test parameter for allowing provisional alarms (default: False for certification)
        allow_provisional = self.user_params.get("allow_provisional_alarms", False)
        logger.info(f"Test running with allow_provisional={allow_provisional}")

        self.step(1)

        self.step(2)
        mask = Clusters.DishwasherAlarm.Attributes.Mask
        mask_value = await self.read_and_check_attributes_from_dishwasher_alarm(mask)

        self.step(3)
        latch_attribute = Clusters.DishwasherAlarm.Attributes.Latch
        latch_value = None
        if await self.attribute_guard(endpoint=self.endpoint, attribute=latch_attribute):
            latch_value = await self.read_and_check_attributes_from_dishwasher_alarm(latch_attribute)

        self.step(4)
        state_attribute = Clusters.DishwasherAlarm.Attributes.State
        state_value = await self.read_and_check_attributes_from_dishwasher_alarm(state_attribute)

        self.step(5)
        supported_attribute = Clusters.DishwasherAlarm.Attributes.Supported
        supported_value = await self.read_and_check_attributes_from_dishwasher_alarm(supported_attribute)

        valid_mask = self._get_valid_alarm_bitmap_mask(allow_provisional)

        # Validate Supported attribute contains only spec-defined bits
        self._validate_alarm_bitmap("Supported", supported_value, valid_mask, supported_value=None)

        # Validate Mask attribute
        self._validate_alarm_bitmap("Mask", mask_value, valid_mask, supported_value)

        # Validate State attribute
        self._validate_alarm_bitmap("State", state_value, valid_mask, supported_value)

        # Validate Latch attribute (if present)
        if latch_value is not None:
            self._validate_alarm_bitmap("Latch", latch_value, valid_mask, supported_value)

        logger.info("All alarm bitmap validations passed successfully")


if __name__ == "__main__":
    default_matter_test_main()
