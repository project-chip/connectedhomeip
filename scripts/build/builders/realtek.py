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
            return 'matter-cli-ftd'
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
                 app: RealtekApp = RealtekApp.LIGHT):
        super(RealtekBuilder, self).__init__(root, runner)
        self.board = board
        self.app = app

    def generate(self):
        logging.info('generate %s', self.output_dir)

    def _build(self):
        cmd = 'third_party/openthread/ot-realtek/Realtek/build.sh {out_folder} {board} {app} {target} '.format(
            out_folder=self.output_dir.strip(),
            board=self.board.BoardName,
            app=self.app.ExampleName,
            target=self.app.TargetName
        )

        # <build root> <build_system> <output_directory> <application>
        cmd += ' '.join([self.root, 'ninja', self.output_dir,
                        self.app.ExampleName])

        self._Execute(['bash', '-c', cmd],
                      title='Generating ' + self.identifier)

    def build_outputs(self):
        logging.info('build_outputs %s', self.output_dir)
