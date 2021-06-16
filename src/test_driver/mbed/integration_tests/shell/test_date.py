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
import time
from time import sleep
import datetime

TEST_DATE = "2021-05-14 13:11:42"
TIMESTAMP_INTERVAL = 3

@pytest.mark.smoketest
def test_date_set_date(device):
    start_timestamp = time.mktime(datetime.datetime.strptime(TEST_DATE, "%Y-%m-%d %H:%M:%S").timetuple())

    # Set device time
    ret = device.send(command="date set {}".format(TEST_DATE), expected_output="Done")
    assert ret != None

    sleep(TIMESTAMP_INTERVAL)

    # Get device time
    ret = device.send(command="date", expected_output="Done")
    assert ret != None and len(ret) > 1

    next_timestamp = time.mktime(datetime.datetime.strptime(ret[-2].rstrip(), "%Y-%m-%d %H:%M:%S").timetuple())

    assert (next_timestamp - start_timestamp) in range(TIMESTAMP_INTERVAL - 1, TIMESTAMP_INTERVAL + 1)