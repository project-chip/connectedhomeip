# Copyright 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import logging

logging.warning(
    'The module mobly.controllers.android_device_lib.snippet_event '
    'is deprecated and will be removed in a future version. Use '
    'module mobly.snippet.callback_event instead.'
)


def from_dict(event_dict):
  """Create a SnippetEvent object from a dictionary.

  DEPRECATED: Use mobly.snippet.callback_event.from_dict instead.

  Args:
    event_dict: a dictionary representing an event.

  Returns:
    A SnippetEvent object.
  """
  return SnippetEvent(
      callback_id=event_dict['callbackId'],
      name=event_dict['name'],
      creation_time=event_dict['time'],
      data=event_dict['data'],
  )


class SnippetEvent:
  """The class that represents callback events for mobly snippet library.

  DEPRECATED: Use mobly.snippet.callback_event.CallbackEvent instead.

  Attributes:
    callback_id: string, the callback ID associated with the event.
    name: string, the name of the event.
    creation_time: int, the epoch time when the event is created on the
      Rpc server side.
    data: dictionary, the data held by the event. Can be None.
  """

  def __init__(self, callback_id, name, creation_time, data):
    self.callback_id = callback_id
    self.name = name
    self.creation_time = creation_time
    self.data = data

  def __repr__(self):
    return (
        'SnippetEvent(callback_id: %s, name: %s, creation_time: %s, data: %s)'
    ) % (self.callback_id, self.name, self.creation_time, self.data)
