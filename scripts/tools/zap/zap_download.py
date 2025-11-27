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
import tempfile
import zipfile
from typing import Optional

import click
import requests

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

log = logging.getLogger(__name__)

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = logging.getLevelNamesMapping()


class DownloadType(enum.Enum):
    RELEASE = enum.auto()  # asking for a zip file release download
    SOURCE = enum.auto()  # asking for a source download (will work on arm64 for example)


def _GetDefaultExtractRoot():
    if 'PW_ENVIRONMENT_ROOT' in os.environ:
        return os.environ['PW_ENVIRONMENT_ROOT']
    return ".zap"


def _LogPipeLines(pipe, prefix):
    _log = log.getChild(prefix)
    for line in iter(pipe.readline, b''):
        line = line.strip().decode('utf-8', errors="ignore")
        _log.info(line)


def _ExecuteProcess(cmd, cwd):
    log.info("Executing in '%s': %s", cwd, shlex.join(cmd))

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
        log.warning("Completely re-creating '%s'", install_directory)
        shutil.rmtree(install_directory)

    os.makedirs(install_directory, exist_ok=True)

    _ExecuteProcess(
        f"git clone --depth 1 --branch {zap_version} https://github.com/project-chip/zap.git .".split(),
        install_directory
    )

    _ExecuteProcess("npm ci".split(), install_directory)


def _GetDefaultPlatform():
    match sys.platform:
        case 'linux':
            return 'linux'
        case 'darwin':
            return 'mac'
        case 'win32':
            return 'win'
        case _:
            raise Exception('Unknown platform - do not know what zip file to download.')


def _GetDefaultArch():
    arch = None

    match sys.platform:
        case 'win32':
            # os.uname is not implemented on Windows, so use an alternative instead.
            import platform
            arch = platform.uname().machine
        case _:
            arch = os.uname().machine

    if arch == 'x86_64' or arch == 'AMD64':
        return 'x64'

    # this should be `arm64` ...
    return arch


def DownloadReleasedZap(install_directory: str, zap_version: str, zap_platform: str, zap_arch: str):
    """
    Downloads the given [zap_version] into "[install_directory]/zap-[zap_version]/".

    Will download the given release from github releases.
    """

    url = f"https://github.com/project-chip/zap/releases/download/{zap_version}/zap-{zap_platform}-{zap_arch}.zip"

    log.info("Fetching: '%s'", url)

    r = requests.get(url, stream=True)
    if zap_platform == 'mac':
        # zipfile does not support symlinks (https://github.com/python/cpython/issues/82102),
        # making a zap.app extracted with it unusable due to embedded frameworks.
        with tempfile.NamedTemporaryFile(suffix='.zip') as tf:
            for chunk in r.iter_content(chunk_size=4096):
                tf.write(chunk)
            tf.flush()
            os.makedirs(install_directory, exist_ok=True)
            _ExecuteProcess(['/usr/bin/unzip', '-oq', tf.name], install_directory)
    else:
        z = zipfile.ZipFile(io.BytesIO(r.content))
        log.info("Data downloaded, extracting ...")
        # extractall() does not preserve permissions (https://github.com/python/cpython/issues/59999)
        for entry in z.filelist:
            path = z.extract(entry, install_directory)
            os.chmod(path, (entry.external_attr >> 16) & 0o777)
    log.info("Done extracting.")


def _GetZapVersionToUse(project_root) -> str:
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
            if tag.startswith("version:"):
                zap_version = tag.removeprefix("version:")

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
@click.option('--platform', default=_GetDefaultPlatform(), show_default=True, help='ZAP Platform to download')
@click.option('--arch', default=_GetDefaultArch(), show_default=True, help='ZAP Architecture to download')
def main(log_level: str, sdk_root: str, extract_root: str, zap_version: Optional[str], zap: DownloadType, platform: str, arch: str):
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
        log.info("Found required zap version to be: '%s'", zap_version)

    log.debug("User requested to download a '%s' zap version '%s' into '%s'", zap, zap_version, extract_root)

    install_directory = os.path.join(extract_root, f"zap-{zap_version}")

    export_cmd = "export"
    if sys.platform == 'win32':
        export_cmd = "set"

    if zap == DownloadType.SOURCE:
        install_directory = install_directory + "-src"
        _SetupSourceZap(install_directory, zap_version)

        # Make sure the results can be used in scripts
        print(f"{export_cmd} ZAP_DEVELOPMENT_PATH={shlex.quote(install_directory)}")
    else:
        DownloadReleasedZap(install_directory, zap_version, platform, arch)

        # Make sure the results can be used in scripts
        print(f"{export_cmd} ZAP_INSTALL_PATH={shlex.quote(install_directory)}")


if __name__ == '__main__':
    main()
