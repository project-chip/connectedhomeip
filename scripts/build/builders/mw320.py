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


class MW320App(Enum):
    ALL_CLUSTERS = auto()

    def ExampleName(self):
        if self == MW320App.ALL_CLUSTERS:
            return 'all-clusters-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == MW320App.ALL_CLUSTERS:
            return 'chip-mw320-all-clusters-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', 'mw320')


class MW320Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: MW320App = MW320App.ALL_CLUSTERS):
        super(MW320Builder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app

    def build_outputs(self):
        items = {}
        for extension in [".bin", ".out", ".out.map"]:
            name = '%s%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
