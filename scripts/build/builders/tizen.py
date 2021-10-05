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

from .builder import Builder


class TizenApp(Enum):
    LIGHT = auto()

    def ExampleName(self):
        if self == TizenApp.LIGHT:
            return 'lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppName(self):
        if self == TizenApp.LIGHT:
            return 'chip-lighting-app'
        else:
            raise Exception('Unknown app type: %r' % self)


class TizenBoard(Enum):
    ARM = auto()

    def TargetCpuName(self):
        if self == TizenBoard.ARM:
            return 'arm'
        else:
            raise Exception('Unknown board type: %r' % self)


class TizenBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 output_prefix: str,
                 app: TizenApp = TizenApp.LIGHT,
                 board: TizenBoard = TizenBoard.ARM):
        super(TizenBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExampleName(), 'linux'),
            runner=runner,
            output_prefix=output_prefix)
        self.app = app
        self.board = board

    def generate(self):
        if not os.path.exists(self.output_dir):
            if not self._runner.dry_run:
                if 'TIZEN_HOME' not in os.environ:
                    raise Exception(
                        "Environment TIZEN_HOME missing, cannot build tizen libraries")

            cmd = '''\
gn gen --check --fail-on-unused-args --root=%s '--args=''' % self.root

            gn_args = {}
            gn_args['target_os'] = 'tizen'
            gn_args['target_cpu'] = self.board.TargetCpuName()
            gn_args['sysroot'] = os.environ['TIZEN_HOME']

            cmd += ' %s\' %s' % (' '.join([
                '%s="%s"' % (key, value)
                for key, value in gn_args.items()]), self.output_dir)

            self._Execute(['bash', '-c', cmd],
                          title='Generating ' + self.identifier)

    def _build(self):
        logging.info('Compiling Tizen at %s', self.output_dir)

        self._Execute(['ninja', '-C', self.output_dir],
                      title='Building ' + self.identifier)

    def build_outputs(self):
        items = {
            '%s' % self.app.AppName():
                os.path.join(self.output_dir, self.app.AppName()),
            '%s.map' % self.app.AppName():
                os.path.join(self.output_dir, '%s.map' % self.app.AppName()),
        }

        return items
