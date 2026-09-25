# Copyright (c) 2026 Project CHIP Authors
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

from runner.runner import Runner

from .builder import Builder, BuilderOutput, OutDirLock, lock_output_dir

log = logging.getLogger(__name__)


class Esp32ZephyrApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == Esp32ZephyrApp.LIGHT:
            return 'lighting-app'
        raise Exception(f'Unknown app type: {self!r}')

    def AppNamePrefix(self):
        if self == Esp32ZephyrApp.LIGHT:
            return 'chip-esp32-zephyr-lighting-example'
        raise Exception(f'Unknown app type: {self!r}')


class Esp32ZephyrBoard(Enum):
    C6_DEVKITC = auto()

    def ZephyrBoardName(self):
        if self == Esp32ZephyrBoard.C6_DEVKITC:
            return 'esp32c6_devkitc/esp32c6/hpcore'
        raise Exception(f'Unknown board type: {self!r}')


class Esp32ZephyrBuilder(Builder):
    """Builds the ESP32 Zephyr examples (examples/<app>/esp32/zephyr) with west.

    Required environment:
      ESP32_ZEPHYR_BASE      Zephyr tree of a west workspace with hal_espressif
                             and its blobs fetched (Zephyr 4.4 or newer).
    Optional environment:
      ESP32_ZEPHYR_SDK_DIR   Zephyr SDK location, if Zephyr cannot find it.
      ESP32_ZEPHYR_VENV      Python venv providing `west` on Python 3.12 or
                             newer. Zephyr 4.4 rejects the Python 3.11 that the
                             CHIP build environment provides, so this venv is
                             put ahead of it on PATH.
    """

    def __init__(self,
                 root: str,
                 runner: Runner,
                 output_dir_lock: OutDirLock,
                 app: Esp32ZephyrApp = Esp32ZephyrApp.LIGHT,
                 board: Esp32ZephyrBoard = Esp32ZephyrBoard.C6_DEVKITC,
                 ):
        super().__init__(root, runner, output_dir_lock)
        self.app = app
        self.board = board

    def get_cmd_prefixes(self):
        if not self._runner.dry_run:
            if 'ESP32_ZEPHYR_BASE' not in os.environ:
                raise Exception("ESP32 Zephyr builds require ESP32_ZEPHYR_BASE")

        cmd = 'export ZEPHYR_BASE="$ESP32_ZEPHYR_BASE"\n'

        if 'ESP32_ZEPHYR_SDK_DIR' in os.environ:
            cmd += 'export ZEPHYR_SDK_INSTALL_DIR="$ESP32_ZEPHYR_SDK_DIR"\n'

        if 'ESP32_ZEPHYR_VENV' in os.environ:
            cmd += 'export PATH="$ESP32_ZEPHYR_VENV/bin:$PATH"\n'

        return cmd

    @lock_output_dir
    def generate(self):
        os.makedirs(self.output_dir, exist_ok=True)

        flags = []
        if self.options.pregen_dir:
            flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        build_flags = " -- " + " ".join(flags) if flags else ""

        cmd = self.get_cmd_prefixes()
        cmd += 'west build --cmake-only -d {outdir} -b {board} {sourcedir}{build_flags}'.format(
            outdir=shlex.quote(self.output_dir),
            board=self.board.ZephyrBoardName(),
            sourcedir=shlex.quote(os.path.join(self.root, 'examples', self.app.ExampleName(), 'esp32', 'zephyr')),
            build_flags=build_flags)

        self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)

    @lock_output_dir
    def _build(self):
        log.info('Compiling ESP32 Zephyr at %s', self.output_dir)

        cmd = self.get_cmd_prefixes() + f"ninja -C {shlex.quote(self.output_dir)}"

        if self.ninja_jobs is not None:
            cmd += f" -j{str(self.ninja_jobs)}"

        self._Execute(['bash', '-c', cmd], title='Building ' + self.identifier)

    @lock_output_dir
    def build_outputs(self):
        app_name = self.app.AppNamePrefix()
        yield BuilderOutput(
            os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
            f'{app_name}.elf')
        yield BuilderOutput(
            os.path.join(self.output_dir, 'zephyr', 'zephyr.bin'),
            f'{app_name}.bin')
        if self.options.enable_link_map_file:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
                f'{app_name}.map')
