# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
