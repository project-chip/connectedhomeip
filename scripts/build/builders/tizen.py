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

import logging
import os
from enum import Enum, auto

from .gn import GnBuilder


class TizenApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == TizenApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == TizenApp.LIGHT:
            return 'chip-tizen-lighting-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == TizenApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)


class TizenBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: TizenApp = TizenApp.LIGHT):
        super(TizenBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExampleName(), 'linux'),
            runner=runner,
            output_prefix=output_prefix)

        self.app = app
        tizen_home = os.environ['TIZEN_HOME']
        os.environ['PKG_CONFIG_SYSROOT_DIR'] = tizen_home
        os.environ['PKG_CONFIG_LIBDIR'] = tizen_home + '/usr/lib/pkgconfig'
        os.environ['PKG_CONFIG_PATH'] = tizen_home + '/usr/lib/pkgconfig'
        self.gn_build_args = ['target_os="tizen" target_cpu="arm" arm_arch="armv7-a" import("//build_overrides/build.gni") target_cflags=[ "--sysroot=' + tizen_home + '" ] target_ldflags=[ "--sysroot=' + tizen_home + '" ] custom_toolchain="${build_root}/toolchain/custom" target_cc="' + tizen_home + '/bin/arm-linux-gnueabi-gcc" target_cxx="' + tizen_home + '/bin/arm-linux-gnueabi-g++" target_ar="' + tizen_home + '/bin/arm-linux-gnueabi-ar"']

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items
