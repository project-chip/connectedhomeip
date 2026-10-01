#!/usr/bin/env python3

#
# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
import subprocess
import sys
from pathlib import Path

import yaml


def get_chip_root() -> Path:
    return next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))


def get_ncs_matter_addon_path(chip_root: Path) -> Path:
    addon_path = chip_root / 'third_party/nrfconnect/ncs-matter'
    if not addon_path.is_dir():
        raise RuntimeError(
            f"ncs-matter add-on not found at {addon_path}. "
            "Initialize it with: git submodule update --init third_party/nrfconnect/ncs-matter")
    return addon_path.resolve()


def ensure_ncs_matter_submodule(chip_root: Path) -> None:
    command = [
        'git', '-C', str(chip_root), 'submodule', 'update', '--init',
        'third_party/nrfconnect/ncs-matter',
    ]
    subprocess.run(command, check=True)


def get_west_topdir(zephyr_base: str) -> Path:
    west_topdir = Path(zephyr_base).resolve().parent
    if not (west_topdir / '.west').is_dir():
        raise RuntimeError(
            f"No west workspace found at {west_topdir}. "
            "Initialize one with the ncs-matter add-on manifest before running this script.")
    return west_topdir


def get_commit_sha(repository_location, rev):
    command = ['git', '-C', repository_location, 'rev-list', '-n', '1', rev]
    process = subprocess.run(command, check=True, stdout=subprocess.PIPE)
    return process.stdout.decode('ascii').strip()


def get_recommended_nrf_revision(ncs_matter_addon: Path) -> str:
    west_yml = ncs_matter_addon / 'west.yml'
    try:
        with open(west_yml, encoding='utf-8') as f:
            manifest = yaml.safe_load(f)
    except OSError as exc:
        raise RuntimeError(f"Unable to read ncs-matter manifest at {west_yml}.") from exc

    for project in manifest.get('manifest', {}).get('projects', []):
        if project.get('name') == 'nrf':
            revision = project.get('revision')
            if revision:
                return revision

    raise RuntimeError("Unable to find the nrf project revision in ncs-matter west.yml.")


def get_manifest_path(west_topdir: Path) -> Path:
    command = ['west', 'config', 'manifest.path']
    process = subprocess.run(command, cwd=west_topdir, check=True, stdout=subprocess.PIPE, text=True)
    manifest_path = process.stdout.strip()
    if not manifest_path:
        raise RuntimeError("west manifest.path is not configured.")

    resolved = Path(manifest_path)
    if not resolved.is_absolute():
        resolved = (west_topdir / resolved).resolve()
    else:
        resolved = resolved.resolve()

    return resolved


def switch_manifest_to_addon(west_topdir: Path, ncs_matter_addon: Path) -> None:
    command = ['west', 'config', 'manifest.path', str(ncs_matter_addon)]
    subprocess.run(command, cwd=west_topdir, check=True)


def update_ncs(west_topdir: Path, fetch_shallow: bool):
    command = ['west', 'update']
    command += ['--fetch', 'smart', '--narrow',
                '-o=--depth=1'] if fetch_shallow else []
    subprocess.run(command, cwd=west_topdir, check=True)


def print_messages(messages: list, yellow_text: bool):
    # Add colour formatting if yellow text was set
    if yellow_text:
        messages = [f"\33[33m{message}\x1b[0m" for message in messages]

    for message in messages:
        print(message)


def print_check_revision_warning_message(current_revision, recommended_revision):
    current_revision_message = f"WARNING: Your current NCS revision ({current_revision})"
    recommended_revision_message = f"differs from the recommended ({recommended_revision})."
    allowed_message = "Please be aware that it may lead to encountering unexpected problems."
    update_message = "Consider updating NCS to the recommended revision, by calling:"
    call_command_message = os.path.abspath(__file__) + " --update"

    # Get the longest message lenght, to fit warning frame size.
    longest_message_len = max([len(current_revision_message), len(recommended_revision_message), len(
        allowed_message), len(update_message), len(call_command_message)])

    # To keep right frame shape the space characters are added to messages shorter than the longest one.
    fmt = f"# {{:<{longest_message_len}}}#"

    print_messages([
        (longest_message_len+3)*'#', fmt.format(current_revision_message),
        fmt.format(recommended_revision_message), fmt.format(''),
        fmt.format(allowed_message), fmt.format(update_message),
        fmt.format(call_command_message), (longest_message_len+3)*'#'
    ], sys.stdout.isatty())


