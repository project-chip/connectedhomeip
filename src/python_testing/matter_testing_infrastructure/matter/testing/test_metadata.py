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

from metadata import (NEEDS_FOLLOWUP_FIXES_TAG, NIGHTLY_TAG, OPENSSL_3_5_TAG, Metadata, MetadataReader, RunArgumentsError,
                      TagFilter, extract_runs_args, failure_fails_job, is_valid_tag_name, runs_in_pull_requests,
                      script_runs_in_pull_requests, unused_tag_values)


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
    #   run4:
    #     app: ${CHIP_LOCK_APP}
    #     tags: [nightly, needs-followup-fixes]
    #     issues:
    #       - https://github.com/project-chip/connectedhomeip/issues/3
    #   run5:
    #     app: ${CHIP_LOCK_APP}
    #     tags: [openssl-3.5]
    #   run6:
    #     app: ${CHIP_LOCK_APP}
    #     tags: [slow]
    # === END CI TEST ARGUMENTS ===
    '''

    def test_tags_and_issues(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_file = self.generate_temp_file(temp_dir, self.tagged_file_content)
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            runs = {run.run: run for run in MetadataReader(env_file).parse_script(test_file)}

        self.assertEqual(runs["run1"].tags, [])
        self.assertEqual(runs["run1"].issues, [])
        self.assertEqual(runs["run2"].tags, [NIGHTLY_TAG])
        self.assertEqual(runs["run2"].issues, [])
        self.assertEqual(runs["run3"].tags, [NEEDS_FOLLOWUP_FIXES_TAG])
        self.assertEqual(runs["run3"].issues, ["https://github.com/project-chip/connectedhomeip/issues/1",
                                               "https://github.com/project-chip/connectedhomeip/issues/2"])
        self.assertEqual(runs["run4"].tags, [NIGHTLY_TAG, NEEDS_FOLLOWUP_FIXES_TAG])
        self.assertEqual(runs["run4"].issues, ["https://github.com/project-chip/connectedhomeip/issues/3"])
        self.assertEqual(runs["run5"].tags, [OPENSSL_3_5_TAG])
        self.assertEqual(runs["run6"].tags, ["slow"])

        self.assertEqual([name for name, run in runs.items() if runs_in_pull_requests(run.tags)], ["run1", "run3", "run6"])
        self.assertEqual([name for name, run in runs.items() if failure_fails_job(run.tags)],
                         ["run1", "run2", "run3", "run5", "run6"])

    def test_tag_filter(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            runs = extract_runs_args(self.generate_temp_file(temp_dir, self.tagged_file_content))

        cases = {
            (): ["run1", "run3", "run6"],
            ("skip-nightly", "skip-openssl-3.5"): ["run1", "run3", "run6"],
            ("skip-nightly",): ["run1", "run3", "run6"],
            ("nightly",): ["run2", "run4"],
            ("openssl-3.5",): ["run5"],
            ("needs-followup-fixes",): ["run3"],
            ("skip-needs-followup-fixes",): ["run1", "run6"],
            ("slow",): ["run6"],
            ("skip-slow",): ["run1", "run3"],
            ("nightly", "needs-followup-fixes"): ["run4"],
            ("nightly", "skip-needs-followup-fixes"): ["run2"],
            ("nightly", "skip-nightly"): [],
        }
        for values, expected in cases.items():
            with self.subTest(values=values):
                tag_filter = TagFilter.parse(values)
                self.assertEqual([name for name, args in runs.items() if tag_filter.matches(args["tags"])], expected)

        self.assertEqual(TagFilter.parse([]), TagFilter())
        self.assertEqual(str(TagFilter()), "skip-nightly skip-openssl-3.5")
        self.assertEqual(str(TagFilter.parse(["nightly", "skip-needs-followup-fixes"])),
                         "nightly skip-needs-followup-fixes skip-openssl-3.5")

    def test_tag_filter_invalid_tag(self):
        for value in ["Nightly", "skip-Nightly", "", "skip-", "skip-skip-slow", "-slow", "slow tag"]:
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, f"invalid tag in {re.escape(repr(value))}"):
                TagFilter.parse([value])

    def test_tag_names(self):
        for name in [NIGHTLY_TAG, NEEDS_FOLLOWUP_FIXES_TAG, OPENSSL_3_5_TAG, "slow", "a", "tls1.3"]:
            with self.subTest(name=name):
                self.assertTrue(is_valid_tag_name(name))
        for name in ["", "Nightly", "skip-slow", "-slow", ".slow", "slow tag", "slow_tag"]:
            with self.subTest(name=name):
                self.assertFalse(is_valid_tag_name(name))

    def test_unused_tag_values(self):
        values = ["nightly", "skip-slow", "nightyl", "skip-nigthly"]
        self.assertEqual(unused_tag_values(values, {"nightly", "slow"}), ["nightyl", "skip-nigthly"])
        self.assertEqual(unused_tag_values([], set()), [])

    def test_invalid_tags_and_issues(self):
        issue = "https://github.com/project-chip/connectedhomeip/issues/1"
        cases = {
            "tags: nightly": "tags must be a list",
            "tags: [nightly, Slow]": "invalid tags ['Slow']",
            "tags: [skip-slow]": "invalid tags ['skip-slow']",
            "tags: [needs-followup-fixes]": "needs-followup-fixes needs issues",
            "tags: [nightly, needs-followup-fixes]": "needs-followup-fixes needs issues",
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

    def test_invalid_run_blocks(self):
        cases = {
            "an empty run": (["#   run1:", "#   run2:", "#     app: ${ALL_CLUSTERS_APP}"], "run 'run1': expected arguments"),
            "a run that is not a mapping": (["#   run1: ${ALL_CLUSTERS_APP}"], "run 'run1': expected arguments"),
            "no runs": ([], "test-runner-runs must list the runs"),
            "runs that are not a mapping": (["#   - run1"], "test-runner-runs must list the runs"),
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            for name, (run_lines, expected_error) in cases.items():
                with self.subTest(name):
                    test_file = self.generate_temp_file(temp_dir, '\n'.join([
                        "# === BEGIN CI TEST ARGUMENTS ===",
                        "# test-runner-runs:",
                        *run_lines,
                        "# === END CI TEST ARGUMENTS ===",
                    ]))
                    with self.assertRaisesRegex(RunArgumentsError, f"^{re.escape(test_file)}: .*{re.escape(expected_error)}"):
                        extract_runs_args(test_file)

    def test_script_without_ci_arguments(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            test_file = self.generate_temp_file(temp_dir, "# A support module with no CI test arguments\n")
            env_file = self.generate_temp_file(temp_dir, self.env_file_content)

            self.assertEqual(extract_runs_args(test_file), {})
            self.assertEqual(MetadataReader(env_file).parse_script(test_file), [])

    def test_matches_script(self):
        def one_run_file_content(*block_lines: str) -> str:
            return '\n'.join([
                "# === BEGIN CI TEST ARGUMENTS ===",
                "# test-runner-runs:",
                "#   run1:",
                "#     app: ${CHIP_LOCK_APP}",
                *(f"#     {line}" for line in block_lines),
                "# === END CI TEST ARGUMENTS ===",
            ])
        issue = "https://github.com/project-chip/connectedhomeip/issues/1"
        # name: (file content, called by PR CI, called with --tag nightly)
        cases = {
            "untagged and tagged runs": (self.tagged_file_content, True, True),
            "only a nightly run": (one_run_file_content("tags: [nightly]"), False, True),
            "only a needs-followup-fixes run":
                (one_run_file_content("tags: [needs-followup-fixes]", f"issues: [{issue}]"), True, False),
            "only a nightly needs-followup-fixes run":
                (one_run_file_content("tags: [nightly, needs-followup-fixes]", f"issues: [{issue}]"), False, True),
            "only an openssl-3.5 run": (one_run_file_content("tags: [openssl-3.5]"), False, False),
            "only a run with a tag that only selects": (one_run_file_content("tags: [slow]"), True, False),
            "no CI test arguments": ("# A support module with no CI test arguments\n", True, False),
        }
        nightly_filter = TagFilter.parse(["nightly"])
        with tempfile.TemporaryDirectory() as temp_dir:
            for name, (file_content, in_pull_requests, in_nightly) in cases.items():
                with self.subTest(name):
                    runs = extract_runs_args(self.generate_temp_file(temp_dir, file_content))
                    self.assertEqual(script_runs_in_pull_requests(runs), in_pull_requests)
                    self.assertEqual(nightly_filter.matches_script(runs), in_nightly)


if __name__ == "__main__":
    unittest.main()
