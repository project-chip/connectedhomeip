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

"""Unit tests for the native log suppression used during the wildcard subscription setup.

The tests replace the Matter stack and the native log filter calls with recorders, so they need
neither a DUT nor a Matter stack.
"""

import unittest
from unittest import mock

import matter.logging

from matter.testing.matter_testing import _native_logs_below_error_dropped

PREVIOUS_FILTER = matter.logging.LOG_CATEGORY_DETAIL


class RecordingStack:
    """Stands in for ChipStack and records when a task runs on the Matter thread."""

    def __init__(self, order: list):
        self.order = order

    async def CallAsyncWithResult(self, callFunct):
        self.order.append("matter_thread_task")
        return callFunct()


class TestNativeLogSuppression(unittest.IsolatedAsyncioTestCase):

    def setUp(self):
        self.order = []
        self.stack = RecordingStack(self.order)
        get_patch = mock.patch.object(matter.logging, "GetLogFilter", return_value=PREVIOUS_FILTER)
        set_patch = mock.patch.object(matter.logging, "SetLogFilter",
                                      side_effect=lambda category: self.order.append(("set_filter", category)))
        get_patch.start()
        set_patch.start()
        self.addCleanup(get_patch.stop)
        self.addCleanup(set_patch.stop)

    async def test_filter_set_to_error_inside_and_restored_after(self):
        async with _native_logs_below_error_dropped(self.stack):
            self.order.append("body")

        self.assertEqual(self.order, [
            "matter_thread_task",
            ("set_filter", matter.logging.LOG_CATEGORY_ERROR),
            "body",
            ("set_filter", PREVIOUS_FILTER),
        ])

    async def test_filter_restored_when_body_raises(self):
        with self.assertRaises(RuntimeError):
            async with _native_logs_below_error_dropped(self.stack):
                raise RuntimeError("subscribe failed")

        self.assertEqual(self.order[-1], ("set_filter", PREVIOUS_FILTER))


if __name__ == "__main__":
    unittest.main()
