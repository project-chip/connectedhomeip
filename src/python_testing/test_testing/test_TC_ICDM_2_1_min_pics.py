#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
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

import sys

from common_icdm_data import ICDMData, c, run_tests

TEST_CASES = [
    # Validate that the test script can succeed with the minimum set of PICS
    ICDMData(0, 1, 0, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
]


def main():
    pics = {"ICDM.S.A0000": True, "ICDM.S.A0001": True, "ICDM.S.A0002": True, "ICDM.S.A0003": False, "ICDM.S.A0004": False,
            "ICDM.S.A0005": False, "ICDM.S.A0006": False, "ICDM.S.A0007": False, "ICDM.S.A0008": False, "ICDM.S.A0009": False, }

    return run_tests(pics, 'TC_ICDM_2_1', TEST_CASES, 'test_TC_ICDM_2_1')


if __name__ == "__main__":
    sys.exit(main())
