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
    RPC_CONSOLE = auto()
    MIN_MDNS = auto()

    def ExamplePath(self):
        if self == HostApp.ALL_CLUSTERS:
            return 'all-clusters-app/linux'
        elif self == HostApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == HostApp.THERMOSTAT:
            return 'thermostat/linux'
        elif self == HostApp.RPC_CONSOLE:
            return 'common/pigweed/rpc_console'
        if self == HostApp.MIN_MDNS:
            return 'minimal-mdns'
        else:
            raise Exception('Unknown app type: %r' % self)

    def OutputNames(self):
        if self == HostApp.ALL_CLUSTERS:
            yield 'chip-all-clusters-app'
            yield 'chip-all-clusters-app.map'
        elif self == HostApp.CHIP_TOOL:
            yield 'chip-tool'
            yield 'chip-tool.map'
        elif self == HostApp.THERMOSTAT:
            yield 'thermostat-app'
            yield 'thermostat-app.map'
        elif self == HostApp.RPC_CONSOLE:
            yield 'chip_rpc_console_wheels'
        elif self == HostApp.MIN_MDNS:
            yield 'mdns-advertiser'
            yield 'mdns-advertiser.map'
            yield 'minimal-mdns-client'
            yield 'minimal-mdns-client.map'
            yield 'minimal-mdns-server'
            yield 'minimal-mdns-server.map'
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

    def __init__(self, root, runner, app: HostApp, board=HostBoard.NATIVE, enable_ipv4=True):
        super(HostBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []

        if not enable_ipv4:
            self.extra_gn_options.append('chip_inet_config_enable_ipv4=false')

    def GnBuildArgs(self):
        if self.board == HostBoard.NATIVE:
            return self.extra_gn_options
        elif self.board == HostBoard.ARM64:
            self.extra_gn_options.extend(
                [
                    'target_cpu="arm64"',
                    'is_clang=true',
                    'chip_crypto="mbedtls"',
                    'sysroot="%s"' % self.SysRootPath('SYSROOT_AARCH64')
                ]
            )

            return self.extra_gn_options
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
        outputs = {}

        for name in self.app.OutputNames():
            path = os.path.join(self.output_dir, name)
            if os.path.isdir(path):
                for root, dirs, files in os.walk(path):
                    for file in files:
                        outputs.update({
                            file: os.path.join(root, file)
                        })
            else:
                outputs.update({
                    name: os.path.join(self.output_dir, name)
                })

        return outputs
