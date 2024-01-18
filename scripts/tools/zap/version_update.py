#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This automates ZAP version changes as the zap version is repeated
# in many places

import logging
import os
import re
import sys
from enum import Flag, auto

import click
import coloredlogs

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

# A version is of the form: v2023.01.09-nightly
#
# At this time we hard-code nightly however we may need to figure out a more
# generic version string once we stop using nightly builds
ZAP_VERSION_RE = re.compile(r'v(\d\d\d\d)\.(\d\d)\.(\d\d)-nightly')

# A list of files where ZAP is maintained. You can get a similar list using:
#
# rg v2023.01.09-nightly --hidden --no-ignore --files-with-matches
#
# Excluding THIS file and excluding anything in .environment (logs)
#
# Set as a separate list to not pay the price of a full grep as the list of
# files is not likely to change often
USAGE_FILES_DEPENDING_ON_ZAP_VERSION = [
    'integrations/docker/images/chip-cert-bins/Dockerfile',
    'scripts/setup/zap.json',
    'scripts/setup/zap.version',
]


class UpdateChoice(Flag):
    # Usage updates the CI, chip-cert and execution logic. Generally everything
    # that would make use of the updated zap version
    USAGE = auto()


__UPDATE_CHOICES__ = {
    'usage': UpdateChoice.USAGE,
}

# Apart from the above files which contain an exact ZAP version, the zap
# execution script contains the mimimal zap execution version, which generally
# we also enforce to be the current version.
#
# That line is of the form "MIN_ZAP_VERSION = '2021.1.9'"
ZAP_EXECUTION_SCRIPT = 'scripts/tools/zap/zap_execution.py'
ZAP_EXECUTION_MIN_RE = re.compile(
    r'(MIN_ZAP_VERSION = .)(\d\d\d\d\.\d\d?\.\d\d?)(.)')

CHIP_ROOT_DIR = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '..', '..', '..'))


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--update',
    default='usage',
    type=click.Choice(__UPDATE_CHOICES__.keys(), case_sensitive=False),
    help='What to update: usage (only choice currently).')
@click.option(
    '--new-version',
    default=None,
    help='What version of ZAP to update to (like "v2023.01.09-nightly". If not set, versions will just be printed.')
def version_update(log_level, update, new_version):
    coloredlogs.install(level=__LOG_LEVELS__[
                        log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')

    update = __UPDATE_CHOICES__[update]

    if new_version:
        parsed = ZAP_VERSION_RE.match(new_version)
        if not parsed:
            logging.error(
                f"Version '{new_version}' does not seem to parse as a ZAP VERSION")
            sys.exit(1)

        # get the numeric version for zap_execution
        #
        # This makes every group element (date section) to a base 10 integer,
        # so for 'v2023.01.11-nightly' this gets (2023, 1, 11)
        zap_min_version = tuple(map(lambda x: int(x, 10), parsed.groups()))

    files_to_update = []
    if UpdateChoice.USAGE in update:
        files_to_update += USAGE_FILES_DEPENDING_ON_ZAP_VERSION

    for name in files_to_update:
        with open(os.path.join(CHIP_ROOT_DIR, name), 'rt') as f:
            file_data = f.read()

        # Write out any matches. Note that we only write distinct matches as
        # zap versions may occur several times in the same file
        found_versions = set()
        for m in ZAP_VERSION_RE.finditer(file_data):
            version = file_data[m.start():m.end()]
            if version not in found_versions:
                logging.info('%s currently used in %s', version, name)
                found_versions.add(version)

        # If we update, perform the update
        if new_version:
            search_pos = 0
            need_replace = False
            m = ZAP_VERSION_RE.search(file_data, search_pos)
            while m:
                version = file_data[m.start():m.end()]
                if version == new_version:
                    logging.warning(
                        "Nothing to replace. Version already %s", version)
                    break
                file_data = file_data[:m.start()] + \
                    new_version + file_data[m.end():]
                need_replace = True
                search_pos = m.end()  # generally ok since our versions are fixed length
                m = ZAP_VERSION_RE.search(file_data, search_pos)

            if need_replace:
                logging.info('Replacing with version %s in %s',
                             new_version, name)

                with open(os.path.join(CHIP_ROOT_DIR, name), 'wt') as f:
                    f.write(file_data)

    # Finally, check zap_execution for any version update
    if UpdateChoice.USAGE in update:
        with open(os.path.join(CHIP_ROOT_DIR, ZAP_EXECUTION_SCRIPT), 'rt') as f:
            file_data = f.read()

        m = ZAP_EXECUTION_MIN_RE.search(file_data)
        logging.info("Min version %s in %s", m.group(2), ZAP_EXECUTION_SCRIPT)
        if new_version:
            new_min_version = ("%d.%d.%d" % zap_min_version)
            file_data = file_data[:m.start()] + m.group(1) + \
                new_min_version + m.group(3) + file_data[m.end():]
            logging.info('Updating min version to %s in %s',
                         new_min_version, ZAP_EXECUTION_SCRIPT)

            with open(os.path.join(CHIP_ROOT_DIR, ZAP_EXECUTION_SCRIPT), 'wt') as f:
                f.write(file_data)


if __name__ == '__main__':
    version_update()
