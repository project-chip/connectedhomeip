# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import re
import tempfile
import unittest

from metadata import (Metadata, MetadataReader, RunArgumentsError, RunTag, extract_runs_args, failure_fails_job,
                      runs_in_pull_requests, script_runs_in_pull_requests)


class TestMetadataReader(unittest.TestCase):

    test_file_content = '''
    # === BEGIN CI TEST ARGUMENTS ===
    # test-runner-runs:
    #  run1:
    #   app: ${ALL_CLUSTERS_APP}
    #   app-args: --discriminator 1234 --trace-to json:${TRACE_APP}.json
    #   script-args: >
    #    --commissioning-method on-network
    #    --trace-to json:${TRACE_TEST_JSON}.json
    #    --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
    #   factory-reset: true
    #   quiet: true
    # === END CI TEST ARGUMENTS ===
    '''

    env_file_content = '''
    ALL_CLUSTERS_APP: out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app
    CHIP_LOCK_APP: out/linux-x64-lock-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-lock-app
    ENERGY_GATEWAY_APP: out/linux-x64-energy-gateway-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-energy-gateway-app
    EVSE_APP: out/linux-x64-evse-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-evse-app
    WATER_HEATER_APP: out/linux-x64-water-heater-ipv6only-no-ble-no-wifi-tsan-clang-test/matter-water-heater-app
    TRACE_APP: out/trace_data/app-{SCRIPT_BASE_NAME}
    TRACE_TEST_JSON: out/trace_data/test-{SCRIPT_BASE_NAME}
    TRACE_TEST_PERFETTO: out/trace_data/test-{SCRIPT_BASE_NAME}
    '''

    expected_metadata = Metadata(
        script_args="--commissioning-method on-network --trace-to json:out/trace_data/test-{SCRIPT_BASE_NAME}.json --trace-to perfetto:out/trace_data/test-{SCRIPT_BASE_NAME}.perfetto",
        py_script_path="",
        app_args="--discriminator 1234 --trace-to json:out/trace_data/app-{SCRIPT_BASE_NAME}.json",
        run="run1",
        app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
        factory_reset=True,
        quiet=True
    )

    def generate_temp_file(self, directory: str, file_content: str) -> str:
        fd, temp_file_path = tempfile.mkstemp(dir=directory)
        with os.fdopen(fd, 'w') as fp:
            fp.write(file_content)
        return temp_file_path

    def test_run_arg_generation(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_file = self.generate_temp_file(temp_dir, self.test_file_content)
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            reader = MetadataReader(env_file)
            self.expected_metadata.py_script_path = test_file
            self.assertEqual(self.expected_metadata, reader.parse_script(test_file)[0])

    tagged_file_content = '''
    # === BEGIN CI TEST ARGUMENTS ===
    # test-runner-runs:
    #   run1:
    #     app: ${ALL_CLUSTERS_APP}
    #   run2:
    #     app: ${CHIP_LOCK_APP}
    #     tags: [nightly]
    #   run3:
    #     app: ${CHIP_LOCK_APP}
    #     tags: [needs-followup-fixes]
    #     issues:
    #       - https://github.com/project-chip/connectedhomeip/issues/1
    #       - https://github.com/project-chip/connectedhomeip/issues/2
    # === END CI TEST ARGUMENTS ===
    '''

    def test_tags_and_issues(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_file = self.generate_temp_file(temp_dir, self.tagged_file_content)
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            runs = {run.run: run for run in MetadataReader(env_file).parse_script(test_file)}

        self.assertEqual(runs["run1"].tags, [])
        self.assertEqual(runs["run1"].issues, [])
        self.assertEqual(runs["run2"].tags, [RunTag.NIGHTLY])
        self.assertEqual(runs["run2"].issues, [])
        self.assertEqual(runs["run3"].tags, [RunTag.NEEDS_FOLLOWUP_FIXES])
        self.assertEqual(runs["run3"].issues, ["https://github.com/project-chip/connectedhomeip/issues/1",
                                               "https://github.com/project-chip/connectedhomeip/issues/2"])

        self.assertEqual([name for name, run in runs.items() if runs_in_pull_requests(run.tags)], ["run1"])
        self.assertEqual([name for name, run in runs.items() if failure_fails_job(run.tags)], ["run1", "run2"])

    def test_invalid_tags_and_issues(self):
        issue = "https://github.com/project-chip/connectedhomeip/issues/1"
        cases = {
            "tags: nightly": "tags must be a list",
            "tags: [nightly, slow]": "unknown tags ['slow']",
            "tags: [nightly, needs-followup-fixes]": "can't be combined",
            "tags: [needs-followup-fixes]": "needs-followup-fixes needs issues",
            f"tags: [needs-followup-fixes]\n#     issues: {issue}": "issues must be a list",
            "tags: [needs-followup-fixes]\n#     issues: [project-chip/connectedhomeip#1]": "issues must be GitHub issue URLs",
            f"tags: [needs-followup-fixes]\n#     issues: [{issue}/]": "issues must be GitHub issue URLs",
            f"issues: [{issue}]": "only used with the needs-followup-fixes tag",
            f"tags: [nightly]\n#     issues: [{issue}]": "only used with the needs-followup-fixes tag",
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            for block_lines, expected_error in cases.items():
                with self.subTest(block_lines=block_lines):
                    test_file = self.generate_temp_file(temp_dir, '\n'.join([
                        "# === BEGIN CI TEST ARGUMENTS ===",
                        "# test-runner-runs:",
                        "#   run1:",
                        "#     app: ${ALL_CLUSTERS_APP}",
                        f"#     {block_lines}",
                        "# === END CI TEST ARGUMENTS ===",
                    ]))
                    with self.assertRaisesRegex(RunArgumentsError, f"run 'run1': .*{re.escape(expected_error)}"):
                        extract_runs_args(test_file)

    def test_script_without_ci_arguments(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_file = self.generate_temp_file(temp_dir, "# A support module with no CI test arguments\n")
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            self.assertEqual(extract_runs_args(test_file), {})
            self.assertEqual(MetadataReader(env_file).parse_script(test_file), [])

    def test_script_runs_in_pull_requests(self):
        only_tagged_file_content = '\n'.join([
            "# === BEGIN CI TEST ARGUMENTS ===",
            "# test-runner-runs:",
            "#   run1:",
            "#     app: ${CHIP_LOCK_APP}",
            "#     tags: [nightly]",
            "# === END CI TEST ARGUMENTS ===",
        ])
        cases = {
            "untagged and tagged runs": (self.tagged_file_content, True),
            "only tagged runs": (only_tagged_file_content, False),
            "no CI test arguments": ("# A support module with no CI test arguments\n", True),
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            for name, (file_content, expected) in cases.items():
                with self.subTest(name):
                    test_file = self.generate_temp_file(temp_dir, file_content)
                    self.assertEqual(script_runs_in_pull_requests(extract_runs_args(test_file)), expected)


if __name__ == "__main__":
    unittest.main()
