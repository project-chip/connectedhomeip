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
from typing import Optional

from .gn import GnBuilder


class TIApp(Enum):
    LOCK = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    LIGHTING = auto()
    SHELL = auto()

    def ExampleName(self):
        if self == TIApp.LOCK:
            return 'lock-app'
        elif self == TIApp.PUMP:
            return 'pump-app'
        elif self == TIApp.PUMP_CONTROLLER:
            return 'pump-controller-app'
        elif self == TIApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == TIApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        elif self == TIApp.LIGHTING:
            return 'lighting-app'
        elif self == TIApp.SHELL:
            return 'shell'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, board):
        if self == TIApp.LOCK:
            return f'chip-{board.BoardName()}-lock-example'
        elif self == TIApp.PUMP:
            return f'chip-{board.BoardName()}-pump-example'
        elif self == TIApp.PUMP_CONTROLLER:
            return f'chip-{board.BoardName()}-pump-controller-example'
        elif self == TIApp.ALL_CLUSTERS:
            return f'chip-{board.BoardName()}-all-clusters-example'
        elif self == TIApp.ALL_CLUSTERS_MINIMAL:
            return f'chip-{board.BoardName()}-all-clusters-minimal-example'
        elif self == TIApp.LIGHTING:
            return f'chip-{board.BoardName()}-lighting-example'
        elif self == TIApp.SHELL:
            return f'chip-{board.BoardName()}-shell-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root, board):
        return os.path.join(root, 'examples', self.ExampleName(), board.FamilyName())


class TIBoard(Enum):
    LP_CC2652R7 = auto()
    LP_EM_CC1354P10_6 = auto()

    def BoardName(self):
        if self == TIBoard.LP_CC2652R7:
            return 'LP_CC2652R7'
        elif self == TIBoard.LP_EM_CC1354P10_6:
            return 'LP_EM_CC1354P10_6'
        else:
            raise Exception('Unknown board type: %r' % self)

    def FamilyName(self):
        if self == TIBoard.LP_CC2652R7:
            return 'cc13x2x7_26x2x7'
        elif self == TIBoard.LP_EM_CC1354P10_6:
            return 'cc13x4_26x4'
        else:
            raise Exception('Unknown board type: %r' % self)


class TIBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 board=TIBoard.LP_CC2652R7,
                 app: TIApp = TIApp.LOCK,
                 openthread_ftd: Optional[bool] = None):
        super(TIBuilder, self).__init__(
            root=app.BuildRoot(root, board),
            runner=runner)
        self.code_root = root
        self.app = app
        self.board = board
        self.openthread_ftd = openthread_ftd

    def GnBuildArgs(self):
        args = [
            'ti_sysconfig_root="%s"' % os.environ['TI_SYSCONFIG_ROOT'],
            'ti_simplelink_board="%s"' % self.board.BoardName(),
        ]

        if self.openthread_ftd:
            args.append('chip_openthread_ftd=true')
            args.append('chip_progress_logging=false')
        elif self.openthread_ftd is not None:
            args.append('chip_openthread_ftd=false')

        return args

    def build_outputs(self):
        items = {}
        if (self.board == TIBoard.LP_CC2652R7):
            if (self.app == TIApp.LOCK
                    or self.app == TIApp.PUMP
                    or self.app == TIApp.PUMP_CONTROLLER):
                extensions = [".out", ".bin", ".out.map", "-bim.hex"]

            else:
                extensions = [".out", ".out.map"]

        else:
            extensions = [".out", ".out.map"]

        for extension in extensions:
            name = '%s%s' % (self.app.AppNamePrefix(self.board), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
