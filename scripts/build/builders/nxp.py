# Copyright (c) 2021-2024 Project CHIP Authors
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

import importlib.util
import logging
import os
from enum import Enum, auto
from typing import Optional

from .builder import BuilderOutput
from .gn import GnBuilder


class NxpOsUsed(Enum):
    FREERTOS = auto()
    ZEPHYR = auto()

    def OsEnv(self):
        if self == NxpOsUsed.ZEPHYR:
            return 'zephyr'
        elif self == NxpOsUsed.FREERTOS:
            return 'freertos'
        else:
            raise Exception('Unknown OS type: %r' % self)


class NxpBuildSystem(Enum):
    GN = auto()
    CMAKE = auto()

    def BuildSystem(self):
        if self == NxpBuildSystem.GN:
            return 'gn'
        elif self == NxpBuildSystem.CMAKE:
            return 'cmake'
        else:
            raise Exception('Unknown build system: %r' % self)


class NxpBoard(Enum):
    K32W0 = auto()
    K32W1 = auto()
    RT1060 = auto()
    RT1170 = auto()
    RW61X = auto()
    RW61X_ETH = auto()
    MCXW71 = auto()

    def Name(self, os_env):
        if self == NxpBoard.K32W0:
            return 'k32w0x'
        elif self == NxpBoard.K32W1:
            return 'k32w1'
        elif self == NxpBoard.RT1060:
            return 'rt1060'
        elif self == NxpBoard.RT1170:
            return 'rt1170'
        elif (self == NxpBoard.RW61X) or (self == NxpBoard.RW61X_ETH):
            if os_env == NxpOsUsed.ZEPHYR:
                if self == NxpBoard.RW61X_ETH:
                    return 'rd_rw612_bga/rw612/ethernet'
                else:
                    return 'rd_rw612_bga'
            else:
                return 'rw61x'
        elif self == NxpBoard.MCXW71:
            return 'mcxw71'
        else:
            raise Exception('Unknown board type: %r' % self)

    def FolderName(self, os_env):
        if self == NxpBoard.K32W0:
            return 'k32w0'
        elif self == NxpBoard.K32W1:
            return 'k32w1'
        elif self == NxpBoard.RT1060:
            return 'rt/rt1060'
        elif self == NxpBoard.RT1170:
            return 'rt/rt1170'
        elif (self == NxpBoard.RW61X) or (self == NxpBoard.RW61X_ETH):
            if os_env == NxpOsUsed.ZEPHYR:
                return 'zephyr'
            else:
                return 'rt/rw61x'
        elif self == NxpBoard.MCXW71:
            return 'mcxw71'
        else:
            raise Exception('Unknown board type: %r' % self)


class NxpBoardVariant(Enum):
    RD = auto()
    FRDM = auto()
    EVKC = auto()
    EVKB = auto()

    def BoardVariantName(self, board):
        if self == NxpBoardVariant.RD:
            return "rdrw612bga"
        elif self == NxpBoardVariant.FRDM:
            return "frdm"
        elif self == NxpBoardVariant.EVKC:
            return "evkcmimxrt1060"


class NxpApp(Enum):
    LIGHTING = auto()
    CONTACT = auto()
    ALLCLUSTERS = auto()
    LAUNDRYWASHER = auto()
    THERMOSTAT = auto()
    LOCK_APP = auto()

    def ExampleName(self):
        if self == NxpApp.LIGHTING:
            return 'lighting-app'
        elif self == NxpApp.CONTACT:
            return "contact-sensor-app"
        elif self == NxpApp.ALLCLUSTERS:
            return "all-clusters-app"
        elif self == NxpApp.LAUNDRYWASHER:
            return "laundry-washer-app"
        elif self == NxpApp.THERMOSTAT:
            return "thermostat"
        elif self == NxpApp.LOCK_APP:
            return "lock-app"
        else:
            raise Exception('Unknown app type: %r' % self)

    def NameSuffix(self):
        if self == NxpApp.LIGHTING:
            return 'light-example'
        elif self == NxpApp.CONTACT:
            return 'contact-example'
        elif self == NxpApp.ALLCLUSTERS:
            return "all-cluster-example"
        elif self == NxpApp.LAUNDRYWASHER:
            return "laundry-washer-example"
        elif self == NxpApp.THERMOSTAT:
            return "thermostat-example"
        elif self == NxpApp.LOCK_APP:
            return "lock-example"
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root, board, os_env):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', board.FolderName(os_env))


