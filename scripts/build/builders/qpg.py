# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
from enum import Enum, auto

from .gn import GnBuilder


class QpgApp(Enum):
    LIGHT = auto()
    LOCK = auto()
    SHELL = auto()
    PERSISTENT_STORAGE = auto()

    def ExampleName(self):
        if self == QpgApp.LIGHT:
            return 'lighting-app'
        elif self == QpgApp.LOCK:
            return 'lock-app'
        elif self == QpgApp.SHELL:
            return 'shell'
        elif self == QpgApp.PERSISTENT_STORAGE:
            return 'persistent-storage'
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
                 enable_rpcs: bool = False):
        super(QpgBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.board = board
        self.enable_rpcs = enable_rpcs

    def GnBuildArgs(self):
        args = ['qpg_target_ic=\"%s\"' % self.board.GnArgName()]
        if self.enable_rpcs:
            args.append('import("//with_pw_rpc.gni")')
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
