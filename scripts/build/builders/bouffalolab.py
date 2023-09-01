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

import logging
import os
from enum import Enum, auto

from .gn import GnBuilder


class BouffalolabApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == BouffalolabApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL602_IoT_Matter_V1 = auto()
    BL602_NIGHT_LIGHT = auto()
    XT_ZB6_DevKit = auto()
    BL706_NIGHT_LIGHT = auto()
    BL706_ETH = auto()
    BL706_WIFI = auto()
    BL704L_DVK = auto()

    def GnArgName(self):
        if self == BouffalolabBoard.BL602_IoT_Matter_V1:
            return 'BL602-IoT-Matter-V1'
        elif self == BouffalolabBoard.BL602_NIGHT_LIGHT:
            return 'BL602-NIGHT-LIGHT'
        elif self == BouffalolabBoard.XT_ZB6_DevKit:
            return 'XT-ZB6-DevKit'
        elif self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        elif self == BouffalolabBoard.BL706_ETH:
            return 'BL706-ETH'
        elif self == BouffalolabBoard.BL706_WIFI:
            return 'BL706-WIFI'
        elif self == BouffalolabBoard.BL704L_DVK:
            return 'BL704L-DVK'
        else:
            raise Exception('Unknown board #: %r' % self)


class BouffalolabBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: BouffalolabApp = BouffalolabApp.LIGHT,
                 board: BouffalolabBoard = BouffalolabBoard.XT_ZB6_DevKit,
                 enable_rpcs: bool = False,
                 module_type: str = "BL706C-22",
                 baudrate=2000000,
                 enable_shell: bool = False,
                 enable_cdc: bool = False,
                 enable_resetCnt: bool = False,
                 enable_rotating_device_id: bool = False
                 ):

        if 'BL602' == module_type:
            bouffalo_chip = 'bl602'
        elif 'BL704L' == module_type:
            bouffalo_chip = 'bl702l'
        elif "BL70" in module_type:
            bouffalo_chip = 'bl702'
        else:
            raise Exception("module_type %s is not supported" % module_type)

        super(BouffalolabBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', bouffalo_chip),
            runner=runner
        )

        self.argsOpt = []
        self.chip_name = bouffalo_chip

        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/bouffalolab/toolchain')
        toolchain = 'custom_toolchain="{}:riscv_gcc"'.format(toolchain)
        if toolchain:
            self.argsOpt.append(toolchain)

        self.app = app
        self.board = board

        self.argsOpt.append('board=\"{}\"'.format(self.board.GnArgName()))
        self.argsOpt.append('baudrate=\"{}\"'.format(baudrate))

        if bouffalo_chip == "bl602":
            self.argsOpt.append('chip_enable_openthread=false')
            self.argsOpt.append('chip_enable_wifi=true')
        if bouffalo_chip == "bl702":
            self.argsOpt.append('module_type=\"{}\"'.format(module_type))
            if board == BouffalolabBoard.BL706_ETH:
                self.argsOpt.append('chip_config_network_layer_ble=false')
                self.argsOpt.append('chip_enable_openthread=false')
                self.argsOpt.append('chip_enable_wifi=false')
            elif board == BouffalolabBoard.BL706_WIFI:
                self.argsOpt.append('chip_enable_openthread=false')
                self.argsOpt.append('chip_enable_wifi=true')
            else:
                self.argsOpt.append('chip_enable_openthread=true')
                self.argsOpt.append('chip_enable_wifi=false')
        elif bouffalo_chip == "bl702l":
            self.argsOpt.append('chip_enable_openthread=true')
            self.argsOpt.append('chip_enable_wifi=false')

        if enable_cdc:
            if bouffalo_chip != "bl702":
                raise Exception('Chip %s does NOT support USB CDC' % bouffalo_chip)
            if board == BouffalolabBoard.BL706_ETH:
                raise Exception('Board %s does NOT support USB CDC' % self.board.GnArgName())

            self.argsOpt.append('enable_cdc_module=true')

        if enable_rpcs:
            self.argsOpt.append('import("//with_pw_rpc.gni")')
        elif enable_shell:
            self.argsOpt.append('chip_build_libshell=true')

        if enable_resetCnt:
            self.argsOpt.append('enable_reset_counter=true')

        if enable_rotating_device_id:
            self.argsOpt.append('chip_enable_additional_data_advertising=true')
            self.argsOpt.append('chip_enable_rotating_device_id=true')

        try:
            self.argsOpt.append('bouffalolab_sdk_root="%s"' % os.environ['BOUFFALOLAB_SDK_ROOT'])
        except KeyError as err:
            logging.fatal('Please make sure Bouffalo Lab SDK installs as below:')
            logging.fatal('\tcd third_party/bouffalolab/repo')
            logging.fatal('\tsudo bash scripts/setup.sh')

            logging.fatal('Please make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
            logging.fatal('\texport BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk')

            raise err

    def GnBuildArgs(self):
        return self.argsOpt

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix(self.chip_name)),
            '%s.out.map' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix(self.chip_name)),
        }

        return items

    def PostBuildCommand(self):

        # Generate Bouffalo Lab format OTA image for development purpose.

        ota_images_folder_path = self.output_dir + "/ota_images"
        ota_images_dev_image = self.output_dir + "/" + self.app.AppNamePrefix(self.chip_name) + ".bin.xz.hash"
        ota_images_image = self.output_dir + "/ota_images/FW_OTA.bin.xz.hash"
        ota_images_firmware = self.output_dir + "/" + self.app.AppNamePrefix(self.chip_name) + ".bin"

        ota_images_flash_tool = self.output_dir + "/" + self.app.AppNamePrefix(self.chip_name) + ".flash.py"

        os.system("rm -rf " + ota_images_folder_path)
        os.system("rm -rf " + ota_images_dev_image)

        if not os.path.isfile(ota_images_firmware):
            return

        os.system("python " + ota_images_flash_tool + " --build > /dev/null")

        if not os.path.isfile(ota_images_image):
            return

        os.system("cp " + ota_images_image + " " + ota_images_dev_image)

        logging.info("PostBuild:")
        logging.info("Bouffalo Lab OTA format image: " + self.app.AppNamePrefix(self.chip_name) + ".bin.xz.hash is generated.")