class NxpLogLevel(Enum):
    DEFAULT = auto()  # default everything
    ALL = auto()  # enable all logging
    PROGRESS = auto()  # progress and above
    ERROR = auto()  # error and above
    NONE = auto()  # no chip_logging at all


class NxpBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: NxpApp = NxpApp.LIGHTING,
                 board: NxpBoard = NxpBoard.K32W0,
                 board_variant: NxpBoardVariant = None,
                 os_env: NxpOsUsed = NxpOsUsed.FREERTOS,
                 build_system: NxpBuildSystem = NxpBuildSystem.GN,
                 low_power: bool = False,
                 smu2: bool = False,
                 enable_factory_data: bool = False,
                 convert_dac_pk: bool = False,
                 use_fro32k: bool = False,
                 enable_lit: bool = False,
                 enable_rotating_id: bool = False,
                 has_sw_version_2: bool = False,
                 disable_ble: bool = False,
                 enable_thread: bool = False,
                 enable_wifi: bool = False,
                 enable_ethernet: bool = False,
                 enable_shell: bool = False,
                 enable_ota: bool = False,
                 enable_factory_data_build: bool = False,
                 disable_pairing_autostart: bool = False,
                 iw416_transceiver: bool = False,
                 w8801_transceiver: bool = False,
                 iwx12_transceiver: bool = False,
                 log_level: NxpLogLevel = NxpLogLevel.DEFAULT,
                 ):
        super(NxpBuilder, self).__init__(
            root=app.BuildRoot(root, board, os_env),
            runner=runner)
        self.code_root = root
        self.app = app
        self.board = board
        self.os_env = os_env
        self.build_system = build_system
        self.low_power = low_power
        self.smu2 = smu2
        self.enable_factory_data = enable_factory_data
        self.convert_dac_pk = convert_dac_pk
        self.use_fro32k = use_fro32k
        self.enable_lit = enable_lit
        self.enable_rotating_id = enable_rotating_id
        self.has_sw_version_2 = has_sw_version_2
        self.disable_ble = disable_ble
        self.enable_thread = enable_thread
        self.enable_wifi = enable_wifi
        self.enable_ethernet = enable_ethernet
        self.enable_ota = enable_ota
        self.enable_shell = enable_shell
        self.enable_factory_data_build = enable_factory_data_build
        self.disable_pairing_autostart = disable_pairing_autostart
        self.board_variant = board_variant
        self.iw416_transceiver = iw416_transceiver
        self.w8801_transceiver = w8801_transceiver
        self.iwx12_transceiver = iwx12_transceiver
        if self.low_power and log_level != NxpLogLevel.NONE:
            logging.warning("Switching log level to 'NONE' for low power build")
            log_level = NxpLogLevel.NONE
        self.log_level = log_level

    def GnBuildArgs(self):
        args = []

        if self.low_power:
            args.append('nxp_use_low_power=true')
            if self.board == NxpBoard.K32W0:
                args.append('chip_pw_tokenizer_logging=false chip_with_OM15082=0')

        if self.smu2:
            args.append('nxp_use_smu2_static=true nxp_use_smu2_dynamic=true')

        if self.enable_factory_data:
            args.append('nxp_use_factory_data=true')

        if self.convert_dac_pk:
            args.append('nxp_convert_dac_private_key=true')

        if self.use_fro32k:
            args.append('use_fro_32k=1')

        if self.enable_lit:
            args.append('chip_enable_icd_lit=true')

        if self.enable_rotating_id:
            args.append('chip_enable_rotating_device_id=1 chip_enable_additional_data_advertising=1')

        if self.log_level == NxpLogLevel.DEFAULT:
            pass
        elif self.log_level == NxpLogLevel.ALL:
            args.append("chip_logging=true")
            args.append("chip_error_logging=true")
            args.append("chip_progress_logging=true")
            args.append("chip_detail_logging=true")
            args.append("chip_automation_logging=true")
        elif self.log_level == NxpLogLevel.PROGRESS:
            args.append("chip_logging=true")
            args.append("chip_error_logging=true")
            args.append("chip_progress_logging=true")
            args.append("chip_detail_logging=false")
            args.append("chip_automation_logging=false")
        elif self.log_level == NxpLogLevel.ERROR:
            args.append("chip_logging=true")
            args.append("chip_error_logging=true")
            args.append("chip_progress_logging=false")
            args.append("chip_detail_logging=false")
            args.append("chip_automation_logging=false")
        elif self.log_level == NxpLogLevel.NONE:
            args.append("chip_logging=false")
        else:
            raise Exception("Unknown log level: %r", self.log_level)

        if self.has_sw_version_2:
            args.append('nxp_software_version=2')

        if self.enable_ota:
            # OTA is enabled by default on kw32
            if self.board == NxpBoard.RW61X:
                args.append('chip_enable_ota_requestor=true no_mcuboot=false')

        if self.enable_wifi:
            args.append('chip_enable_wifi=true')

        if self.disable_ble:
            args.append('chip_enable_ble=false')

        if self.enable_shell:
            args.append('nxp_enable_matter_cli=true')

        if self.enable_thread:
            # thread is enabled by default on kw32
            if self.board == NxpBoard.RW61X:
                args.append('chip_enable_openthread=true chip_inet_config_enable_ipv4=false')
            if self.board == NxpBoard.RT1060:
                args.append('chip_enable_openthread=true chip_inet_config_enable_ipv4=false')
            if self.board == NxpBoard.RT1170:
                args.append('chip_enable_openthread=true chip_inet_config_enable_ipv4=false')

        if self.board_variant:
            if self.board == NxpBoard.RT1060:
                flag_board_variant = "evkname=\\\"%s\\\"" % self.board_variant.BoardVariantName(self.board)
                args.append(flag_board_variant)
            if self.board == NxpBoard.RW61X:
                flag_board_variant = "board_version=\\\"%s\\\"" % self.board_variant.BoardVariantName(self.board)
                args.append(flag_board_variant)

        if self.iw416_transceiver:
            args.append('iw416_transceiver=true')

        if self.w8801_transceiver:
            # BLE not supported on this transceiver
            args.append('w8801_transceiver=true chip_enable_ble=false')

        if self.iwx12_transceiver:
            args.append('iwx12_transceiver=true')

        return args

    def CmakeBuildFlags(self):
        flags = []

        if self.enable_factory_data:
            if self.os_env == NxpOsUsed.ZEPHYR:
                flags.append('-DFILE_SUFFIX=fdata')
            else:
                flags.append("-DCONFIG_CHIP_FACTORY_DATA=true")

        if self.enable_ethernet:
            if self.os_env == NxpOsUsed.ZEPHYR:
                flags.append('-DEXTRA_CONF_FILE="prj_ethernet.conf"')

        if self.has_sw_version_2:
            flags.append("-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2")
            flags.append("-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING=\"2.0\"")

        if self.enable_ota:
            flags.append("-DCONFIG_CHIP_OTA_REQUESTOR=true")
            if self.os_env == NxpOsUsed.FREERTOS and self.board == NxpBoard.RW61X:
                flags.append("-DCONFIG_BOOTLOADER_MCUBOOT=true")

        if self.disable_ble:
            flags.append("-DCONFIG_BT=false")

        if self.enable_wifi:
            flags.append('-DCONFIG_CHIP_WIFI=true')

        if self.enable_thread:
            flags.append("-DCONFIG_NET_L2_OPENTHREAD=true -DCONFIG_CHIP_IPV4=false")

        if self.enable_factory_data_build:
            # Generate the factory data binary
            flags.append("-DCONFIG_CHIP_FACTORY_DATA_BUILD=true -DCONFIG_CHIP_FACTORY_DATA=true")

        if self.enable_shell:
            flags.append("-DCONFIG_CHIP_LIB_SHELL=true")

        if self.disable_pairing_autostart:
            flags.append('-DCONFIG_CHIP_ENABLE_PAIRING_AUTOSTART=false')

        if self.board_variant:
            flag_board_variant = "-DCONFIG_BOARD_VARIANT=\"%s\"" % self.board_variant.BoardVariantName(self.board)
            flags.append(flag_board_variant)

        build_flags = " ".join(flags)

        return build_flags

    def generate(self):
        build_flags = self.CmakeBuildFlags()
        if self.os_env == NxpOsUsed.ZEPHYR:
            if 'ZEPHYR_NXP_SDK_INSTALL_DIR' not in os.environ:
                raise Exception("ZEPHYR_NXP_SDK_INSTALL_DIR need to be set")

            if 'ZEPHYR_NXP_BASE' not in os.environ:
                raise Exception("ZEPHYR_NXP_BASE need to be set")

            cmd = 'export ZEPHYR_SDK_INSTALL_DIR="$ZEPHYR_NXP_SDK_INSTALL_DIR"'
            cmd += '\nexport ZEPHYR_BASE="$ZEPHYR_NXP_BASE"'
            cmd += '\nunset ZEPHYR_TOOLCHAIN_VARIANT'

            cmd += '\nwest build -p --cmake-only -b {board_name} -d {out_folder} {example_folder} {build_flags}'.format(
                board_name=self.board.Name(self.os_env),
                out_folder=self.output_dir,
                example_folder=self.app.BuildRoot(self.code_root, self.board, self.os_env),
                build_flags=build_flags)
            self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)
        else:
            cmd = ''
            # will be used with next sdk version to get sdk path
            if 'NXP_UPDATE_SDK_SCRIPT_DOCKER' in os.environ:
                # Dynamic import of a python file to get platforms sdk path
                spec = importlib.util.spec_from_file_location("None", os.environ['NXP_UPDATE_SDK_SCRIPT_DOCKER'])
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)

                for p in module.ALL_PLATFORM_SDK:
                    if p.sdk_name == 'k32w0':
                        cmd += 'export NXP_K32W0_SDK_ROOT="' + str(p.sdk_storage_location_abspath) + '" \n '
                    elif p.sdk_name == 'common':
                        cmd += 'export NXP_SDK_ROOT="' + str(p.sdk_storage_location_abspath) + '" \n '

            if self.build_system == NxpBuildSystem.CMAKE:
                cmd += '''
                cmake -GNinja {build_flags} -H{example_folder} -B{out_folder}
                '''.format(
                    build_flags=build_flags,
                    example_folder=self.app.BuildRoot(self.code_root, self.board, self.os_env),
                    out_folder=self.output_dir).strip()
                self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)

            elif self.build_system == NxpBuildSystem.GN:
                # add empty space at the end to avoid concatenation issue when there is no --args
                cmd += 'gn gen --check --fail-on-unused-args --export-compile-commands --root=%s ' % self.root

                extra_args = []

                if self.options.pw_command_launcher:
                    extra_args.append('pw_command_launcher="%s"' % self.options.pw_command_launcher)

                if self.options.enable_link_map_file:
                    extra_args.append('chip_generate_link_map_file=true')

                if self.options.pregen_dir:
                    extra_args.append('chip_code_pre_generated_directory="%s"' % self.options.pregen_dir)

                extra_args.extend(self.GnBuildArgs() or [])
                if extra_args:
                    cmd += ' --args="%s' % ' '.join(extra_args) + '" '

                cmd += self.output_dir

                title = 'Generating ' + self.identifier

                self._Execute(['bash', '-c', cmd], title=title)

    def build_outputs(self):
        name = 'chip-%s-%s' % (self.board.Name(self.os_env), self.app.NameSuffix())
        if self.os_env == NxpOsUsed.ZEPHYR:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
                f'{name}.elf')
            if self.options.enable_link_map_file:
                yield BuilderOutput(
                    os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
                    f'{name}.map')
        else:
            if self.build_system == NxpBuildSystem.GN:
                yield BuilderOutput(
                    os.path.join(self.output_dir, name),
                    f'{name}.elf')
                if self.options.enable_link_map_file:
                    yield BuilderOutput(
                        os.path.join(self.output_dir, f'{name}.map'),
                        f'{name}.map')
            elif self.build_system == NxpBuildSystem.CMAKE:
                yield BuilderOutput(
                    os.path.join(self.output_dir, 'out/debug', name),
                    f'{name}.elf')
                if self.options.enable_link_map_file:
                    yield BuilderOutput(
                        os.path.join(self.output_dir, 'out/debug', f'{name}.map'),
                        f'{name}.map')
