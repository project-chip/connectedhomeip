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
from platform import uname, release
from enum import Enum, auto

from .gn import GnBuilder


class HostApp(Enum):
    ALL_CLUSTERS = auto()
    CHIP_TOOL = auto()
    THERMOSTAT = auto()

    def ExamplePath(self):
        if self == HostApp.ALL_CLUSTERS:
            return 'all-clusters-app/linux'
        elif self == HostApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == HostApp.THERMOSTAT:
            return 'thermostat/linux'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BinaryName(self):
        if self == HostApp.ALL_CLUSTERS:
            return 'chip-all-clusters-app'
        elif self == HostApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == HostApp.THERMOSTAT:
            return 'thermostat-app'
        else:
            raise Exception('Unknown app type: %r' % self)


def ConcretePlatformName():
    uname_result = uname()
    return '-'.join([uname_result.system.lower(), release(), uname_result.machine])


class HostBuilder(GnBuilder):

    def __init__(self, root, runner, output_prefix: str, app: HostApp):
        super(HostBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner,
            output_prefix=output_prefix)

        self.app_name = app.BinaryName()
        self.map_name = self.app_name + '.map'

    def build_outputs(self):
        return {
            self.app_name: os.path.join(self.output_dir, self.app_name),
            self.map_name: os.path.join(self.output_dir, self.map_name)
        }

    def SetIdentifier(self, platform: str, board: str, app: str):
        super(HostBuilder, self).SetIdentifier(
            ConcretePlatformName(), board, app)
