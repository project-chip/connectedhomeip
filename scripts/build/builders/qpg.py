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
from typing import Optional

from .builder import BuilderOutput
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


class QpgFlavour(Enum):
    EXT_FLASH = 1

    def GnFlavourName(self):
        if self == QpgFlavour.EXT_FLASH:
            return '_ext_flash'
        else:
            raise Exception('Unknown flavour #: %r' % self)


class QpgBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: QpgApp = QpgApp.LIGHT,
                 board: QpgBoard = QpgBoard.QPG6105,
                 flavour: QpgFlavour = QpgFlavour.EXT_FLASH,
                 enable_rpcs: bool = False,
                 update_image: bool = False,
                 data_model_interface: Optional[str] = None,
                 ):
        super(QpgBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.board = board
        self.flavour = flavour
        self.enable_rpcs = enable_rpcs
        self.update_image = update_image
        self.data_model_interface = data_model_interface

    def GnBuildArgs(self):
        args = ['qpg_target_ic=\"%s\" qpg_flavour=\"%s\"' % (self.board.GnArgName(), self.flavour.GnFlavourName())]
        if self.enable_rpcs:
            args.append('import("//with_pw_rpc.gni")')
        if self.update_image:
            args.append('matter_ota_test_image=true')
        if self.data_model_interface:
            args.append(f'chip_use_data_model_interface="{self.data_model_interface}"')
        return args

    def build_outputs(self):
        extensions = ["out", "out.hex"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix()}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for name in filter(None, [x.strip() for x in f.readlines()]):
                yield BuilderOutput(
                    os.path.join(self.output_dir, name),
                    os.path.join('flashbundle', name))
