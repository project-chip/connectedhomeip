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
import json
import os
import re
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# pylint: disable=wrong-import-position
from python_test_scope import (CI_ARGUMENTS, ENV_FILE_STEP, NO_TESTS, REPL_SELECTION_FILES, RUNNER, SHARD_JOB,  # noqa: E402
                               UNFILTERED_JOB_APPS, built_apps, ci_variables, classify, environment_file, excluded_tests,
                               framework_imports, imported_names, imports_module, load_metadata, main, runner_matches, scan_tree,
                               select, shard_filters, shard_job)


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
        self.assert_python_tests_only("src/python_testing/README")
        self.assert_python_tests_only(
            "src/python_testing/TC_SC_4_1.py",
            "src/python_testing/TC_SC_4_3.py",
            "src/python_testing/mdns_discovery/utils/support.py")

    def test_framework_package_needs_full_run(self):
        reason = self.assert_full_run(
            "src/python_testing/matter_testing_infrastructure/matter/testing/runner.py")
        self.assertIn("framework package", reason)

    def test_files_a_build_may_read_need_full_run(self):
        # Requirements, build and data files: anything that is not Python, test_metadata.yaml or documentation.
        for path in ("src/python_testing/requirements.txt", "src/python_testing/requirements.nfc.txt",
                     "src/python_testing/post_certification_tests/BUILD.gn", "src/python_testing/some_folder/files.gni",
                     "src/python_testing/test_testing/example_pics_xml_basic_info.xml",
                     "src/python_testing/support_modules/test_metadata.yaml", "src/python_testing/READMEs.txt"):
            with self.subTest(path=path):
                self.assertIn("a build may read it", self.assert_full_run(path))

    def test_files_only_the_repl_jobs_read_are_python_test_changes(self):
        # They change which Python tests run, and the REPL jobs always run.
        self.assert_python_tests_only("src/python_testing/test_metadata.yaml")
        self.assert_python_tests_only("src/python_testing/execute_python_tests.py")

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


CI_BLOCK = "# === BEGIN CI TEST ARGUMENTS ===\n# test-runner-runs:\n#   run1:\n#     app: ${ALL_CLUSTERS_APP}\n# === END CI TEST ARGUMENTS ===\n"


