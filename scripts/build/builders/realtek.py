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

import os
from enum import Enum, auto

from .builder import Builder, BuilderOutput


class RtkOsUsed(Enum):
    FREERTOS = auto()
    ZEPHYR = auto()

    def OsEnv(self):
        if self == RtkOsUsed.ZEPHYR:
            return 'zephyr'
        if self == RtkOsUsed.FREERTOS:
            return 'freertos'
        raise Exception('Unknown OS type: %r' % self)


class RealtekBoard(Enum):
    RTL8777G = auto()
    RTL87X2G = auto()

    @property
    def BoardName(self):
        if self == RealtekBoard.RTL8777G:
            return 'rtl8777g'
        if self == RealtekBoard.RTL87X2G:
            return 'rtl87x2g'
        raise Exception('Unknown board type: %r' % self)


class RealtekApp(Enum):
    LIGHT = auto()
    LIGHT_SWITCH = auto()
    LOCK = auto()
    WINDOW = auto()
    ALL_CLUSTERS = auto()
    THERMOSTAT = auto()
    OTA_REQUESTOR = auto()

    @property
    def ExampleName(self):
        if self == RealtekApp.LIGHT:
            return 'lighting-app'
        if self == RealtekApp.LIGHT_SWITCH:
            return 'light-switch-app'
        if self == RealtekApp.LOCK:
            return 'lock-app'
        if self == RealtekApp.WINDOW:
            return 'window-app'
        if self == RealtekApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        if self == RealtekApp.OTA_REQUESTOR:
            return 'ota-requestor-app'
        if self == RealtekApp.THERMOSTAT:
            return 'thermostat'
        raise Exception('Unknown app type: %r' % self)

    @property
    def TargetName(self):
        if self == RealtekApp.LIGHT:
            return 'matter-cli-ftd'
        if self == RealtekApp.LIGHT_SWITCH:
            return 'matter-cli-mtd'
        if self == RealtekApp.LOCK:
            return 'matter-cli-mtd'
        if self == RealtekApp.WINDOW:
            return 'matter-cli-mtd'
        if self == RealtekApp.ALL_CLUSTERS:
            return 'matter-cli-ftd'
        if self == RealtekApp.OTA_REQUESTOR:
            return 'matter-cli-ftd'
        if self == RealtekApp.THERMOSTAT:
            return 'matter-cli-mtd'
        raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == RealtekApp.LIGHT:
            return 'chip-rtl8777g-lighting-app'
        if self == RealtekApp.LIGHT_SWITCH:
            return 'chip-rtl8777g-light-switch-app'
        if self == RealtekApp.LOCK:
            return 'chip-rtl8777g-lock-app'
        if self == RealtekApp.WINDOW:
            return 'chip-rtl8777g-window-app'
        if self == RealtekApp.ALL_CLUSTERS:
            return 'chip-rtl8777g-all-clusters-app'
        if self == RealtekApp.OTA_REQUESTOR:
            return 'chip-rtl8777g-ota-requestor-app'
        if self == RealtekApp.THERMOSTAT:
            return 'chip-rtl8777g-thermostat-app'
        raise Exception('Unknown app type: %r' % self)


class RealtekBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 board: RealtekBoard = RealtekBoard.RTL8777G,
                 app: RealtekApp = RealtekApp.LIGHT,
                 enable_cli: bool = False,
                 enable_rpc: bool = False,
                 enable_shell: bool = False):
        super(RealtekBuilder, self).__init__(root, runner)
        self.board = board
        self.app = app
        self.enable_cli = enable_cli
        self.enable_rpc = enable_rpc
        self.enable_shell = enable_shell
        self.ot_src_dir = os.path.join(os.getcwd(), 'third_party/openthread/ot-realtek')
        self.rtk_matter_dir = os.path.join(self.ot_src_dir, 'third_party/Realtek/rtl87x2g_sdk/subsys/matter')

        if self.board == RealtekBoard.RTL87X2G:
            self.os_env = RtkOsUsed.ZEPHYR
        elif self.board == RealtekBoard.RTL8777G:
            self.os_env = RtkOsUsed.FREERTOS
        else:
            raise Exception('Unknown board type: %r' % self.board)

    def CmakeBuildFlags(self) -> str:
        flags = [
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_TOOLCHAIN_FILE={self.ot_src_dir}/src/bee4/arm-none-eabi.cmake",
            "-DBUILD_TYPE=sdk",
            f"-DBUILD_TARGET={self.board.BoardName}",
            f"-DBUILD_BOARD_TARGET={self.board.BoardName}",
            f"-DOT_CMAKE_NINJA_TARGET={self.app.TargetName}",
            f"-DMATTER_EXAMPLE_PATH={self.root}/examples/{self.app.ExampleName}/realtek/common"
        ]
        if self.enable_cli:
            flags.append("-DENABLE_CLI=ON")
        else:
            flags.append("-DENABLE_CLI=OFF")
        if self.enable_rpc:
            flags.append("-DENABLE_PW_RPC=ON")
        else:
            flags.append("-DENABLE_PW_RPC=OFF")
        if self.enable_shell:
            flags.append("-DENABLE_SHELL=ON")
        else:
            flags.append("-DENABLE_SHELL=OFF")
        return " ".join(flags)

    def get_cmd_prefixes(self):
        # Zephyr base
        if 'ZEPHYR_REALTEK_BASE' not in os.environ:
            raise Exception("Realtek builds require ZEPHYR_REALTEK_BASE")

        cmd = 'export ZEPHYR_BASE="$ZEPHYR_REALTEK_BASE"\n'

        if 'ZEPHYR_REALTEK_SDK_INSTALL_DIR' in os.environ:
            cmd += 'export ZEPHYR_SDK_INSTALL_DIR="$ZEPHYR_REALTEK_SDK_INSTALL_DIR"\n'

        return cmd

    def generate(self):
        if self.os_env != RtkOsUsed.FREERTOS:
            # For freertos, app.ld needs to be precompiled with gcc
            return
        cmd = 'arm-none-eabi-gcc -D BUILD_BANK=0 -E -P -x c {ot_src_dir}/src/bee4/{board_name}/app.ld -o {ot_src_dir}/src/bee4/{board_name}/app.ld.gen'.format(
            ot_src_dir=self.ot_src_dir,
            board_name=self.board.BoardName)
        self._Execute(['bash', '-c', cmd])
        cmd = 'cmake -GNinja -DOT_COMPILE_WARNING_AS_ERROR=ON {build_flags} {example_folder} -B{out_folder}'.format(
            build_flags=self.CmakeBuildFlags(),
            example_folder=self.rtk_matter_dir,
            out_folder=self.output_dir)
        self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)

    def _build(self):
        if self.os_env == RtkOsUsed.FREERTOS:
            cmd = ['ninja', '-C', self.output_dir]
            if self.ninja_jobs is not None:
                cmd.append('-j' + str(self.ninja_jobs))
            cmd.append(self.app.TargetName)
            self._Execute(cmd, title='Building ' + self.identifier)
            cleanup_cmd = ['rm', '-rf', f"{self.root}/third_party/openthread/ot-realtek/src/bee4/{self.board.BoardName}/*.gen"]
            self._Execute(cleanup_cmd, title='Cleaning up generated files')
        else:
            cmd = self.get_cmd_prefixes()
            cmd += '\nsource "$ZEPHYR_BASE/zephyr-env.sh"'
            cmd += '\nwest build -b rtl87x2g_evb -d {out_folder} {example_folder} --pristine'.format(
                out_folder=self.output_dir,
                example_folder=os.path.join(self.root, 'examples', self.app.ExampleName, 'realtek', 'zephyr'))
            self._Execute(['bash', '-c', cmd], title='Building ' + self.identifier)

    def build_outputs(self):
        if self.os_env == RtkOsUsed.FREERTOS:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'bin', self.app.TargetName),
                self.app.AppNamePrefix)
            if self.options.enable_link_map_file:
                yield BuilderOutput(
                    os.path.join(self.output_dir, 'map.map'),
                    self.app.AppNamePrefix + '.map')
        else:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
                self.app.AppNamePrefix + '.elf')
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.bin'),
                self.app.AppNamePrefix + '.bin')
            if self.options.enable_link_map_file:
                yield BuilderOutput(
                    os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
                    self.app.AppNamePrefix + '.map')
