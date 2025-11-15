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

# Module for errors thrown from AndroidDevice object.

from mobly import signals

HIERARCHY_TOKEN = '::'


class Error(signals.ControllerError):
  pass


class DeviceError(Error):
  """Raised for errors specific to an AndroidDevice object."""

  def __init__(self, ad, msg):
    template = '%s %s'
    # If the message starts with the hierarchy token, don't add the extra
    # space.
    if isinstance(msg, str) and msg.startswith(HIERARCHY_TOKEN):
      template = '%s%s'
    new_msg = template % (repr(ad), msg)
    super().__init__(new_msg)


class ServiceError(DeviceError):
  """Raised for errors specific to an AndroidDevice service.

  A service is inherently associated with a device instance, so the service
  error type is a subtype of `DeviceError`.
  """

  SERVICE_TYPE = None

  def __init__(self, device, msg):
    new_msg = '%sService<%s> %s' % (HIERARCHY_TOKEN, self.SERVICE_TYPE, msg)
    super().__init__(device, new_msg)
