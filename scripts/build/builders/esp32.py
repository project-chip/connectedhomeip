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


class Esp32Board(Enum):
    DevKitC = auto()
    M5Stack = auto()
    C3DevKit = auto()


class Esp32App(Enum):
    ALL_CLUSTERS = auto()
    LOCK = auto()
    SHELL = auto()
    BRIDGE = auto()
    TEMPERATURE_MEASUREMENT = auto()

    @property
    def ExampleName(self):
        if self == Esp32App.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == Esp32App.LOCK:
            return 'lock-app'
        elif self == Esp32App.SHELL:
            return 'shell'
        elif self == Esp32App.BRIDGE:
            return 'bridge-app'
        elif self == Esp32App.TEMPERATURE_MEASUREMENT:
            return 'temperature-measurement-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == Esp32App.ALL_CLUSTERS:
            return 'chip-all-clusters-app'
        elif self == Esp32App.LOCK:
            return 'chip-lock-app'
        elif self == Esp32App.SHELL:
            return 'chip-shell'
        elif self == Esp32App.BRIDGE:
            return 'chip-bridge-app'
        elif self == Esp32App.TEMPERATURE_MEASUREMENT:
            return 'chip-temperature-measurement-app'
        else:
            raise Exception('Unknown app type: %r' % self)


def DefaultsFileName(board: Esp32Board, app: Esp32App):
    if app != Esp32App.ALL_CLUSTERS:
        # only all-clusters has a specific defaults name
        return None

    if board == Esp32Board.DevKitC:
        return 'sdkconfig.defaults'
    elif board == Esp32Board.M5Stack:
        return 'sdkconfig_m5stack.defaults'
    elif board == Esp32Board.C3DevKit:
        return 'sdkconfig_c3devkit.defaults'
    else:
        raise Exception('Unknown board type')


class Esp32Builder(Builder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 board: Esp32Board = Esp32Board.M5Stack,
                 app: Esp32App = Esp32App.ALL_CLUSTERS):
        super(Esp32Builder, self).__init__(root, runner, output_prefix)
        self.board = board
        self.app = app

    def _IdfEnvExecute(self, cmd, cwd=None, title=None):
        self._Execute(
            ['bash', '-c', 'source $IDF_PATH/export.sh; %s' % cmd],
            cwd=cwd,
            title=title)

    def generate(self):
        if os.path.exists(os.path.join(self.output_dir, 'build.ninja')):
            return

        defaults = DefaultsFileName(self.board, self.app)

        cmd = 'idf.py'

        if defaults:
            cmd += " -D SDKCONFIG_DEFAULTS='%s'" % defaults

        cmd += ' -C examples/%s/esp32 -B %s reconfigure' % (
            self.app.ExampleName, shlex.quote(self.output_dir))

        # This will do a 'cmake reconfigure' which will create ninja files without rebuilding
        self._IdfEnvExecute(
            cmd, cwd=self.root, title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Esp32 at %s', self.output_dir)

        self._IdfEnvExecute(
            "ninja -C '%s'" % self.output_dir, title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            self.app.AppNamePrefix + '.elf':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.map'),
        }