class TestSelect(unittest.TestCase):
    """Selection over a small tree shaped like src/python_testing, with two shards."""

    TREE = {
        "TC_Alpha.py": CI_BLOCK + "from support_modules.alpha_support import go\n",
        "TC_Beta.py": CI_BLOCK + "from TC_HelperBase import HelperBase\n",
        "TC_Gamma.py": CI_BLOCK + "import pkg.front\n",
        "TC_Zeta.py": CI_BLOCK + "LAUNCHER = 'support_modules/launcher.py'\n",
        "TC_Omega.py": CI_BLOCK + "import support_modules\n",
        "TC_Icd.py": CI_BLOCK.replace("ALL_CLUSTERS_APP", "LIT_ICD_APP") + "# --string-arg server:${PUSH_AV_SERVER} ${TRACE_APP}\n",
        "TC_Unknown.py": CI_BLOCK.replace("ALL_CLUSTERS_APP", "NOT_IN_THE_ENV_FILE_APP"),
        "TC_HelperBase.py": "from support_modules import shared\n",
        "TC_Manual.py": "",
        "TC_Slow_1.py": CI_BLOCK,
        "support_modules/__init__.py": "",
        "support_modules/alpha_support.py": "",
        "support_modules/shared.py": "",
        "support_modules/launcher.py": "",
        "support_modules/orphan.py": "",
        "support_modules/old_idm_support.py": "",
        "pkg/__init__.py": "",
        "pkg/front.py": "from . import back\n",
        "pkg/back.py": "",
        "pkg/tests/test_back.py": "",
        "test_testing/TestThing.py": "",
        "post_certification_tests/checks.py": "",
        "netpkg/__init__.py": "",
        "netpkg/discovery.py": "",
        "TC_Net.py": CI_BLOCK + "from netpkg.discovery import browse\n",
        "matter_testing_infrastructure/matter/testing/commissioning.py": "def probe():\n    from netpkg.discovery import browse\n",
        "README.md": "",
    }
    METADATA = {
        "not_automated": [{"name": "TC_HelperBase.py", "reason": "helper"}, {"name": "TC_Manual.py", "reason": "manual"}],
        "nightly": [{"name": "TC_Slow_1.py", "reason": "slow"}],
    }
    FILTERS = ["", "TC_[A-M]", "!TC_[A-M]"]
    ENV_FILE = "".join(f'echo "{name}: {path}" >> /tmp/test_env.yaml\n' for name, path in [
        ("ALL_CLUSTERS_APP", "objdir-clone/linux-x64-all-clusters/chip-all-clusters-app"),
        ("LIT_ICD_APP", "objdir-clone/linux-x64-lit-icd/lit-icd-app"),
        ("SU_OTA_REQUESTOR_V2", "objdir-clone/chip-ota-requestor-app_v2.min.ota"),
        ("PUSH_AV_SERVER", "src/tools/push_av_server/src/server.py"),
        ("TRACE_APP", "out/trace_data/app-{SCRIPT_BASE_NAME}"),
    ])

    def setUp(self):
        self.root = Path(self.enterContext(tempfile.TemporaryDirectory()))
        tree = self.root / "src/python_testing"
        for rel, text in self.TREE.items():
            (tree / rel).parent.mkdir(parents=True, exist_ok=True)
            (tree / rel).write_text(text)
        (tree / "test_metadata.yaml").write_text(yaml.safe_dump(self.METADATA))
        workflow = {"jobs": {"repl_tests_linux_run": {"strategy": {"matrix": {"filter": self.FILTERS}},
                                                      "steps": [{"name": "Generate an argument environment file",
                                                                 "run": self.ENV_FILE}]}}}
        (self.root / ".github/workflows").mkdir(parents=True)
        (self.root / ".github/workflows/tests.yaml").write_text(yaml.safe_dump(workflow))

    def select(self, *rels):
        return select([f"src/python_testing/{rel}" for rel in rels], self.root)

    def assert_selected(self, selection, *names):
        self.assertIsNone(selection.full_reason)
        self.assertEqual(sorted(selection.tests), sorted(names))

    def test_a_changed_test_runs_alone_in_its_shard(self):
        selection = self.select("TC_Alpha.py")
        self.assert_selected(selection, "TC_Alpha.py")
        self.assertEqual(selection.slices, {"TC_[A-M]": r"/(TC_Alpha)\.py$", "!TC_[A-M]": NO_TESTS})

    def test_a_changed_module_selects_its_importers_across_shards(self):
        selection = self.select("TC_Alpha.py", "support_modules/launcher.py")
        self.assert_selected(selection, "TC_Alpha.py", "TC_Zeta.py")
        self.assertEqual(selection.slices, {"TC_[A-M]": r"/(TC_Alpha)\.py$", "!TC_[A-M]": r"/(TC_Zeta)\.py$"})
        self.assertEqual(selection.tests["TC_Zeta.py"], "uses src/python_testing/support_modules/launcher.py")

    def test_importers_are_followed_through_helpers(self):
        # shared.py <- TC_HelperBase.py <- TC_Beta.py.
        self.assert_selected(self.select("support_modules/shared.py"), "TC_Beta.py")
        # The package init runs for every importer of the package or its modules. TC_Zeta only
        # launches launcher.py by path, which does not run the package init.
        self.assert_selected(self.select("support_modules/__init__.py"), "TC_Alpha.py", "TC_Beta.py", "TC_Omega.py")

    def test_relative_imports_are_resolved(self):
        self.assert_selected(self.select("pkg/back.py"), "TC_Gamma.py")

    def test_a_helper_the_regular_run_leaves_out_is_reported_only_without_importers(self):
        selection = self.select("TC_HelperBase.py")
        self.assert_selected(selection, "TC_Beta.py")
        self.assertEqual(selection.not_run, {})
        selection = self.select("TC_Manual.py", "TC_Slow_1.py")
        self.assert_selected(selection)
        self.assertIn("not_automated", selection.not_run["TC_Manual.py"])
        self.assertIn("nightly", selection.not_run["TC_Slow_1.py"])
        self.assertEqual(selection.slices, {"TC_[A-M]": NO_TESTS, "!TC_[A-M]": NO_TESTS})

    def test_changes_the_shards_never_run_select_nothing(self):
        for rel in ("README.md", "README", "test_testing/TestThing.py", "pkg/tests/test_back.py",
                    "post_certification_tests/checks.py"):
            with self.subTest(rel=rel):
                selection = self.select(rel)
                self.assert_selected(selection)
                self.assertEqual(set(selection.slices.values()), {NO_TESTS})

    def test_deleted_files(self):
        self.assert_selected(self.select("TC_Removed.py"))
        # Nothing imports the deleted module any more; its importers changed with it.
        self.assert_selected(self.select("support_modules/deleted_module.py"))

    def test_the_build_covers_the_selected_tests_and_the_unfiltered_job(self):
        self.assertEqual(self.select("TC_Alpha.py").apps, ["ALL_CLUSTERS_APP", "SU_OTA_REQUESTOR_V2"])
        # Placeholders outside the CI arguments block, and paths that are not build outputs, add no app.
        self.assertEqual(self.select("TC_Icd.py").apps, ["ALL_CLUSTERS_APP", "LIT_ICD_APP", "SU_OTA_REQUESTOR_V2"])
        self.assertEqual(self.select("README.md").apps, ["ALL_CLUSTERS_APP", "SU_OTA_REQUESTOR_V2"])

    def test_an_app_the_environment_file_lacks_builds_everything(self):
        selection = self.select("TC_Unknown.py")
        self.assertIn("NOT_IN_THE_ENV_FILE_APP", selection.full_reason)
        self.assertEqual(selection.apps, [])

    def test_select_command_writes_the_app_list_only_for_a_narrowed_build(self):
        env = self.root / "github_env"
        with contextlib.redirect_stderr(io.StringIO()):
            for change in ("src/python_testing/TC_Icd.py", "src/python_testing/test_metadata.yaml"):
                main(["select", "--output", os.devnull, "--github-env", str(env), "--chip-root", str(self.root)],
                     stdin=io.StringIO(change + "\n"))
        self.assertEqual(env.read_text(), "PYTHON_TEST_APPS=,ALL_CLUSTERS_APP,LIT_ICD_APP,SU_OTA_REQUESTOR_V2,\n")

    def test_a_metadata_change_selects_the_tests_whose_exclusion_it_changes(self):
        tree = self.root / "src/python_testing"
        (tree / "TC_Manual.py").write_text(CI_BLOCK)
        now = {"not_automated": [{"name": "TC_HelperBase.py", "reason": "helper"}],
               "nightly": [{"name": "TC_Slow_1.py", "reason": "slow"}, {"name": "TC_Alpha.py", "reason": "slow"}]}
        (tree / "test_metadata.yaml").write_text(yaml.safe_dump(now))
        selection = select(["src/python_testing/test_metadata.yaml", "src/python_testing/TC_Manual.py"],
                           self.root, previous_metadata=self.METADATA)
        self.assert_selected(selection, "TC_Manual.py")
        self.assertEqual(selection.not_run, {"TC_Alpha.py": "runs only in the nightly workflow"})
        self.assertEqual(selection.slices, {"TC_[A-M]": r"/(TC_Manual)\.py$", "!TC_[A-M]": NO_TESTS})

    def test_a_metadata_change_that_moves_no_test_selects_nothing(self):
        tree = self.root / "src/python_testing"
        reworded = {key: [dict(item, reason="reworded") for item in items] for key, items in self.METADATA.items()}
        (tree / "test_metadata.yaml").write_text(yaml.safe_dump(reworded))
        self.assert_selected(select(["src/python_testing/test_metadata.yaml"], self.root, previous_metadata=self.METADATA))

    def test_select_command_reads_the_previous_metadata(self):
        previous = self.root / "previous.yaml"
        previous.write_text(yaml.safe_dump(self.METADATA))
        output = self.root / "github_output"
        with contextlib.redirect_stderr(io.StringIO()):
            main(["select", "--previous-metadata", str(previous), "--output", str(output), "--chip-root", str(self.root)],
                 stdin=io.StringIO("src/python_testing/test_metadata.yaml\n"))
            main(["select", "--previous-metadata", str(self.root / "missing.yaml"), "--output", str(output),
                  "--chip-root", str(self.root)], stdin=io.StringIO("src/python_testing/test_metadata.yaml\n"))
        narrowed, full = output.read_text().splitlines()
        self.assertEqual(set(json.loads(narrowed.split("=", 1)[1]).values()), {NO_TESTS})
        self.assertEqual(full, "python_test_slices=")

    def test_every_shard_runs_as_usual_when_the_selection_cannot_be_trusted(self):
        cases = {
            "test_metadata.yaml": "decides which tests run",
            "execute_python_tests.py": "decides which tests run",
            "data/values.json": "not a Python, test_metadata.yaml or documentation file",
            "support_modules/orphan.py": "imported by no test",
        }
        for rel, reason in cases.items():
            with self.subTest(rel=rel):
                selection = self.select(rel)
                self.assertIn(reason, selection.full_reason)
                self.assertEqual(selection.slices, {})
        self.assertIn("outside the Python test tree",
                      select(["src/python_testing/TC_Alpha.py", "src/app/server/Server.cpp"], self.root).full_reason)

    def test_a_module_the_framework_imports_runs_every_shard(self):
        # Every test goes through the framework, not only the tests that import the module.
        self.assertIn("imported by the test framework", self.select("netpkg/discovery.py").full_reason)
        self.assertIn("imported by the test framework", self.select("netpkg/__init__.py").full_reason)
        self.assert_selected(self.select("TC_Net.py"), "TC_Net.py")

    def test_a_test_no_shard_filter_matches_runs_every_shard(self):
        (self.root / ".github/workflows/tests.yaml").write_text(yaml.safe_dump(
            {"jobs": {"repl_tests_linux_run": {"strategy": {"matrix": {"filter": ["", "TC_[A-M]"]}}}}}))
        self.assertIn("matches no REPL shard filter", self.select("TC_Zeta.py").full_reason)

    def test_a_renamed_job_or_step_says_what_to_update(self):
        workflow = self.root / ".github/workflows/tests.yaml"
        workflow.write_text(yaml.safe_dump({"jobs": {"renamed_job": {}}}))
        with self.assertRaisesRegex(LookupError, "set SHARD_JOB"):
            self.select("TC_Alpha.py")
        workflow.write_text(yaml.safe_dump(
            {"jobs": {"repl_tests_linux_run": {"strategy": {"matrix": {"filter": self.FILTERS}}, "steps": []}}}))
        with self.assertRaisesRegex(LookupError, "set ENV_FILE_STEP"):
            self.select("TC_Alpha.py")

    def test_select_command_writes_the_output_line_and_summary(self):
        output = self.root / "github_output"
        summary = self.root / "summary.md"
        with contextlib.redirect_stderr(io.StringIO()):
            main(["select", "--output", str(output), "--summary", str(summary), "--chip-root", str(self.root)],
                 stdin=io.StringIO("src/python_testing/TC_Alpha.py\n"))
            main(["select", "--output", str(output), "--chip-root", str(self.root)],
                 stdin=io.StringIO("src/python_testing/test_metadata.yaml\n"))
        first, second = output.read_text().splitlines()
        key, value = first.split("=", 1)
        self.assertEqual(key, "python_test_slices")
        self.assertEqual(json.loads(value)["TC_[A-M]"], r"/(TC_Alpha)\.py$")
        self.assertEqual(second, "python_test_slices=")
        self.assertIn("`TC_Alpha.py`: changed", summary.read_text())


