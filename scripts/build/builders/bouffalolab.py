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


def raise_exception(info):
    logging.fatal('*' * 80)
    logging.fatal('\t%s', info)
    logging.fatal('*' * 80)

    raise Exception(info)


class BouffalolabApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        else:
            raise_exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == BouffalolabApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        else:
            raise_exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise_exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL602_IoT_Matter_V1 = auto()
    BL602_NIGHT_LIGHT = auto()
    XT_ZB6_DevKit = auto()
    BL706_NIGHT_LIGHT = auto()
    BL706DK = auto()
    BL704LDK = auto()

    def GnArgName(self):
        if self == BouffalolabBoard.BL602_IoT_Matter_V1:
            return 'BL602-IoT-Matter-V1'
        elif self == BouffalolabBoard.BL602_NIGHT_LIGHT:
            return 'BL602-NIGHT-LIGHT'
        elif self == BouffalolabBoard.XT_ZB6_DevKit:
            return 'XT-ZB6-DevKit'
        elif self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        elif self == BouffalolabBoard.BL706DK:
            return 'BL706DK'
        elif self == BouffalolabBoard.BL704LDK:
            return 'BL704LDK'
        else:
            raise_exception('Unknown board #: %r' % self)


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
                 enable_rotating_device_id: bool = False,
                 function_mfd: str = "disable",
                 enable_ethernet: bool = False,
                 enable_wifi: bool = False,
                 enable_thread: bool = False,
                 enable_frame_ptr: bool = False
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
        self.enable_frame_ptr = enable_frame_ptr

        openthread_project_core_config_file = None
        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/bouffalolab/toolchain')
        toolchain = 'custom_toolchain="{}:riscv_gcc"'.format(toolchain)
        if toolchain:
            self.argsOpt.append(toolchain)

        self.app = app
        self.board = board

        self.argsOpt.append('board=\"{}\"'.format(self.board.GnArgName()))
        self.argsOpt.append('baudrate=\"{}\"'.format(baudrate))

        if bouffalo_chip == "bl602":

            enable_wifi = True
            if enable_ethernet or enable_thread:
                raise_exception('SoC %s doesn\'t support connectivity Ethernet/Thread.' % bouffalo_chip)

        elif bouffalo_chip == "bl702":

            self.argsOpt.append('module_type=\"{}\"'.format(module_type))
            if board != BouffalolabBoard.BL706DK:
                if enable_ethernet or enable_wifi:
                    raise_exception('Board %s doesn\'t support connectivity Ethernet/Wi-Fi.' % board)

            if not enable_wifi and not enable_ethernet:
                enable_thread = True
                openthread_project_core_config_file = "bl702-openthread-core-bl-config.h"

        elif bouffalo_chip == "bl702l":

            if enable_ethernet or enable_wifi:
                raise_exception('SoC %s doesn\'t support connectivity Ethernet/Wi-Fi currently.' % bouffalo_chip)

            enable_thread = True
            openthread_project_core_config_file = "bl702l-openthread-core-bl-config.h"

        if enable_wifi or enable_thread:
            self.argsOpt.append('chip_config_network_layer_ble=true')
        else:
            # for enable_ethernet, do not need ble for commissioning
            self.argsOpt.append('chip_config_network_layer_ble=false')

        if enable_ethernet:
            self.argsOpt.append('chip_enable_ethernet=true')
        else:
            self.argsOpt.append('chip_enable_ethernet=false')

        if enable_wifi:
            self.argsOpt.append('chip_enable_wifi=true')
        else:
            self.argsOpt.append('chip_enable_wifi=false')

        if enable_thread:
            self.argsOpt.append('chip_enable_openthread=true')
        else:
            self.argsOpt.append('chip_enable_openthread=false')

        if enable_thread:
            self.argsOpt.append('openthread_project_core_config_file="{}"'.format(openthread_project_core_config_file))

        if enable_thread:
            self.argsOpt.append('chip_mdns="platform"')
        elif enable_wifi or enable_ethernet:
            self.argsOpt.append('chip_mdns="minimal"')

        if enable_ethernet or enable_wifi:
            self.argsOpt.append('chip_inet_config_enable_ipv4=true')
        else:
            self.argsOpt.append('chip_inet_config_enable_ipv4=false')

        if enable_cdc:
            if bouffalo_chip != "bl702":
                self.raise_exception('Chip %s does NOT support USB CDC' % bouffalo_chip)
            if enable_ethernet:
                self.raise_exception('BL706 Ethernet does NOT support USB CDC')

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

        if "disable" != function_mfd:
            if bouffalo_chip != "bl602":
                self.raise_exception('Only BL602 support matter factory data feature currently.')

            if "release" == function_mfd:
                self.argsOpt.append("chip_enable_factory_data=true")
            elif "test" == function_mfd:
                self.argsOpt.append("chip_enable_factory_data_test=true")

        if enable_frame_ptr:
            self.argsOpt.append("enable_debug_frame_ptr=true")
        else:
            self.argsOpt.append("enable_debug_frame_ptr=false")

        try:
            self.argsOpt.append('bouffalolab_sdk_root="%s"' % os.environ['BOUFFALOLAB_SDK_ROOT'])
        except KeyError as err:
            self.print_enviroment_error()
            raise err

    def print_enviroment_error(self):
        logging.fatal('*' * 80)
        logging.fatal('\tPlease make sure Bouffalo Lab SDK installs as below:')
        logging.fatal('\t\tcd third_party/bouffalolab/repo')
        logging.fatal('\t\tsudo bash scripts/setup.sh')

        logging.fatal('\tPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
        logging.fatal('\t\texport BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk')
        logging.fatal('*' * 80)

    def GnBuildArgs(self):
        if self.enable_frame_ptr:
            return self.argsOpt + ["debug_output_file=\"{}\"".format(os.path.join(self.output_dir, '%s.out' % self.app.AppNamePrefix(self.chip_name)))]
        else:
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
