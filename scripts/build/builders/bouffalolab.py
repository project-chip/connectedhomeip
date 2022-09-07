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
import platform
from enum import Enum, auto

from .gn import GnBuilder


class BouffalolabApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == BouffalolabApp.LIGHT:
            return 'chip-bl702-lighting-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL706_IoT_DVK = 1
    BL702_IoT_DVK = 2
    BL706_NIGHT_LIGHT = 3

    def GnArgName(self):
        if self == BouffalolabBoard.BL706_IoT_DVK:
            return 'BL706-IoT-DVK'
        elif self == BouffalolabBoard.BL702_IoT_DVK:
            return 'BL702-IoT-DVK'
        elif self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        else:
            raise Exception('Unknown board #: %r' % self)


class BouffalolabBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: BouffalolabApp = BouffalolabApp.LIGHT,
                 board: BouffalolabBoard = BouffalolabBoard.BL702_IoT_DVK,
                 enable_rpcs: bool = False,
                 module_type: str = "BL706C-22",
                 ):

        boufflab_chip = "bl702" if "BL70" in module_type else "bl602"
        super(BouffalolabBuilder, self).__init__(
                root=os.path.join(  root, 'examples',
                                    app.ExampleName(), 'bouffalolab', boufflab_chip),
                runner=runner
            )

        toolchain = os.path.join(root, '../../examples/platform/bouffalolab/common/toolchain')
        objcopy   = toolchain
        platform_os = platform.system()
        if "Linux" == platform_os:
            toolchain   = 'custom_toolchain="{}:linux_riscv_gcc"'.format(toolchain)
            objcopy     = os.path.join( root, '../../third_party/bouffalolab/repo/toolchain/riscv/Linux/bin/riscv64-unknown-elf-objcopy')
        elif "Darwin" == platform_os:
            toolchain   = 'custom_toolchain="{}:darwin_riscv_gcc"'.format(toolchain)
            objcopy     = os.path.join( root, '../../third_party/bouffalolab/repo/toolchain/riscv/Darwin/bin/riscv64-unknown-elf-objcopy')
        else:
            toolchain   = None
            objcopy     = None

        self.app = app
        self.board = board

        self.argsOpt = []
        self.argsOpt.append('board=\"{}\"'.format(self.board.GnArgName()))
        self.argsOpt.append('module_type=\"{}\"'.format(module_type))

        if toolchain:
            self.argsOpt.append(toolchain)
        if objcopy:
            self.argsOpt.append('bouffalolab_objcopy="{}"'.format(objcopy))

        if enable_rpcs:
            self.argsOpt.append('import("//with_pw_rpc.gni")')

    def GnBuildArgs(self):
        return self.argsOpt

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        return items