class TestSelectHelpers(unittest.TestCase):

    def test_runner_matches_follows_the_runner_semantics(self):
        self.assertTrue(runner_matches("TC_[F-I]", "src/python_testing/TC_FAN_3_5.py"))
        self.assertFalse(runner_matches("!TC_[A-S]", "src/python_testing/TC_FAN_3_5.py"))
        self.assertTrue(runner_matches("!TC_[A-S]", "src/python_testing/TC_WEBRTCP_2_1.py"))
        self.assertTrue(runner_matches(r"/(TC_FAN_3_5)\.py$", "src/python_testing/TC_FAN_3_5.py"))
        self.assertFalse(runner_matches(r"/(TC_FAN_3_5)\.py$", "src/python_testing/TC_FAN_3_5_1.py"))

    def test_imported_names(self):
        source = textwrap.dedent("""
            import a.b.c
            from d import e, f
            from .g import h
            from .. import i
            from x import *
        """)
        self.assertEqual(imported_names(source, "pkg/sub/mod.py"),
                         {"a.b.c", "d", "d.e", "d.f", "pkg.sub.g", "pkg.sub.g.h", "pkg", "pkg.i", "x"})
        self.assertEqual(imported_names("def broken(:\n", "TC_X.py"), set())


ONLY = "pull requests that only change Python tests"


