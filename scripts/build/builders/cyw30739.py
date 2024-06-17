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

from .builder import BuilderOutput
from .gn import GnBuilder


class Cyw30739App(Enum):
    LIGHT = auto()
    LIGHT_SWITCH = auto()
    LOCK = auto()
    THERMOSTAT = auto()

    def ExampleName(self):
        if self == Cyw30739App.LIGHT:
            return "lighting-app"
        elif self == Cyw30739App.LIGHT_SWITCH:
            return "light-switch-app"
        elif self == Cyw30739App.LOCK:
            return "lock-app"
        elif self == Cyw30739App.THERMOSTAT:
            return "thermostat"
        else:
            raise Exception("Unknown app type: %r" % self)

    def AppNamePrefix(self):
        return self.ExampleName().replace("-", "_")

    def BuildRoot(self, root):
        return os.path.join(root, "examples", self.ExampleName(), "infineon/cyw30739")


class Cyw30739Board(Enum):
    CYW30739B2_P5_EVK_01 = auto()
    CYW30739B2_P5_EVK_02 = auto()
    CYW30739B2_P5_EVK_03 = auto()
    CYW930739M2EVB_01 = auto()
    CYW930739M2EVB_02 = auto()

    def GnArgName(self):
        if self == Cyw30739Board.CYW30739B2_P5_EVK_01:
            return "CYW30739B2-P5-EVK-01"
        elif self == Cyw30739Board.CYW30739B2_P5_EVK_02:
            return "CYW30739B2-P5-EVK-02"
        elif self == Cyw30739Board.CYW30739B2_P5_EVK_03:
            return "CYW30739B2-P5-EVK-03"
        elif self == Cyw30739Board.CYW930739M2EVB_01:
            return "CYW930739M2EVB-01"
        elif self == Cyw30739Board.CYW930739M2EVB_02:
            return "CYW930739M2EVB-02"
        else:
            raise Exception("Unknown board #: %r" % self)


class Cyw30739Builder(GnBuilder):
    def __init__(
        self,
        root,
        runner,
        app: Cyw30739App = Cyw30739App.LIGHT,
        board: Cyw30739Board = Cyw30739Board.CYW30739B2_P5_EVK_01,
        release: bool = False,
    ):
        super(Cyw30739Builder, self).__init__(
            root=app.BuildRoot(root), runner=runner)
        self.app = app
        self.board = board
        self.release = release
        self.build_command = f"{app.AppNamePrefix()}-{board.GnArgName()}"
        self._output_dir = None

    @property
    def output_dir(self):
        return self._output_dir

    @output_dir.setter
    def output_dir(self, value):
        if hasattr(self, "board"):
            board = self.board.GnArgName().lower().replace("-", "_")
            value = value.replace(f"-{board}", "")
        self._output_dir = value

    def GnBuildArgs(self):
        args = []

        if self.release:
            args.append('is_debug=false')

        return args

    def build_outputs(self):
        extensions = ["elf"]
        if self.options.enable_link_map_file:
            extensions.append("elf.map")
        for ext in extensions:
            name = f"{self.app.AppNamePrefix()}-{self.board.GnArgName()}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
