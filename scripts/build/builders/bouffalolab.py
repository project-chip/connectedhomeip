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
import time
from enum import Enum, auto

from .builder import BuilderOutput
from .gn import GnBuilder


class BouffalolabApp(Enum):
    LIGHT = auto()
    CONTACT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        elif self == BouffalolabApp.CONTACT:
            return 'contact-sensor-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == BouffalolabApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        elif self == BouffalolabApp.CONTACT:
            return ('chip-%s-contact-sensor-example' % chip_name)
        else:
            raise Exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL602DK = auto()
    BL616DK = auto()
    BL704LDK = auto()
    BL706DK = auto()
    BL602_IoT_Matter_V1 = auto()
    BL602_NIGHT_LIGHT = auto()
    XT_ZB6_DevKit = auto()
    BL706_NIGHT_LIGHT = auto()

    def GnArgName(self):

        if self == BouffalolabBoard.BL602DK:
            return 'BL602DK'
        elif self == BouffalolabBoard.BL616DK:
            return 'BL616DK'
        elif self == BouffalolabBoard.BL704LDK:
            return 'BL704LDK'
        elif self == BouffalolabBoard.BL706DK:
            return 'BL706DK'
        elif self == BouffalolabBoard.BL602_IoT_Matter_V1:
            return 'BL602-IoT-Matter-V1'
        elif self == BouffalolabBoard.BL602_NIGHT_LIGHT:
            return 'BL602-NIGHT-LIGHT'
        elif self == BouffalolabBoard.XT_ZB6_DevKit:
            return 'XT-ZB6-DevKit'
        elif self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        else:
            raise Exception('Unknown board #: %r' % self)


