#!/usr/bin/env python3
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
# limitations under the License.
"""Flash an ESP32 device.

This is layered so that a caller can perform individual operations
through an `Flasher` instance, or operations according to a command line.
For `Flasher`, see the class documentation. For the parse_command()
interface or standalone execution:

usage: esp32_firmware_utils.py [-h] [--verbose] [--erase] [--application FILE]
                               [--verify_application] [--reset] [--skip_reset]
                               [--esptool FILE] [--parttool FILE]
                               [--sdkconfig FILE] [--chip CHIP] [--port PORT]
                               [--baud BAUD] [--before ACTION]
                               [--after ACTION] [--flash_mode MODE]
                               [--flash_freq FREQ] [--flash_size SIZE]
                               [--compress] [--bootloader FILE]
                               [--bootloader_offset OFFSET] [--partition FILE]
                               [--partition_offset OFFSET]
                               [--application_offset OFFSET]

Flash ESP32 device

optional arguments:
  -h, --help            show this help message and exit

configuration:
  --verbose, -v         Report more verbosely
  --esptool FILE        File name of the esptool executable
  --parttool FILE       File name of the parttool executable
  --sdkconfig FILE      File containing option defaults
  --chip CHIP           Target chip type
  --port PORT           Serial port device
  --baud BAUD           Serial port baud rate
  --before ACTION       What to do before connecting
  --after ACTION        What to do when finished
  --flash_mode MODE, --flash-mode MODE
                        Flash mode
  --flash_freq FREQ, --flash-freq FREQ
                        Flash frequency
  --flash_size SIZE, --flash-size SIZE
                        Flash size
  --compress, -z        Compress data in transfer
  --bootloader FILE     Bootloader image
  --bootloader_offset OFFSET, --bootloader-offset OFFSET
                        Bootloader offset
  --partition FILE      Partition table image
  --partition_offset OFFSET, --partition-offset OFFSET
                        Partition table offset
  --application_offset OFFSET, --application-offset OFFSET
                        Application offset

operations:
  --erase               Erase device
  --application FILE    Flash an image
  --verify_application, --verify-application
                        Verify the image after flashing
  --reset               Reset device after flashing
  --skip_reset, --skip-reset
                        Do not reset device after flashing
"""

import os
import sys

