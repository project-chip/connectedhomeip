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

from .host import HostBuilder


class IMXApp(Enum):
    CHIP_TOOL = auto()
    LIGHT = auto()

    def ExamplePath(self):
        if self == IMXApp.CHIP_TOOL:
            return 'chip-tool'
        if self == IMXApp.LIGHT:
            return 'lighting-app/linux'

    def OutputNames(self):
        if self == IMXApp.CHIP_TOOL:
            yield 'chip-tool'
            yield 'chip-tool.map'
        if self == IMXApp.LIGHT:
            yield 'chip-lighting-app'
            yield 'chip-lighting-app.map'


class IMXBuilder(HostBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: IMXApp,
                 release: bool = False):
        super(IMXBuilder, self).__init__(
            root=root,
            runner=runner,
            app=app)
        self.release = release

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
