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

import importlib
import os
import sys
from pathlib import Path
from unittest.mock import MagicMock

from chip.clusters import Attribute

try:
    from matter_testing_support import MatterStackState, MatterTestConfig, run_tests_no_exit
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_testing_support import MatterStackState, MatterTestConfig, run_tests_no_exit


class AsyncMock(MagicMock):
    async def __call__(self, *args, **kwargs):
        return super(AsyncMock, self).__call__(*args, **kwargs)


class MockTestRunner():
    def __init__(self, filename: str, classname: str, test: str):
        self.config = MatterTestConfig(
            tests=[test], endpoint=1, dut_node_ids=[1])
        self.stack = MatterStackState(self.config)
        self.default_controller = self.stack.certificate_authorities[0].adminList[0].NewController(
            nodeId=self.config.controller_node_id,
            paaTrustStorePath=str(self.config.paa_trust_store_path),
            catTags=self.config.controller_cat_tags
        )
        module = importlib.import_module(Path(os.path.basename(filename)).stem)
        self.test_class = getattr(module, classname)

    def Shutdown(self):
        self.stack.Shutdown()

    def run_test_with_mock_read(self,  read_cache: Attribute.AsyncReadTransaction.ReadResponse):
        self.default_controller.Read = AsyncMock(return_value=read_cache)
        return run_tests_no_exit(self.test_class, self.config, None, self.default_controller, self.stack)
