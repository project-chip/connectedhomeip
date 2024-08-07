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

import binascii
import logging
import os
import pathlib
import platform
import re
import shutil
import subprocess
import sys

import coloredlogs
import firmware_utils
import toml
from Crypto.Cipher import AES

coloredlogs.install(level='DEBUG')

# Additional options that can be use to configure an `Flasher`
# object (as dictionary keys) and/or passed as command line options.


def any_base_int(s): return int(s, 0)


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
            'help': 'Partition table for board for bl iot sdk',
            'default': None,
            'argparse': {
                'metavar': 'PARTITION_TABLE_FILE',
                'type': pathlib.Path
            }
        },
        'dts': {
            'help': 'Device tree file for bl iot sdk',
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
        'sk': {
            'help': 'private key to sign firmware to flash or sign ota image.',
            'default': None,
            'argparse': {
                'metavar': 'path',
                'type': pathlib.Path
            }
        },
        'mfd': {
            'help': 'matter factory data',
            'default': None,
            'argparse': {
                'metavar': 'path',
                'type': pathlib.Path
            }
        },
        'mfd-str': {
            'help': 'matter factory data string, only available for using bl iot sdk',
            'default': None,
            'argparse': {
                'metavar': 'mfd_str',
            }
        },
        'key': {
            'help': 'data key in security engine for matter factory data decryption',
            'default': None,
            'argparse': {
                'metavar': 'key',
            }
        },
        'boot2': {
            'help': 'boot2 image',
            'default': None,
            'argparse': {
                'metavar': 'path',
            }
        },
        'config': {
            'help': 'firmware programming configuration for bouffalo sdk',
            'default': None,
            'argparse': {
                'metavar': 'path',
            }
        },
        'build-ota': {
            'help': 'build ota image',
            'default': None,
            'argparse': {
                'action': 'store_true'
            },
        },
        'vendor-id': {
            'help': 'vendor id passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'vendor_id',
                "type": any_base_int
            }
        },
        'product-id': {
            'help': 'product id passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'product_id',
                "type": any_base_int
            }
        },
        'version': {
            'help': 'software version (numeric) passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'version',
                "type": any_base_int
            }
        },
        'version-str': {
            'help': 'software version string passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'version_str',
            }
        },
        'digest-algorithm': {
            'help': 'digest algorithm passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'digest_algorithm',
            }
        },
        "min-version": {
            'help': 'minimum software version passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'min_version',
                "type": any_base_int
            }
        },
        "max-version": {
            'help': 'maximum software version passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'max_version',
                "type": any_base_int
            }
        },
        "release-notes": {
            'help': 'release note passes to ota_image_tool.py ota image if --build-ota present',
            'default': None,
            'argparse': {
                'metavar': 'release_notes',
            }
        }
    },
}

MATTER_ROOT = os.getcwd()


class DictObject:
    def __init__(self, data_dict):
        for key, value in data_dict.items():
            if isinstance(value, dict):
                self.__dict__[key] = DictObject(value)
            else:
                self.__dict__[key] = value

    def __getattr__(self, name):
        return self.__dict__.get(name, None)

    def __setattr__(self, name, value):
        self.__dict__[name] = value

    def __repr__(self):
        return str(self.__dict__)


