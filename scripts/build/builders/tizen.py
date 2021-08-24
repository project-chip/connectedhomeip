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
import shlex

from enum import Enum, auto

from .builder import Builder


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


class TizenBuilder(Builder):

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

    def generate(self):
        if not os.path.exists(self.output_dir):
            if not self._runner.dry_run:
                if 'TIZEN_HOME' not in os.environ:
                    raise Exception(
                        "Environment TIZEN_HOME missing, cannot build tizen libraries")

            tizen_home = os.environ['TIZEN_HOME']
            cmd = '''\
export PKG_CONFIG_SYSROOT_DIR=$TIZEN_HOME;
export PKG_CONFIG_LIBDIR=$TIZEN_HOME/usr/lib/pkgconfig;
export PKG_CONFIG_PATH=$TIZEN_HOME/usr/lib/pkgconfig;
gn gen --check --fail-on-unused-args --root=%s '--args=target_os="tizen" \
target_cpu="arm" arm_arch="armv7-a" import("//build_overrides/build.gni") \
target_cflags=[ "--sysroot=%s", "-Wno-sign-compare" ] \
target_ldflags=[ "--sysroot=%s" ] \
custom_toolchain="${build_root}/toolchain/custom" \
target_cc="%s/bin/arm-linux-gnueabi-gcc" \
target_cxx="%s/bin/arm-linux-gnueabi-g++" \
target_ar="%s/bin/arm-linux-gnueabi-ar"' %s''' % (
                self.root, tizen_home, tizen_home, tizen_home, tizen_home, tizen_home, self.output_dir)

            self._Execute(['bash', '-c', cmd],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Telink at %s', self.output_dir)

        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

    def build_outputs(self):
        items = {
            '%s.out' % self.app.AppNamePrefix():
                os.path.join(self.output_dir, '%s.out' %
                             self.app.AppNamePrefix()),
            '%s.out.map' % self.app.AppNamePrefix():
                os.path.join(self.output_dir,
                             '%s.out.map' % self.app.AppNamePrefix()),
        }

        return items
