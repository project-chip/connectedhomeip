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
                 enable_rpcs: bool = False,
                 module_type: str = 'simulator',
                 baudrate=2000000,
                 enable_shell: bool = False,
                 enable_cdc: bool = False,
                 enable_resetCnt: bool = False,
                 enable_rotating_device_id: bool = False,
                 enable_ethernet: bool = False,
                 enable_wifi: bool = False,
                 enable_thread: bool = False,
                 enable_frame_ptr: bool = False,
                 enable_heap_monitoring: bool = False,
                 use_matter_openthread: bool = False,
                 ):

        nuttx_chip = 'nuttx'

        super(NuttXBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), nuttx_chip),
            runner=runner
        )

        self.argsOpt = []
        self.chip_name = nuttx_chip
        self.enable_frame_ptr = enable_frame_ptr
        self.app = app
        self.board = board

    def generate(self):
        self._Execute(['mkdir', '-p', self.output_dir],
                      title='Generating ' + self.identifier)

        nuttx_apps_dir = os.path.join(os.sep, 'opt', 'nuttx', 'apps')
        chip_root = os.getenv('PW_PROJECT_ROOT')
        # Link connectedhomeip as a third-party library to the NuttX SDK.
        self._Execute(['ln', '-s', chip_root, os.path.join(nuttx_apps_dir, 'netutils', 'connectedhomeip', 'connectedhomeip')])

        # The gcc compiler does not generate a cc program during compilation, it is created using the ln command.
        gcc13_bin_path = os.path.join(os.sep, 'opt', 'nuttx', 'gcc-13', 'bin')
        self._Execute(['ln', '-s', os.path.join(gcc13_bin_path, 'gcc'), os.path.join(gcc13_bin_path, 'cc')])

    def _build(self):
        logging.info('Compiling NuttX %s at %s, ',
                     NuttXTarget(self.board, self.app), self.output_dir)
        nuttx_dir = os.path.join(os.sep, 'opt', 'nuttx', 'nuttx')

        os.environ['PATH'] = os.path.join(os.sep, 'opt', 'nuttx', 'gcc-13', 'bin') + ':' + os.environ.get('PATH')

        self._Execute(['cmake', '-S', nuttx_dir, '-B', self.output_dir, '-DBOARD_CONFIG=' + NuttXTarget(self.board, self.app), '-GNinja'],
                      title='Building ' + self.identifier)
        self._Execute(['cmake', '--build', self.output_dir])

    def build_outputs(self):
        logging.info('Compiling outputs NuttX at %s', self.output_dir)
        items = {
            '%s.out' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix(self.chip_name)),
            '%s.out.map' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix(self.chip_name)),
        }

        return items
