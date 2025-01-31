# Copyright (c) 2022-2024 Project CHIP Authors
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


class TelinkApp(Enum):
    AIR_QUALITY_SENSOR = auto()
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
    SHELL = auto()
    SMOKE_CO_ALARM = auto()
    TEMPERATURE_MEASUREMENT = auto()
    THERMOSTAT = auto()
    WINDOW_COVERING = auto()

    def ExampleName(self):
        if self == TelinkApp.AIR_QUALITY_SENSOR:
            return 'air-quality-sensor-app'
        elif self == TelinkApp.ALL_CLUSTERS:
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
        elif self == TelinkApp.SHELL:
            return 'shell'
        elif self == TelinkApp.SMOKE_CO_ALARM:
            return 'smoke-co-alarm-app'
        elif self == TelinkApp.TEMPERATURE_MEASUREMENT:
            return 'temperature-measurement-app'
        elif self == TelinkApp.THERMOSTAT:
            return 'thermostat'
        elif self == TelinkApp.WINDOW_COVERING:
            return 'window-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == TelinkApp.AIR_QUALITY_SENSOR:
            return 'chip-telink-air-quality-sensor-example'
        elif self == TelinkApp.ALL_CLUSTERS:
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
        elif self == TelinkApp.RESOURCE_MONITORING:
            return 'chip-telink-resource-monitoring-example'
        elif self == TelinkApp.SHELL:
            return 'chip-telink-shell-example'
        elif self == TelinkApp.SMOKE_CO_ALARM:
            return 'chip-telink-smoke-co-alarm-example'
        elif self == TelinkApp.TEMPERATURE_MEASUREMENT:
            return 'chip-telink-temperature-measurement-example'
        elif self == TelinkApp.THERMOSTAT:
            return 'chip-telink-thermostat-example'
        elif self == TelinkApp.WINDOW_COVERING:
            return 'chip-telink-window-example'
        else:
            raise Exception('Unknown app type: %r' % self)


class TelinkBoard(Enum):
    TLRS9118BDK40D = auto()
    TLSR9518ADK80D = auto()
    TLSR9528A = auto()
    TLSR9528A_RETENTION = auto()
    TL3218X = auto()
    TL7218X = auto()
    TL7218X_RETENTION = auto()

    def GnArgName(self):
        if self == TelinkBoard.TLRS9118BDK40D:
            return 'tlsr9118bdk40d'
        elif self == TelinkBoard.TLSR9518ADK80D:
            return 'tlsr9518adk80d'
        elif self == TelinkBoard.TLSR9528A:
            return 'tlsr9528a'
        elif self == TelinkBoard.TLSR9528A_RETENTION:
            return 'tlsr9528a_retention'
        elif self == TelinkBoard.TL3218X:
            return 'tl3218x'
        elif self == TelinkBoard.TL7218X:
            return 'tl7218x'
        elif self == TelinkBoard.TL7218X_RETENTION:
            return 'tl7218x_retention'
        else:
            raise Exception('Unknown board type: %r' % self)


class TelinkBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 app: TelinkApp = TelinkApp,
                 board: TelinkBoard = TelinkBoard,
                 enable_ota: bool = False,
                 enable_dfu: bool = False,
                 enable_shell: bool = False,
                 enable_rpcs: bool = False,
                 enable_factory_data: bool = False,
                 enable_4mb_flash: bool = False,
                 mars_board_config: bool = False,
                 usb_board_config: bool = False,
                 compress_lzma_config: bool = False,
                 thread_analyzer_config: bool = False,
                 ):
        super(TelinkBuilder, self).__init__(root, runner)
        self.app = app
        self.board = board
        self.enable_ota = enable_ota
        self.enable_dfu = enable_dfu
        self.enable_shell = enable_shell
        self.enable_rpcs = enable_rpcs
        self.enable_factory_data = enable_factory_data
        self.enable_4mb_flash = enable_4mb_flash
        self.mars_board_config = mars_board_config
        self.usb_board_config = usb_board_config
        self.compress_lzma_config = compress_lzma_config
        self.thread_analyzer_config = thread_analyzer_config

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
        if self.enable_ota:
            flags.append("-DCONFIG_CHIP_OTA_REQUESTOR=y")

        if self.enable_dfu:
            flags.append("-DCONFIG_BOOTLOADER_MCUBOOT=y")

        if self.enable_shell:
            flags.append("-DCONFIG_CHIP_LIB_SHELL=y")

        if self.enable_rpcs:
            flags.append("-DOVERLAY_CONFIG=rpc.overlay")

        if self.enable_factory_data:
            flags.append("-DCONFIG_CHIP_FACTORY_DATA=y -DCONFIG_CHIP_FACTORY_DATA_BUILD=y -DCONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE=y")

        if self.enable_4mb_flash:
            flags.append("-DFLASH_SIZE=4m")

        if self.mars_board_config:
            flags.append("-DTLNK_MARS_BOARD=y")

        if self.usb_board_config:
            flags.append("-DTLNK_USB_DONGLE=y")

        if self.compress_lzma_config:
            flags.append("-DCONFIG_COMPRESS_LZMA=y")

        if self.thread_analyzer_config:
            flags.append("-DCONFIG_THREAD_ANALYZER=y")

        if self.options.pregen_dir:
            flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

        build_flags = " -- " + " ".join(flags) if len(flags) > 0 else ""

        cmd = self.get_cmd_prefixes()
        cmd += '\nsource "$ZEPHYR_BASE/zephyr-env.sh";'

        cmd += '\nwest build --cmake-only -d {outdir} -b {board} {sourcedir}{build_flags}\n'.format(
            outdir=shlex.quote(self.output_dir),
            board=self.board.GnArgName(),
            sourcedir=shlex.quote(os.path.join(self.root, 'examples', self.app.ExampleName(), 'telink')),
            build_flags=build_flags).strip()

        self._Execute(['bash', '-c', cmd],
                      title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Telink at %s', self.output_dir)

        cmd = self.get_cmd_prefixes() + ("ninja -C %s" % self.output_dir)

        if self.ninja_jobs is not None:
            cmd += " -j%s" % str(self.ninja_jobs)

        self._Execute(['bash', '-c', cmd], title='Building ' + self.identifier)

    def build_outputs(self):
        yield BuilderOutput(
            os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
            '%s.elf' % self.app.AppNamePrefix())
        if self.options.enable_link_map_file:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
                '%s.map' % self.app.AppNamePrefix())