class RepositoryGuard(unittest.TestCase):
    """Base for the checks that read this repository; the Lint Code Base workflow runs them.

    A failure says what to fix and, under GitHub Actions, also annotates the line to change.
    """

    ROOT = Path(__file__).resolve().parents[2]
    WORKFLOWS = ".github/workflows"
    TESTS_WORKFLOW = f"{WORKFLOWS}/tests.yaml"
    TREE = "src/python_testing"
    SCOPE_SCRIPT = "scripts/tests/python_test_scope.py"

    @staticmethod
    def message(problem: str, fix: str, *code: str, note: str | None = None) -> str:
        """Lay out a failure: what is wrong, the fix, any code to paste on lines of its own, then a note."""
        return "\n".join([problem, f"Fix: {fix}", *(f"    {line}" for line in code), *([note] if note else [])])

    def fail_with_fix(self, path: str, line_pattern: str, message: str, after: str | None = None) -> None:
        """Fail with `message`, annotating the first line matching `line_pattern`, after the first matching `after`."""
        if os.environ.get("GITHUB_ACTIONS") == "true":
            lines = (self.ROOT / path).read_text().splitlines()
            start = next((n for n, text in enumerate(lines) if re.search(after, text)), 0) if after else 0
            line = next((n for n, text in enumerate(lines[start:], start + 1) if re.search(line_pattern, text)), 1)
            # Workflow commands take one line; GitHub shows %0A as a line break in the annotation.
            text = message.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
            print(f"\n::error file={path},line={line}::{text}", flush=True)
        self.fail(message)

    def read(self, reader, *args):
        """Call a python_test_scope.py reader of the repository; a LookupError fails with the fix it names."""
        try:
            return reader(*args)
        except LookupError as error:
            setting = re.search(r"(?:set|update) (\w+)", str(error))
            self.fail_with_fix(self.SCOPE_SCRIPT, rf"^(def )?{setting.group(1) if setting else 'WORKFLOW'}\b", str(error))

    def environment(self) -> dict[str, str]:
        """Return the REPL environment file's variables; it must name at least one build output."""
        variables = self.read(environment_file, self.ROOT)
        if not built_apps(variables):
            self.fail_with_fix(self.SCOPE_SCRIPT, r"^def environment_file\b",
                               self.message(f"python_test_scope.py finds no app in the '{ENV_FILE_STEP}' step of "
                                            f"{self.TESTS_WORKFLOW}, so it cannot tell which apps a pull request's tests need.",
                                            "if that step's echo lines or the objdir-clone/ app paths changed format, update "
                                            f"environment_file() and built_apps() in {self.SCOPE_SCRIPT}."))
        return variables

    def runnable_tests(self) -> list[str]:
        excluded = excluded_tests(load_metadata(self.ROOT / self.TREE / "test_metadata.yaml"))
        return sorted(p.name for p in (self.ROOT / self.TREE).glob("*.py") if p.name not in excluded)

    @staticmethod
    def needs(job: dict) -> list[str]:
        needs = job.get("needs", [])
        return [needs] if isinstance(needs, str) else needs


