#!/usr/bin/env python3
#
# Copyright (c) 2022 Project CHIP Authors
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
import configparser
import logging
import os
import subprocess
from collections import namedtuple

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

ALL_PLATFORMS = set([
    'ameba',
    'android',
    'bl602',
    'bouffalolab',
    'cc13x2_26x2',
    'cc32xx',
    'darwin',
    'efr32',
    'esp32',
    'infineon',
    'k32w0',
    'linux',
    'mbed',
    'nrfconnect',
    'qpg',
    'telink',
    'tizen',
    'webos',
    'mw320',
    'genio',
    'openiotsdk',
    'silabs_docker',
])

Module = namedtuple('Module', 'name path platforms')


def load_module_info() -> None:
    config = configparser.ConfigParser()
    config.read(os.path.join(CHIP_ROOT, '.gitmodules'))

    for name, module in config.items():
        if name != 'DEFAULT':
            platforms = module.get('platforms', '').split(',')
            platforms = set(filter(None, platforms))
            assert not (platforms - ALL_PLATFORMS), "Submodule's platform not contained in ALL_PLATFORMS"
            name = name.replace('submodule "', '').replace('"', '')
            yield Module(name=name, path=module['path'], platforms=platforms)


def module_matches_platforms(module: Module, platforms: set) -> bool:
    # If the module is not associated with any specific platform, treat it as a match.
    if not module.platforms:
        return True
    return bool(platforms & module.platforms)


def module_initialized(module: Module) -> bool:
    return bool(os.listdir(os.path.join(CHIP_ROOT, module.path)))


def make_chip_root_safe_directory() -> None:
    # Can't use check_output, git will exit(1) if the setting has no existing value
    config = subprocess.run(['git', 'config', '--global', '--null', '--get-all',
                            'safe.directory'], stdout=subprocess.PIPE, text=True)
    existing = []
    if config.returncode != 1:
        config.check_returncode()
        existing = config.stdout.split('\0')
    if CHIP_ROOT not in existing:
        logging.info("Adding CHIP_ROOT to global git safe.directory configuration")
        subprocess.check_call(['git', 'config', '--global', '--add', 'safe.directory', CHIP_ROOT])


def checkout_modules(modules: list, shallow: bool, force: bool, recursive: bool) -> None:
    names = ', '.join([module.name for module in modules])
    logging.info(f'Checking out: {names}')

    cmd = ['git', '-C', CHIP_ROOT, 'submodule', '--quiet', 'update', '--init']
    cmd += ['--depth', '1'] if shallow else []
    cmd += ['--force'] if force else []
    cmd += ['--recursive'] if recursive else []
    cmd += [module.path for module in modules]

    subprocess.check_call(cmd)


def deinit_modules(modules: list, force: bool) -> None:
    names = ', '.join([module.name for module in modules])
    logging.info(f'Deinitializing: {names}')

    cmd = ['git', '-C', CHIP_ROOT, 'submodule', '--quiet', 'deinit']
    cmd += ['--force'] if force else []
    cmd += [module.path for module in modules]

    subprocess.check_call(cmd)


def main():
    logging.basicConfig(format='%(message)s', level=logging.INFO)

    parser = argparse.ArgumentParser(description='Checkout or update relevant git submodules')
    parser.add_argument('--allow-changing-global-git-config', action='store_true',
                        help='Allow global git options to be modified if necessary, e.g. safe.directory')
    parser.add_argument('--shallow', action='store_true', help='Fetch submodules without history')
    parser.add_argument('--platform', nargs='+', choices=ALL_PLATFORMS, default=[],
                        help='Process submodules for specific platforms only')
    parser.add_argument('--force', action='store_true', help='Perform action despite of warnings')
    parser.add_argument('--deinit-unmatched', action='store_true',
                        help='Deinitialize submodules for non-matching platforms')
    parser.add_argument('--recursive', action='store_true', help='Recursive init of the listed submodules')
    args = parser.parse_args()

    modules = list(load_module_info())
    selected_platforms = set(args.platform)
    selected_modules = [m for m in modules if module_matches_platforms(m, selected_platforms)]
    unmatched_modules = [m for m in modules if not module_matches_platforms(
        m, selected_platforms) and module_initialized(m)]

    if args.allow_changing_global_git_config:
        make_chip_root_safe_directory()  # ignore directory ownership issues for sub-modules
    checkout_modules(selected_modules, args.shallow, args.force, args.recursive)

    if args.deinit_unmatched and unmatched_modules:
        deinit_modules(unmatched_modules, args.force)


if __name__ == '__main__':
    main()
