#
#   Copyright (c) 2025 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TV_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
# === END CI TEST ARGUMENTS ===


import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters import Attribute
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_AUDIOOUTPUT_7_5(MatterBaseTest):
    def desc_TC_AUDIOOUTPUT_7_5(self) -> str:
        return "[TC_AUDIOOUTPUT-7.5] AudioOutput CurrentOutput attribute validation"

    def pics_TC_AUDIOOUTPUT_7_5(self):
        return [
            "AUDIOOUTPUT.S",  # AudioOutput cluster is supported
        ]

    def steps_TC_AUDIOOUTPUT_7_5(self):
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads the OutputList attribute"),
            TestStep(3, "TH reads the CurrentOutput attribute"),
        ]

    @async_test_body
    async def test_TC_AUDIOOUTPUT_7_5(self):
        self.endpoint = self.get_endpoint()
        cluster = Clusters.AudioOutput
        attributes = cluster.Attributes

        self.step(1)

        # Step 2: Read OutputList
        self.step(2)
        output_list = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.OutputList
        )

        asserts.assert_true(isinstance(output_list, list), "OutputList should be a list")
        valid_indices = []

        for output in output_list:
            # Validate struct type
            asserts.assert_is_instance(output, cluster.Structs.OutputInfoStruct, f"Expected OutputInfoStruct, got {type(output)}")

            # Validate 'index'
            asserts.assert_true(hasattr(output, "index"), "'index' field is missing")
            asserts.assert_is_instance(output.index, int, "'index' should be an unsigned integer")
            asserts.assert_in(output.index, range(256), f"'index' should be in uint8 range (0-255)")
            valid_indices.append(output.index)

            # Validate 'outputType'
            asserts.assert_true(hasattr(output, "outputType"), "'outputType' field is missing")
            asserts.assert_is_instance(output.outputType, int, "'outputType' should be an integer")
            asserts.assert_in(output.outputType, range(256), f"'outputType' should be in enum8 range (0-255)")

            # Validate 'name'
            asserts.assert_true(hasattr(output, "name"), "'name' field is missing")
            asserts.assert_is_instance(output.name, str, "'name' should be a string")

            logging.info(f"Output Struct - index: {output.index}, outputType: {output.outputType}, name: {output.name}")

    # Step 3: Read CurrentOutput (only if OutputList is non-empty)
        if valid_indices:
            self.step(3)
            current_output = await self.read_single_attribute_check_success(
                cluster=cluster,
                attribute=attributes.CurrentOutput
            )

            asserts.assert_is_instance(current_output, int, "CurrentOutput should be an integer")
            asserts.assert_in(current_output, range(256), "CurrentOutput should be in uint8 range (0-255)")
            asserts.assert_in(
                current_output, valid_indices,
                f"CurrentOutput index ({current_output}) must be one of: {valid_indices}"
            )


if __name__ == "__main__":
    default_matter_test_main()
