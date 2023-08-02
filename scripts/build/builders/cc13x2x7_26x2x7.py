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


class cc13x2x7_26x2x7App(Enum):
    LOCK = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    SHELL = auto()

    def ExampleName(self):
        if self == cc13x2x7_26x2x7App.LOCK:
            return 'lock-app'
        elif self == cc13x2x7_26x2x7App.PUMP:
            return 'pump-app'
        elif self == cc13x2x7_26x2x7App.PUMP_CONTROLLER:
            return 'pump-controller-app'
        elif self == cc13x2x7_26x2x7App.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == cc13x2x7_26x2x7App.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app'
        elif self == cc13x2x7_26x2x7App.SHELL:
            return 'shell'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == cc13x2x7_26x2x7App.LOCK:
            return 'chip-LP_CC2652R7-lock-example'
        elif self == cc13x2x7_26x2x7App.PUMP:
            return 'chip-LP_CC2652R7-pump-example'
        elif self == cc13x2x7_26x2x7App.PUMP_CONTROLLER:
            return 'chip-LP_CC2652R7-pump-controller-example'
        elif self == cc13x2x7_26x2x7App.ALL_CLUSTERS:
            return 'chip-LP_CC2652R7-all-clusters-example'
        elif self == cc13x2x7_26x2x7App.ALL_CLUSTERS_MINIMAL:
            return 'chip-LP_CC2652R7-all-clusters-minimal-example'
        elif self == cc13x2x7_26x2x7App.SHELL:
            return 'chip-LP_CC2652R7-shell-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'cc13x2x7_26x2x7')


class cc13x2x7_26x2x7Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: cc13x2x7_26x2x7App = cc13x2x7_26x2x7App.LOCK,
                 openthread_ftd: bool = None):
        super(cc13x2x7_26x2x7Builder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.code_root = root
        self.app = app
        self.openthread_ftd = openthread_ftd

    def GnBuildArgs(self):
        args = [
            'ti_sysconfig_root="%s"' % os.environ['TI_SYSCONFIG_ROOT'],
        ]

        if self.openthread_ftd is None:
            pass
        elif self.openthread_ftd:
            args.append('chip_openthread_ftd=true')
            args.append('chip_progress_logging=false')
        else:
            args.append('chip_openthread_ftd=false')

        return args

    def build_outputs(self):
        items = {}
        if (self.app == cc13x2x7_26x2x7App.LOCK
                or self.app == cc13x2x7_26x2x7App.PUMP
                or self.app == cc13x2x7_26x2x7App.PUMP_CONTROLLER):
            extensions = [".out", ".bin", ".out.map", "-bim.hex"]

        elif self.app == cc13x2x7_26x2x7App.ALL_CLUSTERS or cc13x2x7_26x2x7App.ALL_CLUSTERS_MINIMAL or self.app == cc13x2x7_26x2x7App.SHELL:
            extensions = [".out", ".out.map"]

        else:
            raise Exception('Unknown app type: %r' % self.app)

        for extension in extensions:
            name = '%s%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
