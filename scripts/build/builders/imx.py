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
from enum import Enum, auto

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
        entries = os.listdir(self.SysRootPath('IMX_SDK_ROOT'))
        for entry in entries:
            if re.match(r'^environment-setup-', entry):
                env_setup_script = entry
                break

        with open(os.path.join(self.SysRootPath('IMX_SDK_ROOT'), env_setup_script), 'r') as env_setup_script_fd:
            lines = env_setup_script_fd.readlines()
            for line in lines:
                line = line.strip('\n')
                if re.match(r'export SDKTARGETSYSROOT=', line):
                    sdk_target_sysroot = line[len(r'export SDKTARGETSYSROOT='):]
                if re.match(r'export CC=', line):
                    # remove the quotation marks, replace a shell env
                    cc = line[len(r'export CC=')+1: -1]
                    cc = cc.replace('$SDKTARGETSYSROOT', sdk_target_sysroot)
                if re.match(r'export CXX=', line):
                    # remove the quotation marks, replace a shell env
                    cxx = line[len(r'export CXX=')+1: -1]
                    cxx = cxx.replace('$SDKTARGETSYSROOT', sdk_target_sysroot)
                if re.match(r'export ARCH=', line):
                    target_cpu = line[len(r'export ARCH='):]
                    if target_cpu == 'arm64':
                        arm_arch = 'armv8-a'
                    elif target_cpu == 'arm':
                        arm_arch = 'armv7ve'
                if re.match(r'export CROSS_COMPILE=', line):
                    cross_compile = line[len(r'export CROSS_COMPILE='):-1]
                    print(cross_compile)

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
