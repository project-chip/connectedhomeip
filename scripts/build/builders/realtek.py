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

from .builder import Builder, BuilderOutput


class RealtekBoard(Enum):
    RTL8777G = auto()

    @property
    def BoardName(self):
        return 'rtl8777g'


class RealtekApp(Enum):
    LIGHT = auto()
    LIGHT_SWITCH = auto()
    LOCK = auto()
    WINDOW = auto()

    @property
    def ExampleName(self):
        if self == RealtekApp.LIGHT:
            return 'lighting-app'
        elif self == RealtekApp.LIGHT_SWITCH:
            return 'light-switch-app'
        elif self == RealtekApp.LOCK:
            return 'lock-app'
        elif self == RealtekApp.WINDOW:
            return 'window-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def TargetName(self):
        if self == RealtekApp.LIGHT:
            return 'matter-cli-ftd'
        elif self == RealtekApp.LIGHT_SWITCH:
            return 'matter-cli-mtd'
        elif self == RealtekApp.LOCK:
            return 'matter-cli-mtd'
        elif self == RealtekApp.WINDOW:
            return 'matter-cli-mtd'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == RealtekApp.LIGHT:
            return 'chip-rtl8777g-lighting-app'
        elif self == RealtekApp.LIGHT_SWITCH:
            return 'chip-rtl8777g-light-switch-app'
        elif self == RealtekApp.LOCK:
            return 'chip-rtl8777g-lock-app'
        elif self == RealtekApp.WINDOW:
            return 'chip-rtl8777g-window-app'
        else:
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

        os.environ['OT_SRCDIR'] = os.path.join(os.getcwd(), 'third_party/openthread/ot-realtek')
        os.environ['REALTEK_SDK_PATH'] = os.path.join(os.environ['OT_SRCDIR'], 'third_party/Realtek/rtl87x2g_sdk')
        os.environ['BUILD_BANK'] = 'bank0'
        os.environ['OT_CMAKE_NINJA_TARGET'] = self.app.TargetName

    def CmakeBuildFlags(self):
        flags = []

        flags.append("-DCMAKE_BUILD_TYPE=Release")
        flags.append("-DCMAKE_TOOLCHAIN_FILE=src/bee4/arm-none-eabi.cmake")
        flags.append("-DBUILD_TYPE=sdk")
        flags.append(f"-DBUILD_TARGET={self.board.BoardName}")
        flags.append(f"-DBUILD_BOARD_TARGET={self.board.BoardName}")
        flags.append(f"-DOT_CMAKE_NINJA_TARGET={self.app.TargetName}")

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

        build_flags = " ".join(flags)

        return build_flags

    def generate(self):
        os.environ['OUT_FOLDER'] = self.output_dir

        cmd = 'cd {}/openthread \n'.format(os.environ['OT_SRCDIR'])
        cmd += 'git checkout thread-reference-20230706'
        self._Execute(['bash', '-c', cmd])

        cmd = 'arm-none-eabi-gcc -D BUILD_BANK=0 -E -P -x c {ot_src_dir}/src/bee4/{board_name}/app.ld -o {ot_src_dir}/src/bee4/{board_name}/app.ld.gen'.format(
            ot_src_dir=os.environ['OT_SRCDIR'],
            board_name=self.board.BoardName)
        self._Execute(['bash', '-c', cmd])

        cmd = f'export MATTER_EXAMPLE_PATH={self.root}/examples/{self.app.ExampleName}/realtek_bee \n'

        cmd += 'cmake -GNinja -DOT_COMPILE_WARNING_AS_ERROR=ON {build_flags} {example_folder} -B{out_folder}'.format(
            build_flags=self.CmakeBuildFlags(),
            example_folder=os.environ['OT_SRCDIR'],
            out_folder=self.output_dir)

        self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)

    def _build(self):
        cmd = ['ninja', '-C', self.output_dir]

        if self.ninja_jobs is not None:
            cmd.append('-j' + str(self.ninja_jobs))

        cmd.append(self.app.TargetName)

        self._Execute(cmd, title='Building ' + self.identifier)

        self.PostBuildCommand()

    def build_outputs(self):
        logging.info('build_outputs %s', self.output_dir)

    def PostBuildCommand(self):
        cmd = f'{self.root}/third_party/openthread/ot-realtek/Realtek/post_build '

        # <bank> <target_name>
        cmd += ' '.join([os.environ.get('BUILD_BANK'), self.app.TargetName])

        self._Execute(['bash', '-c', cmd], title='PostBuild ' + self.identifier)

        os.system(f"rm -rf {self.root}/third_party/openthread/ot-realtek/src/bee4/{self.board.BoardName}/*.gen")
