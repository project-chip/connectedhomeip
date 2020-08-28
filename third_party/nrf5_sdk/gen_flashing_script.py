# Copyright (c) 2020 Project CHIP Authors
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
# limitations under the License.!/usr/bin/env python

"""Generate script to flash or erase an NRF5 device."""

import argparse
import os
import stat
import sys

SCRIPT = """#!/usr/bin/env python

import sys

SCRIPTS_DIR = '{scripts_dir}'
DEFAULTS = {{
{defaults}
}}

sys.path.append(SCRIPTS_DIR)
import nrf5_firmware_utils

if __name__ == '__main__':
    sys.exit(nrf5_firmware_utils.flash_command(sys.argv, DEFAULTS))
"""


def main(argv):
    """Generate script to flash or erase an NRF5 device."""
    parser = argparse.ArgumentParser(description='Generate a flashing script')
    parser.add_argument(
        '--output',
        metavar='FILENAME',
        required=True,
        help='flashing script name')
    parser.add_argument(
        '--scripts-dir',
        metavar='DIR',
        required=True,
        help='nrf5 script utilities directory')
    parser.add_argument(
        '--nrfjprog',
        metavar='FILENAME',
        help='nrfjprog command')
    parser.add_argument(
        '--family',
        metavar='FAMILY',
        help='device family')
    parser.add_argument(
        '--softdevice',
        metavar='GLOB',
        help='softdevice file pattern')
    parser.add_argument(
        '--application',
        metavar='FILENAME',
        help='program to flash')

    args = parser.parse_args(argv)
    options = vars(args)

    defaults = []
    for key in ['nrfjprog', 'family', 'softdevice', 'application']:
        if options.get(key):
            defaults.append('  {}: {},'.format(repr(key), repr(options[key])))

    script = SCRIPT.format(
        scripts_dir=args.scripts_dir, defaults='\n'.join(defaults))
    try:
        with open(args.output, 'w') as script_file:
            script_file.write(script)
        os.chmod(args.output, (stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR
                             | stat.S_IXGRP | stat.S_IRGRP
                             | stat.S_IXOTH | stat.S_IROTH))
    except OSError as exception:
        print(exception, sys.stderr)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
