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

import asyncio
import importlib
import os
import sys
from pathlib import Path
from unittest.mock import MagicMock

from chip.clusters import Attribute
from chip.testing.matter_testing import MatterStackState, MatterTestConfig, run_tests_no_exit


class AsyncMock(MagicMock):
    async def __call__(self, *args, **kwargs):
        return super(AsyncMock, self).__call__(*args, **kwargs)


class MockTestRunner():

    def __init__(self, filename: str, classname: str, test: str, endpoint: int = None, pics: dict[str, bool] = None, paa_trust_store_path=None):
        self.kvs_storage = 'kvs_admin.json'
        self.config = MatterTestConfig(endpoint=endpoint, paa_trust_store_path=paa_trust_store_path,
                                       pics=pics, storage_path=self.kvs_storage)
        self.set_test(filename, classname, test)

        self.set_test_config(self.config)

        self.stack = MatterStackState(self.config)
        self.default_controller = self.stack.certificate_authorities[0].adminList[0].NewController(
            nodeId=self.config.controller_node_id,
            paaTrustStorePath=str(self.config.paa_trust_store_path),
            catTags=self.config.controller_cat_tags
        )

    def set_test(self, filename: str, classname: str, test: str):
        self.test = test
        self.config.tests = [self.test]

        module_name = Path(os.path.basename(filename)).stem

        try:
            module = importlib.import_module(module_name)
        except ModuleNotFoundError:
            sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
            module = importlib.import_module(module_name)

        self.test_class = getattr(module, classname)

    def set_test_config(self, test_config: MatterTestConfig = MatterTestConfig()):
        self.config = test_config
        self.config.tests = [self.test]
        self.config.storage_path = self.kvs_storage
        if not self.config.dut_node_ids:
            self.config.dut_node_ids = [1]

    def Shutdown(self):
        self.stack.Shutdown()

    def run_test_with_mock_read(self,  read_cache: Attribute.AsyncReadTransaction.ReadResponse, hooks=None):
        self.default_controller.Read = AsyncMock(return_value=read_cache)
        # This doesn't need to do anything since we are overriding the read anyway
        self.default_controller.FindOrEstablishPASESession = AsyncMock(return_value=None)
        with asyncio.Runner() as runner:
            return run_tests_no_exit(self.test_class, self.config, runner.get_loop(),
                                     hooks, self.default_controller, self.stack)
