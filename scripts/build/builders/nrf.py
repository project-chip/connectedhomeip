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
        if self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'examples/all-clusters-minimal-app'
        if self == NrfApp.LIGHT:
            return 'examples/lighting-app'
        if self == NrfApp.SWITCH:
            return 'examples/light-switch-app'
        if self == NrfApp.LOCK:
            return 'examples/lock-app'
        if self == NrfApp.SHELL:
            return 'examples/shell'
        if self == NrfApp.PUMP:
            return 'examples/pump-app'
        if self == NrfApp.PUMP_CONTROLLER:
            return 'examples/pump-controller-app'
        if self == NrfApp.WINDOW_COVERING:
            return 'examples/window-app'
        if self == NrfApp.UNIT_TESTS:
            return 'src/test_driver'
        raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == NrfApp.ALL_CLUSTERS:
            return 'chip-nrf-all-clusters-example'
        if self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-nrf-all-clusters-minimal-example'
        if self == NrfApp.LIGHT:
            return 'chip-nrf-lighting-example'
        if self == NrfApp.SWITCH:
            return 'chip-nrf-light-switch-example'
        if self == NrfApp.LOCK:
            return 'chip-nrf-lock-example'
        if self == NrfApp.SHELL:
            return 'chip-nrf-shell'
        if self == NrfApp.PUMP:
            return 'chip-nrf-pump-example'
        if self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrf-pump-controller-example'
        if self == NrfApp.WINDOW_COVERING:
            return 'chip-nrf-window-example'
        if self == NrfApp.UNIT_TESTS:
            return 'chip-nrf-unit-tests'
        raise Exception('Unknown app type: %r' % self)

    def _FlashBundlePrefix(self):
        if self == NrfApp.ALL_CLUSTERS:
            return 'chip-nrfconnect-all-clusters-app-example'
        if self == NrfApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-nrfconnect-all-clusters-minimal-app-example'
        if self == NrfApp.LIGHT:
            return 'chip-nrfconnect-lighting-example'
        if self == NrfApp.SWITCH:
            return 'chip-nrfconnect-switch-example'
        if self == NrfApp.LOCK:
            return 'chip-nrfconnect-lock-example'
        if self == NrfApp.SHELL:
            return 'chip-nrfconnect-shell-example'
        if self == NrfApp.PUMP:
            return 'chip-nrfconnect-pump-example'
        if self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrfconnect-pump-controller-example'
        if self == NrfApp.WINDOW_COVERING:
            return 'chip-nrfconnect-window-example'
        if self == NrfApp.UNIT_TESTS:
            raise Exception(
                'Unit tests compile natively and do not have a flashbundle')
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
            return 'nrf52840dk/nrf52840'
        if self == NrfBoard.NRF52840DONGLE:
            return 'nrf52840dongle/nrf52840'
        if self == NrfBoard.NRF5340DK:
            return 'nrf5340dk/nrf5340cpuapp'
        if self == NrfBoard.NATIVE_SIM:
            return 'native_sim'
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

    def _check_ncs_version(self):
        # validate the the ZEPHYR_BASE is up to date (generally the case in docker images)
        try:
            self._Execute(
                ['python3', 'scripts/setup/nrfconnect/update_ncs.py', '--check'])
        except Exception:
            logging.exception('Failed to validate ZEPHYR_BASE status')
            logging.error(
                'To update $ZEPHYR_BASE run: python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow')

            raise Exception('ZEPHYR_BASE validation failed')

    def _prepare_environment(self):
        # Source the zephyr-env.sh script to set up the environment
        # The zephyr-env.sh script changes the python environment, so we need to
        # source the activate.sh script after zephyr-env.sh to ensure that the
        # all python packages and dependencies are available.
        return 'source "$ZEPHYR_BASE/zephyr-env.sh";\nsource scripts/activate.sh;\n'

    def _get_build_flags(self):
        flags = []
        if self.enable_rpcs:
            flags.append("-DOVERLAY_CONFIG=rpc.overlay")

        if self.options.pregen_dir:
            flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        return " -- " + " ".join(flags) if len(flags) > 0 else ""


    def generate(self):
        if not os.path.exists(self.output_dir):
            if not self._runner.dry_run:
                self._check_ncs_version()

                zephyr_base = os.environ['ZEPHYR_BASE']
                nrfconnect_sdk = os.path.dirname(zephyr_base)

                # NRF builds will both try to change .west/config in nrfconnect and
                # overall perform a git fetch on that location
                if not os.access(nrfconnect_sdk, os.W_OK):
                    raise Exception(
                        "Directory %s not writable. NRFConnect builds require updates to this directory." % nrfconnect_sdk)

            cmd = self._prepare_environment()

            cmd += 'west build --cmake-only -d {outdir} -b {board} --sysbuild {sourcedir}{build_flags}\n'.format(
                outdir=shlex.quote(self.output_dir),
                board=self.board.GnArgName(),
                sourcedir=shlex.quote(os.path.join(
                    self.root, self.app.AppPath(), 'nrfconnect')),
                build_flags=self._get_build_flags()
            )
            self._Execute(['bash', '-c', cmd.strip()],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling NrfConnect at %s', self.output_dir)

        cmd = self._prepare_environment()
        cmd += f'ninja -C {self.output_dir}'

        if self.ninja_jobs is not None:
            cmd += '-j' + str(self.ninja_jobs)

        self._Execute(['bash', '-c', cmd.strip()], title='Building ' + self.identifier)

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
