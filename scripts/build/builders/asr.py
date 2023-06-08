# Copyright (c) 2023 Project CHIP Authors
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


class ASRApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == ASRApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == ASRApp.LIGHT:
            return 'chip-asr-lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'asr')


class ASRBoard(Enum):
    ASR582X = auto()
    ASR595X = auto()

    def GetIC(self):
        if self == ASRBoard.ASR582X:
            return 'asr582x'
        elif self == ASRBoard.ASR595X:
            return 'asr595x'
        else:
            raise Exception('Unknown board #: %r' % self)


class ASRBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: ASRApp = ASRApp.LIGHT,
                 board: ASRBoard = ASRBoard.ASR582X,
                 chip_build_libshell: bool = False,
                 chip_logging: bool = True,
                 enable_factory: bool = False,
                 enable_ota_requestor: bool = False):
        super(ASRBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)

        self.board = board
        self.app = app

        asr_chip = self.board.GetIC()
        self.extra_gn_options = ['asr_ic_family="%s"' % asr_chip]

        if asr_chip == "asr582x":
            ASR_ARCH = "arm"
            ASR_SDK_ROOT = "//third_party/connectedhomeip/third_party/asr/asr582x"
        elif asr_chip == "asr595x":
            ASR_ARCH = "riscv"
            ASR_SDK_ROOT = "//third_party/connectedhomeip/third_party/asr/asr595x"
        self.extra_gn_options.append('target_cpu="%s"' % ASR_ARCH)

        toolchain = os.path.join(root, os.path.split(os.path.realpath(__file__))[0], '../../../config/asr/toolchain')
        toolchain = 'custom_toolchain="{}:asrtoolchain"'.format(toolchain)
        if toolchain:
            self.extra_gn_options.append(toolchain)

        self.extra_gn_options.append('asr_sdk_build_root="%s"' % ASR_SDK_ROOT)
        self.extra_gn_options.append('mbedtls_target="%s:asr_build"' % ASR_SDK_ROOT)

        if (asr_chip == "asr582x"
                or asr_chip == "asr595x"):
            self.extra_gn_options.append('chip_config_network_layer_ble=true')

        if enable_ota_requestor:
            self.extra_gn_options.append('chip_enable_ota_requestor=true')

        if chip_build_libshell:
            self.extra_gn_options.append('chip_build_libshell=true')

        if chip_logging is False:
            self.extra_gn_options.append('chip_logging=false')

        if enable_factory:
            self.extra_gn_options.append('chip_use_transitional_commissionable_data_provider=false')
            self.extra_gn_options.append('chip_enable_factory_data=true')

        self.extra_gn_options.append('asr_toolchain_root="%s"' % os.environ['ASR_TOOLCHAIN_PATH'])

    def GnBuildArgs(self):
        return self.extra_gn_options

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        return items
