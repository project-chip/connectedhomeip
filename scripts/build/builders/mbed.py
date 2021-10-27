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
import pathlib

from enum import Enum, auto
from .builder import Builder

from mbed_tools.project import MbedProgram
from mbed_tools.build import generate_config, build_project

class MbedApp(Enum):
    LOCK = auto()
    LIGHT = auto()
    ALL_CLUSTERS = auto()
    PIGWEED = auto()
    SHELL = auto()

    @property
    def ExampleName(self):
        if self == MbedApp.LOCK:
            return 'lock-app'
        elif self == MbedApp.LIGHT:
            return 'lighting-app'
        elif self == MbedApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == MbedApp.PIGWEED:
            return 'pigweed-app'
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
        elif self == MbedApp.PIGWEED:
            return 'chip-mbed-pigweed-app-example'
        elif self == MbedApp.SHELL:
            return 'shell'
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
        self.mbed_os_path = os.path.join(os.environ['PW_PROJECT_ROOT'], 'third_party', 'mbed-os', 'repo')
        self.mbed_os_posix_socket_path = os.path.join(os.environ['PW_PROJECT_ROOT'], 'third_party', 'mbed-os-posix-socket', 'repo')

        cmake_build_subdir = pathlib.Path(self.board.BoardName.upper(), self.profile.ProfileName.lower(), self.toolchain.upper())
        self.program = MbedProgram.from_existing(pathlib.Path(self.ExamplePath), cmake_build_subdir, pathlib.Path(self.mbed_os_path))
        

    @property
    def ExamplePath(self):
        return os.path.join('examples', self.app.ExampleName, 'mbed')

    def generate(self):
        if not os.path.exists(self.output_dir):
            self.program.files.cmake_build_dir = pathlib.Path(self.output_dir)
            _, output_path = generate_config(self.board.BoardName.upper(), self.toolchain, self.program)
            logging.info(f"mbed_config.cmake has been generated and written to '{str(output_path.resolve())}'")

            self._Execute(['cmake', '-S', shlex.quote(self.ExamplePath), '-B', shlex.quote(self.output_dir), '-GNinja', 
                        '-DCMAKE_BUILD_TYPE={}'.format(self.profile.ProfileName.lower()),
                        '-DMBED_OS_PATH={}'.format(shlex.quote(self.mbed_os_path)),
                        '-DMBED_OS_POSIX_SOCKET_PATH={}'.format(shlex.quote(self.mbed_os_posix_socket_path)),
                        ], title='Generating ' + self.identifier)
            

    def _build(self):
        logging.info('Compiling Mbed at %s', self.output_dir)
        build_project(pathlib.Path(self.output_dir))

    def build_outputs(self):
        return {
            self.app.AppNamePrefix + '.elf':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.elf'),
            self.app.AppNamePrefix + '.map':
                os.path.join(self.output_dir, self.app.AppNamePrefix + '.map'),
        }
