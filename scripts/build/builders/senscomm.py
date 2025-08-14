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


class SenscommApp(Enum):
    LOCK = auto()
    LIGHT = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()

    def ExampleName(self):
        if self == SenscommApp.LOCK:
            return 'lock-app'
        elif self == SenscommApp.LIGHT:
            return 'lighting-app'
        elif self == SenscommApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == SenscommApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == SenscommApp.LOCK:
            return 'chip-psoc6-lock-example'
        elif self == SenscommApp.LIGHT:
            return 'chip-psoc6-lighting-example'
        elif self == SenscommApp.ALL_CLUSTERS:
            return 'chip-psoc6-clusters-example'
        elif self == SenscommApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-psoc6-clusters-minimal-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == SenscommApp.LOCK:
            return 'lock_app.flashbundle.txt'
        elif self == SenscommApp.ALL_CLUSTERS:
            return 'clusters_app.flashbundle.txt'
        elif self == SenscommApp.ALL_CLUSTERS_MINIMAL:
            return 'clusters_minimal_app.flashbundle.txt'
        elif self == SenscommApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'senscomm/scm1612s')


class SenscommBoard(Enum):
    EVBQFN40 = auto()
    MOD1212A = auto()
    MOD1212G = auto()
    MOD1614H = auto()

    def GnArgName(self):
        if self == SenscommBoard.EVBQFN40:
            return 'SCM1612S-EVB-QFN40'
        elif self == SenscommBoard.MOD1212A:
            return 'SCM1612S-MOD-1212A'
        elif self == SenscommBoard.MOD1212G:
            return 'SCM1612S-MOD-1212G'
        elif self == SenscommBoard.MOD1614H:
            return 'SCM1612S-MOD-1614H'


class SenscommBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: SenscommApp = SenscommApp.LOCK,
                 board: SenscommBoard = SenscommBoard.EVBQFN40):
        super(SenscommBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)

        self.app = app
        self.extra_gn_options = ['scm1612s_board="%s"' % board.GnArgName()]

    def GnBuildArgs(self):
        return self.extra_gn_options

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
