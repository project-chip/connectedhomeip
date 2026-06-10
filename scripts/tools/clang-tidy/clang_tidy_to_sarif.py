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

"""Convert clang-tidy text diagnostics to SARIF 2.1.0 for GitHub Code Scanning.

Reads the combined log produced by run-clang-tidy-on-compile-commands.py (or raw
clang-tidy output) and extracts the standard diagnostic lines:

    <path>:<line>:<col>: warning|error: <message> [<check-name>]

Paths are normalized to repo-root-relative so Code Scanning can map them to
source. Stdlib only.

Usage:
    clang_tidy_to_sarif.py [LOG] --repo-root . > clang-tidy.sarif
    (LOG defaults to stdin)
"""

import argparse
import json
import os
import re
import sys

# A clang-tidy diagnostic line. The path token excludes whitespace and quotes so
# it survives the runner's `TIDY <file>: '<output>'` log wrapping; on Linux paths
# contain no ':' before the line/col, so ':' also terminates the path.
# Greedy msg so the LAST bracket is taken as the check name; tolerate a trailing
# quote because run-clang-tidy-on-compile-commands.py wraps output as `'<output>'`,
# so the final diagnostic line ends with `]'`.
DIAG_RE = re.compile(
    r"(?P<path>[^\s'\":]+):(?P<line>\d+):(?P<col>\d+):\s+"
    r"(?P<level>warning|error):\s+(?P<msg>.*)\s+\[(?P<checks>[A-Za-z][\w.,-]*)\]'?\s*$"
)

CLANG_TIDY_CHECKS_URL = "https://clang.llvm.org/extra/clang-tidy/checks/list.html"


def check_help_uri(check):
    """Best-effort doc URL for a clang-tidy check (module-name/check-name.html)."""
    if "-" in check:
        module, name = check.split("-", 1)
        return f"https://clang.llvm.org/extra/clang-tidy/checks/{module}/{name}.html"
    return CLANG_TIDY_CHECKS_URL


def to_repo_relative(path, repo_root):
    """Resolve a diagnostic path to a repo-root-relative POSIX path, or None if
    it lands outside the repo (system / third-party headers)."""
    abs_path = path if os.path.isabs(path) else os.path.join(repo_root, path)
    rel = os.path.relpath(os.path.realpath(abs_path), repo_root)
    if rel.startswith(".."):
        return None
    return rel.replace(os.sep, "/")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("log", nargs="?", help="clang-tidy log file (default: stdin)")
    ap.add_argument("--repo-root", default=os.getcwd(),
                    help="repo root for relativizing paths (default: cwd)")
    ap.add_argument("--tool-name", default="clang-tidy")
    args = ap.parse_args()

    repo_root = os.path.realpath(args.repo_root)
    stream = open(args.log, encoding="utf-8", errors="replace") if args.log else sys.stdin

    rules = {}      # check-name -> rule object
    results = []
    seen = set()    # dedup (rel, line, col, check, msg)

    for raw in stream:
        m = DIAG_RE.search(raw.rstrip("\n"))
        if not m:
            continue
        rel = to_repo_relative(m["path"], repo_root)
        if rel is None:
            continue
        line, col = int(m["line"]), int(m["col"])
        # The bracket can carry the WarningsAsErrors suffix (e.g.
        # "[bugprone-foo,-warnings-as-errors]"); the primary check is first.
        check = m["checks"].split(",")[0].strip()
        # Skip compiler diagnostics (clang-diagnostic-*): these are build errors
        # (e.g. a TU that failed to parse due to a missing generated header), not
        # clang-tidy lint findings — they would be noise in Code Scanning.
        if check.startswith("clang-diagnostic"):
            continue
        msg = m["msg"].strip()
        level = "error" if m["level"] == "error" else "warning"

        key = (rel, line, col, check, msg)
        if key in seen:
            continue
        seen.add(key)

        if check not in rules:
            rules[check] = {
                "id": check,
                "name": check,
                "shortDescription": {"text": check},
                "helpUri": check_help_uri(check),
            }
        results.append({
            "ruleId": check,
            "level": level,
            "message": {"text": msg},
            "locations": [{
                "physicalLocation": {
                    "artifactLocation": {"uri": rel},
                    "region": {"startLine": line, "startColumn": col},
                }
            }],
        })

    sarif = {
        "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
        "version": "2.1.0",
        "runs": [{
            "tool": {"driver": {
                "name": args.tool_name,
                "informationUri": "https://clang.llvm.org/extra/clang-tidy/",
                "rules": list(rules.values()),
            }},
            "results": results,
        }],
    }
    json.dump(sarif, sys.stdout, indent=2)
    sys.stdout.write("\n")
    print(f"clang_tidy_to_sarif: {len(results)} results, {len(rules)} rules",
          file=sys.stderr)


if __name__ == "__main__":
    main()
