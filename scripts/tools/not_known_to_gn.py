#!/usr/bin/env python3
#
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
#

"""
Lists files specific files from a source tree and ensures
they are covered by GN in some way.

'Covered' is very loosely and it just tries to see if the GN text
contains that word without trying to validate if this is a
comment or some actual 'source' element.

It is intended as a failsafe to not forget adding source files
to gn.
"""

import logging
import os
import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, Set

import click
import coloredlogs

log = logging.getLogger(__name__)

__LOG_LEVELS__ = logging.getLevelNamesMapping()


class OrphanChecker:
    def __init__(self):
        self.gn_data: Dict[str, str] = defaultdict(str)
        self.known_failures: Set[str] = set()
        self.fatal_failures = 0
        self.failures = 0
        self.found_failures: Set[str] = set()

    def AppendGnData(self, gn: Path):
        """Adds a GN file to the list of internally known GN data.

        Will read the entire content of the GN file in memory for future reference.
        """
        log.debug("Adding GN '%s' for '%s'", gn, gn.parent)
        self.gn_data[str(gn.parent)] += gn.read_text('utf-8')

    def AddKnownFailure(self, k: str):
        self.known_failures.add(k)

    def _IsKnownFailure(self, path: str) -> bool:
        """check if failing on the given path is a known/acceptable failure"""
        for k in self.known_failures:
            if path == k or path.endswith(os.path.sep + k):
                # mark some found failures to report if something is supposed
                # to be known but it is not
                self.found_failures.add(k)
                return True
        return False

    def Check(self, top_dir: str, file: Path):
        """
        Validates that the given path is somehow referenced in GN files in any
        of the parent sub-directories of the file.

        `file` must be relative to `top_dir`. Top_dir is used to resolve relative
        paths in error reports and known failure checks.
        """
        # Check logic:
        #   - ensure the file name is included in some GN file inside this or
        #     upper directory (although upper directory is not ideal)
        for p in file.parents:
            data = self.gn_data.get(str(p), None)
            if not data:
                continue

            # Search for the full file name
            # e.g. `cluster-config.h` should not match `config.h`
            if re.search("(^|\\s|[/'\"])" + re.escape(file.name) + r"\W", data):
                log.debug("'%s' found in BUILD.gn for '%s'", file, p)
                return

        path = str(file.relative_to(top_dir))
        if not self._IsKnownFailure(path):
            log.error("UNKNOWN to gn: '%s'", path)
            self.fatal_failures += 1
        else:
            log.warning("UNKNOWN to gn: '%s' (known error)", path)

        self.failures += 1


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output',
)
@click.option(
    '--gn-extra',
    type=click.Path(exists=True, dir_okay=False),
    multiple=True,
    help=(
        'Extra GN files which should be treated as BUILD.gn (e.g. *.gni files '
        'included by BUILD.gn)'),
)
@click.option(
    '-e', '--extension',
    default=["cpp", "cc", "c", "h", "hpp"],
    type=str, multiple=True,
    help='What file extensions to consider',
)
@click.option(
    '--known-failure',
    type=click.Path(dir_okay=False),
    multiple=True,
    help='What paths are known to fail',
)
@click.option(
    '--skip-dir',
    type=click.Path(),
    multiple=True,
    help='Skip a specific sub-directory from checks',
)
@click.argument('dirs',
                type=click.Path(exists=True, file_okay=False, resolve_path=True), nargs=-1)
def main(log_level, gn_extra, extension, known_failure, skip_dir, dirs):
    coloredlogs.install(level=__LOG_LEVELS__[log_level],
                        fmt='%(asctime)s %(levelname)-7s %(message)s')

    if not dirs:
        log.error("Please provide at least one directory to scan")
        sys.exit(1)

    if not extension:
        log.error("Need at least one extension")
        sys.exit(1)

    checker = OrphanChecker()
    for k in known_failure:
        checker.AddKnownFailure(k)

    # ensure all GN data is loaded
    for directory in dirs:
        for name in Path(directory).rglob("BUILD.gn"):
            checker.AppendGnData(name)
    for name in gn_extra:
        checker.AppendGnData(Path(name).absolute())

    skip_dir = set(skip_dir)

    # Go through all files and check for orphaned (if any)
    extensions = set(extension)
    for directory in dirs:
        for path, dirnames, filenames in os.walk(directory):
            if any(s in path for s in skip_dir):
                continue
            for f in filenames:
                full_path = Path(os.path.join(path, f))
                if not full_path.suffix or full_path.suffix[1:] not in extensions:
                    continue
                checker.Check(directory, full_path)

    if checker.failures:
        log.warning("%d files not known to GN (%d fatal)", checker.failures, checker.fatal_failures)

    if checker.known_failures != checker.found_failures:
        not_failing = checker.known_failures - checker.found_failures
        log.warning("NOTE: %d failures are not found anymore:", len(not_failing))
        for name in not_failing:
            log.warning("   - '%s'", name)
        # Assume this is fatal - remove some of the "known-failing" should be easy.
        # This forces scripts to always be correct and not accumulate bad input.
        sys.exit(1)

    if checker.fatal_failures > 0:
        sys.exit(1)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
