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


class K32WBoard(Enum):
    K32W0 = auto()
    K32W1 = auto()

    def Name(self):
        if self == K32WBoard.K32W0:
            return 'k32w0x'
        elif self == K32WBoard.K32W1:
            return 'k32w1'
        else:
            raise Exception('Unknown board type: %r' % self)

    def FolderName(self):
        if self == K32WBoard.K32W0:
            return 'k32w/k32w0'
        elif self == K32WBoard.K32W1:
            return 'k32w/k32w1'
        else:
            raise Exception('Unknown board type: %r' % self)


class K32WApp(Enum):
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    CONTACT = auto()

    def ExampleName(self):
        if self == K32WApp.LIGHT:
            return 'lighting-app'
        elif self == K32WApp.LOCK:
            return 'lock-app'
        elif self == K32WApp.SHELL:
            return 'shell'
        elif self == K32WApp.CONTACT:
            return "contact-sensor-app"
        else:
            raise Exception('Unknown app type: %r' % self)

    def NameSuffix(self):
        if self == K32WApp.LIGHT:
            return 'light-example'
        elif self == K32WApp.LOCK:
            return 'lock-example'
        elif self == K32WApp.SHELL:
            return 'shell-example'
        elif self == K32WApp.CONTACT:
            return 'contact-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root, board):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', board.FolderName())


class K32WBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: K32WApp = K32WApp.LIGHT,
                 board: K32WBoard = K32WBoard.K32W0,
                 release: bool = False,
                 low_power: bool = False,
                 tokenizer: bool = False,
                 disable_ble: bool = False,
                 disable_ota: bool = False,
                 disable_logs: bool = False,
                 se05x: bool = False,
                 tinycrypt: bool = False,
                 crypto_platform: bool = False,
                 openthread_ftd: bool = False):
        super(K32WBuilder, self).__init__(
            root=app.BuildRoot(root, board),
            runner=runner)
        self.code_root = root
        self.app = app
        self.board = board
        self.low_power = low_power
        self.tokenizer = tokenizer
        self.release = release
        self.disable_ble = disable_ble
        self.disable_ota = disable_ota
        self.disable_logs = disable_logs
        self.se05x = se05x
        self.tinycrypt = tinycrypt
        self.crypto_platform = crypto_platform
        self.openthread_ftd = openthread_ftd

    def GnBuildArgs(self):
        args = []

        if self.low_power:
            args.append('chip_with_low_power=1 chip_logging=false')
            if self.board == K32WBoard.K32W0:
                args.append('chip_pw_tokenizer_logging=false chip_with_OM15082=0')
        else:
            args.append('chip_with_low_power=0')

        if self.tokenizer:
            args.append('chip_pw_tokenizer_logging=true')

        if self.release:
            args.append('is_debug=false')

        if self.disable_ble:
            args.append('chip_enable_ble=false')

        if self.disable_ota:
            args.append('chip_enable_ota_requestor=false')

        if self.disable_logs:
            args.append('chip_logging=false')

        if self.se05x:
            args.append('chip_with_se05x=true')

        if self.tinycrypt:
            args.append('chip_crypto=\"platform\" chip_crypto_flavor=\"tinycrypt\"')

        if self.crypto_platform:
            args.append('chip_crypto=\"platform\"')

        if self.openthread_ftd:
            args.append('chip_openthread_ftd=true')

        return args

    def generate(self):
        super(K32WBuilder, self).generate()

    def build_outputs(self):
        name = 'chip-%s-%s' % (self.board.Name(), self.app.NameSuffix())
        return {
            '%s.elf' % name: os.path.join(self.output_dir, name),
            '%s.map' % name: os.path.join(self.output_dir, '%s.map' % name)
        }
