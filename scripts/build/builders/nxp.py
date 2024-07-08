# Copyright (c) 2021-2024 Project CHIP Authors
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
from enum import Enum, auto

from .builder import BuilderOutput
from .gn import GnBuilder


class NxpOsUsed(Enum):
    FREERTOS = auto()
    ZEPHYR = auto()

    def OsEnv(self):
        if self == NxpOsUsed.ZEPHYR:
            return 'zephyr'
        elif self == NxpOsUsed.FREERTOS:
            return 'freertos'
        else:
            raise Exception('Unknown OS type: %r' % self)


class NxpBoard(Enum):
    K32W0 = auto()
    K32W1 = auto()
    RW61X = auto()

    def Name(self):
        if self == NxpBoard.K32W0:
            return 'k32w0x'
        elif self == NxpBoard.K32W1:
            return 'k32w1'
        elif self == NxpBoard.RW61X:
            return 'rd_rw612_bga'
        else:
            raise Exception('Unknown board type: %r' % self)

    def FolderName(self):
        if self == NxpBoard.K32W0:
            return 'k32w/k32w0'
        elif self == NxpBoard.K32W1:
            return 'k32w/k32w1'
        elif self == NxpBoard.RW61X:
            return 'zephyr'
        else:
            raise Exception('Unknown board type: %r' % self)


class NxpApp(Enum):
    LIGHTING = auto()
    CONTACT = auto()
    ALLCLUSTERS = auto()
    LAUNDRYWASHER = auto()
    THERMOSTAT = auto()

    def ExampleName(self):
        if self == NxpApp.LIGHTING:
            return 'lighting-app'
        elif self == NxpApp.CONTACT:
            return "contact-sensor-app"
        elif self == NxpApp.ALLCLUSTERS:
            return "all-clusters-app"
        elif self == NxpApp.LAUNDRYWASHER:
            return "laundry-washer-app"
        elif self == NxpApp.THERMOSTAT:
            return "thermostat"
        else:
            raise Exception('Unknown app type: %r' % self)

    def NameSuffix(self):
        if self == NxpApp.LIGHTING:
            return 'light-example'
        elif self == NxpApp.CONTACT:
            return 'contact-example'
        elif self == NxpApp.ALLCLUSTERS:
            return "all-cluster-example"
        elif self == NxpApp.LAUNDRYWASHER:
            return "laundry-washer-example"
        elif self == NxpApp.THERMOSTAT:
            return "thermostat-example"
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root, board):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', board.FolderName())


class NxpBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: NxpApp = NxpApp.LIGHTING,
                 board: NxpBoard = NxpBoard.K32W0,
                 os_env: NxpOsUsed = NxpOsUsed.FREERTOS,
                 low_power: bool = False,
                 smu2: bool = False,
                 enable_factory_data: bool = False,
                 convert_dac_pk: bool = False,
                 use_fro32k: bool = False,
                 enable_lit: bool = False,
                 enable_rotating_id: bool = False,
                 has_sw_version_2: bool = False):
        super(NxpBuilder, self).__init__(
            root=app.BuildRoot(root, board),
            runner=runner)
        self.code_root = root
        self.app = app
        self.board = board
        self.os_env = os_env
        self.low_power = low_power
        self.smu2 = smu2
        self.enable_factory_data = enable_factory_data
        self.convert_dac_pk = convert_dac_pk
        self.use_fro32k = use_fro32k
        self.enable_lit = enable_lit
        self.enable_rotating_id = enable_rotating_id
        self.has_sw_version_2 = has_sw_version_2

    def GnBuildArgs(self):
        args = []

        if self.low_power:
            args.append('chip_with_low_power=1 chip_logging=false')
            if self.board == NxpBoard.K32W0:
                args.append('chip_pw_tokenizer_logging=false chip_with_OM15082=0')

        if self.smu2:
            args.append('use_smu2_static=true use_smu2_dynamic=true')

        if self.enable_factory_data:
            args.append('chip_with_factory_data=1')

        if self.convert_dac_pk:
            args.append('chip_convert_dac_private_key=1')

        if self.use_fro32k:
            args.append('use_fro_32k=1')

        if self.enable_lit:
            args.append('chip_enable_icd_lit=true')

        if self.enable_rotating_id:
            args.append('chip_enable_rotating_device_id=1 chip_enable_additional_data_advertising=1')

        if self.has_sw_version_2:
            args.append('nxp_software_version=2')

        return args

    def WestBuildArgs(self):
        args = []
        if self.enable_factory_data:
            args.append('-DFILE_SUFFIX=fdata')

        if self.has_sw_version_2:
            args.append('-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2')

        build_args = " -- " + " ".join(args) if len(args) > 0 else ""
        return build_args

    def generate(self):
        if self.os_env == NxpOsUsed.ZEPHYR:
            if 'ZEPHYR_NXP_SDK_INSTALL_DIR' in os.environ:
                cmd = 'export ZEPHYR_SDK_INSTALL_DIR="$ZEPHYR_NXP_SDK_INSTALL_DIR"\n'
            else:
                raise Exception("ZEPHYR_SDK_INSTALL_DIR need to be set")
            if 'ZEPHYR_NXP_BASE' in os.environ:
                cmd += 'export ZEPHYR_BASE="$ZEPHYR_NXP_BASE"\n'
            else:
                raise Exception("ZEPHYR_NXP_BASE need to be set")
            build_args = self.WestBuildArgs()
            cmd += '''
            west build -p --cmake-only -b {board_name} -d {out_folder} {example_folder} {build_args}
            '''.format(
                board_name=self.board.Name(),
                out_folder=self.output_dir,
                example_folder=self.app.BuildRoot(self.code_root, self.board),
                build_args=build_args).strip()
            self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)
        else:
            super(NxpBuilder, self).generate()

    def build_outputs(self):
        name = 'chip-%s-%s' % (self.board.Name(), self.app.NameSuffix())
        if self.os_env == NxpOsUsed.ZEPHYR:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
                f'{name}.elf')
            if self.options.enable_link_map_file:
                yield BuilderOutput(
                    os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
                    f'{name}.map')
        else:
            yield BuilderOutput(
                os.path.join(self.output_dir, name),
                f'{name}.elf')
            if self.options.enable_link_map_file:
                yield BuilderOutput(
                    os.path.join(self.output_dir, f'{name}.map'),
                    f'{name}.map')
