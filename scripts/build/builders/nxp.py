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

import os
from enum import Enum, auto

from .gn import GnBuilder


class NxpBoard(Enum):
    K32W0 = auto()
    K32W1 = auto()

    def Name(self):
        if self == NxpBoard.K32W0:
            return 'k32w0x'
        elif self == NxpBoard.K32W1:
            return 'k32w1'
        else:
            raise Exception('Unknown board type: %r' % self)

    def FolderName(self):
        if self == NxpBoard.K32W0:
            return 'k32w/k32w0'
        elif self == NxpBoard.K32W1:
            return 'k32w/k32w1'
        else:
            raise Exception('Unknown board type: %r' % self)


class NxpApp(Enum):
    LIGHTING = auto()
    CONTACT = auto()

    def ExampleName(self):
        if self == NxpApp.LIGHTING:
            return 'lighting-app'
        elif self == NxpApp.CONTACT:
            return "contact-sensor-app"
        else:
            raise Exception('Unknown app type: %r' % self)

    def NameSuffix(self):
        if self == NxpApp.LIGHTING:
            return 'light-example'
        elif self == NxpApp.CONTACT:
            return 'contact-example'
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

    def generate(self):
        super(NxpBuilder, self).generate()

    def build_outputs(self):
        name = 'chip-%s-%s' % (self.board.Name(), self.app.NameSuffix())
        return {
            '%s.elf' % name: os.path.join(self.output_dir, name),
            '%s.map' % name: os.path.join(self.output_dir, '%s.map' % name)
        }
