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


class K32WApp(Enum):
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()

    def ExampleName(self):
        if self == K32WApp.LIGHT:
            return 'lighting-app'
        elif self == K32WApp.LOCK:
            return 'lock-app'
        elif self == K32WApp.SHELL:
            return 'shell'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == K32WApp.LIGHT:
            return 'chip-k32w061-light-example'
        elif self == K32WApp.LOCK:
            return 'chip-k32w061-lock-example'
        elif self == K32WApp.SHELL:
            return 'chip-k32w061-shell-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', 'k32w', 'k32w0')


class K32WBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: K32WApp = K32WApp.LIGHT,
                 release: bool = False,
                 low_power: bool = False,
                 tokenizer: bool = False):
        super(K32WBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.code_root = root
        self.app = app
        self.low_power = low_power
        self.tokenizer = tokenizer
        self.release = release

    def GnBuildArgs(self):
        args = [
            'k32w0_sdk_root="%s"' % os.environ['NXP_K32W061_SDK_ROOT'],
        ]

        if self.low_power:
            args.append('chip_with_low_power=1')
        else:
            args.append('chip_with_low_power=0')

        if self.tokenizer:
            args.append('chip_pw_tokenizer_logging=true')

        if self.release:
            args.append('is_debug=false')

        return args

    def generate(self):
        self._Execute([os.path.join(
            self.code_root, 'third_party/nxp/k32w0_sdk/sdk_fixes/patch_k32w_sdk.sh')])

        super(K32WBuilder, self).generate()

    def build_outputs(self):
        items = {}
        for extension in ["", ".map", ".hex"]:
            name = '%s%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