class TestSelectionGuards(RepositoryGuard):
    """The selection rules against the test tree and what execute_python_tests.py really runs."""

    def runner_source(self) -> str:
        if not (self.ROOT / RUNNER).is_file():
            self.fail_with_fix(self.SCOPE_SCRIPT, r"^RUNNER =",
                               self.message(f"{RUNNER} does not exist.",
                                            f"in {self.SCOPE_SCRIPT}, set RUNNER to the runner's new path:",
                                            'RUNNER = "src/python_testing/<new name>.py"'))
        return (self.ROOT / RUNNER).read_text()

    def runner_runs(self, *regexes: str) -> set[str]:
        """Return the tests `execute_python_tests.py run --regex ...` runs, from its dry run."""
        self.runner_source()
        with tempfile.NamedTemporaryFile("w", suffix=".yaml") as env_file:
            env_file.write("{}\n")
            env_file.flush()
            command = [sys.executable, RUNNER, "run", "--dry-run", "--env-file", env_file.name, "--search-directory", self.TREE]
            for regex in regexes:
                command += ["--regex", regex]
            result = subprocess.run(command, cwd=self.ROOT, capture_output=True, text=True, check=False)
        if result.returncode and "No tests to execute" not in result.stdout + result.stderr:
            error = (result.stderr.strip().splitlines() or ["no output"])[-1]
            if "ModuleNotFoundError" in error and os.environ.get("GITHUB_ACTIONS") != "true":
                self.skipTest(f"{RUNNER} cannot run in this Python: {error}")
            self.fail_with_fix(self.SCOPE_SCRIPT, r"^RUNNER =",
                               self.message(f"Could not list the tests the runner runs: {RUNNER} run --dry-run failed with "
                                            f"{error}.",
                                            f"if the runner moved, set RUNNER in {self.SCOPE_SCRIPT} to its new path; if it fails "
                                            "on an import, fix that import; if its command line changed, update runner_runs() in "
                                            "scripts/tests/test_python_test_scope.py."))
        return {Path(script).name for script in re.findall(r"--script (\S+\.py)", result.stdout)}

    def test_every_runnable_test_is_selected_into_exactly_one_shard(self):
        self.environment()
        selection = self.read(select, [f"{self.TREE}/{name}" for name in self.runnable_tests()])
        if selection.full_reason:
            self.fail_with_fix(self.SCOPE_SCRIPT, r"^def select",
                               self.message("Selecting every test the REPL shards run fell back to running everything: "
                                            f"{selection.full_reason}.",
                                            "correct what that names; if it names nothing wrong, the rules in select() in "
                                            f"{self.SCOPE_SCRIPT} no longer match the test tree."))
        self.assertEqual(sorted(selection.tests), self.runnable_tests())
        filters = self.read(shard_filters, self.ROOT)
        for name in self.runnable_tests():
            shards = [f for f in filters if runner_matches(f, f"{self.TREE}/{name}")]
            if len(shards) != 1:
                self.fail_with_fix(self.TESTS_WORKFLOW, r"^\s+filter: \[",
                                   self.message(f"{name} matches {len(shards)} of the REPL shard filters {shards}, so "
                                                f"{'no shard runs it' if not shards else 'more than one shard runs it'}.",
                                                f"in {self.TESTS_WORKFLOW}, change the filter list of job {SHARD_JOB} so that it "
                                                "matches each test exactly once.",
                                                note="scripts/tools/estimate_repl_test_split.py reports overlaps."))

    def test_each_shard_runs_what_the_runner_runs(self):
        # Every test selected, so each shard's selection regex names its whole share.
        self.environment()
        runnable = self.runnable_tests()
        selection = self.read(select, [f"{self.TREE}/{name}" for name in runnable])
        for shard in self.read(shard_filters, self.ROOT):
            expected = {name for name in runnable if runner_matches(shard, f"{self.TREE}/{name}")}
            for regexes in ([shard], [shard, selection.slices.get(shard, NO_TESTS)]):
                if regexes[-1] == NO_TESTS:
                    continue
                actual = self.runner_runs(*regexes)
                if actual != expected:
                    narrowed = ", when also given the list of tests a pull request needs," if len(regexes) > 1 else ","
                    self.fail_with_fix(self.SCOPE_SCRIPT, r"^EXCLUDED_SECTIONS =",
                                       self.message(f"For the REPL shard {shard!r}{narrowed} {RUNNER} runs {len(actual)} tests, "
                                                    f"but {self.SCOPE_SCRIPT}, which must predict exactly which tests each shard "
                                                    f"runs, expects {len(expected)}. Only the runner: {sorted(actual - expected)[:5]}. "
                                                    f"Only the script: {sorted(expected - actual)[:5]}.",
                                                    f"in {self.SCOPE_SCRIPT}, make EXCLUDED_SECTIONS list the test_metadata.yaml "
                                                    "sections the runner leaves out (see cmd_run), and make runner_matches() match "
                                                    "files the way its --regex option does."))

    def test_the_runner_imports_nothing_else_from_the_test_tree(self):
        files = scan_tree(self.ROOT / self.TREE)
        imported = imported_names(self.runner_source(), Path(RUNNER).name)
        for rel, info in sorted(files.items()):
            if f"{self.TREE}/{rel}" not in REPL_SELECTION_FILES and imports_module(imported, info.module):
                self.fail_with_fix(self.SCOPE_SCRIPT, r"^REPL_SELECTION_FILES =",
                                   self.message(f"{RUNNER} imports {self.TREE}/{rel}, so changing that file can change which "
                                                "tests run, but such a change would not make every shard run.",
                                                f"in {self.SCOPE_SCRIPT}, add this path to REPL_SELECTION_FILES:",
                                                f'"{self.TREE}/{rel}"'))

    def test_modules_the_framework_imports_run_every_shard(self):
        tree = self.ROOT / self.TREE
        for package in sorted(framework_imports(tree, scan_tree(tree))):
            module = next(iter(sorted(tree.glob(f"{package}/**/*.py"))), tree / f"{package}.py")
            with self.subTest(package=package):
                selection = select([module.relative_to(self.ROOT).as_posix()])
                self.assertIn("imported by the test framework", selection.full_reason)