import firmware_utils

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.
ESP32_OPTIONS = {
    # Configuration options define properties used in flashing operations.
    'configuration': {
        # Tool configuration options.
        'esptool': {
            'help': 'File name of the esptool executable',
            'default': None,
            'argparse': {
                'metavar': 'FILE',
            },
            'command': [
                {'option': 'esptool'},
                {'optional': 'port'},
                {'optional': 'baud'},
                {'optional': 'before'},
                {'optional': 'after'},
                ()
            ],
            'verify': ['{esptool}', 'version'],
            'error':
                """\
                Unable to execute {esptool}.

                Please ensure that this tool is installed and
                that $IDF_PATH is set. See the ESP32 example
                README for installation instructions.
                """,
        },
        'parttool': {
            'help': 'File name of the parttool executable',
            'default': None,
            'argparse': {
                'metavar': 'FILE'
            },
            'command': [
                {'option': 'parttool'},
                {'optional': 'port'},
                {'optional': 'baud'},
                {
                    'option': 'partition',
                    'result': ['--partition-table-file', '{partition}'],
                    'expand': True
                },
                ()
            ],
            'verify': ['{parttool}', '--quiet'],
            'error':
                """\
                Unable to execute {parttool}.

                Please ensure that this tool is installed and
                that $IDF_PATH is set. See the ESP32 example
                README for installation instructions.
                """,
        },
        'sdkconfig': {
            'help': 'File containing option defaults',
            'default': None,
            'argparse': {
                'metavar': 'FILE'
            },
        },

        # Device configuration options.
        'chip': {
            'help': 'Target chip type',
            'default': 'esp32',
            'argparse': {
                'metavar': 'CHIP'
            },
            'sdkconfig': 'CONFIG_IDF_TARGET',
        },
        'port': {
            'help': 'Serial port device',
            'default': None,
            'argparse': {
                'metavar': 'PORT',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_PORT',
        },
        'baud': {
            'help': 'Serial port baud rate',
            'default': None,
            'argparse': {
                'metavar': 'BAUD',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_BAUD',
        },
        'before': {
            'help': 'What to do before connecting',
            'default': None,
            'argparse': {
                'metavar': 'ACTION',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_BEFORE',
        },
        'after': {
            'help': 'What to do when finished',
            'default': None,
            'argparse': {
                'metavar': 'ACTION',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_AFTER',
        },
        'flash_mode': {
            'help': 'Flash mode',
            'default': None,
            'argparse': {
                'metavar': 'MODE',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_FLASHMODE',
        },
        'flash_freq': {
            'help': 'Flash frequency',
            'default': None,
            'argparse': {
                'metavar': 'FREQ',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_FLASHFREQ',
        },
        'flash_size': {
            'help': 'Flash size',
            'default': None,
            'argparse': {
                'metavar': 'SIZE',
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_FLASHSIZE',
        },
        'compress': {
            'help': 'Compress data in transfer',
            'default': None,
            'alias': ['-z'],
            'argparse': {
                'action': 'store_true'
            },
            'sdkconfig': 'CONFIG_ESPTOOLPY_COMPRESSED',
        },

        # Flashing things.
        'bootloader': {
            'help': 'Bootloader image',
            'default': None,
            'argparse': {
                'metavar': 'FILE'
            },
        },
        'bootloader_offset': {
            'help': 'Bootloader offset',
            'default': None,
            'argparse': {
                'metavar': 'OFFSET'
            },
            'sdkconfig': 'CONFIG_BOOTLOADER_OFFSET_IN_FLASH',
        },
        'partition': {
            'help': 'Partition table image',
            'default': None,
            'argparse': {
                'metavar': 'FILE'
            },
        },
        'partition_offset': {
            'help': 'Partition table offset',
            'default': None,
            'argparse': {
                'metavar': 'OFFSET'
            },
            'sdkconfig': 'CONFIG_PARTITION_TABLE_OFFSET',
        },
        'application_offset': {
            'help': 'Application offset',
            'default': None,
            'argparse': {
                'metavar': 'OFFSET'
            },
        },
    },
}


def namespace_defaults(dst, src):
    for key, value in src.items():
        if key not in dst or getattr(dst, key) is None:
            setattr(dst, key, value)


class Flasher(firmware_utils.Flasher):
    """Manage esp32 flashing."""

    def __init__(self, **options):
        super().__init__(platform='ESP32', module=__name__, **options)
        self.define_options(ESP32_OPTIONS)

    def _postprocess_argv(self):
        if self.option.sdkconfig:
            namespace_defaults(self.option,
                               self.read_sdkconfig(self.option.sdkconfig))
#       idf = os.environ.get('IDF_PATH')
#       if idf:
#           if self.option.esptool is None:
#               self.option.esptool = os.path.join(
#                   idf,
#                   'components/esptool_py/esptool/esptool.py')
#           if self.option.parttool is None:
#               self.option.parttool = os.path.join(
#                   idf,
#                   'components/partition_table/parttool.py')

    def read_sdkconfig(self, filename):
        """Given an ESP32 sdkconfig file, read it for values of options
           not otherwise set.
        """
        config_map = {}
        for key, info in vars(self.info).items():
            config_key = info.get('sdkconfig')
            if config_key:
                config_map[config_key] = key
        result = {}
        with open(filename) as f:
            for line in f:
                k, eq, v = line.strip().partition('=')
                if eq == '=' and k in config_map:
                    result[config_map[k]] = v.strip('"')
        return result

    IDF_PATH_TOOLS = {
        'esptool': 'components/esptool_py/esptool/esptool.py',
        'parttool': 'components/partition_table/parttool.py',
    }

    def locate_tool(self, tool):
        if tool in self.IDF_PATH_TOOLS:
            idf_path = os.environ.get('IDF_PATH')
            if idf_path:
                return os.path.join(idf_path, self.IDF_PATH_TOOLS[tool])
        return super().locate_tool(tool)

    # Common command line arguments for esptool flashing subcommands.
    FLASH_ARGUMENTS = [
        {'optional': 'flash_mode'},
        {'optional': 'flash_freq'},
        {'optional': 'flash_size'},
        {
            'match': '{compress}',
            'test': [(True, '-z'), ('y', '-z')],
            'default': '-u'
        },
    ]

    def erase(self):
        """Perform `commander device masserase`."""
        return self.run_tool('esptool', ['erase_flash'], {}, 'Erase device')

    def verify(self, image, address=0):
        """Verify image(s)."""
        if not isinstance(image, list):
            image = [address, image]
        return self.run_tool(
            'esptool',
            ['verify_flash', self.FLASH_ARGUMENTS, image],
            name='Verify',
            pass_message='Verified',
            fail_message='Not verified',
            fail_level=2)

    def flash(self, image, address=0):
        """Flash image(s)."""
        if not isinstance(image, list):
            image = [address, image]
        return self.run_tool(
            'esptool',
            ['write_flash', self.FLASH_ARGUMENTS, image],
            name='Flash')

    PARTITION_INFO = {
        'phy': ['--partition-type', 'data', '--partition-subtype', 'phy'],
        'application': ['--partition-boot-default'],
        'ota': ['--partition-type', 'data', '--partition-subtype', 'ota'],
        'factory': [
            '--partition-type', 'app', '--partition-subtype', 'factory'
        ],
    }

    def get_partition_info(self, item, info, options=None):
        """Run parttool to get partition information."""
        return self.run_tool(
            'parttool',
            ['get_partition_info', self.PARTITION_INFO[item], '--info', info],
            options or {},
            capture_output=True).stdout.strip()

    def make_wrapper(self, argv):
        self.parser.add_argument(
            '--use-parttool',
            metavar='FILENAME',
            help='partition tool to configure flashing script')
        self.parser.add_argument(
            '--use-partition-file',
            metavar='FILENAME',
            help='partition file to configure flashing script')
        self.parser.add_argument(
            '--use-sdkconfig',
            metavar='FILENAME',
            help='sdkconfig to configure flashing script')
        super().make_wrapper(argv)

    def _platform_wrapper_args(self, args):
        if args.use_sdkconfig:
            # Include values from sdkconfig so that it isn't needed at
            # flashing time.
            namespace_defaults(args, self.read_sdkconfig(args.use_sdkconfig))
        parttool = args.use_parttool or args.parttool
        partfile = args.use_partition_file or args.partition
        if parttool and partfile:
            # Get unspecified offsets from the partition file now,
            # so that parttool isn't needed at flashing time.
            if args.application and args.application_offset is None:
                args.application_offset = self.get_partition_info(
                    'application', 'offset',
                    {'parttool': parttool, 'partition': partfile})

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        if self.option.erase:
            if self.erase().err:
                return self

        bootloader = self.optional_file(self.option.bootloader)
        application = self.optional_file(self.option.application)
        partition = self.optional_file(self.option.partition)

        if bootloader or application or partition:
            # Collect the flashable items.
            flash = []
            if bootloader:
                flash += [self.option.bootloader_offset, bootloader]
            if application:
                offset = self.option.application_offset
                if offset is None:
                    offset = self.get_partition_info('application', 'offset')
                flash += [offset, application]
            if partition:
                flash += [self.option.partition_offset, partition]

            # esptool.py doesn't have an independent reset command, so we add
            # an `--after` option to the final operation, which may be either
            # flash or verify.
            if self.option.after is None:
                if self.option.reset or self.option.reset is None:
                    self.option.after = 'hard_reset'
                else:
                    self.option.after = 'no_reset'
            if self.option.verify_application:
                verify_after = self.option.after
                self.option.after = 'no_reset'

            if self.flash(flash).err:
                return self
            if self.option.verify_application:
                self.option.after = verify_after
                if self.verify(flash).err:
                    return self

        return self

### Mobly integration
class ESP32Platform:
  def __init__(self, flasher_args):
      self.flasher = Flasher(**flasher_args)

  def flash(self):
      self.flasher.flash_command([os.getcwd()])

def verify_platform_args(platform_args):
    required_args = [
        'application',
        'parttool',
        'port',
        'baud',
        'before',
        'after',
        'flash_mode',
        'flash_freq',
        'flash_size',
        'compress',
        'bootloader',
        'partition',
        'partition_offset',
        'application_offset',
    ]
    difference = set(required_args) - set(platform_args)
    if difference:
        raise ValueError("Required arguments missing: %s" % difference)

def create_platform(platform_args):
    verify_platform_args(platform_args[0])
    return ESP32Platform(platform_args[0])

### End of Mobly integration

if __name__ == '__main__':
    sys.exit(Flasher().flash_command(sys.argv))
