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

from ctypes import CFUNCTYPE, c_void_p, c_size_t, c_uint32

# General callback of 'network credentials requested. No python-data
# is available as the underlying callback is used internally
NetworkCredentialsRequested = CFUNCTYPE(None)

# Operational credential request
# Can access the csr data using ctypes.string_at
OperationalCredentialsRequested = CFUNCTYPE(None, c_void_p, c_size_t)

# Notification that pairing has been coompleted
PairingComplete = CFUNCTYPE(None, c_uint32)
