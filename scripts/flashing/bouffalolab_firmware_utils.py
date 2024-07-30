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

import importlib.metadata
import logging
import os
import pathlib
import re
import shutil
import subprocess
import sys

import coloredlogs
import firmware_utils

coloredlogs.install(level='DEBUG')

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
            'help': 'XTAL configuration for bl iot sdk',
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
            'help': 'Build OTA image',
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
        'pk': {
            'help': 'public key to sign firmware to flash or sign ota image.',
            'default': None,
            'argparse': {
                'metavar': 'path',
                'type': pathlib.Path
            }
        },
        'sk': {
            'help': 'private key to sign firmware to flash or sign ota image.',
            'default': None,
            'argparse': {
                'metavar': 'path',
                'type': pathlib.Path
            }
        },
        'boot2': {
            'help': 'boot2 image.',
            'default': None,
            'argparse': {
                'metavar': 'path',
            }
        }
    },
}


class Flasher(firmware_utils.Flasher):

    isErase = False

    def __init__(self, **options):
        super().__init__(platform=None, module=__name__, **options)
        self.define_options(BOUFFALO_OPTIONS)

    def get_boot_image(self, config_path, boot2_image):

        boot_image_guess = None

        for root, dirs, files in os.walk(config_path, topdown=False):
            for name in files:
                if "boot2" not in name.lower():
                    continue
                if boot2_image:
                    return os.path.join(root, boot2_image)
                else:
                    if name == "boot2_isp_release.bin":
                        return os.path.join(root, name)
                    elif not boot_image_guess and name.find("release") >= 0:
                        boot_image_guess = os.path.join(root, name)

        return boot_image_guess

    def get_dts_file(self, config_path, xtal_value, chip_name):

        for root, dirs, files in os.walk(config_path, topdown=False):
            for name in files:
                if chip_name == 'bl702':
                    if name.find("bl_factory_params_IoTKitA_32M.dts") >= 0:
                        return os.path.join(config_path, name)
                else:
                    if name.find(xtal_value) >= 0:
                        return os.path.join(config_path, name)
        return None

    def verify(self):
        """Not supported"""
        self.log(0, "Verification is done after image flashed.")

    def reset(self):
        """Not supported"""
        self.log(0, "Reset is triggered automatically after image flashed.")

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        tool_path = os.environ.get('BOUFFALOLAB_SDK_ROOT') + "/flashtool/BouffaloLabDevCube-v1.9.0"
        bflb_tools_dict = {
            "linux": {"flash_tool": "bflb_iot_tool-ubuntu"},
            "win32": {"flash_tool": "bflb_iot_tool.exe"},
            "darwin": {"flash_tool": "bflb_iot_tool-macos"},
        }

        try:
            flashtool_exe = tool_path + "/" + bflb_tools_dict[sys.platform]["flash_tool"]
        except Exception as e:
            raise Exception("Do NOT support {} operating system to program firmware.".format(sys.platform))

        if not os.path.exists(flashtool_exe):
            logging.fatal('*' * 80)
            logging.error('Flashtool is not installed, or environment variable BOUFFALOLAB_SDK_ROOT is not exported.')
            logging.fatal('\tPlease make sure Bouffalo Lab SDK installs as below:')
            logging.fatal('\t\t./integrations/docker/images/stage-2/chip-build-bouffalolab/setup.sh')

            logging.fatal('\tPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
            logging.fatal('\t\texport BOUFFALOLAB_SDK_ROOT="your install path"')
            logging.fatal('*' * 80)
            raise Exception(e)

        options_keys = BOUFFALO_OPTIONS["configuration"].keys()
        arguments = [__file__]
        work_dir = None

        if self.option.reset:
            self.reset()
        if self.option.verify_application:
            self.verify()

        chip_name = None
        chip_config_path = None
        boot2_image = None
        dts_path = None
        xtal_value = None

        is_for_ota_image_building = None
        is_for_programming = False
        has_private_key = False
        has_public_key = False
        ota_output_folder = None

        boot2_image = None

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
            elif key == "boot2":
                boot2_image = value
                continue
            elif key in options_keys:
                pass
            else:
                continue

            if value:
                if value is True:
                    arg = ("--{}".format(key)).strip()
                elif isinstance(value, pathlib.Path):
                    arg = ("--{}={}".format(key, os.path.join(os.getcwd(), str(value)))).strip()
                else:
                    arg = ("--{}={}".format(key, value)).strip()

                arguments = arguments + arg.split('=')

            if key == "chipname":
                chip_name = value
            elif key == "xtal":
                xtal_value = value
            elif key == "dts":
                dts_path = value
            elif "port" == key:
                if value:
                    is_for_programming = True
            elif "build" == key:
                if value:
                    is_for_ota_image_building = True
            elif "pk" == key:
                if value:
                    has_public_key = True
            elif "sk" == key:
                if value:
                    has_private_key = True
            elif "ota" == key and value:
                ota_output_folder = os.path.join(os.getcwd(), value)

        if is_for_ota_image_building and is_for_programming:
            logging.error("ota imge build can't work with image programming")
            raise Exception("Wrong operation.")

        if not ((has_private_key and has_public_key) or (not has_public_key and not has_private_key)):
            logging.error("Key pair expects a pair of public key and private.")
            raise Exception("Wrong key pair.")

        if is_for_ota_image_building == "ota_sign" and (not has_private_key or not has_public_key):
            logging.error("Expecting key pair to sign OTA image.")
            raise Exception("Wrong key pair.")

        if not dts_path and xtal_value:
            chip_config_path = os.path.join(tool_path, "chips", chip_name, "device_tree")
            dts_path = self.get_dts_file(chip_config_path, xtal_value, chip_name)
            arguments.append("--dts")
            arguments.append(dts_path)

        if boot2_image:
            chip_config_path = os.path.join(tool_path, "chips", chip_name, "builtin_imgs")
            boot2_image = self.get_boot_image(chip_config_path, boot2_image)
            arguments.append("--boot2")
            arguments.append(boot2_image)
        else:
            if self.option.erase:
                arguments.append("--erase")

            if chip_name in {"bl602", "bl702"}:
                chip_config_path = os.path.join(tool_path, "chips", chip_name, "builtin_imgs")
                boot2_image = self.get_boot_image(chip_config_path, boot2_image)
                arguments.append("--boot2")
                arguments.append(boot2_image)

        os.chdir(work_dir)
        arguments[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', flashtool_exe)
        sys.argv = arguments

        if ota_output_folder:
            if os.path.exists(ota_output_folder):
                shutil.rmtree(ota_output_folder)
            os.mkdir(ota_output_folder)

        logging.info("Arguments {}".format(" ".join(arguments)))
        process = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        while process.poll() is None:
            line = process.stdout.readline().decode('utf-8').rstrip()
            if line:
                logging.info(line)

        if ota_output_folder:
            ota_images = os.listdir(ota_output_folder)
            for img in ota_images:
                if img not in ['FW_OTA.bin.xz.hash']:
                    os.remove(os.path.join(ota_output_folder, img))

        return self


if __name__ == '__main__':

    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])

    sys.exit(Flasher().flash_command(sys.argv))
