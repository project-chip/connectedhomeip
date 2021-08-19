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

from .gn import GnBuilder


class InfineonApp(Enum):
    LOCK = auto()

    def ExampleName(self):
        if self == InfineonApp.LOCK:
            return 'lock-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == InfineonApp.LOCK:
            return 'chip-p6-lock-example'
        else:
            raise Exception('Unknown app type: %r' % self)


class InfineonBoard(Enum):
    P6BOARD = 1

    def GnArgName(self):
        if self == InfineonBoard.P6BOARD:
            return 'CY8CKIT-062S2-43012'


class InfineonBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: InfineonApp = InfineonApp.LOCK,
                 board: InfineonBoard = InfineonBoard.P6BOARD):
        super(InfineonBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExampleName(), 'p6'),
            runner=runner,
            output_prefix=output_prefix)

        self.app = app
        self.gn_build_args = ['p6_board="%s"' % board.GnArgName()]

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
