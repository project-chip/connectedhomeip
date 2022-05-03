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


class IMXApp(Enum):
    CHIP_TOOL = auto()
    LIGHT = auto()
    THERMOSTAT = auto()
    ALL_CLUSTERS = auto()
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

    def GnBuildEnv(self):
        return {
            'PKG_CONFIG_PATH': self.SysRootPath('IMX_SDK_ROOT') + '/sysroots/cortexa53-crypto-poky-linux/lib/aarch64-linux-gnu/pkgconfig',
        }

    def GnBuildArgs(self):
        args = [
            'target_os="linux"',
            'target_cpu="arm64"',
            'arm_arch="armv8-a"',
            'import(\"//build_overrides/build.gni\")',
            'custom_toolchain=\"${build_root}/toolchain/custom\"',
            'sysroot="%s/sysroots/cortexa53-crypto-poky-linux"' % self.SysRootPath('IMX_SDK_ROOT'),
            'target_cflags=[ "-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\\"mlan0\\"", "-DCHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD=\\"udhcpc -b -i %s \\"" ]',
            'target_cc="%s/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc"' % self.SysRootPath(
                'IMX_SDK_ROOT'),
            'target_cxx="%s/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++"' % self.SysRootPath(
                'IMX_SDK_ROOT'),
            'target_ar="%s/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ar"' % self.SysRootPath(
                'IMX_SDK_ROOT'),
        ]

        if self.release:
            args.append('is_debug=false')

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
