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


class NrfApp(Enum):
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()

    def ExampleName(self):
        if self == NrfApp.LIGHT:
            return 'lighting-app'
        elif self == NrfApp.LOCK:
            return 'lock-app'
        elif self == NrfApp.SHELL:
            return 'shell'
        elif self == NrfApp.PUMP:
            return 'pump-app'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'pump-controller-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == NrfApp.LIGHT:
            return 'chip-nrf-lighting-example'
        elif self == NrfApp.LOCK:
            return 'chip-nrf-lock-example'
        elif self == NrfApp.SHELL:
            return 'chip-nrf-shell'
        elif self == NrfApp.PUMP:
            return 'chip-nrf-pump-example'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrf-pump-controller-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def _FlashBundlePrefix(self):
        if self == NrfApp.LIGHT:
            return 'chip-nrfconnect-lighting-example'
        elif self == NrfApp.LOCK:
            return 'chip-nrfconnect-lock-example'
        elif self == NrfApp.SHELL:
            return 'chip-nrfconnect-shell-example'
        elif self == NrfApp.PUMP:
            return 'chip-nrfconnect-pump-example'
        elif self == NrfApp.PUMP_CONTROLLER:
            return 'chip-nrfconnect-pump-controller-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        '''Nrf build script will generate a file naming <project_name>.flashbundle.txt, go through the output dir to find the file and return it.'''
        return self._FlashBundlePrefix() + '.flashbundle.txt'


class NrfBoard(Enum):
    NRF52840 = auto()
    NRF5340 = auto()

    def GnArgName(self):
        if self == NrfBoard.NRF52840:
            return 'nrf52840dk_nrf52840'
        elif self == NrfBoard.NRF5340:
            return 'nrf5340dk_nrf5340_cpuapp'
        else:
            raise Exception('Unknown board type: %r' % self)


class NrfConnectBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: NrfApp = NrfApp.LIGHT,
                 board: NrfBoard = NrfBoard.NRF52840):
        super(NrfConnectBuilder, self).__init__(root, runner, output_prefix)
        self.app = app
        self.board = board

    def generate(self):
        if not os.path.exists(self.output_dir):
            # NRF does a in-place update  of SDK tools
            if not self._runner.dry_run:
                if 'ZEPHYR_BASE' not in os.environ:
                    raise Exception("NRF builds require ZEPHYR_BASE to be set")

                zephyr_base = os.environ['ZEPHYR_BASE']
                nrfconnect_sdk = os.path.dirname(zephyr_base)

                # NRF builds will both try to change .west/config in nrfconnect and
                # overall perform a git fetch on that location
                if not os.access(nrfconnect_sdk, os.W_OK):
                    raise Exception(
                        "Directory %s not writable. NRFConnect builds require updates to this directory." % nrfconnect_sdk)

                # validate the the ZEPHYR_BASE is up to date (generally the case in docker images)
                try:
                    self._Execute(
                        ['python3', 'scripts/setup/nrfconnect/update_ncs.py', '--check'])
                except Exception as e:
                    logging.exception('Failed to validate ZEPHYR_BASE status')
                    logging.error(
                        'To update $ZEPHYR_BASE run: python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow')

                    raise Exception('ZEPHYR_BASE validation failed')

            cmd = '''
source "$ZEPHYR_BASE/zephyr-env.sh";
export GNUARMEMB_TOOLCHAIN_PATH="$PW_PIGWEED_CIPD_INSTALL_DIR";
west build --cmake-only -d {outdir} -b {board} {sourcedir}
        '''.format(
                outdir=shlex.quote(self.output_dir),
                board=self.board.GnArgName(),
                sourcedir=shlex.quote(os.path.join(
                    self.root, 'examples', self.app.ExampleName(), 'nrfconnect'))
            ).strip()

            self._Execute(['bash', '-c', cmd],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling NrfConnect at %s', self.output_dir)

        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

    def _generate_flashbundle(self):
        logging.info(f'Generating flashbundle at {self.output_dir}')

        self._Execute(['ninja', '-C', self.output_dir, 'flashing_script'],
                      title='Generating flashable files of ' + self.identifier)

    def build_outputs(self):
        return {
            '%s.elf' % self.app.AppNamePrefix(): os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
            '%s.map' % self.app.AppNamePrefix(): os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
        }

    def flashbundle(self):
        with open(os.path.join(self.output_dir, self.app.FlashBundleName()), 'r') as fp:
            return {
                l.strip(): os.path.join(self.output_dir, l.strip()) for l in fp.readlines() if l.strip()
            }
