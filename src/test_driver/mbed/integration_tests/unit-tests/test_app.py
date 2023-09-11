# SPDX-FileCopyrightText: 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0

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
