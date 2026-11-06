#
#    Copyright (c) 2026 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${ALL_DEVICES_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import concurrent.futures
import logging
import os
import sys
from dataclasses import dataclass

from matter.testing.matter_stack_state import MatterStackState
from matter.testing.runner import InternalTestRunnerHooks, convert_args_to_matter_config, matter_test_args_parser, run_tests_no_exit

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from TC_DD_1_11 import TC_DD_1_11  # noqa: E402

log = logging.getLogger(__name__)


@dataclass
class TestSpec:
    name: str
    step1_response: str
    commission_on_step1: bool
    expect_pass: bool


TEST_CASES = [
    TestSpec(
        name="Concatenated QR code commissioning (option 1)",
        step1_response="1",
        commission_on_step1=True,
        expect_pass=True,
    ),
    TestSpec(
        name="Individual QR codes commissioning (option 2)",
        step1_response="2",
        commission_on_step1=False,
        expect_pass=True,
    ),
    TestSpec(
        name="Commissioner reports failure (option 3)",
        step1_response="3",
        commission_on_step1=False,
        expect_pass=False,
    ),
    TestSpec(
        name="Uncommissioned apps fail operational discovery check",
        step1_response="1",
        commission_on_step1=False,
        expect_pass=False,
    ),
]


class _TC_DD_1_11_Wrapper(TC_DD_1_11):
    spec: TestSpec
    next_node_id: int = 1

    def _commission_with_code(self, qr_code: str) -> None:
        node_id = _TC_DD_1_11_Wrapper.next_node_id
        _TC_DD_1_11_Wrapper.next_node_id += 1
        with concurrent.futures.ThreadPoolExecutor(max_workers=1) as executor:
            future = executor.submit(
                asyncio.run,
                self.default_controller.CommissionWithCode(qr_code, node_id),
            )
            future.result()

    def wait_for_user_input(
        self,
        prompt_msg: str,
        prompt_msg_placeholder: str = "Submit anything to continue",
        default_value: str = "y",
    ) -> str | None:
        qr_code = prompt_msg.splitlines()[1].strip()
        if self.current_step_index == 1:
            if self.spec.commission_on_step1:
                # Step 1 displays a single concatenated QR code containing payloads for both
                # example apps, so commission both apps off of that concatenated QR code.
                self._commission_with_code(qr_code)
                self._commission_with_code(qr_code)
            return self.spec.step1_response

        # Steps 2 and 3 are only reached when step 1 returned "2" (individual QR codes
        # requested); step 4 has no user prompt. Each prompt displays a single app's
        # individual QR code, so commission that app once per step.
        if self.current_step_index in (2, 3):
            self._commission_with_code(qr_code)
            return ""

        return ""


def main() -> int:
    parser = matter_test_args_parser()
    matter_test_config = convert_args_to_matter_config(parser.parse_args())
    matter_test_config.tests = ["test_TC_DD_1_11"]

    stack = MatterStackState(matter_test_config)
    default_controller = stack.certificate_authorities[0].adminList[0].NewController(
        nodeId=matter_test_config.controller_node_id,
        paaTrustStorePath=str(matter_test_config.paa_trust_store_path),
        catTags=matter_test_config.controller_cat_tags,
    )

    failures = []
    with asyncio.Runner() as runner:
        for spec in TEST_CASES:
            log.info("=== Running TC_DD_1_11 test case: %s ===", spec.name)
            _TC_DD_1_11_Wrapper.spec = spec
            hooks = InternalTestRunnerHooks()
            ok = run_tests_no_exit(
                _TC_DD_1_11_Wrapper,
                matter_test_config,
                runner.get_loop(),
                hooks,
                default_controller,
                stack,
            )
            if ok != spec.expect_pass:
                failures.append(f"{spec.name}: expected pass={spec.expect_pass}, got pass={ok}")

    stack.Shutdown()

    print(
        f"Test of TC-DD-1.11: run {len(TEST_CASES)}, "
        f"test response correct: {len(TEST_CASES) - len(failures)}, "
        f"test response incorrect: {len(failures)}"
    )
    for failure in failures:
        print(failure)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
