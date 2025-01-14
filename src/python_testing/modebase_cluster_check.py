#
#    Copyright (c) 2023 Project CHIP Authors
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

from chip.clusters.Types import NullValue
from mobly import asserts

logger = logging.getLogger(__name__)

# Maximum value for ModeTags according to specs is 16bits.
MAX_MODE_TAG = 0xFFFF
# According to specs, the specific MfgTags should be defined in the range 0x8000 - 0xBFFF
START_MFGTAGS_RANGE = 0x8000
END_MFGTAGS_RANGE = 0xBFFF


class ModeBaseClusterChecks:
    """ Class that holds the common Mode checks between TCs

    Several TCs have similar checks in place for functionality that is common among them.
    This class holds most of this common functionality to avoid duplicating code with the same validations.

    Link to spec:
    https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/modebase_common.adoc


    Attributes:
        modebase_derived_cluster: A reference to the cluster to be tested, it should be a derived from the Mode Base cluster.
    """

    def __init__(self, modebase_derived_cluster):
        self.mode_tags = [tag.value for tag in modebase_derived_cluster.Enums.ModeTag]
        self.cluster = modebase_derived_cluster
        self.attributes = modebase_derived_cluster.Attributes

    async def check_supported_modes_and_labels(self, endpoint):
        """ Verifies the device supported modes and labels.

        Checks that the SupportedModes attribute has the expected structure and values like:
        - Between 2 and 255 entries.
        - The Mode values of all entries are unique.
        - The Label values of all entries are unique.

        Args:
          endpoint: The endpoint used for the requests to the cluster.

        Returns:
          A list of ModeOptionStruct supported by the cluster.
        """
        # Get the supported modes
        supported_modes = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                         cluster=self.cluster,
                                                                         attribute=self.attributes.SupportedModes)

        # Check if the list of supported modes is larger than 2
        asserts.assert_greater_equal(len(supported_modes), 2, "SupportedModes must have at least 2 entries!")
        # Check that supported modes are less than 255
        asserts.assert_less_equal(len(supported_modes), 255, "SupportedModes must have at most 255 entries!")

        # Check for repeated labels or modes
        labels = set()
        modes = set()
        for mode_option_struct in supported_modes:
            # Verify that the modes in all ModeOptionStruct in SupportedModes are unique.
            if mode_option_struct.mode in modes:
                asserts.fail("SupportedModes can't have repeated Mode values")
            else:
                modes.add(mode_option_struct.mode)
            # Verify that the labels in all ModeOptionStruct in SupportedModes are unique.
            if mode_option_struct.label in labels:
                asserts.fail("SupportedModes can't have repeated Label values")
            else:
                labels.add(mode_option_struct.label)

        return supported_modes

    def check_tags_in_lists(self, supported_modes, required_tags=None):
        """ Validates the ModeTags values.

        This function evaluates the ModeTags of each ModeOptionStruct:
        - Should have at least one tag.
        - Should be maximum 16bits in size.
        - Should be a Mfg tag or one of the supported ones (either common or specific).
        - Should have at least one common or specific tag.
        - If defined, verify that at least one of the "required_tags" exists.

        Args:
          supported_modes: A list of ModeOptionStruct.
          required_tags: List of tags that are required according to the cluster spec.
        """
        # Verify the ModeTags on each ModeOptionStruct
        for mode_option_struct in supported_modes:
            # Shuld have at least one entry
            if len(mode_option_struct.modeTags) == 0:
                asserts.fail("The ModeTags field should have at least one entry.")

            # Check each ModelTag
            at_least_one_common_or_derived = False
            for tag in mode_option_struct.modeTags:
                # Value should not larger than 16bits
                if not (0 <= tag.value <= MAX_MODE_TAG):
                    asserts.fail("Tag should not be larger than 16bits.")

                # Check if is tag is common, derived or mfg.
                is_mfg = (START_MFGTAGS_RANGE <= tag.value <= END_MFGTAGS_RANGE)
                if not (is_mfg or tag.value in self.mode_tags):
                    asserts.fail("Mode tag value is not a common, derived or vendor tag.")

                # Confirm if tag is common or derived.
                if not is_mfg:
                    at_least_one_common_or_derived = True

            if not at_least_one_common_or_derived:
                asserts.fail("There should be at least one common or derived tag on each ModeOptionsStruct")

        if required_tags:
            has_required_tags = False
            for mode_options_struct in supported_modes:
                has_required_tags = any(tag.value in required_tags for tag in mode_options_struct.modeTags)
                if has_required_tags:
                    break
            asserts.assert_true(has_required_tags, "No ModeOptionsStruct has the required tags.")

    async def read_and_check_mode(self, endpoint, mode, supported_modes, is_nullable=False):
        """Evaluates the current mode

        This functions checks if the requested mode attribute has a valid value from the SupportedModes,
        supports optional nullable values.

        Args:
          endpoint: The endpoint used for the requests to the cluster.
          mode: Mode that will be verified.
          supported_modes: A list of ModeOptionStruct.
          is_nullable: Optional argument to indicate if the tested mode allows NullValue
        """
        mode_value = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                    cluster=self.cluster,
                                                                    attribute=mode)
        supported_modes_dut = {mode_option_struct.mode for mode_option_struct in supported_modes}
        is_valid = mode_value in supported_modes_dut
        if is_nullable and mode_value == NullValue:
            is_valid = True
        asserts.assert_true(is_valid, f"{mode} not supported")
