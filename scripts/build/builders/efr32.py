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


class Efr32App(Enum):
    LIGHT = auto()
    LOCK = auto()
    WINDOW_COVERING = auto()

    def ExampleName(self):
        if self == Efr32App.LIGHT:
            return 'lighting-app'
        elif self == Efr32App.LOCK:
            return 'lock-app'
        elif self == Efr32App.WINDOW_COVERING:
            return 'window-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == Efr32App.LIGHT:
            return 'chip-efr32-lighting-example'
        elif self == Efr32App.LOCK:
            return 'chip-efr32-lock-example'
        elif self == Efr32App.WINDOW_COVERING:
            return 'chip-efr32-window-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == Efr32App.LIGHT:
            return 'lighting_app.flashbundle.txt'
        elif self == Efr32App.LOCK:
            return 'lock_app.flashbundle.txt'
        elif self == Efr32App.WINDOW_COVERING:
            return 'window_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class Efr32Board(Enum):
    BRD4161A = 1

    def GnArgName(self):
        if self == Efr32Board.BRD4161A:
            return 'BRD4161A'


class Efr32Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: Efr32App = Efr32App.LIGHT,
                 board: Efr32Board = Efr32Board.BRD4161A):
        super(Efr32Builder, self).__init__(
            root=os.path.join(root, 'examples', app.ExampleName(), 'efr32'),
            runner=runner,
            output_prefix=output_prefix)

        self.app = app
        self.gn_build_args = ['efr32_board="%s"' % board.GnArgName()]

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items
