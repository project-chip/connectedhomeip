#!/usr/bin/env python3
# Copyright (c) 2021 Project CHIP Authors
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
# limitations under the License.

import re
import sys
import os
import pathlib
import firmware_utils

from bflb_iot_tool.__main__ import run_main


# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
BOUFFALO_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        'chipname': {
            'help': "Bouffalolab chip name",
            'default': None,
            'argparse': {
                'metavar': 'CHIP_NAME',
            }
        },
        'pt': {
            'help': 'Partition table for board',
            'default': None,
            'argparse': {
                'metavar': 'PARTITION_TABLE_FILE',
                'type': pathlib.Path
            }
        },
        'dts': {
            'help': 'Device tree file',
            'default': None,
            'argparse': {
                'metavar': 'DEVICE_TREE_FILE',
                'type': pathlib.Path
            }
        },
        'xtal': {
            'help': 'XTAL for board',
            'default': None,
            'argparse': {
                'metavar': 'XTAL',
            }
        },
        'port': {
            'help': 'UART port to flash device',
            'default': None,
            'argparse': {
                'metavar': 'PORT',
            }
        },
        'baudrate': {
            'help': 'UART baudrate to flash device',
            'default': None,
            'argparse': {
                'metavar': 'BAUDRATE',
            },
        },
        'build': {
            'help': 'Build image',
            'default': None,
            'argparse': {
                'action': 'store_true'
            }
        },
        'ota': {
            'help': 'output directory of ota image build',
            'default': None,
            'argparse': {
                'metavar': 'DIR',
                'type': pathlib.Path
            }
        },
    },
}


class Flasher(firmware_utils.Flasher):

    isErase = False

    def __init__(self, **options):
        super().__init__(platform=None, module=__name__, **options)
        self.define_options(BOUFFALO_OPTIONS)

    def verify(self):
        """Not supported"""
        self.log(0, "Verification is done after image flashed.")

    def reset(self):
        """Not supported"""
        self.log(0, "Reset is triggered automatically after image flashed.")

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        options_keys = BOUFFALO_OPTIONS["configuration"].keys()
        arguments = [__file__]
        work_dir = None

        print("self.option", self.option)

        if self.option.reset:
            self.reset()
        if self.option.verify_application:
            self.verify()

        command_args = {}
        for (key, value) in dict(vars(self.option)).items():

            if self.option.build and value:
                if "port" in command_args.keys():
                    continue
            else:
                if "ota" in command_args.keys():
                    continue

            if key == "application":
                key = "firmware"
                work_dir = os.path.dirname(os.path.join(os.getcwd(), str(value)))
            elif key in options_keys:
                pass
            else:
                continue

            if value:
                if value == True:
                    arg = ("--{}".format(key)).strip()
                elif isinstance(value, pathlib.Path):
                    arg = ("--{}={}".format(key, os.path.join(os.getcwd(), str(value)))).strip()
                else:
                    arg = ("--{}={}".format(key, value)).strip()

            arguments.append(arg)

        if self.option.erase:
            arguments.append("--erase")

        os.chdir(work_dir)
        arguments[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', arguments[0])
        sys.argv = arguments

        run_main()

        return self


if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(Flasher().flash_command(sys.argv))
