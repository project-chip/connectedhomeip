#!/usr/bin/env -S python3 -B

# Copyright (c) 2022 Project CHIP Authors
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

import sys
import xmlrpc.client

IP = '127.0.0.1'
PORT = 9000

if sys.platform == 'linux':
    IP = '10.10.10.5'

# Passing in sys.argv[2:] gets rid of the script name and key to the apps register. The remaining
# values in the list are key-value pairs, e.g. [option1, value1, option2, value2, ...]
with xmlrpc.client.ServerProxy('http://' + IP + ':' + str(PORT) + '/', allow_none=True) as proxy:
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    proxy.compareFiles(file1, file2)
