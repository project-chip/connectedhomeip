#!/usr/bin/env python3
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

"""Check that nightly.yaml sets the variables and builds the apps that nightly Python tests use.

PR CI skips nightly test runs, so a variable that only tests.yaml sets passes PR CI
and fails the next nightly run.
"""

import importlib.util
import re
import sys
from pathlib import Path

import yaml

ENV_LINE = re.compile(r'echo "([A-Z0-9_]+): ([^"]+)" >> /tmp/test_env.yaml')
TARGET = re.compile(r'--target\s+(\S+)')
VAR = re.compile(r'\$\{([A-Z0-9_]+)\}')


def load_metadata_module(root: Path):
    path = root / 'src/python_testing/matter_testing_infrastructure/matter/testing/metadata.py'
    spec = importlib.util.spec_from_file_location('ci_metadata', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def nightly_runs(root: Path, metadata: dict, module):
    """Yield (file name, run name, run args) for each run the nightly job executes, or (file name, None, error)."""
    # Before per-run tags existed, every run of a nightly-list file ran in nightly and no other run did.
    runs_in_pull_requests = getattr(module, 'runs_in_pull_requests', lambda tags: not tags)
    # extract_runs_args raises AttributeError on a missing block before RunArgumentsError existed.
    parse_errors = (AttributeError, getattr(module, 'RunArgumentsError', AttributeError))
    nightly_files = {entry['name'] for entry in metadata['nightly']}
    not_automated = {entry['name'] for entry in metadata['not_automated']}

    for script in sorted((root / 'src/python_testing').glob('*.py')):
        if script.name in not_automated:
            continue
        error = 'no runs found in the CI TEST ARGUMENTS block'
        try:
            runs = module.extract_runs_args(str(script))
        except parse_errors as e:
            runs = {}
            if not isinstance(e, AttributeError):
                error = e
        if script.name in nightly_files and not runs:
            yield script.name, None, error
        for run, args in runs.items():
            if script.name in nightly_files or not runs_in_pull_requests(args.get('tags', [])):
                yield script.name, run, args


def main(root: Path) -> int:
    module = load_metadata_module(root)
    metadata = yaml.safe_load((root / 'src/python_testing/test_metadata.yaml').read_text())
    jobs = yaml.safe_load((root / '.github/workflows/nightly.yaml').read_text())['jobs']

    # Only the REPL build job's targets reach the run job, through the objdir-clone archive.
    env = dict(ENV_LINE.findall(yaml.safe_dump(jobs['nightly_repl_tests_linux_run'], width=1 << 20)))
    targets = set(TARGET.findall(yaml.safe_dump(jobs['nightly_repl_tests_linux_build'], width=1 << 20)))

    errors = []
    checked = 0
    for name, run, args in nightly_runs(root, metadata, module):
        if run is None:
            errors.append(f'{name}: {args}')
            continue
        checked += 1
        used = {var for value in args.values() if isinstance(value, str) for var in VAR.findall(value)}
        for var in sorted(used):
            if var not in env:
                errors.append(f'{name} {run}: ${{{var}}} is not written to /tmp/test_env.yaml in nightly.yaml')
                continue
            parts = Path(env[var]).parts
            if len(parts) > 2 and parts[0] == 'objdir-clone' and parts[1] not in targets:
                errors.append(f'{name} {run}: ${{{var}}} points to {parts[1]}, which nightly.yaml does not build')

    for error in errors:
        print(error)
    print(f'{checked} nightly runs checked, {len(errors)} problem(s)')
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main(Path(sys.argv[1] if len(sys.argv) > 1 else '.')))
