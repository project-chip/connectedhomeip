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


class TelinkApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == TelinkApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == TelinkApp.LIGHT:
            return 'chip-telink-lighting-example'
        else:
            raise Exception('Unknown app type: %r' % self)


class TelinkBoard(Enum):
    TLSR9518ADK80D = auto()

    def GnArgName(self):
        if self == TelinkBoard.TLSR9518ADK80D:
            return 'tlsr9518adk80d'
        else:
            raise Exception('Unknown board type: %r' % self)


class TelinkBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: TelinkApp = TelinkApp.LIGHT,
                 board: TelinkBoard = TelinkBoard.TLSR9518ADK80D):
        super(TelinkBuilder, self).__init__(root, runner, output_prefix)
        self.app = app
        self.board = board

    def generate(self):

        if not os.path.exists(self.output_dir):

            if not self._runner.dry_run:

                # Check Zephyr base
                if 'TELINK_ZEPHYR_BASE' not in os.environ:
                    raise Exception(
                        "Telink builds require TELINK_ZEPHYR_BASE to be set")

                # Check Telink toolchain
                if 'TELINK_TOOLCHAIN_PATH' not in os.environ:
                    raise Exception(
                        "Telink requires TELINK_TOOLCHAIN_PATH to be set")

            cmd = '''
source "$TELINK_ZEPHYR_BASE/zephyr-env.sh";
export ZEPHYR_TOOLCHAIN_VARIANT=cross-compile;
export CROSS_COMPILE=$TELINK_TOOLCHAIN_PATH/riscv32-elf-;
west build -d {outdir} -b {board} {sourcedir}
        '''.format(
                outdir=shlex.quote(
                    self.output_dir), board=self.board.GnArgName(), sourcedir=shlex.quote(
                    os.path.join(
                        self.root, 'examples', self.app.ExampleName(), 'telink'))).strip()

            self._Execute(['bash', '-c', cmd],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Telink at %s', self.output_dir)

        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

    def build_outputs(self):
        return {
            '%s.elf' %
            self.app.AppNamePrefix(): os.path.join(
                self.output_dir,
                'zephyr',
                'zephyr.elf'),
            '%s.map' %
            self.app.AppNamePrefix(): os.path.join(
                self.output_dir,
                'zephyr',
                'zephyr.map'),
        }
