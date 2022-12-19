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

from .builder import Builder


class AmebaBoard(Enum):
    AMEBAD = auto()


class AmebaApp(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    LIGHT = auto()
    PIGWEED = auto()
    LIGHT_SWITCH = auto()

    @property
    def ExampleName(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == AmebaApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        elif self == AmebaApp.LIGHT:
            return 'lighting-app'
        elif self == AmebaApp.LIGHT_SWITCH:
            return 'light-switch-app'
        elif self == AmebaApp.PIGWEED:
            return 'pigweed-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'chip-ameba-all-clusters-app'
        elif self == AmebaApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-ameba-all-clusters-minimal-app'
        elif self == AmebaApp.LIGHT:
            return 'chip-ameba-lighting-app'
        elif self == AmebaApp.LIGHT_SWITCH:
            return 'chip-ameba-light-switch-app'
        elif self == AmebaApp.PIGWEED:
            return 'chip-ameba-pigweed-app'
        else:
            raise Exception('Unknown app type: %r' % self)


class AmebaBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 board: AmebaBoard = AmebaBoard.AMEBAD,
                 app: AmebaApp = AmebaApp.ALL_CLUSTERS):
        super(AmebaBuilder, self).__init__(root, runner)
        self.board = board
        self.app = app

    def generate(self):
        cmd = '$AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE/build.sh '
        if self.app.ExampleName == 'pigweed-app':
            # rpc flag: -r
            cmd += '-r '

        # <build root> <build_system> <output_directory> <application>
        cmd += ' '.join([self.root, 'ninja', self.output_dir,
                        self.app.ExampleName])

        self._Execute(['bash', '-c', cmd],
                      title='Generating ' + self.identifier)

    def _build(self):
        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            self.app.AppNamePrefix + '.axf':
                os.path.join(self.output_dir, 'asdk', 'target_image2.axf'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir, 'asdk', 'target_image2.map'),
            'km0_boot_all.bin':
                os.path.join(self.output_dir, 'asdk',
                             'bootloader', 'km0_boot_all.bin'),
            'km4_boot_all.bin':
                os.path.join(self.output_dir, 'asdk',
                             'bootloader', 'km4_boot_all.bin'),
            'km0_km4_image2.bin':
                os.path.join(self.output_dir, 'asdk',
                             'image', 'km0_km4_image2.bin'),
        }
