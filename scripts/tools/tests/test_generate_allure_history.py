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

import hashlib
import io
import json
import os
import sys
import tarfile
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

# pylint: disable=wrong-import-position
from generate_allure_history import fetch_allure, main, prune_reports  # noqa: E402

SUBFOLDER = "allure-report/ci_tests"

# Stands in for the Allure launcher: like `allure generate`, it carries the
# results' history/ and executor.json into the report.
FAKE_ALLURE = f"""#!{sys.executable}
import shutil, sys
from pathlib import Path
results, report = Path(sys.argv[3]), Path(sys.argv[5])
report.mkdir()
(report / "index.html").write_text("report")
if (results / "history").is_dir():
    shutil.copytree(results / "history", report / "history")
else:
    (report / "history").mkdir()
shutil.copy(results / "executor.json", report / "executor.json")
"""


class TestGenerateAllureHistory(unittest.TestCase):

    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.gh_pages = self.root / "gh-pages"
        self.published = self.gh_pages / SUBFOLDER
        self.published.mkdir(parents=True)
        (self.root / "junit-results").mkdir()
        self.allure = self.root / "allure"
        self.allure.write_text(FAKE_ALLURE)
        self.allure.chmod(0o755)
        self.history = self.root / "allure-history" / SUBFOLDER

    def tearDown(self) -> None:
        self._tmp.cleanup()

    def run_main(self, run_number: int = 7, report_name: str = "CI Tests") -> None:
        main([
            "--allure", str(self.allure),
            "--results", str(self.root / "junit-results"),
            "--gh-pages", str(self.gh_pages),
            "--output", str(self.root / "allure-history"),
            "--report", str(self.root / "allure-report"),
            "--subfolder", SUBFOLDER,
            "--keep-reports", "3",
            "--report-name", report_name,
            "--run-number", str(run_number),
            "--run-id", "42",
            "--repository", "project-chip/connectedhomeip",
            "--server-url", "https://github.com",
        ])

    def test_prune_keeps_newest_reports_by_number(self) -> None:
        for name in ["9", "10", "100", "11", "last-history"]:
            (self.published / name).mkdir()
        (self.published / "CNAME").write_text("")

        prune_reports(self.published, keep_reports=3)

        self.assertEqual(sorted(p.name for p in self.published.iterdir()), ["100", "11", "CNAME", "last-history"])

    def test_first_run_redirects_to_new_report(self) -> None:
        self.run_main()

        self.assertEqual((self.history / "7" / "index.html").read_text(), "report")
        self.assertTrue((self.history / "last-history").is_dir())
        self.assertIn('content="0; URL=https://project-chip.github.io/connectedhomeip/allure-report/ci_tests/7/index.html"',
                      (self.history / "index.html").read_text())

    def test_published_history_is_carried_forward(self) -> None:
        (self.published / "last-history").mkdir()
        (self.published / "last-history" / "history-trend.json").write_text('["run 6"]')

        self.run_main()

        self.assertEqual((self.history / "last-history" / "history-trend.json").read_text(), '["run 6"]')

    def test_old_reports_are_pruned_and_new_one_added(self) -> None:
        for name in ["3", "4", "5", "6"]:
            (self.published / name).mkdir()

        self.run_main()

        self.assertEqual(sorted(p.name for p in self.history.iterdir()), ["5", "6", "7", "index.html", "last-history"])

    def test_rerun_replaces_report_of_same_run(self) -> None:
        (self.published / "7").mkdir()
        (self.published / "7" / "stale.txt").write_text("previous attempt")

        self.run_main()

        self.assertEqual(sorted(p.name for p in (self.history / "7").iterdir()), ["executor.json", "history", "index.html"])

    def test_missing_results_still_produce_report(self) -> None:
        # download-artifact creates no directory when no artifact matches.
        (self.root / "junit-results").rmdir()

        self.run_main()

        self.assertEqual((self.history / "7" / "index.html").read_text(), "report")

    def test_executor_links_back_to_run(self) -> None:
        self.run_main(report_name='Nightly "CI" Tests')

        executor = json.loads((self.history / "7" / "executor.json").read_text())
        self.assertEqual(executor["reportName"], 'Nightly "CI" Tests')
        self.assertEqual(executor["reportUrl"], "https://project-chip.github.io/connectedhomeip/allure-report/ci_tests/7")
        self.assertEqual(executor["buildUrl"], "https://github.com/project-chip/connectedhomeip/actions/runs/42")
        self.assertEqual(executor["buildOrder"], 7)


class TestFetchAllure(unittest.TestCase):

    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.archive = self.root / "allure.tgz"
        launcher = b"#!/bin/sh\n"
        with tarfile.open(self.archive, "w:gz") as tar:
            info = tarfile.TarInfo("allure-9.9.9/bin/allure")
            info.size = len(launcher)
            info.mode = 0o755
            tar.addfile(info, io.BytesIO(launcher))
        self.url = self.archive.as_uri()
        self.dest = self.root / "dest"
        self.dest.mkdir()

    def tearDown(self) -> None:
        self._tmp.cleanup()

    def test_matching_checksum_unpacks_launcher(self) -> None:
        launcher = fetch_allure(self.dest, self.url, hashlib.sha256(self.archive.read_bytes()).hexdigest())

        self.assertEqual(launcher, self.dest / "allure-9.9.9" / "bin" / "allure")
        self.assertTrue(os.access(launcher, os.X_OK))

    def test_checksum_mismatch_is_rejected_before_unpacking(self) -> None:
        with self.assertRaises(ValueError):
            fetch_allure(self.dest, self.url, "0" * 64)

        self.assertFalse((self.dest / "allure-9.9.9").exists())


if __name__ == "__main__":
    unittest.main()
