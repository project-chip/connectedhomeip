#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
import asyncio
import logging
from typing import Any


class AsyncEventQueue(asyncio.Queue):
    def __init__(self, loop: None | asyncio.AbstractEventLoop = None, maxSize: int = 0):
        super().__init__(maxsize=maxSize)
        self._loop = loop or asyncio.get_running_loop()

    def put(self, value: Any):
        """
        Thread safe override of Queue.put
        Schedules a non-blocking call to the event loop.

        Args:
            value (Any): The value to be added to the queue.

        If the event loop is closed, an error is logged, and the value is ignored.

        """
        if not self._loop.is_closed():
            self._loop.call_soon_threadsafe(self.put_nowait, value)
        else:
            logging.error(f"Ignoring value {value} because event loop is not running")

    async def get(self, timeout: int | None = None):
        """
        Override of Queue.get
        Async get with optional timeout seconds.

        Args:
            timeout (int | None): The maximum time in seconds to wait for an item.
            If None, the function will wait indefinitely.

        Returns:
            The item retrieved from the queue.

        Raises:
            asyncio.TimeoutError: If the timeout is reached and no item is available.

        """
        if timeout:
            return await asyncio.wait_for(super().get(), timeout)
        return await super().get()

    def clear(self):
        while not self.empty():
            self.get_nowait()
