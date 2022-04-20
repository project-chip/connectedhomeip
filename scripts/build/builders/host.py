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
from platform import uname

from .gn import GnBuilder


class HostApp(Enum):
    ALL_CLUSTERS = auto()
    CHIP_TOOL = auto()
    THERMOSTAT = auto()
    RPC_CONSOLE = auto()
    MIN_MDNS = auto()
    ADDRESS_RESOLVE = auto()
    TV_APP = auto()
    LOCK = auto()
    TESTS = auto()
    SHELL = auto()
    CERT_TOOL = auto()
    OTA_PROVIDER = auto()
    OTA_REQUESTOR = auto()
    PYTHON_BINDINGS = auto()
    NL_TEST_RUNNER = auto()

    def ExamplePath(self):
        if self == HostApp.ALL_CLUSTERS:
            return 'all-clusters-app/linux'
        elif self == HostApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == HostApp.THERMOSTAT:
            return 'thermostat/linux'
        elif self == HostApp.RPC_CONSOLE:
            return 'common/pigweed/rpc_console'
        elif self == HostApp.MIN_MDNS:
            return 'minimal-mdns'
        elif self == HostApp.TV_APP:
            return 'tv-app/linux'
        elif self == HostApp.LOCK:
            return 'lock-app/linux'
        elif self == HostApp.SHELL:
            return 'shell/standalone'
        elif self == HostApp.OTA_PROVIDER:
            return 'ota-provider-app/linux'
        elif self == HostApp.OTA_REQUESTOR:
            return 'ota-requestor-app/linux'
        elif self in [HostApp.ADDRESS_RESOLVE, HostApp.TESTS, HostApp.PYTHON_BINDINGS, HostApp.CERT_TOOL]:
            return '../'
        elif self == HostApp.NL_TEST_RUNNER:
            return '../src/test_driver/efr32'
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
        elif self == HostApp.ADDRESS_RESOLVE:
            yield 'address-resolve-tool'
            yield 'address-resolve-tool.map'
        elif self == HostApp.TV_APP:
            yield 'chip-tv-app'
            yield 'chip-tv-app.map'
        elif self == HostApp.LOCK:
            yield 'chip-lock-app'
            yield 'chip-lock-app.map'
        elif self == HostApp.TESTS:
            pass
        elif self == HostApp.SHELL:
            yield 'chip-shell'
            yield 'chip-shell.map'
        elif self == HostApp.CERT_TOOL:
            yield 'chip-cert'
            yield 'chip-cert.map'
        elif self == HostApp.OTA_PROVIDER:
            yield 'chip-ota-provider-app'
            yield 'chip-ota-provider-app.map'
        elif self == HostApp.OTA_REQUESTOR:
            yield 'chip-ota-requestor-app'
            yield 'chip-ota-requestor-app.map'
        elif self == HostApp.PYTHON_BINDINGS:
            yield 'controller/python'  # Directory containing WHL files
        elif self == HostApp.NL_TEST_RUNNER:
            yield 'chip_nl_test_runner_wheels'
        else:
            raise Exception('Unknown app type: %r' % self)


class HostBoard(Enum):
    NATIVE = auto()

    # cross-compile support
    ARM64 = auto()

    # for test support
    FAKE = auto()

    def BoardName(self):
        if self == HostBoard.NATIVE:
            uname_result = uname()
            arch = uname_result.machine

            # standardize some common platforms
            if arch == 'x86_64':
                arch = 'x64'
            elif arch == 'i386' or arch == 'i686':
                arch = 'x86'
            elif arch in ('aarch64', 'aarch64_be', 'armv8b', 'armv8l'):
                arch = 'arm64'

            return arch
        elif self == HostBoard.ARM64:
            return 'arm64'
        elif self == HostBoard.FAKE:
            return 'fake'
        else:
            raise Exception('Unknown host board type: %r' % self)

    def PlatformName(self):
        if self == HostBoard.NATIVE:
            return uname().system.lower()
        elif self == HostBoard.FAKE:
            return 'fake'
        else:
            # Cross compilation assumes linux currently
            return 'linux'


class HostBuilder(GnBuilder):

    def __init__(self, root, runner, app: HostApp, board=HostBoard.NATIVE, enable_ipv4=True,
                 enable_ble=True, enable_wifi=True, use_tsan=False,  use_asan=False, separate_event_loop=True,
                 test_group=False, use_libfuzzer=False, use_clang=False, interactive_mode=True,
                 use_platform_mdns=False):
        super(HostBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []

        if not enable_ipv4:
            self.extra_gn_options.append('chip_inet_config_enable_ipv4=false')

        if not enable_ble:
            self.extra_gn_options.append('chip_config_network_layer_ble=false')

        if not enable_wifi:
            self.extra_gn_options.append('chip_enable_wifi=false')

        if use_tsan:
            self.extra_gn_options.append('is_tsan=true')

        if use_asan:
            self.extra_gn_options.append('is_asan=true')

        if not separate_event_loop:
            self.extra_gn_options.append('config_use_separate_eventloop=false')

        if not interactive_mode:
            self.extra_gn_options.append('config_use_interactive_mode=false')

        if test_group:
            self.extra_gn_options.append(
                'chip_enable_group_messaging_tests=true')

        if use_libfuzzer:
            self.extra_gn_options.append('is_libfuzzer=true')

        if use_clang:
            self.extra_gn_options.append('is_clang=true')

        if use_platform_mdns:
            self.extra_gn_options.append('chip_mdns="platform"')

        if app == HostApp.TESTS:
            self.extra_gn_options.append('chip_build_tests=true')
            self.build_command = 'check'

        if app == HostApp.NL_TEST_RUNNER:
            self.build_command = 'runner'

        if app == HostApp.CERT_TOOL:
            # Certification only built for openssl
            if self.board == HostBoard.ARM64:
                # OpenSSL and mbedTLS conflicts.
                # We only cross compile with mbedTLS.
                raise Exception(
                    "Cannot cross compile CERT TOOL: ssl library conflict")
            self.extra_gn_options.append('chip_crypto="openssl"')
            self.build_command = 'src/tools/chip-cert'
        elif app == HostApp.ADDRESS_RESOLVE:
            self.build_command = 'src/lib/address_resolve:address-resolve-tool'
        elif app == HostApp.PYTHON_BINDINGS:
            self.extra_gn_options.append('enable_rtti=false')
            self.extra_gn_options.append('chip_project_config_include_dirs=["//config/python"]')
            self.build_command = 'python'

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
        elif self.board == HostBoard.FAKE:
            self.extra_gn_options.extend(
                [
                    'custom_toolchain="//build/toolchain/fake:fake_x64_gcc"',
                    'chip_link_tests=true',
                    'chip_device_platform="fake"',
                ]
            )
            return self.extra_gn_options
        else:
            raise Exception('Unknown host board type: %r' % self)

    def GnBuildEnv(self):
        if self.board == HostBoard.NATIVE:
            return None
        elif self.board == HostBoard.FAKE:
            return None
        elif self.board == HostBoard.ARM64:
            return {
                'PKG_CONFIG_PATH': os.path.join(
                    self.SysRootPath('SYSROOT_AARCH64'),
                    'lib/aarch64-linux-gnu/pkgconfig'),
            }
        else:
            raise Exception('Unknown host board type: %r' % self)

    def SysRootPath(self, name):
        if name not in os.environ:
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
