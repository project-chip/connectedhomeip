#
#    Copyright (c) 2025 Project CHIP Authors
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

"""
This module contains global variables that are used across several scripts and classes.
"""
from typing import final


class TestingDefaults:

    ADMIN_VENDOR_ID: final = 0xFFF1
    STORAGE_PATH: final = "admin_storage.json"
    LOG_PATH: final = "/tmp/matter_testing/logs"
    CONTROLLER_NODE_ID: final = 112233
    DUT_NODE_ID: final = 0x12344321
    TRUST_ROOT_INDEX: final = 1
