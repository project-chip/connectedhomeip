#
#    Copyright (c) 2026 Project CHIP Authors
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
import ctypes
import unittest

from matter.clusters.Command import AsyncCommandTransaction, CommandPath, Status
from matter.interaction_model import InteractionModelError
from matter.interaction_model import Status as InteractionModelStatus
from matter.native import PyChipError


class TestAsyncCommandTransaction(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.loop = asyncio.get_running_loop()
        self.loopErrors = []
        self.loop.set_exception_handler(lambda loop, context: self.loopErrors.append(context))
        self.future = self.loop.create_future()
        self.transaction = AsyncCommandTransaction(self.future, self.loop, None)
        # Matches the reference the C++ side holds; handleDone releases it.
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(self.transaction))

    async def test_done_without_response_resolves_none(self):
        # A suppressed invoke that the peer acked ends with done and no response.
        self.transaction.handleDone()
        self.assertIsNone(await asyncio.wait_for(self.future, 1))
        self.assertEqual(self.loopErrors, [])

    async def test_error_then_done_keeps_error(self):
        self.transaction.handleError(Status(IMStatus=InteractionModelStatus.Failure, ClusterStatus=0), PyChipError.from_code(0))
        self.transaction.handleDone()
        with self.assertRaises(InteractionModelError):
            await asyncio.wait_for(self.future, 1)
        await asyncio.sleep(0)
        self.assertEqual(self.loopErrors, [])

    async def test_response_then_done_keeps_response(self):
        path = CommandPath(EndpointId=1, ClusterId=6, CommandId=0)
        self.transaction.handleResponse(path, 0, Status(IMStatus=InteractionModelStatus.Success, ClusterStatus=0), b'')
        self.transaction.handleDone()
        self.assertIsNone(await asyncio.wait_for(self.future, 1))
        await asyncio.sleep(0)
        self.assertEqual(self.loopErrors, [])


if __name__ == '__main__':
    unittest.main()
