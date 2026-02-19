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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${EVSE_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
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

from modebase_cluster_check import ModeBaseClusterChecks

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

cluster_demm_mode = Clusters.DeviceEnergyManagementMode


class TC_DEMM_1_2(MatterBaseTest, ModeBaseClusterChecks):

    def __init__(self, *args):
        MatterBaseTest.__init__(self, *args)
        ModeBaseClusterChecks.__init__(self,
                                       modebase_derived_cluster=cluster_demm_mode)

    def desc_TC_DEMM_1_2(self) -> str:
        return "[TC-DEMM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_DEMM_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedModes attribute."),
            TestStep(3, "TH reads from the DUT the CurrentMode attribute."),
        ]

    def pics_TC_DEMM_1_2(self) -> list[str]:
        return [
            "DEMM.S"
        ]

    def validate_no_optimization_modes(self, supported_modes):
        """
        Verify that supported modes do not combine the 'No Optimization' tag with other optimization tags.

        Args:
            supported_modes (list): A list of supported modes that contain tags to be checked.

        Raises:
            AssertionError: If a mode contains 'No Optimization' combined with another optimization tag.
        """
        all_modes_valid = True

        for mode in supported_modes:
            tags = [tag.value for tag in mode.modeTags]

            # Checking if the No Optimization tag is present in the list of tags
            if cluster_demm_mode.Enums.ModeTag.kNoOptimization in tags:
                # Verifying that No Optimization is not combined with other optimization tags
                if (
                    cluster_demm_mode.Enums.ModeTag.kDeviceOptimization in tags or
                    cluster_demm_mode.Enums.ModeTag.kLocalOptimization in tags or
                    cluster_demm_mode.Enums.ModeTag.kGridOptimization in tags
                ):
                    # Fails - No Optimization is combined with another optimization tag
                    all_modes_valid = False
                    self.assert_true(False, "No Optimization cannot be combined with other optimization tags")
                else:
                    # Success - No Optimization is not combined with other tags
                    log.info(
                        f'Extra Check - Mode {mode.label}: No Optimization is valid, not combined with any other optimization tags.')
            else:
                # No Optimization tag is not found in mode
                log.info(f'Extra Check - {mode.label}: No Optimization tag not found.')

        # Log the final result for the extra check after processing all modes
        if all_modes_valid:
            log.info('Extra Check: All modes passed No Optimization validation successfully.')
        else:
            log.info('Extra Check: One or more modes failed No Optimization validation.')

    @property
    def default_endpoint(self) -> int:
        return 1

    @async_test_body
    async def test_TC_DEMM_1_2(self):

        # Setup common mode check
        endpoint = self.get_endpoint()

        self.step(1)

        self.step(2)
        # Verify common checks for Mode Base as described in the TC-DEMM-1.2
        supported_modes = await self.check_supported_modes_and_labels(endpoint=endpoint)
        # According to the spec, No Optimization tag should be a supported tag.
        additional_tags = [cluster_demm_mode.Enums.ModeTag.kNoOptimization]
        self.check_tags_in_lists(supported_modes=supported_modes, required_tags=additional_tags)

        # Validation function for No Optimization check
        self.validate_no_optimization_modes(supported_modes)

        self.step(3)
        # Verify that the CurrentMode attribute has a valid value.
        mode = self.cluster.Attributes.CurrentMode
        await self.read_and_check_mode(endpoint=endpoint, mode=mode, supported_modes=supported_modes)


if __name__ == "__main__":
    default_matter_test_main()
