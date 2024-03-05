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
import io
import json
import logging
import os
import shlex
import shutil
import subprocess
import sys
import zipfile
from typing import Optional

import click
import requests

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
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


def _GetDefaultExtractRoot():
    if 'PW_ENVIRONMENT_ROOT' in os.environ:
        return os.environ['PW_ENVIRONMENT_ROOT']
    else:
        return ".zap"


def _LogPipeLines(pipe, prefix):
    log = logging.getLogger().getChild(prefix)
    for line in iter(pipe.readline, b''):
        line = line.strip().decode('utf-8', errors="ignore")
        log.info('%s' % line)


def _ExecuteProcess(cmd, cwd):
    logging.info('Executing %r in %s' % (cmd, cwd))

    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=cwd)

    with process.stdout:
        _LogPipeLines(process.stdout, cmd[0])

    exitcode = process.wait()
    if exitcode != 0:
        raise Exception("Error executing process: %d" % exitcode)


def _SetupSourceZap(install_directory: str, zap_version: str):
    if os.path.exists(install_directory):
        logging.warning("Completely re-creating %s", install_directory)
        shutil.rmtree(install_directory)

    os.makedirs(install_directory, exist_ok=True)

    _ExecuteProcess(
        f"git clone --depth 1 --branch {zap_version} https://github.com/project-chip/zap.git .".split(),
        install_directory
    )

    _ExecuteProcess("npm ci".split(), install_directory)


def _SetupReleaseZap(install_directory: str, zap_version: str):
    """
    Downloads the given [zap_version] into "[install_directory]/zap-[zap_version]/".

    Will download the given release from github releases.
    """

    if sys.platform == 'linux':
        zap_platform = 'linux'
    elif sys.platform == 'darwin':
        zap_platform = 'mac'
    else:
        raise Exception('Unknown platform - do not know what zip file to download.')

    arch = os.uname().machine
    if arch == 'arm64':
        zap_arch = 'arm64'
    elif arch == 'x86_64':
        zap_arch = 'x64'
    else:
        raise Exception(f'Unknown architecture "${arch}" - do not know what zip file to download.')

    url = f"https://github.com/project-chip/zap/releases/download/{zap_version}/zap-{zap_platform}-{zap_arch}.zip"

    logging.info("Fetching: %s", url)

    r = requests.get(url, stream=True)
    z = zipfile.ZipFile(io.BytesIO(r.content))

    logging.info("Data downloaded, extracting ...")
    # extractall() does not preserve permissions (https://github.com/python/cpython/issues/59999)
    for entry in z.filelist:
        path = z.extract(entry, install_directory)
        os.chmod(path, (entry.external_attr >> 16) & 0o777)
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

    zap_version = ""
    zap_path = os.path.join(project_root, "scripts/setup/zap.json")
    zap_json = json.load(open(zap_path))
    for package in zap_json.get("packages", []):
        for tag in package.get("tags", []):
            if tag.startswith("version:2@"):
                zap_version = tag.removeprefix("version:2@")
                suffix_index = zap_version.rfind(".")
                if suffix_index != -1:
                    zap_version = zap_version[:suffix_index]
                return zap_version

    raise Exception(f"Failed to determine version from {zap_path}")


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
        coloredlogs.install(level=log_level, fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s %(name)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    if extract_root == ".zap":
        # Place .zap in the project root
        extract_root = os.path.join(sdk_root, extract_root)

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
