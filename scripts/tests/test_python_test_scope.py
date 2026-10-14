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
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

import yaml

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# pylint: disable=wrong-import-position
from python_test_scope import (NO_TESTS, UNFILTERED_JOB_APPS, built_apps, ci_variables, classify, environment_file,  # noqa: E402
                               excluded_tests, framework_imports, imported_names, load_metadata, main, runner_matches, scan_tree,
                               select, shard_filters)


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
        for rel in ("README.md", "test_testing/TestThing.py", "pkg/tests/test_back.py",
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
            "data/values.json": "not Python",
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
        with self.assertRaisesRegex(LookupError, "Fix: update SHARD_JOB"):
            self.select("TC_Alpha.py")
        workflow.write_text(yaml.safe_dump(
            {"jobs": {"repl_tests_linux_run": {"strategy": {"matrix": {"filter": self.FILTERS}}, "steps": []}}}))
        with self.assertRaisesRegex(LookupError, "Fix: update ENV_FILE_STEP"):
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


class RepositoryGuard(unittest.TestCase):
    """Base for the checks that read this repository; the Lint Code Base workflow runs them.

    A failure says what to fix and, under GitHub Actions, also annotates the line to change.
    """

    ROOT = Path(__file__).resolve().parents[2]
    WORKFLOWS = ".github/workflows"
    TREE = "src/python_testing"

    def fail_with_fix(self, path: str, line_pattern: str, message: str) -> None:
        if os.environ.get("GITHUB_ACTIONS") == "true":
            lines = (self.ROOT / path).read_text().splitlines()
            line = next((n for n, text in enumerate(lines, 1) if re.search(line_pattern, text)), 1)
            print(f"\n::error file={path},line={line}::{message}", flush=True)
        self.fail(message)

    def runnable_tests(self) -> list[str]:
        excluded = excluded_tests(load_metadata(self.ROOT / self.TREE / "test_metadata.yaml"))
        return sorted(p.name for p in (self.ROOT / self.TREE).glob("*.py") if p.name not in excluded)


class TestSelectionGuards(RepositoryGuard):

    def test_every_runnable_test_is_selected_into_exactly_one_shard(self):
        selection = select([f"{self.TREE}/{name}" for name in self.runnable_tests()])
        if selection.full_reason:
            self.fail_with_fix("scripts/tests/python_test_scope.py", r"^def select",
                               f"Selecting every runnable test fell back to a full run: {selection.full_reason}. "
                               "Fix: the rules in select() no longer match the test tree.")
        self.assertEqual(sorted(selection.tests), self.runnable_tests())
        filters = shard_filters(self.ROOT)
        for name in self.runnable_tests():
            shards = [f for f in filters if runner_matches(f, f"{self.TREE}/{name}")]
            if len(shards) != 1:
                self.fail_with_fix(f"{self.WORKFLOWS}/tests.yaml", r"^\s+filter: \[",
                                   f"{name} matches {len(shards)} REPL shard filters {shards}. "
                                   "Fix: make the filter list match each test exactly once "
                                   "(scripts/tools/estimate_repl_test_split.py reports overlaps).")

    def test_modules_the_framework_imports_run_every_shard(self):
        tree = self.ROOT / self.TREE
        for package in sorted(framework_imports(tree, scan_tree(tree))):
            module = next(iter(sorted(tree.glob(f"{package}/**/*.py"))), tree / f"{package}.py")
            with self.subTest(package=package):
                selection = select([module.relative_to(self.ROOT).as_posix()])
                self.assertIn("imported by the test framework", selection.full_reason)


class TestBuildGuards(RepositoryGuard):
    """The REPL build job's app conditions against the environment file they serve."""

    def setUp(self):
        self.workflow = yaml.safe_load((self.ROOT / self.WORKFLOWS / "tests.yaml").read_text())
        self.variables = environment_file(self.ROOT)
        self.apps = built_apps(self.variables)

    def build_steps(self):
        for step in self.workflow["jobs"]["repl_tests_linux_build"]["steps"]:
            targets = re.findall(r"--target\s+(\S+)", step.get("run") or "")
            if targets:
                yield step, targets

    def produced(self, targets):
        return sorted(n for n in self.apps if any(self.variables[n].startswith(f"objdir-clone/{t}/") for t in targets))

    def test_each_app_build_runs_when_the_selection_needs_one_of_its_apps(self):
        for step, targets in self.build_steps():
            expected = "env.PYTHON_TEST_APPS == ''" + "".join(
                f" || contains(env.PYTHON_TEST_APPS, ',{n},')" for n in self.produced(targets))
            if step.get("if") != expected:
                self.fail_with_fix(f"{self.WORKFLOWS}/tests.yaml", rf"- name: {re.escape(step['name'])}$",
                                   f"REPL build step {step['name']!r} does not build when the selection needs its apps. "
                                   f"Fix: give it  if: {expected}")

    def test_every_app_in_the_environment_file_has_a_build_step(self):
        produced = {n for _, targets in self.build_steps() for n in self.produced(targets)}
        # Apps an always-running step makes, such as the OTA images, are listed as always built.
        for name in sorted(self.apps - produced - set(UNFILTERED_JOB_APPS)):
            self.fail_with_fix(f"{self.WORKFLOWS}/tests.yaml", rf'echo "{name}:',
                               f"{name} is in the REPL environment file, but no REPL build step with --target builds it. "
                               f"Fix: build it in a step with  if: env.PYTHON_TEST_APPS == '' || "
                               f"contains(env.PYTHON_TEST_APPS, ',{name},'), or, if an always-running step makes it, "
                               "add it to UNFILTERED_JOB_APPS in scripts/tests/python_test_scope.py.")

    def test_the_unfiltered_job_needs_only_the_apps_always_built(self):
        used = set()
        for step in self.workflow["jobs"]["repl_tests_linux_run"]["steps"]:
            if "matrix.filter == ''" not in str(step.get("if")).replace("matrix.filter==''", "matrix.filter == ''"):
                continue
            run = step.get("run") or ""
            used.update(n for n in self.apps if self.variables[n].split("/")[1] in run)
            for script in re.findall(r"--script (\S+\.py)", run):
                used.update(ci_variables((self.ROOT / script).read_text()) & self.apps)
        self.assertTrue(used)
        if extra := sorted(used - set(UNFILTERED_JOB_APPS)):
            self.fail_with_fix("scripts/tests/python_test_scope.py", r"^UNFILTERED_JOB_APPS =",
                               f"The unfiltered REPL job runs tests on {extra}, which a narrowed build skips. "
                               "Fix: add them to UNFILTERED_JOB_APPS in scripts/tests/python_test_scope.py.")


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

    def test_workflows_are_gated(self):
        self.assertGreater(len(list(self.gated_workflows())), 0)

    def test_scope_job_runs_on_pull_requests_only(self):
        for path, jobs in self.gated_workflows():
            if jobs["pr_scope"] != self.SCOPE_JOB:
                self.fail_with_fix(path, r"^\s+pr_scope:\s*$",
                                   "The pr_scope job must call the gate on pull requests only. "
                                   "Fix: give it exactly  if: github.event_name == 'pull_request'  and  "
                                   "uses: ./.github/workflows/pull-request-scope.yaml")

    def test_gated_jobs_still_run_when_the_scope_job_is_skipped(self):
        for path, jobs in self.gated_workflows():
            for job, body in jobs.items():
                needs = body.get("needs", [])
                if "pr_scope" not in ([needs] if isinstance(needs, str) else needs):
                    continue
                condition = str(body.get("if", ""))
                if not condition.startswith("${{ !cancelled() && ") or self.GATE not in condition:
                    self.fail_with_fix(path, rf"^\s+{re.escape(job)}:\s*$",
                                       f"Job {job} needs pr_scope, so without !cancelled() it stops running on master "
                                       "and in the merge queue, where pr_scope is skipped. "
                                       f"Fix: if: ${{{{ !cancelled() && <its own condition> && {self.GATE} }}}}")


if __name__ == "__main__":
    unittest.main()
