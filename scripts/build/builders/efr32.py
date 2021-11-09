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
    UNIT_TEST = auto()

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
        elif self == Efr32App.UNIT_TEST:
            return 'chip-efr32-device_tests'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == Efr32App.LIGHT:
            return 'lighting_app.flashbundle.txt'
        elif self == Efr32App.LOCK:
            return 'lock_app.flashbundle.txt'
        elif self == Efr32App.WINDOW_COVERING:
            return 'window_app.flashbundle.txt'
        elif self == Efr32App.UNIT_TEST:
            return 'efr32_device_tests.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        if self == Efr32App.UNIT_TEST:
            return os.path.join(root, 'src', 'test_driver', 'efr32')
        else:
            return os.path.join(root, 'examples', self.ExampleName(), 'efr32')


class Efr32Board(Enum):
    BRD4161A = 1

    def GnArgName(self):
        if self == Efr32Board.BRD4161A:
            return 'BRD4161A'


class Efr32Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: Efr32App = Efr32App.LIGHT,
                 board: Efr32Board = Efr32Board.BRD4161A,
                 enable_rpcs: bool = False):
        super(Efr32Builder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.board = board
        self.enable_rpcs = enable_rpcs

    def GnBuildArgs(self):
        args = ['efr32_board="%s"' % self.board.GnArgName()]
        if self.enable_rpcs:
            args.append('import("//with_pw_rpc.gni")')
        return args

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        if self.app == Efr32App.UNIT_TEST:
            # Include test runner python wheels
            items["chip_nl_test_runner_wheels"] = os.path.join(
                self.output_dir, 'chip_nl_test_runner_wheels')

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items
