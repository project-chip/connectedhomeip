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

import ctypes
import unittest

from matter.ChipDeviceCtrl import CommissioningStage
from matter.native import GetLibraryHandle, HandleFlags


class TestCommissioningStage(unittest.TestCase):
    def test_matches_native_enum(self):
        handle = GetLibraryHandle(HandleFlags(0))  # StageToString() does not need an initialized stack
        stageToString = handle.pychip_CommissioningStageToString
        stageToString.argtypes = [ctypes.c_uint8]
        stageToString.restype = ctypes.c_char_p

        for value in range(256):
            with self.subTest(value=value):
                nativeName = stageToString(value).decode("utf-8")
                if nativeName == "???":  # StageToString() result for values that are not a stage
                    nativeName = None
                try:
                    stage = CommissioningStage(value)
                except ValueError:
                    stage = None

                self.assertEqual(stage.name if stage is not None else None, nativeName)


if __name__ == "__main__":
    unittest.main()
