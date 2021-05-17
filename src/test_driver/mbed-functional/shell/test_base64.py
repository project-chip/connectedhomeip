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
def test_base64_encode_decode(device):
    hex_string = "1234"
    ret = device.send(command="base64 encode {}".format(hex_string), expected_output="Done")
    assert ret != None and len(ret) > 1
    base64code = ret[-2]
    ret = device.send(command="base64 decode {}".format(base64code), expected_output="Done")
    assert ret != None and len(ret) > 1
    assert ret[-2].rstrip() == hex_string