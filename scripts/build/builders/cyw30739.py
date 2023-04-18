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


class Cyw30739App(Enum):
    LIGHT = auto()
    LOCK = auto()
    OTA_REQUESTOR = auto()
    SWITCH = auto()

    def ExampleName(self):
        if self == Cyw30739App.LIGHT:
            return "lighting-app"
        elif self == Cyw30739App.LOCK:
            return "lock-app"
        elif self == Cyw30739App.OTA_REQUESTOR:
            return "ota-requestor-app"
        elif self == Cyw30739App.SWITCH:
            return "light-switch-app"
        else:
            raise Exception("Unknown app type: %r" % self)

    def AppNamePrefix(self):
        if self == Cyw30739App.LIGHT:
            return "chip-cyw30739-lighting-example"
        elif self == Cyw30739App.LOCK:
            return "chip-cyw30739-lock-example"
        elif self == Cyw30739App.OTA_REQUESTOR:
            return "chip-cyw30739-ota-requestor-example"
        elif self == Cyw30739App.SWITCH:
            return "chip-cyw30739-light-switch-example"
        else:
            raise Exception("Unknown app type: %r" % self)

    def BuildRoot(self, root):
        return os.path.join(root, "examples", self.ExampleName(), "infineon/cyw30739")


class Cyw30739Board(Enum):
    CYW930739M2EVB_01 = 1

    def GnArgName(self):
        if self == Cyw30739Board.CYW930739M2EVB_01:
            return "CYW930739M2EVB-01"
        else:
            raise Exception("Unknown board #: %r" % self)


class Cyw30739Builder(GnBuilder):
    def __init__(
        self,
        root,
        runner,
        app: Cyw30739App = Cyw30739App.LIGHT,
        board: Cyw30739Board = Cyw30739Board.CYW930739M2EVB_01,
        release: bool = False,
    ):
        super(Cyw30739Builder, self).__init__(
            root=app.BuildRoot(root), runner=runner)
        self.app = app
        self.board = board
        self.release = release

    def GnBuildArgs(self):
        args = []

        if self.release:
            args.append('is_debug=false')

        return args

    def build_outputs(self):
        items = {}
        for extension in ["elf", "elf.map"]:
            name = "%s.%s" % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)
        return items
