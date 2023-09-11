# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
from enum import Enum, auto

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
        items = {}
        for extension in ["out", "out.map", "out.hex"]:
            name = '%s.%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items
