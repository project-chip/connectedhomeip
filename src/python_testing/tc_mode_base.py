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

from mobly import asserts
from chip.clusters.Types import NullValue

logger = logging.getLogger(__name__)

# Maximum value for ModeTags according to specs is 16bits.
MAX_MODE_TAG = 0xFFFF
# According to specs, the specific MfgTags should be defined in the range 0x8000 - 0xBFFF
START_MFGTAGS_RANGE = 0x8000
END_MFGTAGS_RANGE = 0xBFFF


class ClusterModeCheck:
    """ Class that holds the common Mode checks between TCs

    Several TCs have similar checks in place for functionality that is common among them.
    This class holds most of this common functionality to avoid duplicating code with the same validations.

    Link to spec:
    https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/cluster/modebase_common.adoc


    Attributes:
        requested_cluster: A reference to the cluster to be tested, it should be a derived from the Mode Base cluster.
    """

    def __init__(self, requested_cluster):
        self.modeTags = [tag.value for tag in requested_cluster.Enums.ModeTag]
        self.requested_cluster = requested_cluster
        self.attributes = requested_cluster.Attributes
        self.supported_modes_dut = set()
        self.supported_modes = None

    async def check_supported_modes_and_labels(self, endpoint):
        """ Verifies the device supported modes and labels.

        Checks that the SupportedModes attribute has the expected structure and values like:
        - Between 2 and 255 entries.
        - The Mode values of all entries are unique.
        - The Label values of all entries are unique.
        """
        # Get the supported modes
        self.supported_modes = await self.read_single_attribute_check_success(endpoint=endpoint,
                                                                              cluster=self.requested_cluster,
                                                                              attribute=self.attributes.SupportedModes)

        # Check if the list of supported modes is larger than 2
        asserts.assert_greater_equal(len(self.supported_modes), 2, "SupportedModes must have at least 2 entries!")
        # Check that supported modes are less than 255
        asserts.assert_less_equal(len(self.supported_modes), 255, "SupportedModes must have at most 255 entries!")

        # Check for repeated labels or modes
        labels = set()
        for mode_options_struct in self.supported_modes:
            # Verify that the modes in all ModeOptionsStruct in SupportedModes are unique.
            if mode_options_struct.mode in self.supported_modes_dut:
                asserts.fail("SupportedModes can't have repeated Mode values")
            else:
                self.supported_modes_dut.add(mode_options_struct.mode)
            # Verify that the labels in all ModeOptionsStruct in SupportedModes are unique.
            if mode_options_struct.label in labels:
                asserts.fail("SupportedModes can't have repeated Label values")
            else:
                labels.add(mode_options_struct.label)

    def check_tags_in_lists(self, requiredtags=None):
        """ Validates the ModeTags values.

        This function evaluates the ModeTags of each ModeOptionsStruct:
        - Should have at least one tag.
        - Should be maximum 16bits in size.
        - Should be a Mfg tag or one of the supported ones (either common or specific).
        - Should have at least one common or specific tag.
        - If defined, verify that at least one of the "requiredTags" exists.
        """
        # Verify the ModeTags on each ModeOptionsStruct
        for mode_options_struct in self.supported_modes:
            # Shuld have at least one entry
            if len(mode_options_struct.modeTags) == 0:
                asserts.fail("The ModeTags field should have at least one entry.")

            # Check each ModelTag
            at_least_one_common_or_derived = False
            for tag in mode_options_struct.modeTags:
                # Value should not larger than 16bits
                if tag.value > MAX_MODE_TAG or tag.value < 0:
                    asserts.fail("Tag should not be larger than 16bits.")

                # Check if is tag is common, derived or mfg.
                is_mfg = (START_MFGTAGS_RANGE <= tag.value <= END_MFGTAGS_RANGE)
                if (tag.value not in self.modeTags and
                        not is_mfg):
                    asserts.fail("Mode tag value is not a common, derived or vendor tag.")

                # Confirm if tag is common or derived.
                if not is_mfg:
                    at_least_one_common_or_derived = True

            if not at_least_one_common_or_derived:
                asserts.fail("There should be at least one common or derived tag on each ModeOptionsStruct")

        if requiredtags:
            has_required_tags = False
            for mode_options_struct in self.supported_modes:
                has_required_tags = any(tag.value in requiredtags for tag in mode_options_struct.modeTags)
                if has_required_tags:
                    break
            asserts.assert_true(has_required_tags, "No ModeOptionsStruct has the required tags.")

    async def read_and_check_mode(self, endpoint, mode, is_nullable=False):
        """Evaluates the current mode

        This functions checks if the requested mode attribute has a valid value from the SupportedModes,
        supports optional nullable values.
        """
        mode_value = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=self.requested_cluster, attribute=mode)
        is_valid = mode_value in self.supported_modes_dut
        if is_nullable and mode_value == NullValue:
            is_valid = True
        asserts.assert_true(is_valid, f"{mode} not supported")
