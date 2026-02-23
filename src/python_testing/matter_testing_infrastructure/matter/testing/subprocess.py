#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
from dataclasses import dataclass, replace
from enum import StrEnum
import pathlib


class SubprocessKind(StrEnum):
    APP = 'app'
    TOOL = 'tool'
    MGMT = 'mgmt'


@dataclass
class SubprocessInfo:
    kind: SubprocessKind
    path: pathlib.Path
    wrapper: tuple[str, ...] = ()
    args: tuple[str, ...] = ()

    def __post_init__(self):
        self.path = pathlib.Path(self.path)

    def with_args(self, *args: str):
        return replace(self, args=self.args + tuple(args))

    def wrap_with(self, *args: str):
        return replace(self, wrapper=tuple(args) + self.wrapper)

    def to_cmd(self) -> list[str]:
        return list(self.wrapper) + [str(self.path)] + list(self.args)
