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

"""Generate scripts to flash or erase an NRF5 device.
"""

import argparse
import glob
import os
import stat
import sys
import textwrap

DEFAULT_NRFJPROG = "nrfjprog"
DEFAULT_NRF_FAMILY = "nrf52"


def generate_script(output_filename, body):
    """Write a Python script with boilerplate plus the given `body`."""
    try:
        with open(output_filename, 'w') as f:
            f.write('#!/usr/bin/env python' + textwrap.dedent("""

                import os
                import subprocess
                import sys

                """))
            f.write(body)
        os.chmod(output_filename, (stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR
                                | stat.S_IXGRP | stat.S_IRGRP
                                | stat.S_IXOTH | stat.S_IROTH))
    except Exception as e:
        print(e, sys.stderr)
        return 1
    return 0


def format_subprocess(arguments):
    """Return a string containing Python code to call a subprocess."""
    t = 'subprocess.check_call([\n'
    for a in arguments:
        t += '  ' + a + ',\n'
    t += '  ])\n'
    return t


def format_nrfjprog(args, options):
    """Return a string containing Python code to call nrfjprog."""
    return format_subprocess(
        [repr(args.nrfjprog), '"--family"', repr(args.family)]
        + options)


def generate_flash_script(args):
    """Generate a script to flash an image."""
    image = args.image
    if args.script_relative:
        # Have the generated script refer to the named image file
        # in the same directory as the script.
        image = (
            "os.path.join(os.path.dirname(sys.argv[0]), \"{name}\")".format(
                name=image))
    elif args.wildcard:
        # The image name is a glob wildcard that must expand to exactly one
        # file name.
        files = glob.glob(image)
        if len(files) == 0:
            print('No match for image pattern: {}'.format(image))
            return 1
        if len(files) > 1:
            print('Ambiguous image pattern: {}'.format(image))
            return 1
        image = repr(files[0])
    else:
        image = repr(image)
    body = format_nrfjprog(args, ['"--program"', image, '"--sectorerase"'])
    body += format_nrfjprog(args, ['"--reset"'])
    return generate_script(args.output, body)


def generate_erase_script(args):
    """Generate a script to erase a device."""
    body = format_nrfjprog(args, ['"--eraseall"'])
    return generate_script(args.output, body)


def main(argv):
    parser = argparse.ArgumentParser(description='Generate a flashing script')
    parser.add_argument(
        '--output',
        metavar='FILENAME',
        required=True,
        help='flashing script name')
    parser.add_argument(
        '--family',
        metavar='FAMILY',
        default=DEFAULT_NRF_FAMILY,
        help='device family')
    parser.add_argument(
        '--nrfjprog',
        metavar='FILENAME',
        default=DEFAULT_NRFJPROG,
        help='nrfjprog command')
    subparsers = parser.add_subparsers(
        title='subcommands', required=True, dest='function',
        metavar='SUBCOMMAND', help='sub-command help')

    # Flashing script.
    parser_flash = subparsers.add_parser(
        'flash', help='Generate a script to flash an image')
    parser_flash_image = parser_flash.add_mutually_exclusive_group()
    parser_flash_image.add_argument(
        '--script-relative',
        action='store_true',
        help='Image is in the output script directory')
    parser_flash_image.add_argument(
        '--wildcard',
        action='store_true',
        help='Find the image by pattern at generation')
    parser_flash.add_argument(
        'image', metavar='IMAGE', help='image file to flash')
    parser_flash.set_defaults(func=generate_flash_script)

    # Device erase script.
    parser_erase = subparsers.add_parser(
        'erase', help='Generate a script to erase device')
    parser_erase.set_defaults(func=generate_erase_script)
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
