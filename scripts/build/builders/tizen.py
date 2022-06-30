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
from xml.etree import ElementTree as ET

from .gn import GnBuilder


class TizenApp(Enum):

    CHIP_TOOL = auto()
    LIGHT = auto()

    def ExamplePath(self):
        if self == TizenApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == TizenApp.LIGHT:
            return 'lighting-app/tizen'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppName(self):
        if self == TizenApp.LIGHT:
            return 'chip-lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def PackageName(self):
        return self.manifest.get('package')

    def PackageVersion(self):
        return self.manifest.get('version')

    def parse_manifest(self, manifest: str):
        self.manifest = ET.parse(manifest).getroot()


class TizenBoard(Enum):
    ARM = auto()

    def TargetCpuName(self):
        if self == TizenBoard.ARM:
            return 'arm'
        else:
            raise Exception('Unknown board type: %r' % self)


class TizenBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: TizenApp = TizenApp.LIGHT,
                 board: TizenBoard = TizenBoard.ARM,
                 enable_ble: bool = True,
                 enable_wifi: bool = True,
                 use_asan: bool = False,
                 use_tsan: bool = False,
                 ):
        super(TizenBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []

        try:
            # Try to load Tizen application XML manifest. We have to use
            # try/except here, because of TestBuilder test. This test runs
            # in a fake build root /TEST/BUILD/ROOT which obviously does
            # not have Tizen manifest file.
            self.app.parse_manifest(
                os.path.join(self.root, "tizen-manifest.xml"))
        except FileNotFoundError:
            pass

        if not enable_ble:
            self.extra_gn_options.append('chip_config_network_layer_ble=false')
        if not enable_wifi:
            self.extra_gn_options.append('chip_enable_wifi=false')
        if use_asan:
            self.extra_gn_options.append('is_asan=true')
        if use_tsan:
            raise Exception("TSAN sanitizer not supported by Tizen toolchain")

    def GnBuildArgs(self):
        # Make sure that required ENV variables are defined
        for env in ('TIZEN_SDK_ROOT', 'TIZEN_SDK_SYSROOT'):
            if env not in os.environ:
                raise Exception(
                    "Environment %s missing, cannot build Tizen target" % env)

        return self.extra_gn_options + [
            'target_os="tizen"',
            'target_cpu="%s"' % self.board.TargetCpuName(),
            'tizen_sdk_root="%s"' % os.environ['TIZEN_SDK_ROOT'],
            'tizen_sdk_sysroot="%s"' % os.environ['TIZEN_SDK_SYSROOT'],
        ]

    def _generate_flashbundle(self):
        logging.info('Packaging %s', self.output_dir)
        cmd = ['ninja', '-C', self.output_dir, self.app.AppName() + ':tpk']
        self._Execute(cmd, title='Packaging ' + self.identifier)

    def build_outputs(self):
        return {
            '%s' % self.app.AppName():
                os.path.join(self.output_dir, self.app.AppName()),
            '%s.map' % self.app.AppName():
                os.path.join(self.output_dir, '%s.map' % self.app.AppName()),
        }

    def flashbundle(self):
        tpk = f'{self.app.PackageName()}-{self.app.PackageVersion()}.tpk'
        return {
            tpk: os.path.join(self.output_dir, 'package', 'out', tpk),
        }
