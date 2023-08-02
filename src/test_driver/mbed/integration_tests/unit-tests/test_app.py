# Copyright (c) 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0
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

import re


def test_unit_tests(device):
    device.reset(duration=1)
    # smoke test
    ret = device.wait_for_output("Mbed unit-tests application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Mbed unit-tests application run", 60)
    assert ret is not None and len(ret) > 0

    ret = device.wait_for_output("CHIP test status:", 500)
    # extract number of failures:
    test_status = ret[-1]
    result = re.findall(r'\d+', test_status)
    assert len(result) == 1
    assert int(result[0]) == 0
