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
            raise Exception('Unknown board #: %r' % self)


class BouffalolabMfd(Enum):
    MFD_DISABLE = auto()
    MFD_TEST = auto()
    MFD_RELEASE = auto()


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
                 function_mfd: BouffalolabMfd = BouffalolabMfd.MFD_DISABLE,
                 enable_ethernet: bool = False,
                 enable_wifi: bool = False,
                 enable_thread: bool = False,
                 enable_frame_ptr: bool = False,
                 enable_heap_monitoring: bool = False,
                 use_matter_openthread: bool = False
                 ):

        if 'BL602' == module_type:
            bouffalo_chip = 'bl602'
        elif 'BL704L' == module_type:
            bouffalo_chip = 'bl702l'
        elif "BL70" in module_type:
            bouffalo_chip = 'bl702'
        else:
            raise Exception(f"module_type {module_type} is not supported")

        super(BouffalolabBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', bouffalo_chip),
            runner=runner
        )

        self.argsOpt = []
        self.chip_name = bouffalo_chip
        self.enable_frame_ptr = enable_frame_ptr

        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/bouffalolab/toolchain')
        toolchain = f'custom_toolchain="{toolchain}:riscv_gcc"'
        if toolchain:
            self.argsOpt.append(toolchain)

        self.app = app
        self.board = board

        self.argsOpt.append(f'board="{self.board.GnArgName()}"')
        self.argsOpt.append(f'baudrate="{baudrate}"')

        if not (enable_wifi or enable_thread or enable_ethernet):
            # decide default connectivity for each chip
            if bouffalo_chip == "bl602":
                enable_wifi, enable_thread, enable_ethernet = True, False, False
            elif bouffalo_chip == "bl702":
                enable_wifi, enable_thread, enable_ethernet = False, True, False
            elif bouffalo_chip == "bl702l":
                enable_wifi, enable_thread, enable_ethernet = False, True, False

        if (enable_ethernet or enable_wifi) and enable_thread:
            raise Exception('Currently, Thread can NOT be enabled with Wi-Fi or Ethernet')

        if enable_thread:
            chip_mdns = "platform"
        elif enable_ethernet or enable_wifi:
            chip_mdns = "minimal"

        # hardware connectivity support check
        if bouffalo_chip == "bl602":
            if enable_ethernet or enable_thread:
                raise Exception(f"SoC {bouffalo_chip} does NOT support connectivity Ethernet/Thread.")
        elif bouffalo_chip == "bl702":
            self.argsOpt.append(f'module_type="{module_type}"')
            if board != BouffalolabBoard.BL706DK:
                if enable_ethernet or enable_wifi:
                    raise Exception(f"Board {self.board.GnArgName()} does NOT support connectivity Ethernet/Wi-Fi.")
        elif bouffalo_chip == "bl702l":
            if enable_ethernet or enable_wifi:
                raise Exception(f"SoC {bouffalo_chip} does NOT support connectivity Ethernet/Wi-Fi currently.")

        self.argsOpt.append(f'chip_enable_ethernet={str(enable_ethernet).lower()}')
        self.argsOpt.append(f'chip_enable_wifi={str(enable_wifi).lower()}')
        self.argsOpt.append(f'chip_enable_openthread={str(enable_thread).lower()}')

        # for enable_ethernet, do not need ble for commissioning
        self.argsOpt.append(f'chip_config_network_layer_ble={str(enable_wifi or enable_thread).lower()}')

        self.argsOpt.append(f'chip_mdns="{chip_mdns}"')
        self.argsOpt.append(f'chip_inet_config_enable_ipv4={str(enable_ethernet or enable_wifi).lower()}')

        if enable_thread:
            self.argsOpt.append('chip_system_config_use_open_thread_inet_endpoints=true')
            self.argsOpt.append('chip_with_lwip=false')
            self.argsOpt.append(f'openthread_project_core_config_file="{bouffalo_chip}-openthread-core-bl-config.h"')
            if not use_matter_openthread:
                self.argsOpt.append(
                    'openthread_root="//third_party/connectedhomeip/third_party/bouffalolab/repo/components/network/thread/openthread"')

        if enable_cdc:
            if bouffalo_chip != "bl702":
                raise Exception(f'SoC {bouffalo_chip} does NOT support USB CDC')
            if enable_ethernet:
                raise Exception(f'SoC {bouffalo_chip} can NOT have both of USB CDC and Ethernet functions together.')

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

        if BouffalolabMfd.MFD_DISABLE != function_mfd:
            if BouffalolabMfd.MFD_RELEASE == function_mfd:
                self.argsOpt.append("chip_enable_factory_data=true")
            elif BouffalolabMfd.MFD_TEST == function_mfd:
                self.argsOpt.append("chip_enable_factory_data_test=true")

        self.argsOpt.append(f"enable_debug_frame_ptr={str(enable_frame_ptr).lower()}")
        self.argsOpt.append(f"enable_heap_monitoring={str(enable_heap_monitoring).lower()}")

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
            debug_output_file = os.path.join(self.output_dir, '%s.out' % self.app.AppNamePrefix(self.chip_name))
            return self.argsOpt + [f'debug_output_file="{debug_output_file}"']
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
        logging.info("Bouffalo Lab unsigned OTA image: " +
                     self.app.AppNamePrefix(self.chip_name) + ".bin.xz.hash is generated.")
