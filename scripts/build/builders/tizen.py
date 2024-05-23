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
from collections import namedtuple
from enum import Enum
from xml.etree import ElementTree as ET

from .gn import GnBuilder

Board = namedtuple('Board', ['target_cpu'])
App = namedtuple('App', ['name', 'source', 'outputs'])
Tool = namedtuple('Tool', ['name', 'source', 'outputs'])
TestDriver = namedtuple('TestDriver', ['name', 'source'])


class TizenBoard(Enum):

    ARM = Board('arm')


class TizenApp(Enum):

    ALL_CLUSTERS = App(
        'chip-all-clusters-app',
        'examples/all-clusters-app/tizen',
        ('chip-all-clusters-app',
         'chip-all-clusters-app.map'))
    ALL_CLUSTERS_MINIMAL = App(
        'chip-all-clusters-minimal-app',
        'examples/all-clusters-minimal-app/tizen',
        ('chip-all-clusters-minimal-app',
         'chip-all-clusters-minimal-app.map'))
    LIGHT = App(
        'chip-lighting-app',
        'examples/lighting-app/tizen',
        ('chip-lighting-app',
         'chip-lighting-app.map'))

    CHIP_TOOL = Tool(
        'chip-tool',
        'examples/chip-tool',
        ('chip-tool',
         'chip-tool.map'))

    TESTS = TestDriver(
        'tests',
        'src/test_driver/tizen')

    @property
    def is_tpk(self):
        """If True, this app is a TPK."""
        return isinstance(self.value, App)

    @property
    def package(self):
        return f'{self.package_name}-{self.package_version}.tpk'

    @property
    def package_name(self):
        return self.manifest.get('package')

    @property
    def package_version(self):
        return self.manifest.get('version')

    def parse_manifest(self, manifest: str):
        self.manifest = ET.parse(manifest).getroot()


class TizenBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: TizenApp = TizenApp.LIGHT,
                 board: TizenBoard = TizenBoard.ARM,
                 enable_ble: bool = True,
                 enable_thread: bool = True,
                 enable_wifi: bool = True,
                 use_asan: bool = False,
                 use_tsan: bool = False,
                 use_ubsan: bool = False,
                 with_ui: bool = False,
                 ):
        super(TizenBuilder, self).__init__(
            root=os.path.join(root, app.value.source),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []

        if self.app.is_tpk:
            try:
                # Try to load Tizen application XML manifest. We have to use
                # try/except here, because of TestBuilder test. This test runs
                # in a fake build root /TEST/BUILD/ROOT which obviously does
                # not have Tizen manifest file.
                self.app.parse_manifest(
                    os.path.join(self.root, "tizen-manifest.xml"))
            except FileNotFoundError:
                pass

        if app == TizenApp.TESTS:
            self.extra_gn_options.append('chip_build_tests=true')
            self.build_command = 'check'

        if not enable_ble:
            self.extra_gn_options.append('chip_config_network_layer_ble=false')
        if not enable_thread:
            self.extra_gn_options.append('chip_enable_openthread=false')
        if not enable_wifi:
            self.extra_gn_options.append('chip_enable_wifi=false')
        if use_asan:
            self.extra_gn_options.append('is_asan=true')
        if use_tsan:
            raise Exception("TSAN sanitizer not supported by Tizen toolchain")
        if use_ubsan:
            self.extra_gn_options.append('is_ubsan=true')
        if with_ui:
            self.extra_gn_options.append('chip_examples_enable_ui=true')

    def GnBuildArgs(self):
        # Make sure that required ENV variables are defined
        for env in ('TIZEN_SDK_ROOT', 'TIZEN_SDK_SYSROOT'):
            if env not in os.environ:
                raise Exception(
                    "Environment %s missing, cannot build Tizen target" % env)

        return self.extra_gn_options + [
            'target_os="tizen"',
            'target_cpu="%s"' % self.board.value.target_cpu,
            'tizen_sdk_root="%s"' % os.environ['TIZEN_SDK_ROOT'],
            'tizen_sdk_sysroot="%s"' % os.environ['TIZEN_SDK_SYSROOT'],
        ]

    def _generate_flashbundle(self):
        if self.app.is_tpk:
            logging.info('Packaging %s', self.output_dir)
            cmd = ['ninja', '-C', self.output_dir, self.app.value.name + ':tpk']
            self._Execute(cmd, title='Packaging ' + self.identifier)

    def build_outputs(self):
        return {
            output: os.path.join(self.output_dir, output)
            for output in self.app.value.outputs
        }

    def flashbundle(self):
        if not self.app.is_tpk:
            return {}
        return {
            self.app.package: os.path.join(self.output_dir,
                                           self.app.package_name, 'out',
                                           self.app.package),
        }
