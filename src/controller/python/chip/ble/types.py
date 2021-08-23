#
#    Copyright (c) 2021 Project CHIP Authors
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

from ctypes import CFUNCTYPE, py_object, c_char_p, c_uint16

DeviceScannedCallback = CFUNCTYPE(None, py_object, c_char_p, c_uint16,
                                  c_uint16, c_uint16)

ScanDoneCallback = CFUNCTYPE(None, py_object)
