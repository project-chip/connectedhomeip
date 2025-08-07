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
import chip.clusters as Clusters
from chip.testing.matter_testing import (
    MatterBaseTest,
    TestStep,
    default_matter_test_main,
    async_test_body
)
from mobly import asserts


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
        asserts.assert_true(len(output_list) > 0, "OutputList should not be empty")

        valid_indices = []

        for output in output_list:
            # Validate 'index'
            asserts.assert_true(hasattr(output, "index"), "'index' field is missing")
            asserts.assert_true(isinstance(output.index, int), "'index' should be an unsigned integer")
            asserts.assert_true(0 <= output.index <= 255, f"'index' should be in uint8 range (0-255), got {output.index}")
            valid_indices.append(output.index)

            # Validate 'outputType'
            asserts.assert_true(hasattr(output, "outputType"), "'outputType' field is missing")
            asserts.assert_true(isinstance(output.outputType, int), "'outputType' should be an  integer")
            asserts.assert_true(0 <= output.outputType <= 255,
                                f"'outputType' should be in enum8 range (0-255), got {output.outputType}")

            # Validate 'name'
            asserts.assert_true(hasattr(output, "name"), "'name' field is missing")
            asserts.assert_true(isinstance(output.name, str), "'name' should be a string")

        # Ensure 'index' values are unique
        asserts.assert_equal(
            len(valid_indices), len(set(valid_indices)),
            "Each 'index' in OutputList must be unique"
        )
        logging.info(f"Output Struct - index: {output.index}, outputType: {output.outputType}, name: {output.name}")

        # Step 3: Read CurrentOutput
        self.step(3)
        current_output = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.CurrentOutput
        )

        asserts.assert_true(isinstance(current_output, int), "CurrentOutput should be an integer")
        asserts.assert_true(0 <= current_output <= 255, "CurrentOutput should be in uint8 range (0-255)")
        asserts.assert_true(
            current_output in valid_indices,
            f"CurrentOutput index ({current_output}) must be one of: {valid_indices}"
        )


if __name__ == "__main__":
    default_matter_test_main()
