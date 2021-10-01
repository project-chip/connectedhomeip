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

from platform import uname
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


class HostBoard(Enum):
    NATIVE = auto()

    # cross-compile support
    ARM64 = auto()

    def BoardName(self):
        if self == HostBoard.NATIVE:
            uname_result = uname()
            arch = uname_result.machine

            # standardize some common platforms
            if arch == 'x86_64':
                arch = 'x64'
            elif arch == 'i386' or arch == 'i686':
                arch = 'x86'
            elif arch == 'aarch64' or arch == 'aarch64_be' or arch == 'armv8b' or arch == 'armv8l':
                arch = 'arm64'

            return arch
        elif self == HostBoard.ARM64:
            return 'arm64'
        else:
            raise Exception('Unknown host board type: %r' % self)

    def PlatformName(self):
        if self == HostBoard.NATIVE:
            return uname().system.lower()
        else:
            # Cross compilation assumes linux currently
            return 'linux'


class HostBuilder(GnBuilder):

    def __init__(self, root, runner, output_prefix: str, app: HostApp, board=HostBoard.NATIVE):
        super(HostBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner,
            output_prefix=output_prefix)

        self.app_name = app.BinaryName()
        self.map_name = self.app_name + '.map'
        self.board = board

    def GnBuildArgs(self):
        if self.board == HostBoard.NATIVE:
            return None
        elif self.board == HostBoard.ARM64:
            return [
                'target_cpu="arm64"',
                'is_clang=true',
                'chip_crypto="mbedtls"',
                'sysroot="%s"' % self.SysRootPath('SYSROOT_AARCH64'),
            ]
        else:
            raise Exception('Unknown host board type: %r' % self)

    def GnBuildEnv(self):
        if self.board == HostBoard.NATIVE:
            return None
        elif self.board == HostBoard.ARM64:
            return {
                'PKG_CONFIG_PATH': self.SysRootPath('SYSROOT_AARCH64') + '/lib/aarch64-linux-gnu/pkgconfig',
            }
        else:
            raise Exception('Unknown host board type: %r' % self)

    def SysRootPath(self, name):
        if not name in os.environ:
            raise Exception('Missing environment variable "%s"' % name)
        return os.environ[name]

    def build_outputs(self):
        return {
            self.app_name: os.path.join(self.output_dir, self.app_name),
            self.map_name: os.path.join(self.output_dir, self.map_name)
        }

# todo
    def SetIdentifier(self, platform: str, board: str, app: str, enable_rpcs: bool = False):
        super(HostBuilder, self).SetIdentifier(
            self.board.PlatformName(), self.board.BoardName(), app, enable_rpcs)
