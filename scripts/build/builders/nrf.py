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
from typing import Optional

from .builder import Builder, BuilderOutput


class NrfApp(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()
    SWITCH = auto()
    WINDOW_COVERING = auto()
    UNIT_TESTS = auto()

    def AppPath(self):
        if self == NrfApp.ALL_CLUSTERS:
            return 'examples/all-clusters-app'
        elif self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'examples/all-clusters-minimal-app'
        elif self == NrfApp.LIGHT:
            return 'examples/lighting-app'
        elif self == NrfApp.SWITCH:
            return 'examples/light-switch-app'
        elif self == NrfApp.LOCK:
            return 'examples/lock-app'
        elif self == NrfApp.SHELL:
            return 'examples/shell'
        elif self == NrfApp.PUMP:
            return 'examples/pump-app'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'examples/pump-controller-app'
        elif self == NrfApp.WINDOW_COVERING:
            return 'examples/window-app'
        elif self == NrfApp.UNIT_TESTS:
            return 'src/test_driver'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == NrfApp.ALL_CLUSTERS:
            return 'chip-nrf-all-clusters-example'
        elif self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-nrf-all-clusters-minimal-example'
        elif self == NrfApp.LIGHT:
            return 'chip-nrf-lighting-example'
        elif self == NrfApp.SWITCH:
            return 'chip-nrf-light-switch-example'
        elif self == NrfApp.LOCK:
            return 'chip-nrf-lock-example'
        elif self == NrfApp.SHELL:
            return 'chip-nrf-shell'
        elif self == NrfApp.PUMP:
            return 'chip-nrf-pump-example'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrf-pump-controller-example'
        elif self == NrfApp.WINDOW_COVERING:
            return 'chip-nrf-window-example'
        elif self == NrfApp.UNIT_TESTS:
            return 'chip-nrf-unit-tests'
        else:
            raise Exception('Unknown app type: %r' % self)

    def _FlashBundlePrefix(self):
        if self == NrfApp.ALL_CLUSTERS:
            return 'chip-nrfconnect-all-clusters-app-example'
        elif self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-nrfconnect-all-clusters-minimal-app-example'
        elif self == NrfApp.LIGHT:
            return 'chip-nrfconnect-lighting-example'
        elif self == NrfApp.SWITCH:
            return 'chip-nrfconnect-switch-example'
        elif self == NrfApp.LOCK:
            return 'chip-nrfconnect-lock-example'
        elif self == NrfApp.SHELL:
            return 'chip-nrfconnect-shell-example'
        elif self == NrfApp.PUMP:
            return 'chip-nrfconnect-pump-example'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrfconnect-pump-controller-example'
        elif self == NrfApp.WINDOW_COVERING:
            return 'chip-nrfconnect-window-example'
        elif self == NrfApp.UNIT_TESTS:
            raise Exception(
                'Unit tests compile natively and do not have a flashbundle')
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        '''
        Nrf build script will generate a file naming <project_name>.flashbundle.txt,
        go through the output dir to find the file and return it.
        '''
        return self._FlashBundlePrefix() + '.flashbundle.txt'


class NrfBoard(Enum):
    NRF52840DK = auto()
    NRF52840DONGLE = auto()
    NRF5340DK = auto()
    NATIVE_SIM = auto()

    def GnArgName(self):
        if self == NrfBoard.NRF52840DK:
            return 'nrf52840dk_nrf52840'
        elif self == NrfBoard.NRF52840DONGLE:
            return 'nrf52840dongle_nrf52840'
        elif self == NrfBoard.NRF5340DK:
            return 'nrf5340dk_nrf5340_cpuapp'
        elif self == NrfBoard.NATIVE_SIM:
            return 'native_sim'
        else:
            raise Exception('Unknown board type: %r' % self)


class NrfConnectBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 app: NrfApp = NrfApp.LIGHT,
                 board: NrfBoard = NrfBoard.NRF52840DK,
                 enable_rpcs: bool = False,
                 ):
        super(NrfConnectBuilder, self).__init__(root, runner)
        self.app = app
        self.board = board
        self.enable_rpcs = enable_rpcs

    def generate(self):
        if not os.path.exists(self.output_dir):
            zephyr_sdk_dir = None

            if not self._runner.dry_run:
                if 'ZEPHYR_BASE' not in os.environ:
                    raise Exception("NRF builds require ZEPHYR_BASE to be set")

                # Users are expected to set ZEPHYR_SDK_INSTALL_DIR but additionally cover the Docker
                # case by inferring ZEPHYR_SDK_INSTALL_DIR from NRF5_TOOLS_ROOT.
                if not os.environ.get('ZEPHYR_SDK_INSTALL_DIR') and not os.environ.get('NRF5_TOOLS_ROOT'):
                    raise Exception("NRF buils require ZEPHYR_SDK_INSTALL_DIR to be set")

                zephyr_base = os.environ['ZEPHYR_BASE']
                nrfconnect_sdk = os.path.dirname(zephyr_base)
                zephyr_sdk_dir = os.environ.get('ZEPHYR_SDK_INSTALL_DIR') or os.path.join(
                    os.environ['NRF5_TOOLS_ROOT'], 'zephyr-sdk-0.16.1')

                # NRF builds will both try to change .west/config in nrfconnect and
                # overall perform a git fetch on that location
                if not os.access(nrfconnect_sdk, os.W_OK):
                    raise Exception(
                        "Directory %s not writable. NRFConnect builds require updates to this directory." % nrfconnect_sdk)

                # validate the the ZEPHYR_BASE is up to date (generally the case in docker images)
                try:
                    self._Execute(
                        ['python3', 'scripts/setup/nrfconnect/update_ncs.py', '--check'])
                except Exception:
                    logging.exception('Failed to validate ZEPHYR_BASE status')
                    logging.error(
                        'To update $ZEPHYR_BASE run: python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow')

                    raise Exception('ZEPHYR_BASE validation failed')

            flags = []
            if self.enable_rpcs:
                flags.append("-DOVERLAY_CONFIG=rpc.overlay")

            if self.options.pregen_dir:
                flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

            build_flags = " -- " + " ".join(flags) if len(flags) > 0 else ""

            cmd = 'source "$ZEPHYR_BASE/zephyr-env.sh";\nexport ZEPHYR_TOOLCHAIN_VARIANT=zephyr;'

            if zephyr_sdk_dir:
                cmd += f'\nexport ZEPHYR_SDK_INSTALL_DIR={zephyr_sdk_dir};'

            cmd += '\nwest build --cmake-only -d {outdir} -b {board} --sysbuild {sourcedir}{build_flags}\n'.format(
                outdir=shlex.quote(self.output_dir),
                board=self.board.GnArgName(),
                sourcedir=shlex.quote(os.path.join(
                    self.root, self.app.AppPath(), 'nrfconnect')),
                build_flags=build_flags
            )
            self._Execute(['bash', '-c', cmd.strip()],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling NrfConnect at %s', self.output_dir)

        cmd = ['ninja', '-C', self.output_dir]

        if self.ninja_jobs is not None:
            cmd.append('-j' + str(self.ninja_jobs))

        self._Execute(cmd, title='Building ' + self.identifier)

        if self.app == NrfApp.UNIT_TESTS:
            # Note: running zephyr/zephyr.elf has the same result except it creates
            # a flash.bin in the current directory. ctest has more options and does not
            # pollute the source directory
            self._Execute(['ctest', '--build-nocmake', '-V', '--output-on-failure', '--test-dir', os.path.join(self.output_dir, 'nrfconnect'), '--no-tests=error'],
                          title='Run Tests ' + self.identifier)

    def _bundle(self):
        logging.info(f'Generating flashbundle at {self.output_dir}')

        self._Execute(['ninja', '-C', os.path.join(self.output_dir, 'nrfconnect'), 'flashing_script'],
                      title='Generating flashable files of ' + self.identifier)

    def build_outputs(self):
        yield BuilderOutput(
            os.path.join(self.output_dir, 'nrfconnect', 'zephyr', 'zephyr.elf'),
            '%s.elf' % self.app.AppNamePrefix())
        if self.options.enable_link_map_file:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'nrfconnect', 'zephyr', 'zephyr.map'),
                '%s.map' % self.app.AppNamePrefix())

    def bundle_outputs(self):
        if self.app == NrfApp.UNIT_TESTS:
            return
        with open(os.path.join(self.output_dir, 'nrfconnect', self.app.FlashBundleName())) as f:
            for line in filter(None, [x.strip() for x in f.readlines()]):
                yield BuilderOutput(os.path.join(self.output_dir, 'nrfconnect', line), line)
