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

It is intended as a failsafe to not foget adding source files
to gn.
"""
import click
import coloredlogs

import logging
import os
import glob
import sys
from pathlib import Path, PurePath
from typing import Dict

__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

class OrphanChecker:
    def __init__(self):
        self.gn_data: Dict[str, str] = {}
        self.known_failures: Set[str] = set()
        self.fatal_failures = 0
        self.failures = 0

    def AppendGnData(self, gn: PurePath):
        logging.debug(f'Adding GN {gn!s} for {gn.parent!s}')
        self.gn_data[str(gn.parent)] = gn.read_text('utf-8')

    def AddKnownFailure(self, k: str):
        self.known_failures.add(k)

    def _IsKnownFailure(self, path: str) -> bool:
        """check if failing on the given path is a known/acceptable failure"""
        for k in self.known_failures:
            if path == k or path.endswith(os.path.sep + k):
                return True
        return False

    def Check(self, top_dir, file: PurePath):
        # Check logic:
        #   - ensure the file name is included in some GN file inside this or
        #     upper directory (although upper directory is not ideal)
        for p in file.parents:
            data = self.gn_data.get(str(p), None)
            if not data:
                continue

            if file.name in data:
                logging.debug("%s found in BUILD.gn for %s", file, p)
                return

        path = str(file.relative_to(top_dir))
        if not self._IsKnownFailure(path):
            logging.error("UNKNOWN to gn: %s", path)
            self.fatal_failures += 1
        else:
            logging.warning("UNKNOWN to gn: %s (known error)", path)

        self.failures += 1


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output',
)
@click.option(
    '--extensions',
    default=["cpp", "cc", "c", "h", "hpp"],
    type=str, multiple=True,
    help='What file extensions to consider',
)
@click.option(
    '--known-failure',
    type=str, multiple=True,
    help='What paths are known to fail',
)
@click.option(
    '--skip-dir',
    type=str,
    multiple=True,
    help='Skip a specific sub-directory from checks',
)
@click.argument('dirs',
    type=click.Path(exists=True, file_okay=False, resolve_path=True), nargs=-1)
def main(log_level, extensions, dirs, known_failure, skip_dir):
    coloredlogs.install(level=__LOG_LEVELS__[log_level],
                        fmt='%(asctime)s %(levelname)-7s %(message)s')

    if not dirs:
        logging.error("Please provide at least one directory to scan")
        sys.exit(1)

    if not extensions:
        logging.error("Need at  least one extension")
        sys.exit(1)

    checker = OrphanChecker()
    for k in known_failure:
        checker.AddKnownFailure(k)

    # ensure all GN data is loaded
    for directory in dirs:
        for name in Path(directory).rglob("BUILD.gn"):
            checker.AppendGnData(name)

    skip_dir = set(skip_dir)

    # Go through all files and check for orphaned (if any)
    extensions = set(extensions)
    for directory in dirs:
        for path, dirnames, filenames in os.walk(directory):
            if any([s in path for s in skip_dir]):
                continue
            for f in filenames:
                full_path = Path(os.path.join(path, f))
                if not full_path.suffix or full_path.suffix[1:] not in extensions:
                    continue
                checker.Check(directory, full_path)

    if checker.failures:
        logging.warning("%d files not known to GN (%d fatal)", checker.failures, checker.fatal_failures)

    if checker.fatal_failures > 0:
        sys.exit(1)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
