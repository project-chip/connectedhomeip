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

from .gn import GnBuilder


class TizenApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == TizenApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppName(self):
        if self == TizenApp.LIGHT:
            return 'chip-lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)


class TizenBoard(Enum):
    ARM = auto()

    def TargetCpuName(self):
        if self == TizenBoard.ARM:
            return 'arm'
        else:
            raise Exception('Unknown board type: %r' % self)


class TizenBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: TizenApp = TizenApp.LIGHT,
                 board: TizenBoard = TizenBoard.ARM,
                 enable_ble: bool = True,
                 enable_wifi: bool = True,
                 use_asan: bool = False,
                 use_tsan: bool = False,
                 ):
        super(TizenBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExampleName(), 'linux'),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []

        if not enable_ble:
            self.extra_gn_options.append('chip_config_network_layer_ble=false')
        if not enable_wifi:
            self.extra_gn_options.append('chip_enable_wifi=false')
        if use_asan:
            self.extra_gn_options.append('is_asan=true')
        if use_tsan:
            raise Exception("TSAN sanitizer not supported by Tizen toolchain")

    def GnBuildArgs(self):
        if 'TIZEN_HOME' not in os.environ:
            raise Exception(
                "Environment TIZEN_HOME missing, cannot build Tizen target")
        return self.extra_gn_options + [
            'target_os="tizen"',
            'target_cpu="%s"' % self.board.TargetCpuName(),
            'sysroot="%s"' % os.environ['TIZEN_HOME'],
        ]

    def build_outputs(self):
        return {
            '%s' % self.app.AppName():
                os.path.join(self.output_dir, self.app.AppName()),
            '%s.map' % self.app.AppName():
                os.path.join(self.output_dir, '%s.map' % self.app.AppName()),
        }
