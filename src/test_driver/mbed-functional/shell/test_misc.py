# Copyright (c) 2009-2020 Arm Limited
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

import pytest

@pytest.mark.smoketest
def test_echo_check(device):
    ret = device.send(command="echo Hello", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "Hello" in ret[-2]

@pytest.mark.smoketest
def test_log_check(device):
    ret = device.send(command="log Hello", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert "CHIP:TOO: Hello" in ret[-2]

@pytest.mark.smoketest
def test_rand_check(device):
    ret = device.send(command="rand", expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip().isdigit()