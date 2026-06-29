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

import contextlib
import os
from enum import Enum, auto

from runner.runner import Runner
from .builder import Builder, BuilderOutput, OutDirLock, lock_output_dir

# All Ameba targets delegate generation to the same Realtek SDK tree
# ($AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE), whose configure
# step (asdk/config.cmake's configure_file + the 'linux' GEN_OS scratch dir) is
# shared across applications rather than scoped to the per-target output_dir.
# When several Ameba targets generate in parallel they race on this shared tree,
# producing nondeterministic failures such as
# "mkdir: cannot create directory 'linux': File exists" and
# "CMake Error at asdk/config.cmake:20 (configure_file): No such file or
# directory", which leaves no build.ninja behind. Serialize generation on this
# constant key (compilation still runs in parallel, each in its own output_dir).
AMEBA_SDK_GENERATE_LOCK_KEY = 'ameba:realtek_amebaD_va0_example/GCC-RELEASE'


class AmebaBoard(Enum):
    AMEBAD = auto()


class AmebaApp(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    LIGHT = auto()
    LIGHT_SWITCH = auto()

    @property
    def ExampleName(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        if self == AmebaApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        if self == AmebaApp.LIGHT:
            return 'lighting-app'
        if self == AmebaApp.LIGHT_SWITCH:
            return 'light-switch-app'
        raise Exception('Unknown app type: %r' % self)

    @property
    def AppNamePrefix(self):
        if self == AmebaApp.ALL_CLUSTERS:
            return 'chip-ameba-all-clusters-app'
        if self == AmebaApp.ALL_CLUSTERS_MINIMAL:
            return 'chip-ameba-all-clusters-minimal-app'
        if self == AmebaApp.LIGHT:
            return 'chip-ameba-lighting-app'
        if self == AmebaApp.LIGHT_SWITCH:
            return 'chip-ameba-light-switch-app'
        raise Exception('Unknown app type: %r' % self)


class AmebaBuilder(Builder):

    def __init__(self,
                 root: str,
                 runner: Runner,
                 output_dir_lock: OutDirLock,
                 board: AmebaBoard = AmebaBoard.AMEBAD,
                 app: AmebaApp = AmebaApp.ALL_CLUSTERS):
        super().__init__(root, runner, output_dir_lock)
        self.board = board
        self.app = app

    @lock_output_dir
    def generate(self):
        cmd = '$AMEBA_PATH/project/realtek_amebaD_va0_example/GCC-RELEASE/build.sh '
        # <build root> <build_system> <output_directory> <application>

        cmd += ' '.join([self.root, 'ninja', self.output_dir,
                        self.app.ExampleName])

        # Serialize the Realtek SDK configure step across all Ameba targets: they
        # share one SDK tree, so concurrent generation races on it (see
        # AMEBA_SDK_GENERATE_LOCK_KEY). lock_dir() handles a None lock gracefully.
        shared_lock = self.output_dir_lock.lock_dir(AMEBA_SDK_GENERATE_LOCK_KEY) \
            if self.output_dir_lock is not None else contextlib.nullcontext()
        with shared_lock:
            self._Execute(['bash', '-c', cmd],
                          title='Generating ' + self.identifier)

    @lock_output_dir
    def _build(self):
        cmd = ['ninja', '-C', self.output_dir]

        if self.ninja_jobs is not None:
            cmd.append('-j' + str(self.ninja_jobs))

        self._Execute(cmd, title='Building ' + self.identifier)

    @lock_output_dir
    def build_outputs(self):
        yield BuilderOutput(
            os.path.join(self.output_dir, 'asdk', 'target_image2.axf'),
            self.app.AppNamePrefix + '.axf')
        if self.options.enable_link_map_file:
            yield BuilderOutput(
                os.path.join(self.output_dir, 'asdk', 'target_image2.map'),
                self.app.AppNamePrefix + '.map')
        yield BuilderOutput(
            os.path.join(self.output_dir, 'asdk', 'bootloader', 'km0_boot_all.bin'),
            'km0_boot_all.bin')
        yield BuilderOutput(
            os.path.join(self.output_dir, 'asdk', 'bootloader', 'km4_boot_all.bin'),
            'km4_boot_all.bin')
        yield BuilderOutput(
            os.path.join(self.output_dir, 'asdk', 'image', 'km0_km4_image2.bin'),
            'km0_km4_image2.bin')
