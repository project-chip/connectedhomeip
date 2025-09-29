#!/usr/bin/env python3

import logging
import os
import shlex
import subprocess
import sys
import tempfile

import click

sys.path.append(os.path.abspath(os.path.dirname(__file__)))
from zap_download import DownloadReleasedZap  # noqa: E402 isort:skip


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


def _ZapArchitectures():
    """Iterates through required releases.

    Returns a tuple:
      - zap zip 'platform'
      - zap zip 'arch'
      - CIPD package tuple
    """

    yield 'linux', 'arm64', 'linux-arm64'
    yield 'linux', 'x64', 'linux-amd64'
    yield 'mac', 'arm64', 'mac-arm64'
    yield 'mac', 'x64', 'mac-amd64'
    yield 'win', 'x64', 'windows-amd64'


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    show_default=True,
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    callback=lambda c, p, v: __LOG_LEVELS__[v],
    help='Determines the verbosity of script output')
@click.option('--no-temp-clean', is_flag=True)
@click.option('--version', help='Zap version to use', required=True)
def main(log_level: str, version: str, no_temp_clean: bool):
    if _has_coloredlogs:
        coloredlogs.install(level=log_level, fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s %(name)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    with tempfile.TemporaryDirectory(prefix="zap_", suffix="_cipd", delete=(not no_temp_clean)) as tmpdir:
        logging.info("Temporary Directory: %s", tmpdir)

        for platform, arch, cipd_dir in _ZapArchitectures():
            download_dir = f"zap-{platform}-{arch}"
            download_path = os.path.join(tmpdir, download_dir)
            logging.info("Downloading %s-%s into %s", platform, arch, download_path)
            DownloadReleasedZap(download_path, version, platform, arch)

            # Release downloaded, create a CIPD definition and upload it
            cipd_def_file = f"cipd_{platform}_{arch}.yaml"
            with open(os.path.join(tmpdir, cipd_def_file), "wt") as f:
                f.write(f"package: experimental/matter/zap/{cipd_dir}\n")
                f.write(f"description: ZAP release {version} of {platform}-{arch}\n")
                f.write("install_mode: copy\n")
                f.write("data:\n")
                f.write(f"  - dir: {download_dir}\n")

            cmd = [
                "cipd",
                "create",
                f"-pkg-def={cipd_def_file}",
                "-tag",
                f"version:{version}",
            ]
            logging.info("Creating CIPD: %s", shlex.join(cmd))

            subprocess.check_call(cmd, cwd=tmpdir)


if __name__ == '__main__':
    main()
