#
#    Copyright (c) 2022 Project CHIP Authors
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
