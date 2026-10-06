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

import glob
import logging
import os
import shlex
import sys
from enum import Enum, auto

from runner.runner import Runner

from .builder import Builder, BuilderOutput, OutDirLock, lock_output_dir

log = logging.getLogger(__name__)


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
        raise Exception(f'Unknown app type: {self!r}')

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
        raise Exception(f'Unknown app type: {self!r}')

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
        raise Exception(f'Unknown app type: {self!r}')

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
    NRF54L15DK = auto()
    NRF54L15TAG = auto()
    NRF54LM20DK = auto()
    NATIVE_SIM = auto()

    def GnArgName(self):
        if self == NrfBoard.NRF52840DK:
            return 'nrf52840dk/nrf52840'
        if self == NrfBoard.NRF52840DONGLE:
            return 'nrf52840dongle/nrf52840'
        if self == NrfBoard.NRF5340DK:
            return 'nrf5340dk/nrf5340/cpuapp'
        if self == NrfBoard.NRF54L15DK:
            return 'nrf54l15dk/nrf54l15/cpuapp'
        if self == NrfBoard.NRF54L15TAG:
            return 'nrf54l15t/nrf54l15/cpuapp'
        if self == NrfBoard.NRF54LM20DK:
            return 'nrf54lm20dk/nrf54lm20b/cpuapp'
        if self == NrfBoard.NATIVE_SIM:
            return 'native_sim'
        raise Exception(f'Unknown board type: {self!r}')


class NrfConnectBuilder(Builder):

    def __init__(self,
                 root: str,
                 runner: Runner,
                 output_dir_lock: OutDirLock,
                 app: NrfApp = NrfApp.LIGHT,
                 board: NrfBoard = NrfBoard.NRF52840DK,
                 enable_rpcs: bool = False,
                 enable_wifi: bool = False,
                 enable_bledfu: bool = False,
                 enable_release: bool = False,
                 ):
        super().__init__(root, runner, output_dir_lock)
        self.app = app
        self.board = board
        self.enable_rpcs = enable_rpcs
        self.enable_wifi = enable_wifi
        self.enable_bledfu = enable_bledfu
        self.enable_release = enable_release

    def _run_in_nrfconnect_env(self, script: str, title: str = None):
        cmd = self._prepare_environment() + script
        self._Execute(['bash', '-c', cmd.strip()], title=title)

    def _validate_ncs_environment(self):
        # validate the the ZEPHYR_BASE is up to date (generally the case in docker images)
        try:
            self._run_in_nrfconnect_env(
                'python3 scripts/setup/nrfconnect/update_ncs.py --check && '
                'test -w "$(dirname "$ZEPHYR_BASE")"',
                title='Validating NCS environment')
        except Exception:
            log.exception('Failed to validate ZEPHYR_BASE status')
            log.error(
                'To update $ZEPHYR_BASE run: python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow')
            log.error(
                'Ensure the nRF Connect environment is active: '
                'source scripts/setup/nrfconnect/activate.sh')

            raise Exception('ZEPHYR_BASE validation failed')

    def _prepare_environment(self):
        # Activate the nRF Connect SDK and Matter build environments. Skip saving
        # shell state because build commands run in a subshell.
        return 'NRFCONNECT_SKIP_ENV_STATE=1 source scripts/setup/nrfconnect/activate.sh;\n'

    def _get_build_flags(self):
        flags = []

        flags.append("-DSB_CONFIG_MERGED_HEX_FILES=y")
        if self.enable_rpcs:
            flags.append("-DOVERLAY_CONFIG=rpc.overlay")
        if self.enable_wifi:
            flags.append("-Dnrfconnect_SHIELD=nrf7002eb2")
            flags.append("-DSB_CONFIG_WIFI_NRF70=y")
            flags.append("-DCONFIG_CHIP_WIFI=y")
        if self.enable_bledfu:
            flags.append("-DCONFIG_CHIP_DFU_OVER_BT_SMP=y")
        if self.enable_release:
            flags.append("-DFILE_SUFFIX=release")

        if self.options.pregen_dir:
            flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        return " -- " + " ".join(flags) if len(flags) > 0 else ""

    def _find_merged_hex_files(self):
        merged_hex_files = sorted(glob.glob(os.path.join(self.output_dir, 'merged*.hex')))
        if len(merged_hex_files) > 0:
            return merged_hex_files

        board_target = self.board.GnArgName().replace('/', '_')
        expected = os.path.join(self.output_dir, f'merged_{board_target}.hex')
        if os.path.isfile(expected):
            return [expected]

        return []

    def _log_flash_instructions(self):
        if self._runner.dry_run:
            return

        if self.app == NrfApp.UNIT_TESTS or self.board == NrfBoard.NATIVE_SIM:
            return

        merged_hex_files = self._find_merged_hex_files()
        if len(merged_hex_files) == 0:
            log.warning('Merged HEX file not found in %s', self.output_dir)
            return

        banner = '=' * 80
        lines = [
            '',
            banner,
            ' BUILD COMPLETE - FLASH THE DEVICE ',
            banner,
            '',
            'Flash using nrfutil and the merged HEX file(s):',
            '',
        ]
        for merged_hex in merged_hex_files:
            lines.append(
                f'  nrfutil device program --firmware {shlex.quote(merged_hex)} --options chip_erase_mode=ERASE_ALL')
        lines.extend([
            '',
            'If more than one device is connected, list serial numbers with:',
            '  nrfutil device list',
            '',
            'Then add --serial-number <serial_number> to the flash command(s) above, for example:',
            f'  nrfutil device program --firmware {shlex.quote(merged_hex_files[0])} --options chip_erase_mode=ERASE_ALL '
            '--serial-number <serial_number>',
            '',
            banner,
            '',
        ])
        # Print to stderr so the flash instructions stay visible regardless of log level.
        print('\n'.join(lines), file=sys.stderr, flush=True)

    @lock_output_dir
    def generate(self):
        if not os.path.exists(self.output_dir):
            if not self._runner.dry_run:
                self._validate_ncs_environment()

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

    @lock_output_dir
    def _build(self):
        log.info('Compiling NrfConnect at %s', self.output_dir)

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
        else:
            self._log_flash_instructions()

    @lock_output_dir
    def _bundle(self):
        log.info('Generating flashbundle at %s', self.output_dir)

        self._Execute(['ninja', '-C', os.path.join(self.output_dir, 'nrfconnect'), 'flashing_script'],
                      title='Generating flashable files of ' + self.identifier)

    @lock_output_dir
    def build_outputs(self):
        yield BuilderOutput(
            os.path.join(self.output_dir, 'nrfconnect', 'zephyr', 'zephyr.elf'), f'{self.app.AppNamePrefix()}.elf')
        if self.options.enable_link_map_file:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'nrfconnect', 'zephyr', 'zephyr.map'), f'{self.app.AppNamePrefix()}.map')

    @lock_output_dir
    def bundle_outputs(self):
        if self.app == NrfApp.UNIT_TESTS:
            return
        with open(os.path.join(self.output_dir, 'nrfconnect', self.app.FlashBundleName())) as f:
            for line in filter(None, [x.strip() for x in f.readlines()]):
                yield BuilderOutput(os.path.join(self.output_dir, 'nrfconnect', line), line)
