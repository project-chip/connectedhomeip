# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import os
from enum import Enum, auto

from .gn import GnBuilder


class cc32xxApp(Enum):
    LOCK = auto()

    def ExampleName(self):
        if self == cc32xxApp.LOCK:
            return 'lock-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == cc32xxApp.LOCK:
            return 'chip-CC3235SF_LAUNCHXL-lock-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'cc32xx')


class cc32xxBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: cc32xxApp = cc32xxApp.LOCK):
        super(cc32xxBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.code_root = root
        self.app = app

    def GnBuildArgs(self):
        args = [
            'ti_sysconfig_root="%s"' % os.environ['TI_SYSCONFIG_ROOT'],
        ]

        return args

    def build_outputs(self):
        items = {}
        if (self.app == cc32xxApp.LOCK):
            extensions = [".out", ".bin", ".out.map"]

        else:
            raise Exception('Unknown app type: %r' % self.app)

        for extension in extensions:
            name = '%s%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
