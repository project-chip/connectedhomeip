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

from .builder import BuilderOutput
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
                 use_coverage: bool = False,
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
            # Tizen test driver creates ISO image with all unit test files. So,
            # it uses twice as much space as regular build. Due to CI storage
            # limitations, we need to strip debug symbols from executables.
            self.extra_gn_options.append('strip_symbols=true')
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
        self.use_coverage = use_coverage
        if use_coverage:
            self.extra_gn_options.append('use_coverage=true')
        if with_ui:
            self.extra_gn_options.append('chip_examples_enable_ui=true')

    def generate(self):
        super(TizenBuilder, self).generate()
        if self.app == TizenApp.TESTS and self.use_coverage:
            self.coverage_dir = os.path.join(self.output_dir, 'coverage')
            self._Execute(['mkdir', '-p', self.coverage_dir], title="Create coverage output location")

    def lcov_args(self):
        gcov = os.path.join(os.environ['TIZEN_SDK_TOOLCHAIN'], 'bin/arm-linux-gnueabi-gcov')
        return [
            'lcov', '--gcov-tool', gcov, '--ignore-errors', 'unused,mismatch', '--capture', '--directory', os.path.join(
                self.output_dir, 'obj'),
            '--exclude', '**/src/controller/*',
            '--exclude', '**/connectedhomeip/zzz_generated/*',
            '--exclude', '**/connectedhomeip/third_party/*',
            '--exclude', '/opt/*',
        ]

    def PreBuildCommand(self):
        if self.app == TizenApp.TESTS and self.use_coverage:
            cmd = ['ninja', '-C', self.output_dir]

            if self.ninja_jobs is not None:
                cmd.append('-j' + str(self.ninja_jobs))

            cmd.append('Tizen')

            self._Execute(cmd, title="Build-only")

            self._Execute(self.lcov_args() + [
                '--initial',
                '--output-file', os.path.join(self.coverage_dir, 'lcov_base.info')
            ], title="Initial coverage baseline")

    def PostBuildCommand(self):
        if self.app == TizenApp.TESTS and self.use_coverage:

            self._Execute(self.lcov_args() + ['--output-file', os.path.join(self.coverage_dir,
                          'lcov_test.info')], title="Update coverage")

            gcov = os.path.join(os.environ['TIZEN_SDK_TOOLCHAIN'], 'bin/arm-linux-gnueabi-gcov')
            self._Execute(['lcov', '--gcov-tool', gcov, '--add-tracefile', os.path.join(self.coverage_dir, 'lcov_base.info'),
                           '--add-tracefile', os.path.join(self.coverage_dir, 'lcov_test.info'),
                           '--output-file', os.path.join(self.coverage_dir, 'lcov_final.info')
                           ], title="Final coverage info")
            self._Execute(['genhtml', os.path.join(self.coverage_dir, 'lcov_final.info'), '--output-directory',
                           os.path.join(self.coverage_dir, 'html')], title="HTML coverage")

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

    def _bundle(self):
        if self.app.is_tpk:
            logging.info('Packaging %s', self.output_dir)
            cmd = ['ninja', '-C', self.output_dir, self.app.value.name + ':tpk']
            self._Execute(cmd, title='Packaging ' + self.identifier)

    def build_outputs(self):
        for name in self.app.value.outputs:
            if not self.options.enable_link_map_file and name.endswith(".map"):
                continue
            yield BuilderOutput(
                os.path.join(self.output_dir, name),
                name)

    def bundle_outputs(self):
        if not self.app.is_tpk:
            return
        source = os.path.join(self.output_dir, self.app.package_name,
                              'out', self.app.package)
        yield BuilderOutput(source, self.app.package)
