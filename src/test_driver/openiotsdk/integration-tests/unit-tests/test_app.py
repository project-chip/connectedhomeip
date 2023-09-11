#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import logging
import re

import pytest

log = logging.getLogger(__name__)


@pytest.fixture(scope="session")
def binaryPath(request, rootDir):
    if request.config.getoption('binaryPath'):
        return request.config.getoption('binaryPath')
    else:
        assert False


def test_unit_tests(device):
    ret = device.wait_for_output("Open IoT SDK unit-tests start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Open IoT SDK unit-tests run")
    assert ret is not None and len(ret) > 0

    ret = device.wait_for_output("Test status:", 1200)
    # Get test status
    test_status = ret[-1]
    result = re.findall(r'\d+', test_status)
    assert len(result) == 1
    assert int(result[0]) == 0
