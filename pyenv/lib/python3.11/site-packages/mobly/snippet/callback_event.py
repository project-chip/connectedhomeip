# Copyright 2022 Google Inc.
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
"""The class that represents callback events for Mobly Snippet Lib."""


def from_dict(event_dict):
  """Creates a CallbackEvent object from a dictionary.

  Args:
    event_dict: dict, a dictionary representing an event.

  Returns:
    A CallbackEvent object.
  """
  return CallbackEvent(
      callback_id=event_dict['callbackId'],
      name=event_dict['name'],
      creation_time=event_dict['time'],
      data=event_dict['data'],
  )


class CallbackEvent:
  """The class that represents callback events for Mobly Snippet Library.

  Attributes:
    callback_id: str, the callback ID associated with the event.
    name: str, the name of the event.
    creation_time: int, the epoch time when the event is created on the
      RPC server side.
    data: dict, the data held by the event. Can be None.
  """

  def __init__(self, callback_id, name, creation_time, data):
    self.callback_id = callback_id
    self.name = name
    self.creation_time = creation_time
    self.data = data

  def __repr__(self):
    return (
        f'CallbackEvent(callback_id: {self.callback_id}, name: {self.name}, '
        f'creation_time: {self.creation_time}, data: {self.data})'
    )
