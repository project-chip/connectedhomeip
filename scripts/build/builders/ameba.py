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
import shlex

from enum import Enum, auto

from .builder import Builder


class AmebaBoard(Enum):
    AMEBAD = auto()


class AmebaApp(Enum):
    ALL_CLUSTERS = auto()

    @property
    def ExampleName(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'chip-all-clusters-app'
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
        cmd = '$AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE/build.sh %s ninja %s' % (
            self.root, self.output_dir)
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
        }
