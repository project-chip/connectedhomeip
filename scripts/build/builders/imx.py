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
import re
import shlex
from enum import Enum, auto

from .builder import BuilderOutput
from .gn import GnBuilder


class IMXApp(Enum):
    CHIP_TOOL = auto()
    LIGHT = auto()
    THERMOSTAT = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    OTA_PROVIDER = auto()

    def ExamplePath(self):
        if self == IMXApp.CHIP_TOOL:
            return 'chip-tool'
        if self == IMXApp.LIGHT:
            return 'lighting-app/linux'
        if self == IMXApp.THERMOSTAT:
            return 'thermostat/linux'
        if self == IMXApp.ALL_CLUSTERS:
            return 'all-clusters-app/linux'
        if self == IMXApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app/linux'
        if self == IMXApp.OTA_PROVIDER:
            return 'ota-provider-app/linux'

    def OutputNames(self):
        if self == IMXApp.CHIP_TOOL:
            yield 'chip-tool'
            yield 'chip-tool.map'
        if self == IMXApp.LIGHT:
            yield 'chip-lighting-app'
            yield 'chip-lighting-app.map'
        if self == IMXApp.THERMOSTAT:
            yield 'thermostat-app'
            yield 'thermostat-app.map'
        if self == IMXApp.ALL_CLUSTERS:
            yield 'chip-all-clusters-app'
            yield 'chip-all-clusters-app.map'
        if self == IMXApp.ALL_CLUSTERS_MINIMAL:
            yield 'chip-all-clusters-minimal-app'
            yield 'chip-all-clusters-minimal-app.map'
        if self == IMXApp.OTA_PROVIDER:
            yield 'chip-ota-provider-app'
            yield 'chip-ota-provider-app.map'


class IMXBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: IMXApp,
                 release: bool = False):
        super(IMXBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner)
        self.release = release
        self.app = app

    def GnBuildArgs(self):
        try:
            entries = os.listdir(self.SysRootPath('IMX_SDK_ROOT'))
        except FileNotFoundError:
            if self.SysRootPath('IMX_SDK_ROOT') == 'IMX_SDK_ROOT':
                # CI test, use default value
                target_cpu = 'arm64'
                arm_arch = 'armv8-a'
                sdk_target_sysroot = os.path.join(self.SysRootPath('IMX_SDK_ROOT'), 'sysroots/cortexa53-crypto-poky-linux')
                cross_compile = 'aarch64-poky-linux'
                cc = 'aarch64-poky-linux-gcc'
                cxx = 'aarch64-poky-linux-g++'
            else:
                raise Exception('the value of env IMX_SDK_ROOT is not a valid path.')
        else:
            for entry in entries:
                if entry.startswith(r'environment-setup-'):
                    env_setup_script = entry
                    break

            try:
                env_setup_script
            except NameError:
                raise Exception('The SDK environment setup script is not found, make sure the env IMX_SDK_ROOT is correctly set.')
            else:

                with open(os.path.join(self.SysRootPath('IMX_SDK_ROOT'), env_setup_script), 'r') as env_setup_script_fd:
                    lines = env_setup_script_fd.readlines()
                    for line in lines:
                        line = line.strip('\n')
                        m = re.match(r'^\s*export\s+SDKTARGETSYSROOT=(.*)', line)
                        if m:
                            sdk_target_sysroot = shlex.split(m.group(1))[0]

                        m = re.match(r'^\s*export\s+CC=(.*)', line)
                        if m:
                            cc = shlex.split(m.group(1))[0]
                        m = re.match(r'^\s*export\s+CXX=(.*)', line)
                        if m:
                            cxx = shlex.split(m.group(1))[0]

                        m = re.match(r'^\s*export\s+ARCH=(.*)', line)
                        if m:
                            target_cpu = shlex.split(m.group(1))[0]
                            if target_cpu == 'arm64':
                                arm_arch = 'armv8-a'
                            elif target_cpu == 'arm':
                                arm_arch = 'armv7ve'
                            else:
                                raise Exception('ARCH should be arm64 or arm in the SDK environment setup script.')

                        m = re.match(r'^\s*export\s+CROSS_COMPILE=(.*)', line)
                        if m:
                            cross_compile = shlex.split(m.group(1))[0][:-1]

                try:
                    sdk_target_sysroot
                except NameError:
                    raise Exception('SDKTARGETSYSROOT is not found in the SDK environment setup script.')
                else:
                    try:
                        cc
                        cxx
                    except NameError:
                        raise Exception('CC and/or CXX are not found in the SDK environment setup script.')
                    else:
                        cc = cc.replace('$SDKTARGETSYSROOT', sdk_target_sysroot)
                        cxx = cxx.replace('$SDKTARGETSYSROOT', sdk_target_sysroot)
                try:
                    target_cpu
                    cross_compile
                except NameError:
                    raise Exception('ARCH and/or CROSS_COMPILE are not found in the SDK environment setup script.')

        args = [
            'treat_warnings_as_errors=false',
            'target_os="linux"',
            'target_cpu="%s"' % target_cpu,
            'arm_arch="%s"' % arm_arch,
            'import(\"//build_overrides/build.gni\")',
            'custom_toolchain=\"${build_root}/toolchain/custom\"',
            'sysroot="%s"' % sdk_target_sysroot,
            'target_cflags=[ "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\\"mlan0\\"", "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\\"udhcpc -b -i %s \\"" ]',
            'target_cc="%s/sysroots/x86_64-pokysdk-linux/usr/bin/%s/%s"' % (self.SysRootPath('IMX_SDK_ROOT'), cross_compile,
                                                                            cc),
            'target_cxx="%s/sysroots/x86_64-pokysdk-linux/usr/bin/%s/%s"' % (self.SysRootPath('IMX_SDK_ROOT'), cross_compile,
                                                                             cxx),
            'target_ar="%s/sysroots/x86_64-pokysdk-linux/usr/bin/%s/%s-ar"' % (self.SysRootPath('IMX_SDK_ROOT'), cross_compile,
                                                                               cross_compile),
        ]

        if self.release:
            args.append('is_debug=false')
        else:
            args.append('optimize_debug=true')

        return args

    def SysRootPath(self, name):
        if name not in os.environ:
            raise Exception('Missing environment variable "%s"' % name)
        return os.environ[name]

    def build_outputs(self):
        for name in self.app.OutputNames():
            if not self.options.enable_link_map_file and name.endswith(".map"):
                continue
            path = os.path.join(self.output_dir, name)
            if os.path.isdir(path):
                for root, dirs, files in os.walk(path):
                    for file in files:
                        yield BuilderOutput(os.path.join(root, file), file)
            else:
                yield BuilderOutput(os.path.join(self.output_dir, name), name)
