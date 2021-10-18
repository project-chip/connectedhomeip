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

    def FlashBundleName(self):
        return self.AppNamePrefix + '.flashbundle.txt'


def DefaultsFileName(board: Esp32Board, app: Esp32App, enable_rpcs: bool):
    if app != Esp32App.ALL_CLUSTERS:
        return 'sdkconfig.defaults'

    rpc = "_rpc" if enable_rpcs else ""
    if board == Esp32Board.DevKitC:
        return 'sdkconfig{}.defaults'.format(rpc)
    elif board == Esp32Board.M5Stack:
        return 'sdkconfig_m5stack{}.defaults'.format(rpc)
    elif board == Esp32Board.C3DevKit:
        return 'sdkconfig_c3devkit{}.defaults'.format(rpc)
    else:
        raise Exception('Unknown board type')


class Esp32Builder(Builder):

    def __init__(self,
                 root,
                 runner,
                 board: Esp32Board = Esp32Board.M5Stack,
                 app: Esp32App = Esp32App.ALL_CLUSTERS,
                 enable_rpcs: bool = False,
                 enable_ipv4: bool = True
                 ):
        super(Esp32Builder, self).__init__(root, runner)
        self.board = board
        self.app = app
        self.enable_rpcs = enable_rpcs
        self.enable_ipv4 = enable_ipv4

    def _IdfEnvExecute(self, cmd, title=None):
        self._Execute(
            ['bash', '-c', 'source $IDF_PATH/export.sh; %s' % cmd],
            title=title)

    @property
    def ExamplePath(self):
        return os.path.join('examples', self.app.ExampleName, 'esp32')

    def generate(self):
        if os.path.exists(os.path.join(self.output_dir, 'build.ninja')):
            return

        defaults = os.path.join(self.ExamplePath, DefaultsFileName(
            self.board, self.app, self.enable_rpcs))

        if not self._runner.dry_run and not os.path.exists(defaults):
            raise Exception('SDK defaults file missing: %s' % defaults)

        defaults_out = os.path.join(self.output_dir, 'sdkconfig.defaults')

        self._Execute(['mkdir', '-p', self.output_dir],
                      title='Generating ' + self.identifier)
        self._Execute(['cp', defaults, defaults_out])
        self._Execute(
            ['rm', '-f', os.path.join(self.ExamplePath, 'sdkconfig')])

        if not self.enable_ipv4:
            self._Execute(
                ['bash', '-c', 'echo CONFIG_DISABLE_IPV4=y >>%s' % shlex.quote(defaults_out)])

        cmd = "\nexport SDKCONFIG_DEFAULTS={defaults}\nidf.py -C {example_path} -B {out} reconfigure".format(
            defaults=shlex.quote(defaults_out),
            example_path=self.ExamplePath,
            out=shlex.quote(self.output_dir)
        )

        # This will do a 'cmake reconfigure' which will create ninja files without rebuilding
        self._IdfEnvExecute(cmd)

    def _build(self):
        logging.info('Compiling Esp32 at %s', self.output_dir)

        # Unfortunately sdkconfig is sticky and needs reset on every build
        self._Execute(
            ['rm', '-f', os.path.join(self.ExamplePath, 'sdkconfig')])

        defaults_out = os.path.join(self.output_dir, 'sdkconfig.defaults')

        # "ninja -C" is insufficient because sdkconfig changes on every 'config' and results
        # in a full reconfiguration with default values
        #
        # This does a regen + reconfigure.
        cmd = "\nexport SDKCONFIG_DEFAULTS={defaults}\nidf.py -C {example_path} -B {out} build".format(
            defaults=shlex.quote(defaults_out),
            example_path=self.ExamplePath,
            out=shlex.quote(self.output_dir)
        )

        self._IdfEnvExecute(cmd, title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            self.app.AppNamePrefix + '.elf':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.map'),
        }

    def flashbundle(self):
        with open(os.path.join(self.output_dir, self.app.FlashBundleName()), 'r') as fp:
            return {
                l.strip(): os.path.join(self.output_dir, l.strip()) for l in fp.readlines() if l.strip()
            }
