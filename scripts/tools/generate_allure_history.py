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

"""
Generates an Allure report and adds it to the report history published on gh-pages.

The resulting <output>/<subfolder> tree is ready to deploy: one folder per run
number, last-history/ feeding the trend graphs of the next run, and an
index.html redirecting to the newest report.
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import tarfile
import tempfile
import urllib.request
from pathlib import Path

ALLURE_VERSION = "2.44.0"
ALLURE_URL = ("https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/"
              f"{ALLURE_VERSION}/allure-commandline-{ALLURE_VERSION}.tgz")
ALLURE_SHA256 = "7021a90828c00cd6ec992027cce48e8a94bd87fad43d0c2dcac4795cadc178d7"


def fetch_allure(dest: Path, url: str = ALLURE_URL, sha256: str = ALLURE_SHA256) -> Path:
    """Downloads and unpacks the Allure command line into dest, returning its launcher."""
    archive = dest / "allure.tgz"
    with urllib.request.urlopen(url, timeout=120) as response, open(archive, "wb") as f:
        shutil.copyfileobj(response, f)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != sha256:
        raise ValueError(f"{url}: SHA-256 is {digest}, expected {sha256}")
    with tarfile.open(archive) as tar:
        tar.extractall(dest, filter="data")
    return next(dest.glob("allure-*/bin/allure"))


def prune_reports(history: Path, keep_reports: int) -> None:
    """Deletes the oldest run reports, leaving room for one more within keep_reports."""
    reports = sorted((p for p in history.iterdir() if p.is_dir() and p.name.isdigit()), key=lambda p: int(p.name))
    for report in reports[:max(len(reports) - keep_reports + 1, 0)]:
        shutil.rmtree(report)


def generate(args: argparse.Namespace, allure: Path) -> None:
    owner, name = args.repository.split("/")
    pages_url = f"https://{owner}.github.io/{name}/{args.subfolder}"
    published = args.gh_pages / args.subfolder
    history = args.output / args.subfolder
    run_report = history / str(args.run_number)

    if published.is_dir():
        shutil.copytree(published, history, dirs_exist_ok=True)
    else:
        history.mkdir(parents=True)
    prune_reports(history, args.keep_reports)

    args.results.mkdir(parents=True, exist_ok=True)
    if (published / "last-history").is_dir():
        shutil.copytree(published / "last-history", args.results / "history", dirs_exist_ok=True)
    executor = {
        "name": "GitHub Actions",
        "type": "github",
        "reportName": args.report_name,
        "url": pages_url,
        "reportUrl": f"{pages_url}/{args.run_number}",
        "buildUrl": f"{args.server_url}/{args.repository}/actions/runs/{args.run_id}",
        "buildName": f"GitHub Actions Run #{args.run_id}",
        "buildOrder": args.run_number,
    }
    (args.results / "executor.json").write_text(json.dumps(executor))

    subprocess.run([str(allure), "generate", "--clean", str(args.results), "-o", str(args.report)],
                   check=True, env={**os.environ, "ALLURE_NO_ANALYTICS": "1"})

    # A re-run keeps its run number, so replace rather than merge with the earlier attempt.
    shutil.rmtree(run_report, ignore_errors=True)
    shutil.copytree(args.report, run_report)
    shutil.rmtree(history / "last-history", ignore_errors=True)
    shutil.copytree(args.report / "history", history / "last-history")
    (history / "index.html").write_text(
        f'<!DOCTYPE html><meta charset="utf-8"><meta http-equiv="refresh" content="0; URL={pages_url}/{args.run_number}/index.html">\n'
        '<meta http-equiv="Pragma" content="no-cache"><meta http-equiv="Expires" content="0">\n')


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    parser.add_argument("--results", type=Path, required=True, help="Directory of JUnit/Allure result files")
    parser.add_argument("--gh-pages", type=Path, required=True, help="Checkout of the gh-pages branch")
    parser.add_argument("--output", type=Path, required=True, help="Directory to write the updated history into")
    parser.add_argument("--report", type=Path, required=True, help="Directory to write this run's report into")
    parser.add_argument("--subfolder", required=True, help="Path of the report history within gh-pages")
    parser.add_argument("--keep-reports", type=int, default=100, help="Maximum number of run reports to keep")
    parser.add_argument("--report-name", required=True, help="Report title shown on the dashboard")
    parser.add_argument("--allure", type=Path, help="Use this Allure launcher instead of downloading Allure")
    parser.add_argument("--run-number", type=int, default=os.environ.get("GITHUB_RUN_NUMBER"))
    parser.add_argument("--run-id", default=os.environ.get("GITHUB_RUN_ID"))
    parser.add_argument("--repository", default=os.environ.get("GITHUB_REPOSITORY"))
    parser.add_argument("--server-url", default=os.environ.get("GITHUB_SERVER_URL", "https://github.com"))
    args = parser.parse_args(argv)
    if args.keep_reports < 1:
        parser.error("--keep-reports must be at least 1")
    for option in ["run_number", "run_id", "repository"]:
        if getattr(args, option) is None:
            parser.error(f"--{option.replace('_', '-')} is required outside GitHub Actions")

    if args.allure:
        generate(args, args.allure)
        return
    with tempfile.TemporaryDirectory() as tmp:
        generate(args, fetch_allure(Path(tmp)))


if __name__ == "__main__":
    main()
