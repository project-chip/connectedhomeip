# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