class TestBuildGuards(RepositoryGuard):
    """The REPL jobs' wiring, step conditions and app builds against the environment file they serve."""

    SELECT_STEP = "select_python_tests"

    def setUp(self):
        self.workflow = yaml.safe_load((self.ROOT / self.TESTS_WORKFLOW).read_text())
        self.shard_job = self.read(shard_job, self.ROOT)
        self.build_job = next((job for job, body in self.workflow["jobs"].items()
                               if any(step.get("id") == self.SELECT_STEP for step in body.get("steps") or [])), None)
        if self.build_job is None:
            self.fail_with_fix(self.TESTS_WORKFLOW, r"- name: Build python env",
                               self.message(f"No job in {self.TESTS_WORKFLOW} has the step 'Select Python tests for this pull "
                                            f"request' (id {self.SELECT_STEP}), so {ONLY} run every REPL test and build every app "
                                            "again.",
                                            f"in {self.TESTS_WORKFLOW}, keep that step in the job that builds the REPL apps, before "
                                            "its app build steps."))
        self.variables = self.environment()
        self.apps = built_apps(self.variables)

    def build_steps(self):
        for step in self.workflow["jobs"][self.build_job]["steps"]:
            targets = re.findall(r"--target\s+(\S+)", step.get("run") or "")
            if targets:
                yield step, targets

    def produced(self, targets):
        return sorted(n for n in self.apps if any(self.variables[n].startswith(f"objdir-clone/{t}/") for t in targets))

    @staticmethod
    def passes_selection(run: str) -> bool:
        """Whether each runner command in a step's script gets the selection, directly or through $SELECTION."""
        commands = [line for line in run.splitlines() if f"{Path(RUNNER).name} run" in line]
        built = re.search(r"\bSELECTION=.*\$PYTHON_TEST_SELECTION", run) is not None
        return bool(commands) and all("PYTHON_TEST_SELECTION" in line or ("$SELECTION" in line and built) for line in commands)

    def test_the_selection_reaches_every_shard(self):
        output = f"${{{{ steps.{self.SELECT_STEP}.outputs.python_test_slices }}}}"
        selection = str((self.shard_job.get("env") or {}).get("PYTHON_TEST_SELECTION"))
        run_steps = [step for step in self.shard_job.get("steps") or []
                     if f"{Path(RUNNER).name} run" in str(step.get("run")) and "matrix.filter" in str(step.get("run"))]
        checks = [
            ((self.workflow["jobs"][self.build_job].get("outputs") or {}).get("python_test_slices") == output, self.build_job,
             f"give job {self.build_job} this output:", ["outputs:", f"    python_test_slices: {output}"]),
            (self.build_job in self.needs(self.shard_job), SHARD_JOB,
             f"add {self.build_job} to the needs of job {SHARD_JOB}.", []),
            (f"needs.{self.build_job}.outputs.python_test_slices" in selection and "[matrix.filter]" in selection, SHARD_JOB,
             f"give job {SHARD_JOB} this environment variable:",
             ["env:", f"    PYTHON_TEST_SELECTION: ${{{{ fromJSON(needs.{self.build_job}.outputs.python_test_slices || '{{}}')"
              "[matrix.filter] }}"]),
            (bool(run_steps) and all(self.passes_selection(step["run"]) for step in run_steps), SHARD_JOB,
             f"in the step of job {SHARD_JOB} that runs {Path(RUNNER).name}, pass it the selection when there is one:",
             ['SELECTION=""', 'if [ -n "$PYTHON_TEST_SELECTION" ]; then', "    SELECTION=\"--regex '$PYTHON_TEST_SELECTION'\"", "fi",
              f"... {Path(RUNNER).name} run ... --regex '${{{{matrix.filter}}}}' $SELECTION ..."]),
        ]
        for ok, job, fix, code in checks:
            if not ok:
                self.fail_with_fix(self.TESTS_WORKFLOW, rf"^\s+{re.escape(job)}:\s*$",
                                   self.message(f"The REPL shards no longer receive the list of tests to run, so on {ONLY} "
                                                "every shard runs all its tests.", f"in {self.TESTS_WORKFLOW}, {fix}", *code))

    def test_shard_steps_skip_shards_with_nothing_to_run(self):
        steps = self.shard_job.get("steps") or []
        checkout = next((n for n, step in enumerate(steps) if str(step.get("uses", "")).startswith("actions/checkout")), 0)
        for step in steps[checkout:]:
            condition = re.sub(r"\s+", "", str(step.get("if", "")))
            # The unfiltered job (filter "") always has tests to run, and failure() steps never run on a shard without tests.
            if ("PYTHON_TEST_SELECTION!='none'" in condition or "matrix.filter==''" in condition
                    or condition.removeprefix("${{").startswith("failure()")):
                continue
            name = step.get("name") or step.get("uses")
            guard = "env.PYTHON_TEST_SELECTION != 'none'"
            current = str(step.get("if", "")).strip()
            if not current:
                wanted = guard
            elif current.startswith("${{") and current.endswith("}}"):
                wanted = f"{current[:-2].rstrip()} && {guard} }}}}"
            else:
                wanted = f"{current} && {guard}"
            self.fail_with_fix(self.TESTS_WORKFLOW, rf"^\s*(- )?(name|uses): {re.escape(name)}\s*$",
                               self.message(f"On {ONLY}, a REPL shard with none of the changed tests skips checkout and "
                                            f"setup, but step {name!r} of job {SHARD_JOB} still runs there.",
                                            f"in {self.TESTS_WORKFLOW}, give that step this condition, as the steps around it "
                                            "have:", f"if: {wanted}"),
                               after=rf"^\s+{SHARD_JOB}:\s*$")

    def test_every_placeholder_a_test_uses_is_in_the_environment_file(self):
        for name in self.runnable_tests():
            path = f"{self.TREE}/{name}"
            source = (self.ROOT / path).read_text(errors="replace")
            if not CI_ARGUMENTS.search(source):
                self.fail_with_fix(path, r"^", self.message(
                    f"The REPL shards run {name}, but it has no CI arguments block, which they need to run it.",
                    f"in {path}, add a '# === BEGIN CI TEST ARGUMENTS ===' block, or list {name} under not_automated in "
                    f"{self.TREE}/test_metadata.yaml.",
                    note=f"If the block markers changed for every test, update CI_ARGUMENTS in {self.SCOPE_SCRIPT} instead."))
            for variable in sorted(ci_variables(source) - set(self.variables)):
                self.fail_with_fix(path, rf"\$\{{{variable}\}}", self.message(
                    f"{name} uses ${{{variable}}} in its CI arguments, but the '{ENV_FILE_STEP}' step of "
                    f"{self.TESTS_WORKFLOW} does not define it.",
                    f"correct the name in {path}, or define it in that step:",
                    f'echo "{variable}: <path>" >> /tmp/test_env.yaml',
                    note=f"If that step's echo lines changed format, update environment_file() in {self.SCOPE_SCRIPT} instead."))

    def test_each_app_build_runs_when_the_selection_needs_one_of_its_apps(self):
        for step, targets in self.build_steps():
            expected = "env.PYTHON_TEST_APPS == ''" + "".join(
                f" || contains(env.PYTHON_TEST_APPS, ',{n},')" for n in self.produced(targets))
            if step.get("if") != expected:
                built = ", ".join(self.produced(targets)) or "nothing a test names in its CI arguments"
                if "if" not in step:
                    problem = f"has no if:, so it always builds {built}, even for a pull request whose changed tests don't use it."
                else:
                    problem = (f"has an if: that doesn't match what it builds ({built}), so on {ONLY} it can skip an app "
                               "the changed tests need, or build one they don't.")
                self.fail_with_fix(self.TESTS_WORKFLOW, rf"- name: {re.escape(step['name'])}$", self.message(
                    f"The REPL build step {step['name']!r} {problem}",
                    f"in {self.TESTS_WORKFLOW}, give that step this condition:", f"if: {expected}",
                    note=f"PYTHON_TEST_APPS lists the apps needed on {ONLY}. On all other runs it is empty, so the step "
                    "always builds."))

    def test_every_app_in_the_environment_file_has_a_build_step(self):
        produced = {n for _, targets in self.build_steps() for n in self.produced(targets)}
        # Apps an always-running step makes, such as the OTA images, are listed as always built.
        for name in sorted(self.apps - produced - set(UNFILTERED_JOB_APPS)):
            self.fail_with_fix(self.TESTS_WORKFLOW, rf'echo "{name}:', self.message(
                f"{name} is defined in the '{ENV_FILE_STEP}' step of {self.TESTS_WORKFLOW}, but no step of job "
                f"{self.build_job} builds it with --target, so nothing builds it when a test needs it.",
                f"in {self.TESTS_WORKFLOW}, build it in a step of job {self.build_job} with this condition:",
                f"if: env.PYTHON_TEST_APPS == '' || contains(env.PYTHON_TEST_APPS, ',{name},')",
                note=f"If a step that always runs makes it, like the OTA images, add {name} to UNFILTERED_JOB_APPS in "
                f"{self.SCOPE_SCRIPT} instead."))

    def test_the_unfiltered_job_needs_only_the_apps_always_built(self):
        used = set()
        for step in self.shard_job["steps"]:
            if "matrix.filter == ''" not in str(step.get("if")).replace("matrix.filter==''", "matrix.filter == ''"):
                continue
            run = step.get("run") or ""
            used.update(n for n in self.apps if self.variables[n].split("/")[1] in run)
            for script in re.findall(r"--script (\S+\.py)", run):
                used.update(ci_variables((self.ROOT / script).read_text()) & self.apps)
        self.assertTrue(used)
        if extra := sorted(used - set(UNFILTERED_JOB_APPS)):
            them = "it" if len(extra) == 1 else "them"
            apps = ", ".join(f'"{app}"' for app in [*UNFILTERED_JOB_APPS, *extra])
            self.fail_with_fix(self.SCOPE_SCRIPT, r"^UNFILTERED_JOB_APPS =", self.message(
                f"The unfiltered REPL job (filter \"\") runs tests on {', '.join(extra)}, but on {ONLY} the REPL build "
                f"job builds {them} only when their tests need {them}.",
                f"in {self.SCOPE_SCRIPT}, add {them} to UNFILTERED_JOB_APPS, the apps built on every pull request:",
                f"UNFILTERED_JOB_APPS = ({apps})"))


