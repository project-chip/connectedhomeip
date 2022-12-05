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
import platform
import logging
from enum import Enum, auto

from .gn import GnBuilder


class BouffalolabApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self, chip_name):
        if self == BouffalolabApp.LIGHT:
            return ('chip-%s-lighting-example' % chip_name)
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == BouffalolabApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class BouffalolabBoard(Enum):
    BL602_IoT_Matter_V1 = auto()
    BL602_IOT_DVK_3S = auto()
    BL602_NIGHT_LIGHT = auto()
    XT_ZB6_DevKit = auto()
    BL706_IoT_DVK = auto()
    BL706_NIGHT_LIGHT = auto()

    def GnArgName(self):
        if self == BouffalolabBoard.BL602_IoT_Matter_V1:
            return 'BL602-IoT-Matter-V1'
        elif self == BouffalolabBoard.BL602_IOT_DVK_3S:
            return 'BL602-IOT-DVK-3S'
        elif self == BouffalolabBoard.BL602_NIGHT_LIGHT:
            return 'BL602-NIGHT-LIGHT'
        elif self == BouffalolabBoard.XT_ZB6_DevKit:
            return 'XT-ZB6-DevKit'
        elif self == BouffalolabBoard.BL706_IoT_DVK:
            return 'BL706-IoT-DVK'
        elif self == BouffalolabBoard.BL706_NIGHT_LIGHT:
            return 'BL706-NIGHT-LIGHT'
        else:
            raise Exception('Unknown board #: %r' % self)


class BouffalolabBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: BouffalolabApp = BouffalolabApp.LIGHT,
                 board: BouffalolabBoard = BouffalolabBoard.BL706_IoT_DVK,
                 enable_rpcs: bool = False,
                 module_type: str = "BL706C-22",
                 baudrate=2000000,
                 enable_shell: bool = False
                 ):

        bouffalo_chip = "bl702" if "BL70" in module_type else "bl602"
        super(BouffalolabBuilder, self).__init__(
            root=os.path.join(root, 'examples',
                              app.ExampleName(), 'bouffalolab', bouffalo_chip),
            runner=runner
        )

        self.argsOpt = []
        self.chip_name = bouffalo_chip
        toolchain = os.path.join(root, '../../examples/platform/bouffalolab/common/toolchain')
        toolchain = 'custom_toolchain="{}:riscv_gcc"'.format(toolchain)
        if toolchain:
            self.argsOpt.append(toolchain)

        self.app = app
        self.board = board

        self.argsOpt.append('board=\"{}\"'.format(self.board.GnArgName()))
        self.argsOpt.append('baudrate=\"{}\"'.format(baudrate))

        if bouffalo_chip == "bl702":
            self.argsOpt.append('module_type=\"{}\"'.format(module_type))

        if enable_rpcs:
            self.argsOpt.append('import("//with_pw_rpc.gni")')
        elif enable_shell:
            self.argsOpt.append('chip_build_libshell=true')

        try:
            self.argsOpt.append('bouffalolab_sdk_root="%s"' % os.environ['BOUFFALOLAB_SDK_ROOT'])
        except KeyError as err:
            logging.fatal('Please make sure Bouffalo Lab SDK installs as below:')
            logging.fatal('\tcd third_party/bouffalolab/repo')
            logging.fatal('\tsudo bash scripts/setup.sh')

            logging.fatal('Please make sure BOUFFALOLAB_SDK_ROOT exports before building as below:')
            logging.fatal('\texport BOUFFALOLAB_SDK_ROOT=/opt/bouffalolab_sdk')

            raise err

    def GnBuildArgs(self):
        return self.argsOpt

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix(self.chip_name)),
            '%s.out.map' % self.app.AppNamePrefix(self.chip_name):
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix(self.chip_name)),
        }

        return items
