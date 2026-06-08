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
#     script-args: >
#       --hex-arg UPPER.CASE.HEX_ARG_WITH_UPPER_CASE_VAL:01020A
#       --hex-arg UPPER.CASE.HEX_ARG_WITH_LOWER_CASE_VAL:01020a
#       --hex-arg lower.case.hex_arg:01020a
#       --hex-arg MiXed.case.hex_arg:01020a
#       --string-arg UPPER.CASE.STRING_ARG:TestyTesty
#       --string-arg lower.case.string_arg:TestyTesty
#       --string-arg MiXed.case.string_arg:TestyTesty
#       --int-arg UPPER.CASE.INT_ARG:15
#       --int-arg lower.case.int_arg:15
#       --int-arg MiXed.case.int_arg:15
#       --bool-arg UPPER.CASE.BOOL_ARG_WITH_LOWER_CASE_VAL:true
#       --bool-arg UPPER.CASE.BOOL_ARG_WITH_UPPER_CASE_VAL:TRUE
#       --bool-arg UPPER.CASE.BOOL_ARG_WITH_MIXED_CASE_VAL:True
#       --bool-arg UPPER.CASE.BOOL_ARG_WITH_NUMBER_VAL_1:1
#       --bool-arg UPPER.CASE.BOOL_ARG_WITH_NUMBER_VAL_0:0
#       --bool-arg lower.case.bool_arg_with_lower_case_val:true
#       --bool-arg lower.case.bool_arg_with_upper_case_val:TRUE
#       --bool-arg lower.case.bool_arg_with_mixed_case_val:True
#       --bool-arg lower.case.bool_arg_with_number_val_1:1
#       --bool-arg lower.case.bool_arg_with_number_val_0:0
#       --bool-arg MiXed.case.bool_arg_with_number_val_0:0
#       --float-arg UPPER.CASE.FLOAT_ARG:1.57
#       --float-arg lower.case.float_arg:1.57
#       --float-arg MiXed.case.float_arg:1.57
#       --json-arg UPPER.CASE.JSON_ARG:{\"jsontestkey\":\"jsontestval\"}
#       --json-arg lower.case.json_arg:{\"jsontestkey\":\"jsontestval\"}
#       --json-arg MiXed.case.json_arg:{\"jsontestkey\":\"jsontestval\"}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import json
from typing import Any

from mobly import asserts

from matter.testing.conversions import bytes_from_hex
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main


class TestFrameworkArgParsing(MatterBaseTest):
    def check_arg(self, expected_name: str, expected_val: Any) -> None:
        # Why do we have both? No idea, but folks expect stuff to be in both, so let's check both
        asserts.assert_in(expected_name, self.matter_test_config.global_test_params.keys(),
                          f"Expected user parameter {expected_name} not found in global_test_params")
        asserts.assert_in(expected_name, self.user_params.keys())
        asserts.assert_in(expected_name, self.matter_test_config.global_test_params.keys(),
                          f"Expected user parameter {expected_name} not found in user_params")
        asserts.assert_equal(
            self.matter_test_config.global_test_params[expected_name], expected_val, f"Unexpected value for {expected_name}")
        asserts.assert_equal(self.user_params[expected_name],
                             expected_val, f"Unexpected value for {expected_name}")

    def test_FrameworkArgParsing(self):
        hex_val = bytes_from_hex('01020A')
        string_val = 'TestyTesty'
        int_val = 15
        float_val = 1.57
        json_val = json.loads('{\"jsontestkey\":\"jsontestval\"}')

        self.check_arg('UPPER.CASE.HEX_ARG_WITH_UPPER_CASE_VAL', hex_val)
        self.check_arg('UPPER.CASE.HEX_ARG_WITH_LOWER_CASE_VAL', hex_val)
        self.check_arg('lower.case.hex_arg', hex_val)
        self.check_arg('MiXed.case.hex_arg', hex_val)

        self.check_arg('UPPER.CASE.STRING_ARG', string_val)
        self.check_arg('lower.case.string_arg', string_val)
        self.check_arg('MiXed.case.string_arg', string_val)

        self.check_arg('UPPER.CASE.INT_ARG', int_val)
        self.check_arg('lower.case.int_arg', int_val)
        self.check_arg('MiXed.case.int_arg', int_val)

        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_LOWER_CASE_VAL', True)
        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_UPPER_CASE_VAL', True)
        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_MIXED_CASE_VAL', True)
        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_MIXED_CASE_VAL', True)
        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_NUMBER_VAL_1', True)
        self.check_arg('UPPER.CASE.BOOL_ARG_WITH_NUMBER_VAL_0', False)

        self.check_arg('lower.case.bool_arg_with_lower_case_val', True)
        self.check_arg('lower.case.bool_arg_with_upper_case_val', True)
        self.check_arg('lower.case.bool_arg_with_mixed_case_val', True)
        self.check_arg('lower.case.bool_arg_with_number_val_1', True)
        self.check_arg('lower.case.bool_arg_with_number_val_0', False)

        self.check_arg('MiXed.case.bool_arg_with_number_val_0', False)

        self.check_arg('UPPER.CASE.FLOAT_ARG', float_val)
        self.check_arg('lower.case.float_arg', float_val)
        self.check_arg('MiXed.case.float_arg', float_val)

        self.check_arg('UPPER.CASE.JSON_ARG', json_val)
        self.check_arg('lower.case.json_arg', json_val)
        self.check_arg('MiXed.case.json_arg', json_val)


if __name__ == "__main__":
    default_matter_test_main()