class TestGateGuards(RepositoryGuard):
    """Guards the gate's wiring in the workflow files.

    A job that needs a skipped job is skipped too, unless its condition uses a status
    function. The scope job is skipped on push, in the merge queue and on every other
    event that is not a pull request, so a gated job without !cancelled() would silently
    stop running on master.
    """

    SCOPE_JOB = {"if": "github.event_name == 'pull_request'", "uses": "./.github/workflows/pull-request-scope.yaml"}
    GATE = "needs.pr_scope.outputs.python_tests_only != 'true'"

    def gated_workflows(self):
        for path in sorted((self.ROOT / self.WORKFLOWS).glob("*.y*ml")):
            jobs = (yaml.safe_load(path.read_text()) or {}).get("jobs") or {}
            if "pr_scope" in jobs:
                yield f"{self.WORKFLOWS}/{path.name}", jobs

    def gated_jobs(self):
        for path, jobs in self.gated_workflows():
            for job, body in jobs.items():
                if "pr_scope" in self.needs(body):
                    yield path, job, body

    def test_workflows_are_gated(self):
        self.assertGreater(len(list(self.gated_workflows())), 0)

    def test_scope_job_runs_on_pull_requests_only(self):
        for path, jobs in self.gated_workflows():
            if jobs["pr_scope"] != self.SCOPE_JOB:
                self.fail_with_fix(path, r"^\s+pr_scope:\s*$", self.message(
                    "The pr_scope job must run the gate on pull requests only.", f"in {path}, make it exactly:",
                    "pr_scope:", "    if: github.event_name == 'pull_request'",
                    "    uses: ./.github/workflows/pull-request-scope.yaml"))

    def test_gated_jobs_still_run_when_the_scope_job_is_skipped(self):
        for path, job, body in self.gated_jobs():
            condition = str(body.get("if", ""))
            if not condition.startswith("${{ !cancelled() && ") or self.GATE not in condition:
                inner = condition.strip().removeprefix("${{").removesuffix("}}").strip()
                own = " && ".join(part.strip() for part in inner.split("&&")
                                  if part.strip() and part.strip() not in ("!cancelled()", self.GATE)) or "<its own condition>"
                if "||" in own:
                    own = f"({own})"
                self.fail_with_fix(path, rf"^\s+{re.escape(job)}:\s*$", self.message(
                    f"Job {job} needs pr_scope, which is skipped on master and in the merge queue, so without !cancelled() "
                    "the job silently stops running there.",
                    f"in {path}, give job {job} this condition:", f"if: ${{{{ !cancelled() && {own} && {self.GATE} }}}}"))

    def test_gated_jobs_read_no_python_test_files(self):
        # What the job's steps name, and what the scripts they run name.
        for path, job, body in self.gated_jobs():
            text = yaml.safe_dump(body)
            sources = {path: text}
            for script in sorted(set(re.findall(r"\bscripts/[\w/.-]+\.(?:py|sh)\b", text))):
                if (self.ROOT / script).is_file():
                    sources[script] = (self.ROOT / script).read_text(errors="replace")
            for source, content in sources.items():
                for read in sorted(set(re.findall(rf"{self.TREE}/[\w.*/-]+", content))):
                    if classify([read])[0]:
                        through = f" through {source}" if source != path else ""
                        self.fail_with_fix(path, rf"^\s+{re.escape(job)}:\s*$", self.message(
                            f"Job {job} is skipped on {ONLY}, but it reads {read}{through}, which they can change, so a "
                            "break there would go unseen until the merge queue.",
                            f"in {path}, remove pr_scope from the needs of job {job} and the python_tests_only check from "
                            "its if:, or stop it reading that file."))


if __name__ == "__main__":
    unittest.main()
