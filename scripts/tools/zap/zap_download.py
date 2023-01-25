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

import enum
import logging
import os
import re
import requests
import zipfile
import io
import sys
import shlex

from typing import Optional

import click

try:
    import coloredlogs
    _has_coloredlogs = True
except:
    _has_coloredlogs = False

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


class DownloadType(enum.Enum):
    RELEASE = enum.auto()  # asking for a zip file release download
    SOURCE = enum.auto()  # asking for a source download (will work on arm64 for example)

# Downloads zap into the current environment
# Arguments:
#    - $1 - zap version to download
#    - $2 - type of download:
#         "release" - attempts to download the release version (a zip file)
#         "devel" - checks out the corresponding tag and does "npm ci"


def _GetDefaultExtractRoot():
    if 'PW_ENVIRONMENT_ROOT' in os.environ:
        return os.environ['PW_ENVIRONMENT_ROOT']
    else:
        # Before bootstrap, this will pick a temporary directory. Probably
        # not ideal, but it likely just works
        return '/tmp/'

def _SetupSourceZap(install_directory: str, zap_version: str):
    # TODO:
    #   - git checkout
    #   - npm ci
    raise Exception("NOT YET IMPLEMENTED")

def _SetupReleaseZap(install_directory: str, zap_version: str):

    if sys.platform == 'linux':
        zap_platform = 'linux'
    elif sys.platform == 'darwin':
        zap_platform = 'mac'
    else:
        raise Exception('Unknown platform - do not know what zip file to download.')


    url = f"https://github.com/project-chip/zap/releases/download/{zap_version}/zap-{zap_platform}.zip"

    logging.info("Fetching: %s", url)

    r = requests.get(url, stream=True)
    z = zipfile.ZipFile(io.BytesIO(r.content))

    logging.info("Data downloaded, extracting ...")
    z.extractall(install_directory)
    logging.info("Done extracting.")


def _GetZapVersionToUse(project_root):
    """
    Heuristic to figure out what zap version should be used.

    Looks at the given project root and tries to figure out the zap tag/version.
    """

    # We have several locations for zap versioning:
    #  - CI is likely the most reliable as long as we use the "latest build"
    #  - zap_execution.py is what is currently used, but it is a minimum version
    #
    # Based on the above, we assume CI is using the latest build (will not be
    # out of sync more than a few days) and even if it is not, zap is often
    # backwards compatible (new features added, but codegen should not change
    # that often for fixed inputs)
    #
    # This heuristic may be bad at times, however then you can also override the
    # version in command line parameters

    match_re = re.compile(r'.*ENV\s+ZAP_VERSION=([^# ]*)')

    docker_path = os.path.join(project_root, "integrations/docker/images/chip-build/Dockerfile")

    with open(docker_path, 'rt') as f:
        for l in f.readlines():
            l = l.strip()
            m = match_re.match(l)
            if not m:
                continue
            return m.group(1)

    raise Exception(f"Failed to determine version from {docker_path}")


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    show_default=True,
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    callback=lambda c, p, v: __LOG_LEVELS__[v],
    help='Determines the verbosity of script output')
@click.option(
    '--sdk-root',
    default=os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', '..')),
    show_default=True,
    help='Path to the SDK root (where zap versioning exists).')
@click.option(
    '--extract-root',
    default=_GetDefaultExtractRoot(),
    show_default=True,
    help='Directory where too unpack/checkout zap')
@click.option(
    '--zap-version',
    default=None,
    help='Force to checkout this zap version instead of trying to auto-detect')
@click.option(
    '--zap',
    default='RELEASE',
    show_default=True,
    type=click.Choice(DownloadType.__members__, case_sensitive=False),
    callback=lambda c, p, v: getattr(DownloadType, v),
    help='What type of zap download to perform')
def main(log_level: str, sdk_root: str, extract_root: str, zap_version: Optional[str], zap: DownloadType):
    if _has_coloredlogs:
        coloredlogs.install(level=log_level, fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    if not zap_version:
        zap_version = _GetZapVersionToUse(sdk_root)
        logging.info('Found required zap version to be: %s' % zap_version)

    logging.debug('User requested to download a %s zap version %s into %s', zap, zap_version, extract_root)

    install_directory = os.path.join(extract_root, f"zap-{zap_version}")
    if zap == DownloadType.SOURCE:
        install_directory = install_directory + "-src"
        _SetupSourceZap(install_directory, zap_version)

        # Make sure the results can be used in scripts
        print(f"export ZAP_DEVELOPMENT_PATH={shlex.quote(install_directory)}")
    else:
        _SetupReleaseZap(install_directory, zap_version)

        # Make sure the results can be used in scripts
        print(f"export ZAP_INSTALL_PATH={shlex.quote(install_directory)}")


if __name__ == '__main__':
    main()
