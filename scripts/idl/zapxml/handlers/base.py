# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import enum
from .context import Context

class HandledDepth:
    """Defines how deep a XML element has been handled."""
    NOT_HANDLED = enum.auto()  # Unknown/parsed element
    ENTIRE_TREE = enum.auto()  # Entire tree can be ignored
    SINGLE_TAG = enum.auto()  # Single tag processed, but not sub-items


class BaseHandler:
    """A generic element handler.

       XML processing is be done in the form of a stack. whenever
       a new element comes in, its processor is moved to the top of
       the stack and poped once the element ends.
    """

    def __init__(self, context: Context, handled=HandledDepth.NOT_HANDLED):
        self.context = context
        self._handled = handled

    def GetNextProcessor(self, name, attrs):
        """Get the next processor to use for the given name"""

        if self._handled == HandledDepth.SINGLE_TAG:
            handled = HandledDepth.NOT_HANDLED
        else:
            handled = self._handled

        return BaseHandler(context=self.context, handled=handled)

    def HandleContent(self, content):
        """Processes some content"""
        pass

    def EndProcessing(self):
        """Finalizes the processing of the current element"""
        if self._handled == HandledDepth.NOT_HANDLED:
            self.context.MarkTagNotHandled()


