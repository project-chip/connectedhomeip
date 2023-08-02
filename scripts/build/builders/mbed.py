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
import shlex
from enum import Enum, auto

from .builder import Builder


class MbedApp(Enum):
    LOCK = auto()
    LIGHT = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    PIGWEED = auto()
    SHELL = auto()
    OTA_REQUESTOR = auto()

    @property
    def ExampleName(self):
        if self == MbedApp.LOCK:
            return 'lock-app'
        elif self == MbedApp.LIGHT:
            return 'lighting-app'
        elif self == MbedApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == MbedApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        elif self == MbedApp.PIGWEED:
            return 'pigweed-app'
        elif self == MbedApp.OTA_REQUESTOR:
            return 'ota-requestor-app'
        elif self == MbedApp.SHELL:
            return 'shell'
        else:
            raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == MbedApp.LOCK:
            return 'chip-mbed-lock-app-example'
        elif self == MbedApp.LIGHT:
            return 'chip-mbed-lighting-app-example'
        elif self == MbedApp.ALL_CLUSTERS:
            return 'chip-mbed-all-clusters-app-example'
        elif self == MbedApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-mbed-all-clusters-minimal-app-example'
        elif self == MbedApp.PIGWEED:
            return 'chip-mbed-pigweed-app-example'
        elif self == MbedApp.OTA_REQUESTOR:
            return 'chip-mbed-ota-requestor-app-example'
        elif self == MbedApp.SHELL:
            return 'chip-mbed-shell-example'
        else:
            raise Exception('Unknown app type: %r' % self)


class MbedBoard(Enum):
    CY8CPROTO_062_4343W = auto()

    @property
    def BoardName(self):
        if self == MbedBoard.CY8CPROTO_062_4343W:
            return 'CY8CPROTO_062_4343W'
        else:
            raise Exception('Unknown board type: %r' % self)


class MbedProfile(Enum):
    RELEASE = auto()
    DEVELOP = auto()
    DEBUG = auto()

    @property
    def ProfileName(self):
        if self == MbedProfile.RELEASE:
            return 'release'
        elif self == MbedProfile.DEVELOP:
            return 'develop'
        elif self == MbedProfile.DEBUG:
            return 'debug'
        else:
            raise Exception('Unknown board type: %r' % self)


class MbedBuilder(Builder):
    def __init__(self,
                 root,
                 runner,
                 app: MbedApp = MbedApp.LOCK,
                 board: MbedBoard = MbedBoard.CY8CPROTO_062_4343W,
                 profile: MbedProfile = MbedProfile.RELEASE):
        super(MbedBuilder, self).__init__(root, runner)
        self.app = app
        self.board = board
        self.profile = profile
        self.toolchain = "GCC_ARM"
        self.mbed_os_path = os.path.join(
            self.root, 'third_party', 'mbed-os', 'repo')
        self.mbed_os_posix_socket_path = os.path.join(
            self.root, 'third_party', 'mbed-os-posix-socket', 'repo')

    @property
    def ExamplePath(self):
        return os.path.join(self.root, 'examples', self.app.ExampleName, 'mbed')

    def generate(self):
        if not os.path.exists(self.output_dir):
            self._Execute(['mbed-tools', 'configure',
                           '-t', self.toolchain,
                           '-m', self.board.BoardName,
                           '-p', self.ExamplePath,
                           '-o', self.output_dir,
                           '--mbed-os-path', self.mbed_os_path,
                           ], title='Generating config ' + self.identifier)

            flags = []
            flags.append(f"-DMBED_OS_PATH={shlex.quote(self.mbed_os_path)}")
            flags.append(f"-DMBED_OS_PATH={shlex.quote(self.mbed_os_path)}")
            flags.append(f"-DMBED_OS_POSIX_SOCKET_PATH={shlex.quote(self.mbed_os_posix_socket_path)}")

            if self.options.pregen_dir:
                flags.append(f"-DCHIP_CODEGEN_PREGEN_DIR={shlex.quote(self.options.pregen_dir)}")

            self._Execute(['cmake', '-S', shlex.quote(self.ExamplePath), '-B', shlex.quote(self.output_dir),
                          '-GNinja'] + flags, title='Generating ' + self.identifier)

    def _build(self):
        # Remove old artifacts to force linking
        cmd = 'rm -rf {}/chip-*'.format(self.output_dir)
        self._Execute(['bash', '-c', cmd],
                      title='Remove old artifacts ' + self.identifier)

        self._Execute(['cmake', '--build', shlex.quote(self.output_dir)],
                      title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            self.app.AppNamePrefix + '.elf':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
            self.app.AppNamePrefix + '.hex':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.hex'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir,
                             self.app.AppNamePrefix + '.elf.map'),
        }