def print_check_manifest_warning_message(current_manifest_path, expected_manifest_path):
    current_manifest_message = f"WARNING: west manifest.path ({current_manifest_path})"
    expected_manifest_message = f"is not the ncs-matter add-on ({expected_manifest_path})."
    update_message = "Switch to the add-on manifest and update the workspace by calling:"
    call_command_message = os.path.abspath(__file__) + " --update"

    longest_message_len = max(
        len(current_manifest_message),
        len(expected_manifest_message),
        len(update_message),
        len(call_command_message),
    )
    fmt = f"# {{:<{longest_message_len}}}#"

    print_messages([
        (longest_message_len + 3) * '#',
        fmt.format(current_manifest_message),
        fmt.format(expected_manifest_message),
        fmt.format(''),
        fmt.format(update_message),
        fmt.format(call_command_message),
        (longest_message_len + 3) * '#',
    ], sys.stdout.isatty())


def main():

    try:
        zephyr_base = os.getenv("ZEPHYR_BASE")
        if not zephyr_base:
            raise RuntimeError(
                "No ZEPHYR_BASE environment variable found, please set ZEPHYR_BASE to a zephyr repository path.")

        parser = argparse.ArgumentParser(
            description='Script helping to update nRF Connect SDK to currently recommended revision.')
        parser.add_argument(
            "-c", "--check",
            help="Check if your current nRF Connect SDK revision is the same as recommended one.", action="store_true")
        parser.add_argument(
            "-u", "--update",
            help="Update your nRF Connect SDK to currently recommended revision.", action="store_true")
        parser.add_argument(
            "-s", "--shallow",
            help="Fetch only specific commits (without the history) when updating nRF Connect SDK.", action="store_true")
        parser.add_argument(
            "-q", "--quiet",
            help="Don't print any message if the check succeeds.", action="store_true")
        args = parser.parse_args()

        chip_root = get_chip_root()
        ensure_ncs_matter_submodule(chip_root)
        ncs_matter_addon = get_ncs_matter_addon_path(chip_root)
        west_topdir = get_west_topdir(zephyr_base)
        ncs_base = os.path.join(zephyr_base, '../nrf')
        recommended_revision = get_recommended_nrf_revision(ncs_matter_addon)

        if args.check:
            if not args.quiet:
                print("Checking current nRF Connect SDK revision...")

            current_manifest_path = get_manifest_path(west_topdir)
            if current_manifest_path != ncs_matter_addon:
                if not args.quiet:
                    print_check_manifest_warning_message(current_manifest_path, ncs_matter_addon)
                sys.exit(1)

            current_sha = get_commit_sha(ncs_base, 'HEAD')
            recommended_sha = get_commit_sha(ncs_base, recommended_revision)

            if current_sha != recommended_sha:
                print_check_revision_warning_message(
                    current_sha, recommended_sha)
                sys.exit(1)

            if not args.quiet:
                print("Your current version is up to date with the recommended one.")

        if args.update:
            print("Switching west manifest to the ncs-matter add-on...")
            switch_manifest_to_addon(west_topdir, ncs_matter_addon)

            print(f"Updating nRF Connect SDK workspace to {recommended_revision} using the add-on manifest...")
            update_ncs(west_topdir, args.shallow)

    except (RuntimeError, subprocess.CalledProcessError) as e:
        print(e)
        sys.exit(1)


if __name__ == '__main__':
    main()
