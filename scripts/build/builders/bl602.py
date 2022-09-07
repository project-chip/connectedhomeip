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


class Bl602App(Enum):
    LOCK = auto()
    LIGHT = auto()
    ALL_CLUSTERS = auto()

    def ExampleName(self):
        if self == Bl602App.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == Bl602App.LIGHT:
            return 'chip-bl602-lighting-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == Bl602App.LOCK:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class Bl602Board(Enum):
    BL602BOARD = 1

    def GnArgName(self):
        if self == Bl602Board.BL602BOARD:
            return 'BL-HWC-G1'


class Bl602Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: Bl602App = Bl602App.LIGHT,
                 board: Bl602Board = Bl602Board.BL602BOARD):
        super(Bl602Builder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', 'bl602'),
            runner=runner)

        self.argsOpt = []

        toolchain = os.path.join(root, '../../examples/platform/bouffalolab/common/toolchain')
        toolchain = 'custom_toolchain="{}:riscv_gcc"'.format(toolchain)
        if toolchain:
            self.argsOpt.append(toolchain)

        self.app = app
        self.board = board

    def GnBuildArgs(self):
        return self.argsOpt + ['bl602_board="%s"' % self.board.GnArgName()]

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
