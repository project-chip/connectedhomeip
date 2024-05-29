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

from .builder import BuilderOutput
from .gn import GnBuilder


class cc32xxApp(Enum):
    LOCK = auto()
    AIR_PURIFIER = auto()

    def ExampleName(self):
        if self == cc32xxApp.LOCK:
            return 'lock-app'
        elif self == cc32xxApp.AIR_PURIFIER:
            return 'air-purifier-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == cc32xxApp.LOCK:
            return 'chip-CC3235SF_LAUNCHXL-lock-example'
        elif self == cc32xxApp.AIR_PURIFIER:
            return 'chip-CC3235SF_LAUNCHXL-air-purifier-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'cc32xx')


class cc32xxBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: cc32xxApp = cc32xxApp.LOCK):
        super(cc32xxBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.code_root = root
        self.app = app

    def GnBuildArgs(self):
        args = [
            'ti_sysconfig_root="%s"' % os.environ['TI_SYSCONFIG_ROOT'],
        ]

        return args

    def build_outputs(self):
        if (self.app == cc32xxApp.LOCK):
            extensions = ["out", "bin"]
        elif (self.app == cc32xxApp.AIR_PURIFIER):
            extensions = ["out", "bin"]
        else:
            raise Exception('Unknown app type: %r' % self.app)
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix()}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
