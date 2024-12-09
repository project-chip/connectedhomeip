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


class TC_MODE_BASE:

    def initialize_tc_base(self, endpoint, attributes, requested_cluster, cluster_objects, *args):
        self.commonTags = {0x0: 'Auto',
                           0x1: 'Quick',
                           0x2: 'Quiet',
                           0x3: 'LowNoise',
                           0x4: 'LowEnergy',
                           0x5: 'Vacation',
                           0x6: 'Min',
                           0x7: 'Max',
                           0x8: 'Night',
                           0x9: 'Day'}
        self.specificTags = [tag.value for tag in requested_cluster.Enums.ModeTag]
        self.supported_modes_dut = set()
        self.cluster_objects = cluster_objects
        self.supported_modes = None
        self.endpoint = endpoint
        self.attributes = attributes

    async def check_supported_modes_and_labels(self):
        # Get the supported modes
        self.supported_modes = await self.read_single_attribute_check_success(endpoint=self.endpoint,
                                                                              cluster=self.cluster_objects,
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

    async def check_if_labels_in_lists(self, requiredtags=None):
        # Verify the ModeTags on each ModeOptionsStruct
        for mode_options_struct in self.supported_modes:
            # Shuld have at least one entry
            if len(mode_options_struct.modeTags) == 0:
                asserts.fail("The ModeTags field should have at least one entry.")

            # Check each ModelTag
            at_least_one_common_or_derived = False
            for tag in mode_options_struct.modeTags:
                # Value should not larger than 16bits
                if tag.value > 0xFFFF or tag.value < 0:
                    asserts.fail("Tag should not be larger than 16bits.")

                # Check if is tag is common, derived or mfg.
                is_mfg = (0x8000 <= tag.value <= 0xBFFF)
                if (tag.value not in self.commonTags and
                    tag.value not in self.specificTags and
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

    async def read_and_check_mode(self, mode, is_nullable=False):
        mode_value = await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=self.cluster_objects, attribute=mode)
        is_valid = mode_value in self.supported_modes_dut
        if is_nullable and mode_value == NullValue:
            is_valid = True
        asserts.assert_true(is_valid, f"{mode} not supported")
