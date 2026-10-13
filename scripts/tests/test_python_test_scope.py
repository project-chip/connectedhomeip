#!/usr/bin/env python3
#
# Copyright (c) 2026 Project CHIP Authors
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
#

"""Unit tests for python_test_scope.py."""

import contextlib
import io
import os
import sys
import unittest
from pathlib import Path

import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# pylint: disable=wrong-import-position
from python_test_scope import classify, main  # noqa: E402


class TestClassify(unittest.TestCase):

    def assert_python_tests_only(self, *paths: str) -> None:
        python_tests_only, reason = classify(paths)
        self.assertTrue(python_tests_only, reason)

    def assert_full_run(self, *paths: str) -> str:
        python_tests_only, reason = classify(paths)
        self.assertFalse(python_tests_only, reason)
        return reason

    def test_test_scripts_and_helpers_are_python_test_changes(self):
        self.assert_python_tests_only("src/python_testing/TC_FAN_3_5.py")
        self.assert_python_tests_only("src/python_testing/TC_WEBRTCPTestBase.py")
        self.assert_python_tests_only("src/python_testing/support_modules/idm_support.py")
        self.assert_python_tests_only("src/python_testing/mdns_discovery/utils/support.py")
        self.assert_python_tests_only("src/python_testing/test_testing/TestDecorators.py")
        self.assert_python_tests_only("src/python_testing/mdns_discovery/README.md")
        self.assert_python_tests_only(
            "src/python_testing/TC_SC_4_1.py",
            "src/python_testing/TC_SC_4_3.py",
            "src/python_testing/mdns_discovery/utils/support.py")

    def test_framework_package_needs_full_run(self):
        reason = self.assert_full_run(
            "src/python_testing/matter_testing_infrastructure/matter/testing/runner.py")
        self.assertIn("framework package", reason)

    def test_python_environment_files_need_full_run(self):
        for path in ("src/python_testing/requirements.txt", "src/python_testing/requirements.nfc.txt"):
            with self.subTest(path=path):
                self.assertIn("Python environment build", self.assert_full_run(path))

    def test_files_only_the_repl_jobs_read_are_python_test_changes(self):
        # They change which Python tests run, and the REPL jobs always run.
        self.assert_python_tests_only("src/python_testing/test_metadata.yaml")
        self.assert_python_tests_only("src/python_testing/execute_python_tests.py")

    def test_build_files_in_the_tree_need_full_run(self):
        self.assertIn("build file", self.assert_full_run("src/python_testing/post_certification_tests/BUILD.gn"))
        self.assertIn("build file", self.assert_full_run("src/python_testing/some_folder/files.gni"))

    def test_paths_outside_the_tree_need_full_run(self):
        self.assert_full_run("scripts/tests/run_python_test.py")
        self.assert_full_run(".github/workflows/tests.yaml")
        self.assert_full_run("src/app/clusters/fan-control-server/fan-control-server.cpp")
        self.assert_full_run("src/app/tests/suites/certification/ci-pics-values")

    def test_lookalike_prefixes_need_full_run(self):
        self.assert_full_run("src/python_testing_extra/TC_X.py")
        self.assert_full_run("src/python_testingTC_X.py")
        self.assert_full_run("python_testing/TC_X.py")

    def test_one_outside_path_makes_the_whole_change_a_full_run(self):
        reason = self.assert_full_run("src/python_testing/TC_FAN_3_5.py", "src/app/server/Server.cpp")
        self.assertIn("src/app/server/Server.cpp", reason)

    def test_moving_a_file_in_from_outside_needs_full_run(self):
        # A rename is listed as the deleted old path and the added new one.
        self.assert_full_run("scripts/tools/helper.py", "src/python_testing/helper.py")

    def test_empty_list_needs_full_run(self):
        self.assert_full_run()
        self.assert_full_run("", "  \n")

    def test_surrounding_whitespace_is_ignored(self):
        self.assert_python_tests_only("src/python_testing/TC_FAN_3_5.py\n", "  ")


class TestMain(unittest.TestCase):

    def run_main(self, text: str) -> tuple[str, str]:
        stdout, stderr = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(stdout), contextlib.redirect_stderr(stderr):
            self.assertEqual(main(["classify"], stdin=io.StringIO(text)), 0)
        return stdout.getvalue(), stderr.getvalue()

    def test_prints_the_output_line_and_the_reason(self):
        stdout, stderr = self.run_main("src/python_testing/TC_FAN_3_5.py\n")
        self.assertEqual(stdout, "python_tests_only=true\n")
        self.assertIn("Python tests only", stderr)

        stdout, stderr = self.run_main("src/python_testing/TC_FAN_3_5.py\nsrc/lib/core/CHIPError.h\n")
        self.assertEqual(stdout, "python_tests_only=false\n")
        self.assertIn("Full run: src/lib/core/CHIPError.h is outside the Python test tree", stderr)

    def test_a_command_is_required(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            main([], stdin=io.StringIO(""))


class TestGatedWorkflows(unittest.TestCase):
    """Guards the gate's wiring in the workflow files.

    A job that needs a skipped job is skipped too, unless its condition uses a status
    function. The scope job is skipped on push, in the merge queue and on every other
    event that is not a pull request, so a gated job without !cancelled() would silently
    stop running on master.
    """

    WORKFLOWS = Path(__file__).resolve().parents[2] / ".github" / "workflows"
    SCOPE_JOB = {"if": "github.event_name == 'pull_request'", "uses": "./.github/workflows/pull-request-scope.yaml"}
    GATE = "needs.pr_scope.outputs.python_tests_only != 'true'"

    def gated_workflows(self):
        for path in sorted(self.WORKFLOWS.glob("*.y*ml")):
            jobs = (yaml.safe_load(path.read_text()) or {}).get("jobs") or {}
            if "pr_scope" in jobs:
                yield path.name, jobs

    def test_workflows_are_gated(self):
        self.assertGreater(len(list(self.gated_workflows())), 0)

    def test_scope_job_runs_on_pull_requests_only(self):
        for name, jobs in self.gated_workflows():
            with self.subTest(workflow=name):
                self.assertEqual(jobs["pr_scope"], self.SCOPE_JOB)

    def test_gated_jobs_still_run_when_the_scope_job_is_skipped(self):
        for name, jobs in self.gated_workflows():
            for job, body in jobs.items():
                needs = body.get("needs", [])
                if "pr_scope" not in ([needs] if isinstance(needs, str) else needs):
                    continue
                with self.subTest(workflow=name, job=job):
                    condition = str(body.get("if", ""))
                    self.assertTrue(condition.startswith("${{ !cancelled() && "), condition)
                    self.assertIn(self.GATE, condition)


if __name__ == "__main__":
    unittest.main()
