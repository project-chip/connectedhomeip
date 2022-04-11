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
from collections import namedtuple
import configparser
import subprocess
import os
import sys

CHIP_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

ALL_PLATFORMS = [
    'ameba',
    'android',
    'bl602',
    'cc13x2_26x2',
    'cyw30739',
    'darwin',
    'efr32',
    'esp32',
    'k32w0',
    'linux',
    'mbed',
    'nrfconnect',
    'p6',
    'qpg',
    'telink',
    'tizen',
    'webos',
]

Module = namedtuple('Module', 'name path platforms')


def load_module_info() -> list:
    config = configparser.ConfigParser()
    config.read(os.path.join(CHIP_ROOT, '.gitmodules'))

    def make_module_info(name, module):
        platforms = module.get('platforms', '').split(',')
        platforms = set(filter(None, platforms))
        return Module(name=name, path=module['path'], platforms=platforms)

    return [make_module_info(name, module) for name, module in config.items() if name != 'DEFAULT']


def module_matches_platforms(module: Module, platforms: set) -> bool:
    # If no platforms have been selected, or the module is not associated with any specific
    # platforms, treat it as a match.
    if not platforms or not module.platforms:
        return True
    return bool(platforms & module.platforms)


def checkout_modules(modules: list, shallow: bool) -> None:
    names = [module.name.replace('submodule "', '').replace('"', '') for module in modules]
    names = ', '.join(names)
    sys.stderr.write(f'Checking out: {names}\n')

    cmd = ['git', '-C', CHIP_ROOT, 'submodule', 'update', '--init']
    cmd += ['--depth', '1'] if shallow else []
    cmd += [module.path for module in modules]

    subprocess.check_call(cmd)


def main():
    parser = argparse.ArgumentParser(description='Checkout or update relevant git submodules')
    parser.add_argument('--shallow', action='store_true', help='Fetch submodules without history')
    parser.add_argument('--platform', nargs='+', choices=ALL_PLATFORMS, default=[],
                        help='Process submodules for specific platforms only')
    args = parser.parse_args()

    modules = load_module_info()
    selected_platforms = set(args.platform)
    selected_modules = [m for m in modules if module_matches_platforms(m, selected_platforms)]

    checkout_modules(selected_modules, args.shallow)


if __name__ == '__main__':
    main()