class Flasher(firmware_utils.Flasher):

    args = {}
    work_dir = None

    def __init__(self, **options):
        super().__init__(platform=None, module=__name__, **options)
        self.define_options(BOUFFALO_OPTIONS)

    def find_file(self, path_dir, name_partten):

        ret_files = []

        for root, dirs, files in os.walk(path_dir, topdown=False):
            for name in files:
                if re.match(name_partten, name):
                    ret_files.append(os.path.join(root, name))

        return ret_files

    def parse_mfd(self):

        def decrypt_data(data_bytearray, key_bytearray, iv_bytearray):
            data_bytearray += bytes([0] * (16 - (len(data_bytearray) % 16)))
            cryptor = AES.new(key_bytearray, AES.MODE_CBC, iv_bytearray)
            plaintext = cryptor.decrypt(data_bytearray)
            return plaintext

        self.args["iv"] = None

        if not self.args["mfd"] and not self.args["mfd_str"]:
            return None

        if self.args['mfd_str']:
            try:
                iv = self.args['mfd_str'].split(":")[1].split(',')[0]
            except Exception:
                raise Exception("Invalid mfd-str format.")

            self.args["iv"] = iv
            return self.args['mfd_str']

        with open(self.args["mfd"], 'rb') as f:
            bytes_obj = f.read()

        sec_len = int.from_bytes(bytes_obj[0:4], byteorder='little')
        if sec_len:
            crc_val_calc = binascii.crc32(bytes_obj[4: 4 + sec_len])
            crc_val = int.from_bytes(bytes_obj[4 + sec_len: 4 + sec_len + 4], byteorder='little')
            if crc_val_calc != crc_val:
                raise Exception("MFD partition file has invalid format in secured data.")
            bytes_sec = bytes_obj[4: 4 + sec_len]

            if not self.args["key"]:
                raise Exception("MFD has secured data, but no key input.")
        else:
            bytes_sec = []

        raw_start = 4 + sec_len + 4
        raw_len = int.from_bytes(bytes_obj[raw_start: raw_start + 4], byteorder='little')
        if raw_len:
            crc_val_calc = binascii.crc32(bytes_obj[raw_start + 4: raw_start + 4 + raw_len])
            crc_val = int.from_bytes(bytes_obj[raw_start + 4 + raw_len: raw_start + 4 + raw_len + 4], byteorder='little')
            if crc_val_calc != crc_val:
                raise Exception("MFD partition file has invalid format in raw data.")
            bytes_raw = bytes_obj[raw_start + 4: raw_start + 4 + raw_len]
        else:
            bytes_raw = []

        offset = 0
        dict_raw = {}
        while offset < raw_len:
            type_id = int.from_bytes(bytes_raw[offset: offset + 2], byteorder='little')
            type_len = int.from_bytes(bytes_raw[offset + 2: offset + 4], byteorder='little')

            if 0x8001 == type_id:
                self.args["iv"] = bytes_raw[offset + 4: offset + 4 + type_len]

            if offset + 4 + type_len <= raw_len:
                dict_raw[type_id] = bytes(bytes_raw[offset + 4: offset + 4 + type_len]).hex()

            offset += (4 + type_len)

        if sec_len > 0 and self.args["key"] and not self.args["iv"]:
            raise Exception("No IV found in secured mfd data.")

        offset = 0
        dict_sec = {}
        if bytes_sec and self.args["iv"] and self.args["key"]:

            bytes_sec = decrypt_data(bytes_sec, bytearray.fromhex(self.args["key"]), self.args["iv"])
            while offset < raw_len:
                type_id = int.from_bytes(bytes_sec[offset: offset + 2], byteorder='little')
                type_len = int.from_bytes(bytes_sec[offset + 2:offset + 4], byteorder='little')

                if offset + 4 + type_len <= sec_len:
                    dict_sec[type_id] = bytes(bytes_sec[offset + 4: offset + 4 + type_len]).hex()

                offset += (4 + type_len)

        self.args["iv"] = self.args["iv"].hex()

        mfd_str = ""
        if dict_sec.keys():
            for idx in range(1, 1 + max(dict_sec.keys())):
                if idx in dict_sec.keys():
                    mfd_str += dict_sec[idx] + ","
                else:
                    mfd_str += ","

        mfd_str = mfd_str + ":"
        for idx in range(0x8001, 1 + max(dict_raw.keys())):
            if idx in dict_raw.keys():
                mfd_str += dict_raw[idx] + ","
            else:
                mfd_str += ","

        self.args["mfd_str"] = mfd_str

        return self.args['mfd_str']

    def iot_sdk_prog(self):

        def get_tools():
            flashtool_path = os.environ.get('BOUFFALOLAB_SDK_ROOT') + "/flashtool/BouffaloLabDevCube-v1.9.0"
            bflb_tools_dict = {
                "linux": {"flash_tool": "bflb_iot_tool-ubuntu"},
                "win32": {"flash_tool": "bflb_iot_tool.exe"},
                "darwin": {"flash_tool": "bflb_iot_tool-macos"},
            }

            try:
                flashtool_exe = flashtool_path + "/" + bflb_tools_dict[sys.platform]["flash_tool"]
            except Exception:
                raise Exception("Do NOT support {} operating system to program firmware.".format(sys.platform))

            if not os.path.exists(flashtool_exe):
                logging.fatal('*' * 80)
                logging.error('Flashtool is not installed, or environment variable BOUFFALOLAB_SDK_ROOT is not exported.')
                logging.fatal('\tPlease make sure Bouffalo Lab SDK installs as below:')
                logging.fatal('\t\t./third_party/bouffalolab/env-setup.sh')

                logging.fatal('\tPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
                logging.fatal('\t\texport BOUFFALOLAB_SDK_ROOT="your install path"')
                logging.fatal('*' * 80)
                raise Exception("Flash tool is not installed.")

            return flashtool_path, flashtool_exe

        def get_dts_file(flashtool_path, dts):

            if dts:
                return dts
            else:
                return os.path.join(flashtool_path, "chips", self.args["chipname"],
                                    "device_tree", "bl_factory_params_IoTKitA_{}.dts".format(self.args["xtal"]))

        def get_boot_image(flashtool_path, boot2_image):

            if boot2_image:
                return boot2_image

            if self.args["chipname"] in ["bl702l"]:
                return None

            builtin_imgs_path = os.path.join(flashtool_path, "chips", self.args["chipname"], "builtin_imgs")
            boot2_images = self.find_file(builtin_imgs_path, r'boot2_isp_release.bin')
            if len(boot2_images) > 1:
                raise Exception("More than one boot2 image found.")
            if len(boot2_images) == 0:
                raise Exception("No boot2 image found.")

            return boot2_images[0]

        def get_mfd_addr():
            with open(self.args["pt"], 'r') as file:
                partition_config = toml.load(file)

            mfd_addr = None
            for v in partition_config["pt_entry"]:
                if v["name"] == "MFD" or v["name"] == "mfd":
                    mfd_addr = v["address0"]

            return mfd_addr

        def exe_gen_ota_image_cmd(flashtool_exe):

            os.system("rm -rf {}/FW_OTA.bin*".format(self.work_dir))
            os.system("rm -rf {}/ota_images".format(self.work_dir))

            if not self.args["build_ota"]:
                return

            gen_ota_img_cmd = [
                flashtool_exe,
                "--build",
                "--chipname", self.args["chipname"],
                "--dts", self.args["dts"],
                "--firmware", self.args["firmware"],
                "--pt", self.args["pt"],
                "--ota", self.work_dir
            ]

            if self.args["sk"]:
                gen_ota_img_cmd += ["--sk", self.args["sk"]]

            logging.info("ota image generating: {}".format(" ".join(gen_ota_img_cmd)))
            process = subprocess.Popen(gen_ota_img_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            while process.poll() is None:
                line = process.stdout.readline().decode('utf-8').rstrip()
                if line:
                    logging.info(line)

            fw_ota_images = self.find_file(self.work_dir, r'^FW_OTA.+\.hash$')
            if not fw_ota_images or 0 == len(fw_ota_images):
                raise Exception("Failed to generate Bouffalo Lab OTA image.")

            os.system("mkdir -p {}/ota_images".format(self.work_dir))
            fw_name = os.path.basename(self.args["firmware"])[:-len(".bin")]
            for img in fw_ota_images:
                ota_img_name = os.path.basename(img)
                new_name = os.path.join(self.work_dir, "ota_images", fw_name + ota_img_name[len("FW_OTA"):])
                os.system("mv {} {}".format(img, new_name))

        def exe_prog_cmd(flashtool_exe, mfd_addr):

            if not self.args["port"]:
                return

            if self.args["mfd"] and not mfd_addr:
                raise Exception("No MFD partition found in partition table.")

            prog_cmd = [
                flashtool_exe,
                "--port", self.args["port"],
                "--baudrate", self.args["baudrate"],
                "--chipname", self.args["chipname"],
                "--firmware", self.args["firmware"],
                "--dts", self.args["dts"],
                "--pt", self.args["pt"],
            ]

            if self.args["boot2"]:
                prog_cmd += ["--boot2", self.args["boot2"]]

            if self.args["sk"]:
                prog_cmd += ["--sk", self.args["sk"]]

            if mfd_addr and self.args["mfd_str"]:
                if self.args["key"] and not self.args["iv"]:
                    logging.warning("mfd file has no iv, do NOT program mfd key.")
                else:
                    prog_cmd += ["--dac_key", self.args["key"]]
                    prog_cmd += ["--dac_iv", self.args["iv"]]
                    prog_cmd += ["--dac_addr", hex(mfd_addr)]
                    prog_cmd += ["--dac_value", self.args["mfd_str"]]

            if self.option.erase:
                prog_cmd += ["--erase"]

            logging.info("firmware programming: {}".format(" ".join(prog_cmd)))
            process = subprocess.Popen(prog_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            while process.poll() is None:
                line = process.stdout.readline().decode('utf-8').rstrip()
                if line:
                    logging.info(line)

        flashtool_path, flashtool_exe = get_tools()
        self.args["pt"] = os.path.join(os.getcwd(), str(self.args["pt"]))
        mfd_addr = get_mfd_addr()

        self.parse_mfd()
        self.args["dts"] = get_dts_file(flashtool_path, self.args["dts"])
        self.args["boot2"] = get_boot_image(flashtool_path, self.args["boot2"])

        os.chdir(self.work_dir)

        exe_gen_ota_image_cmd(flashtool_exe)
        exe_prog_cmd(flashtool_exe, mfd_addr)

    def gen_ota_image(self):
        sys.path.insert(0, os.path.join(MATTER_ROOT, 'src', 'app'))
        import ota_image_tool

        bflb_ota_images = self.find_file(os.path.join(self.work_dir, "ota_images"), r".+\.(ota|hash)$")
        if len(bflb_ota_images) == 0:
            raise Exception("No bouffalo lab OTA image found.")

        ota_image_cfg_list = [
            "vendor_id",
            "product_id",
            "version",
            "version_str",
            "digest_algorithm",
            "min_version",
            "max_version",
            "release_notes",
        ]
        ota_image_cfg = {}
        for k in ota_image_cfg_list:
            if self.args[k] is not None:
                ota_image_cfg[k] = self.args[k]
        ota_image_cfg = DictObject(ota_image_cfg)

        for img in bflb_ota_images:
            ota_image_cfg.input_files = [img]
            ota_image_cfg.output_file = img + ".matter"
            ota_image_tool.validate_header_attributes(ota_image_cfg)
            ota_image_tool.generate_image(ota_image_cfg)

            if re.match(r".+\.(xz.ota|xz.hash)$", img):
                self.log(0, 'Matter OTA compressed image generated: {}'.format(ota_image_cfg.output_file))
            else:
                self.log(0, 'Matter OTA image generated: {}'.format(ota_image_cfg.output_file))

    def verify(self):
        """Not supported"""
        self.log(0, "Verification is done after image flashed.")

    def reset(self):
        """Not supported"""
        self.log(0, "Reset is triggered automatically after image flashed.")

    def actions(self):
        """Perform actions on the device according to self.option."""
        self.log(3, 'Options:', self.option)

        if platform.machine() not in ["x86_64"]:
            raise Exception("Only support x86_64 CPU machine to program firmware.")

        if self.option.reset:
            self.reset()
        if self.option.verify_application:
            self.verify()

        self.args = dict(vars(self.option))
        self.args["application"] = os.path.join(os.getcwd(), str(self.args["application"]))
        self.work_dir = os.path.dirname(self.args["application"])
        if self.args["sk"]:
            self.args["sk"] = str(self.args["sk"])

        if self.args["mfd"]:
            self.args["mfd"] = os.path.join(MATTER_ROOT, str(self.args["mfd"]))

        self.args["application"] = os.path.join(os.getcwd(), str(self.args["application"]))
        self.args["firmware"] = str(pathlib.Path(self.args["application"]).with_suffix(".bin"))
        if self.args["application"] != self.args["firmware"]:
            shutil.copy2(self.args["application"], self.args["firmware"])

        if self.args["build_ota"]:
            if self.args["port"]:
                raise Exception("Do not generate OTA image with firmware programming.")

        if self.args["mfd"] and self.args["mfd_str"]:
            raise Exception("Cannot use option mfd and mfd-str together.")
        self.iot_sdk_prog()

        if self.args["build_ota"]:
            self.gen_ota_image()

        return self


if __name__ == '__main__':

    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])

    sys.exit(Flasher().flash_command(sys.argv))
