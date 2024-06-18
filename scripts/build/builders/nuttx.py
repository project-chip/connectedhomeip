# Copyright (c) 2024 Project CHIP Authors
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

from .builder import BuilderOutput
from .gn import Builder


class NuttXApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == NuttXApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == NuttXApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        else:
            raise Exception('Unknown app type: %r' % self)


class NuttXBoard(Enum):
    SIM = auto()


def NuttXTarget(board, app):
    if board == NuttXBoard.SIM:
        if app == NuttXApp.LIGHT:
            return 'sim:matter'
    return 'none'


class NuttXBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 app: NuttXApp = NuttXApp.LIGHT,
                 board: NuttXBoard = NuttXBoard.SIM,
                 ):

        nuttx_chip = 'nuttx'

        super(NuttXBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), nuttx_chip),
            runner=runner
        )

        self.chip_name = nuttx_chip
        self.app = app
        self.board = board

    def generate(self):
        self._Execute(['mkdir', '-p', self.output_dir],
                      title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling NuttX %s at %s, ',
                     NuttXTarget(self.board, self.app), self.output_dir)
        nuttx_dir = os.path.join(os.sep, 'opt', 'nuttx', 'nuttx')

        self._Execute(['cmake', '-S', nuttx_dir, '-B', self.output_dir, '-DCHIP_ROOT=' + os.getenv('PW_PROJECT_ROOT'),
                       '-DBOARD_CONFIG=' + NuttXTarget(self.board, self.app),
                       '-DCMAKE_C_COMPILER=/opt/nuttx/gcc-13/bin/gcc',
                       '-DCMAKE_CXX_COMPILER=/opt/nuttx/gcc-13/bin/g++',
                       '-GNinja'],
                      title='Building ' + self.identifier)
        self._Execute(['cmake', '--build', self.output_dir])

    def build_outputs(self):
        logging.info('Compiling outputs NuttX at %s', self.output_dir)
        extensions = ["out"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix(self.chip_name)}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
