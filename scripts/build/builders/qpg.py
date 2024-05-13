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
from enum import Enum, auto

from .gn import GnBuilder


class QpgApp(Enum):
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    PERSISTENT_STORAGE = auto()
    LIGHT_SWITCH = auto()
    THERMOSTAT = auto()

    def ExampleName(self):
        if self == QpgApp.LIGHT:
            return 'lighting-app'
        elif self == QpgApp.LOCK:
            return 'lock-app'
        elif self == QpgApp.SHELL:
            return 'shell'
        elif self == QpgApp.PERSISTENT_STORAGE:
            return 'persistent-storage'
        elif self == QpgApp.LIGHT_SWITCH:
            return 'light-switch-app'
        elif self == QpgApp.THERMOSTAT:
            return 'thermostat'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == QpgApp.LIGHT:
            return 'chip-qpg6105-lighting-example'
        elif self == QpgApp.LOCK:
            return 'chip-qpg6105-lock-example'
        elif self == QpgApp.SHELL:
            return 'chip-qpg6105-shell-example'
        elif self == QpgApp.PERSISTENT_STORAGE:
            return 'chip-qpg6105-persistent_storage-example'
        elif self == QpgApp.LIGHT_SWITCH:
            return 'chip-qpg6105-light-switch-example'
        elif self == QpgApp.THERMOSTAT:
            return 'chip-qpg6105-thermostat-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == QpgApp.LIGHT:
            return 'lighting_app.out.flashbundle.txt'
        elif self == QpgApp.LOCK:
            return 'lock_app.out.flashbundle.txt'
        elif self == QpgApp.SHELL:
            return 'shell_app.out.flashbundle.txt'
        elif self == QpgApp.PERSISTENT_STORAGE:
            return 'persistent_storage_app.out.flashbundle.txt'
        elif self == QpgApp.LIGHT_SWITCH:
            return 'light_switch_app.out.flashbundle.txt'
        elif self == QpgApp.THERMOSTAT:
            return 'thermostat.out.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'qpg')


class QpgBoard(Enum):
    QPG6105 = 1

    def GnArgName(self):
        if self == QpgBoard.QPG6105:
            return 'qpg6105'
        else:
            raise Exception('Unknown board #: %r' % self)


class QpgBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: QpgApp = QpgApp.LIGHT,
                 board: QpgBoard = QpgBoard.QPG6105,
                 enable_rpcs: bool = False,
                 update_image: bool = False):
        super(QpgBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.board = board
        self.enable_rpcs = enable_rpcs
        self.update_image = update_image

    def GnBuildArgs(self):
        args = ['qpg_target_ic=\"%s\"' % self.board.GnArgName()]
        if self.enable_rpcs:
            args.append('import("//with_pw_rpc.gni")')
        if self.update_image:
            args.append('matter_device_software_version_string=\"1.1_OTA_TEST\" matter_device_software_version=4')
        return args

    def build_outputs(self):
        items = {}
        for extension in ["out", "out.map", "out.hex"]:
            name = '%s.%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items
