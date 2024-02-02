#
#    Copyright (c) 2023 Project CHIP Authors
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

import platform
import sys

from utils.host.linux import LinuxPlatform
from utils.host.mac import MacPlatform
from utils.log import border_print

p = platform.platform().lower()
if "darwin" in p or "mac" in p:
    impl = MacPlatform()
elif "linux" in p:
    impl = LinuxPlatform()
else:
    border_print("Could not identify if host is Linux or MacOS, exiting!", important=True)
    sys.exit(1)
