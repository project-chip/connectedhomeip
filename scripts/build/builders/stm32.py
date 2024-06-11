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

from .builder import BuilderOutput
from .gn import GnBuilder


class stm32App(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == stm32App.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == stm32App.LIGHT:
            return 'chip-stm32-lighting-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == stm32App.LIGHT:
            return 'lighting_app.out.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'stm32')


class stm32Board(Enum):
    STM32WB55XX = auto()

    def GetIC(self):
        if self == stm32Board.STM32WB55XX:
            return 'STM32WB5MM-DK'
        else:
            raise Exception('Unknown board #: %r' % self)


class stm32Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: stm32App = stm32App.LIGHT,
                 board: stm32Board = stm32Board.STM32WB55XX):
        super(stm32Builder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)

        self.board = board
        self.app = app

        stm32_chip = self.board.GetIC()
        self.extra_gn_options = ['stm32_ic_family="%s"' % stm32_chip]

        self.extra_gn_options.append('chip_config_network_layer_ble=true')
        self.extra_gn_options.append('treat_warnings_as_errors=false')

    def GnBuildArgs(self):

        return self.extra_gn_options

    def build_outputs(self):
        extensions = ["out", "out.hex"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix()}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for name in filter(None, [x.strip() for x in f.readlines()]):
                yield BuilderOutput(
                    os.path.join(self.output_dir, name),
                    os.path.join('flashbundle', name))
