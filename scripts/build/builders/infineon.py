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


class InfineonApp(Enum):
    LOCK = auto()
    LIGHT = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()

    def ExampleName(self):
        if self == InfineonApp.LOCK:
            return 'lock-app'
        elif self == InfineonApp.LIGHT:
            return 'lighting-app'
        elif self == InfineonApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == InfineonApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == InfineonApp.LOCK:
            return 'chip-psoc6-lock-example'
        elif self == InfineonApp.LIGHT:
            return 'chip-psoc6-lighting-example'
        elif self == InfineonApp.ALL_CLUSTERS:
            return 'chip-psoc6-clusters-example'
        elif self == InfineonApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-psoc6-clusters-minimal-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == InfineonApp.LOCK:
            return 'lock_app.flashbundle.txt'
        elif self == InfineonApp.ALL_CLUSTERS:
            return 'clusters_app.flashbundle.txt'
        elif self == InfineonApp.ALL_CLUSTERS_MINIMAL:
            return 'clusters_minimal_app.flashbundle.txt'
        elif self == InfineonApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'infineon/psoc6')


class InfineonBoard(Enum):
    PSOC6BOARD = 1

    def GnArgName(self):
        if self == InfineonBoard.PSOC6BOARD:
            return 'CY8CKIT-062S2-43012'


class InfineonBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: InfineonApp = InfineonApp.LOCK,
                 board: InfineonBoard = InfineonBoard.PSOC6BOARD,
                 enable_ota_requestor: bool = False,
                 update_image: bool = False,
                 enable_trustm: bool = False):
        super(InfineonBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)

        self.app = app
        self.extra_gn_options = ['psoc6_board="%s"' % board.GnArgName()]

        if enable_ota_requestor:
            self.extra_gn_options.append('chip_enable_ota_requestor=true')
        if update_image:
            self.extra_gn_options.append('build_update_image=true')
        if enable_trustm:
            self.extra_gn_options.append('chip_crypto=\"platform\"')
        if enable_trustm is False:
            self.extra_gn_options.append('chip_crypto=\"mbedtls\"')

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

    def flashbundle(self):
        with open(os.path.join(self.output_dir, self.app.FlashBundleName()), 'r') as fp:
            return {
                line.strip(): os.path.join(self.output_dir, line.strip()) for line in fp.readlines() if line.strip()
            }