class BouffalolabThreadType(Enum):
    NONE = auto()
    THREAD_FTD = auto()
    THREAD_MTD = auto()


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
                 enable_mfd: bool = False,
                 enable_ethernet: bool = False,
                 enable_wifi: bool = False,
                 enable_thread_type: BouffalolabThreadType = BouffalolabThreadType.NONE,
                 enable_heap_monitoring: bool = False,
                 use_matter_openthread: bool = False,
                 enable_easyflash: bool = False,
                 enable_littlefs: bool = False,
                 enable_pds: bool = False,
                 enable_debug_coredump: bool = False,
                 ):

        if 'BL602' == module_type:
            bouffalo_chip = 'bl602'
        elif 'BL704L' == module_type:
            bouffalo_chip = 'bl702l'
        elif "BL70" in module_type:
            bouffalo_chip = 'bl702'
        elif "BL616" == module_type:
            bouffalo_chip = "bl616"
        else:
            raise Exception(f"module_type {module_type} is not supported")

        super(BouffalolabBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', bouffalo_chip),
            runner=runner
        )

        self.argsOpt = []
        self.chip_name = bouffalo_chip
        self.app = app
        self.board = board

        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/bouffalolab/toolchain')
        toolchain = f'custom_toolchain="{toolchain}:riscv_gcc"'
        if toolchain:
            self.argsOpt.append(toolchain)

        self.argsOpt.append(f'board="{self.board.GnArgName()}"')
        self.argsOpt.append(f'baudrate="{baudrate}"')

        enable_thread = False if enable_thread_type == BouffalolabThreadType.NONE else True

        if not (enable_wifi or enable_thread or enable_ethernet):
            # decide default connectivity for each chip
            if bouffalo_chip == "bl602":
                enable_wifi, enable_thread, enable_ethernet = True, False, False
            elif bouffalo_chip == "bl702":
                enable_wifi, enable_thread, enable_ethernet = False, True, False
            elif bouffalo_chip == "bl702l":
                enable_wifi, enable_thread, enable_ethernet = False, True, False
            elif bouffalo_chip == "bl616":
                raise Exception("Must select one of wifi and thread to build.")

        if [enable_wifi, enable_thread, enable_ethernet].count(True) > 1:
            raise Exception('Currently, only one of wifi, thread and ethernet supports.')

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
        elif bouffalo_chip == "bl616":
            if enable_ethernet:
                raise Exception(f"SoC {bouffalo_chip} does NOT support connectivity Ethernet currently.")

        if enable_thread:
            chip_mdns = "platform"
        elif enable_ethernet or enable_wifi:
            chip_mdns = "minimal"

        self.argsOpt.append(f'chip_enable_ethernet={str(enable_ethernet).lower()}')
        self.argsOpt.append(f'chip_enable_wifi={str(enable_wifi).lower()}')
        self.argsOpt.append(f'chip_enable_openthread={str(enable_thread).lower()}')

        # for enable_ethernet, do not need ble for commissioning
        self.argsOpt.append(f'chip_config_network_layer_ble={str(enable_wifi or enable_thread).lower()}')

        self.argsOpt.append(f'chip_mdns="{chip_mdns}"')
        self.argsOpt.append(f'chip_inet_config_enable_ipv4={str(enable_ethernet or enable_wifi).lower()}')

        if enable_easyflash and enable_littlefs:
            raise Exception("Only one of easyflash and littlefs can be enabled.")
        if bouffalo_chip == "bl616":
            if not enable_easyflash:
                enable_littlefs = True
        else:
            if not enable_easyflash and not enable_littlefs:
                logging.fatal('*' * 80)
                logging.fatal('littlefs is added to support for flash storage access.')
                logging.fatal('Please consider and select one of easyflash and littlefs to use.')
                logging.fatal('*' * 80)
                raise Exception("None of easyflash and littlefs select to build.")
        self.argsOpt.append(f'bouffalo_sdk_component_easyflash_enabled={"false" if enable_littlefs else "true"}')

        if enable_thread:

            self.argsOpt.append('chip_system_config_use_open_thread_inet_endpoints=true')
            self.argsOpt.append('chip_with_lwip=false')
            self.argsOpt.append(f'openthread_project_core_config_file="{bouffalo_chip}-openthread-core-bl-config.h"')
            self.argsOpt.append(f'openthread_package_version="7e32165be"')

            if enable_thread_type == BouffalolabThreadType.THREAD_FTD:
                self.argsOpt.append(f'chip_openthread_ftd=true')
            else:
                self.argsOpt.append(f'chip_openthread_ftd=false')

            if not use_matter_openthread:
                if bouffalo_chip in {"bl702", "bl702l"}:
                    self.argsOpt.append(
                        'openthread_root="//third_party/connectedhomeip/third_party/bouffalolab/repo/components/network/thread/openthread"')
                else:
                    self.argsOpt.append(
                        'openthread_root="//third_party/connectedhomeip/third_party/bouffalolab/bouffalo_sdk/components/wireless/thread/openthread"')

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

        if enable_mfd:
            self.argsOpt.append("chip_enable_factory_data=true")

        if enable_pds:
            self.argsOpt.append("enable_pds=true")

        self.argsOpt.append(f"enable_heap_monitoring={str(enable_heap_monitoring).lower()}")
        if enable_debug_coredump:
            self.argsOpt.append(f"enable_debug_coredump=true")
            self.argsOpt.append(f"coredump_binary_id={int(time.time())}")

        self.argsOpt.append(f"chip_generate_link_map_file=true")

        try:
            self.argsOpt.append('bouffalolab_sdk_root="%s"' % os.environ['BOUFFALOLAB_SDK_ROOT'])
        except KeyError as err:
            self.print_enviroment_error()
            raise err

    def print_enviroment_error(self):
        logging.fatal('*' * 80)
        logging.error('Flashtool is not installed, or environment variable BOUFFALOLAB_SDK_ROOT is not exported.')
        logging.fatal('\tPlease make sure Bouffalo Lab SDK installs as below:')
        logging.fatal('\t\t./integrations/docker/images/stage-2/chip-build-bouffalolab/setup.sh')

        logging.fatal('\tPlease make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
        logging.fatal('\t\texport BOUFFALOLAB_SDK_ROOT="your install path"')
        logging.fatal('*' * 80)

    def GnBuildArgs(self):
        return self.argsOpt

    def build_outputs(self):
        extensions = ["out"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix(self.chip_name)}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)

    def PreBuildCommand(self):
        os.system("rm -rf {}/config".format(self.output_dir))
        os.system("rm -rf {}/ota_images".format(self.output_dir))
        os.system("rm -rf {}".format(os.path.join(self.output_dir, 'boot2*.bin')))
        os.system("rm -rf {}".format(os.path.join(self.output_dir, '%s*' % self.app.AppNamePrefix(self.chip_name))))

    def PostBuildCommand(self):

        bouffalo_sdk_chips = ["bl616"]
        abs_path_fw = os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".bin")

        if self.chip_name not in bouffalo_sdk_chips:
            abs_path_fw_raw = os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".raw")

        if os.path.isfile(abs_path_fw):
            target_dir = self.output_dir.replace(self.chip_dir, "").strip("/")

            abs_path_fw_bin = os.path.join(self.output_dir, self.app.AppNamePrefix(self.chip_name) + ".bin")
            path_fw = os.path.join(target_dir, self.app.AppNamePrefix(self.chip_name) + ".bin")
            path_flash_script = os.path.join(target_dir, self.app.AppNamePrefix(self.chip_name) + ".flash.py")

            logging.info('*' * 80)

            logging.info("Firmware is built out at: {}".format(path_fw))
            logging.info("Command to generate ota image: ")
            logging.info('./{} --build-ota --vendor-id <vendor id> --product-id <product id> '
                         '--version <version> --version-str <version string> '
                         '--digest-algorithm <digest algorithm>'.format(path_flash_script))
            logging.info("Command to generate and sign ota image: ")
            logging.info('./{} --build-ota --vendor-id <vendor id> --product-id <product id> '
                         '--version <version> --version-str <version string> '
                         '--digest-algorithm <digest algorithm> --sk <private key>'.format(path_flash_script))
            logging.info('*' * 80)
