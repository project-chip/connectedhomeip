# Copyright (c) 2022 Project CHIP Authors
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


class TelinkApp(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    BRIDGE = auto()
    CONTACT_SENSOR = auto()
    LIGHT = auto()
    SWITCH = auto()
    LOCK = auto()
    OTA_REQUESTOR = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()
    TEMPERATURE_MEASUREMENT = auto()
    THERMOSTAT = auto()
    WINDOW_COVERING = auto()

    def ExampleName(self):
        if self == TelinkApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == TelinkApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        elif self == TelinkApp.BRIDGE:
            return 'bridge-app'
        elif self == TelinkApp.CONTACT_SENSOR:
            return 'contact-sensor-app'
        elif self == TelinkApp.LIGHT:
            return 'lighting-app'
        elif self == TelinkApp.SWITCH:
            return 'light-switch-app'
        elif self == TelinkApp.LOCK:
            return 'lock-app'
        elif self == TelinkApp.OTA_REQUESTOR:
            return 'ota-requestor-app'
        elif self == TelinkApp.PUMP:
            return 'pump-app'
        elif self == TelinkApp.PUMP_CONTROLLER:
            return 'pump-controller-app'
        elif self == TelinkApp.TEMPERATURE_MEASUREMENT:
            return 'temperature-measurement-app'
        elif self == TelinkApp.THERMOSTAT:
            return 'thermostat'
        elif self == TelinkApp.WINDOW_COVERING:
            return 'window-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == TelinkApp.ALL_CLUSTERS:
            return 'chip-telink-all-clusters-example'
        elif self == TelinkApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-telink-all-clusters-minimal-example'
        elif self == TelinkApp.BRIDGE:
            return 'chip-telink-bridge-example'
        elif self == TelinkApp.CONTACT_SENSOR:
            return 'chip-telink-contact-sensor-example'
        elif self == TelinkApp.LIGHT:
            return 'chip-telink-lighting-example'
        elif self == TelinkApp.SWITCH:
            return 'chip-telink-light-switch-example'
        elif self == TelinkApp.LOCK:
            return 'chip-telink-lock-example'
        elif self == TelinkApp.OTA_REQUESTOR:
            return 'chip-telink-ota-requestor-example'
        elif self == TelinkApp.PUMP:
            return 'chip-telink-pump-example'
        elif self == TelinkApp.PUMP_CONTROLLER:
            return 'chip-telink-pump-controller-example'
        elif self == TelinkApp.TEMPERATURE_MEASUREMENT:
            return 'chip-telink-temperature-measurement-example'
        elif self == TelinkApp.THERMOSTAT:
            return 'chip-telink-thermostat-example'
        elif self == TelinkApp.WINDOW_COVERING:
            return 'chip-telink-window-example'
        else:
            raise Exception('Unknown app type: %r' % self)


class TelinkBoard(Enum):
    TLSR9518ADK80D = auto()

    def GnArgName(self):
        if self == TelinkBoard.TLSR9518ADK80D:
            return 'tlsr9518adk80d'
        else:
            raise Exception('Unknown board type: %r' % self)


class TelinkBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 app: TelinkApp = TelinkApp,
                 board: TelinkBoard = TelinkBoard.TLSR9518ADK80D,
                 enable_shell: bool = False,
                 enable_rpcs: bool = False,
                 enable_factory_data: bool = False):
        super(TelinkBuilder, self).__init__(root, runner)
        self.app = app
        self.board = board
        self.enable_shell = enable_shell
        self.enable_rpcs = enable_rpcs
        self.enable_factory_data = enable_factory_data

    def get_cmd_prefixes(self):
        if not self._runner.dry_run:
            # Zephyr base
            if 'TELINK_ZEPHYR_BASE' not in os.environ:
                raise Exception("Telink builds require TELINK_ZEPHYR_BASE")

        cmd = 'export ZEPHYR_TOOLCHAIN_VARIANT=zephyr\n'
        cmd += 'export ZEPHYR_BASE="$TELINK_ZEPHYR_BASE"\n'

        if 'TELINK_ZEPHYR_SDK_DIR' in os.environ:
            cmd += 'export ZEPHYR_SDK_INSTALL_DIR="$TELINK_ZEPHYR_SDK_DIR"\n'

        return cmd

    def generate(self):
        if os.path.exists(self.output_dir):
            return

        flags = []
        if self.enable_shell:
            flags.append("-DOVERLAY_CONFIG=shell.overlay")

        if self.enable_rpcs:
            flags.append("-DOVERLAY_CONFIG=rpc.overlay")

        if self.enable_factory_data:
            flags.append("-DOVERLAY_CONFIG=factory_data.overlay")

        if self.options.pregen_dir:
            flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        build_flags = " -- " + " ".join(flags) if len(flags) > 0 else ""

        cmd = self.get_cmd_prefixes()
        cmd += '''
source "$ZEPHYR_BASE/zephyr-env.sh";
west build --cmake-only -d {outdir} -b {board} {sourcedir}{build_flags}
        '''.format(
            outdir=shlex.quote(self.output_dir),
            board=self.board.GnArgName(),
            sourcedir=shlex.quote(os.path.join(self.root, 'examples', self.app.ExampleName(), 'telink')),
            build_flags=build_flags).strip()

        self._Execute(['bash', '-c', cmd],
                      title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Telink at %s', self.output_dir)

        cmd = self.get_cmd_prefixes() + ("ninja -C %s" % self.output_dir)

        self._Execute(['bash', '-c', cmd], title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            '%s.elf' %
            self.app.AppNamePrefix(): os.path.join(
                self.output_dir,
                'zephyr',
                'zephyr.elf'),
            '%s.map' %
            self.app.AppNamePrefix(): os.path.join(
                self.output_dir,
                'zephyr',
                'zephyr.map'),
        }
