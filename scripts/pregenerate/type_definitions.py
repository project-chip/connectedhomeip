# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from dataclasses import dataclass
from enum import Enum, auto


class IdlFileType(Enum):
    ZAP = auto()
    MATTER = auto()


@dataclass
class InputIdlFile:
    file_type: IdlFileType
    relative_path: str
    full_path: str

    @property
    def pregen_subdir(self):
        '''
        Returns the relative path inside the pregenerate directory where
        data for this IDL file should be pregenerated.
        '''
        top_dir = os.path.splitext(self.relative_path)[0]

        if self.file_type == IdlFileType.MATTER:
            return os.path.join(top_dir, "codegen")
        elif self.file_type == IdlFileType.ZAP:
            return os.path.join(top_dir, "zap")
        else:
            raise Exception("Unknown file type for self")
