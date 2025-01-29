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

       XML processing is done in the form of depth-first processing:
          - Tree is descended into using `GetNextProcessor`
          - Processors are expected to extend `BaseHandler` and allow for:
             - GetNextProcessor to recurse
             - HandleContent in case the text content is relevant
             - EndProcessing once the entire tree has been walked (when xml element ends)

       BaseHandler keeps track if it has been handled or ot by its `_handled` setting and
       init parameter. Non-handled elements will be tagged within the context, resulting
       in logs. This is to detect if unknown/new tags appear in XML files.
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
