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
    QEMU = auto()


class Esp32App(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    BRIDGE = auto()
    TEMPERATURE_MEASUREMENT = auto()
    TESTS = auto()
    OTA_REQUESTOR = auto()
    OTA_PROVIDER = auto()

    @property
    def ExamplePath(self):
        if self == Esp32App.ALL_CLUSTERS:
            return 'examples/all-clusters-app'
        elif self == Esp32App.ALL_CLUSTERS_MINIMAL:
            return 'examples/all-clusters-minimal-app'
        elif self == Esp32App.LIGHT:
            return 'examples/lighting-app'
        elif self == Esp32App.LOCK:
            return 'examples/lock-app'
        elif self == Esp32App.SHELL:
            return 'examples/shell'
        elif self == Esp32App.BRIDGE:
            return 'examples/bridge-app'
        elif self == Esp32App.TEMPERATURE_MEASUREMENT:
            return 'examples/temperature-measurement-app'
        elif self == Esp32App.OTA_REQUESTOR:
            return 'examples/ota-requestor-app'
        elif self == Esp32App.OTA_PROVIDER:
            return 'examples/ota-provider-app'
        elif self == Esp32App.TESTS:
            return 'src/test_driver'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == Esp32App.ALL_CLUSTERS:
            return 'chip-all-clusters-app'
        elif self == Esp32App.ALL_CLUSTERS_MINIMAL:
            return 'chip-all-clusters-minimal-app'
        elif self == Esp32App.LIGHT:
            return 'chip-lighting-app'
        elif self == Esp32App.LOCK:
            return 'chip-lock-app'
        elif self == Esp32App.SHELL:
            return 'chip-shell'
        elif self == Esp32App.BRIDGE:
            return 'chip-bridge-app'
        elif self == Esp32App.TEMPERATURE_MEASUREMENT:
            return 'chip-temperature-measurement-app'
        elif self == Esp32App.OTA_REQUESTOR:
            return 'chip-ota-requestor-app'
        elif self == Esp32App.OTA_PROVIDER:
            return 'chip-ota-provider-app'
        elif self == Esp32App.TESTS:
            return None
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def FlashBundleName(self):
        if not self.AppNamePrefix:
            return None

        return self.AppNamePrefix + '.flashbundle.txt'

    def IsCompatible(self, board: Esp32Board):
        if board == Esp32Board.QEMU:
            return self == Esp32App.TESTS
        elif board == Esp32Board.M5Stack:
            return self == Esp32App.ALL_CLUSTERS or self == Esp32App.ALL_CLUSTERS_MINIMAL or self == Esp32App.OTA_REQUESTOR or self == Esp32App.OTA_PROVIDER
        elif board == Esp32Board.C3DevKit:
            return self == Esp32App.ALL_CLUSTERS or self == Esp32App.ALL_CLUSTERS_MINIMAL
        else:
            return (board == Esp32Board.DevKitC) and (self != Esp32App.TESTS)


def DefaultsFileName(board: Esp32Board, app: Esp32App, enable_rpcs: bool):
    rpc_enabled_apps = [Esp32App.ALL_CLUSTERS,
                        Esp32App.ALL_CLUSTERS_MINIMAL,
                        Esp32App.LIGHT,
                        Esp32App.OTA_REQUESTOR,
                        Esp32App.OTA_PROVIDER,
                        Esp32App.TEMPERATURE_MEASUREMENT]
    if app == Esp32App.TESTS:
        return 'sdkconfig_qemu.defaults'
    elif app not in rpc_enabled_apps:
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

        if not app.IsCompatible(board):
            raise Exception(
                "Incompatible app/board combination: %r and %r", app, board)

    def _IdfEnvExecute(self, cmd, title=None):
        # Run activate.sh after export.sh to ensure using the chip environment.
        self._Execute(
            ['bash', '-c', 'source $IDF_PATH/export.sh; source scripts/activate.sh; %s' % cmd],
            title=title)

    @property
    def ExamplePath(self):
        return os.path.join(self.app.ExamplePath, 'esp32')

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
                ['bash', '-c', 'echo -e "\\nCONFIG_DISABLE_IPV4=y\\n" >>%s' % shlex.quote(defaults_out)])

        cmake_flags = []

        if self.options.pregen_dir:
            cmake_flags.append(
                f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        cmake_args = ['-C', self.ExamplePath, '-B',
                      shlex.quote(self.output_dir)] + cmake_flags

        cmake_args = " ".join(cmake_args)
        defaults = shlex.quote(defaults_out)

        cmd = f"\nexport SDKCONFIG_DEFAULTS={defaults}\nidf.py {cmake_args} reconfigure"

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
        if self.app == Esp32App.TESTS:
            # Include the runnable image names as artifacts
            result = dict()
            with open(os.path.join(self.output_dir, 'test_images.txt'), 'rt') as f:
                for name in f.readlines():
                    name = name.strip()
                    result[name] = os.path.join(self.output_dir, name)

            return result

        return {
            self.app.AppNamePrefix + '.elf':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.map'),
        }

    def flashbundle(self):
        if not self.app.FlashBundleName:
            return {}

        with open(os.path.join(self.output_dir, self.app.FlashBundleName), 'r') as fp:
            return {
                l.strip(): os.path.join(self.output_dir, l.strip()) for l in fp.readlines() if l.strip()
            }
