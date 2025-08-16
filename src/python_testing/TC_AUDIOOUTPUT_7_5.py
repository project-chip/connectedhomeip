# Copyright (c) 2025 Project CHIP Authors
# Licensed under the Apache License, Version 2.0

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
from matter.testing.matter_asserts import assert_is_string, assert_valid_uint8
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
        enums = cluster.Enums

        self.step(1)

        # Step 2: Read OutputList
        self.step(2)
        output_list = await self.read_single_attribute_check_success(
            cluster=cluster,
            attribute=attributes.OutputList
        )

        asserts.assert_is_instance(output_list, list, "OutputList should be a list")
        valid_indices = []

        valid_enum_values = [
            enums.OutputTypeEnum.kHdmi,
            enums.OutputTypeEnum.kBt,
            enums.OutputTypeEnum.kOptical,
            enums.OutputTypeEnum.kHeadphone,
            enums.OutputTypeEnum.kInternal,
            enums.OutputTypeEnum.kOther,
        ]

        for output in output_list:
            # Validate struct type
            asserts.assert_is_instance(output, cluster.Structs.OutputInfoStruct, "Expected OutputInfoStruct")

            # Validate 'index'
            assert_valid_uint8(output.index, "'index' should be a valid uint8 value")
            valid_indices.append(output.index)

            # Validate 'outputType'
            assert_valid_uint8(output.outputType, "'outputType' should be a valid enum8 value")
            asserts.assert_in(output.outputType, valid_enum_values, "outputType is not a recognized OutputTypeEnum value")

            # Validate 'name'
            assert_is_string(output.name, "'name' should be a string")

            logging.info(f"Output Struct - index: {output.index}, outputType: {output.outputType}, name: {output.name}")

        # Step 3: Read CurrentOutput (only if OutputList is non-empty)
        if valid_indices:
            self.step(3)
            current_output = await self.read_single_attribute_check_success(
                cluster=cluster,
                attribute=attributes.CurrentOutput
            )

            assert_valid_uint8(current_output, "CurrentOutput should be a valid uint8 value")
            asserts.assert_in(
                current_output, valid_indices,
                "CurrentOutput index must be one of the indices in OutputList"
            )
        else:
            self.skip_test("OutputList is empty, skipping CurrentOutput validation.")


if __name__ == "__main__":
    default_matter_test_main()
