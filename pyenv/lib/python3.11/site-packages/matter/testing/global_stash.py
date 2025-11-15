#
#    Copyright (c) 2025 Project CHIP Authors
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

"""
This module contains functions designed to handle config values of ctypes objects as mobly cannot deal with those.
The methods just use a global dict of uuid -> object to recover items stashed by reference.
"""

import uuid
from typing import Any

# Mobly cannot deal with user config passing of ctypes objects,
# so we use this dict of uuid -> object to recover items stashed
# by reference.
_GLOBAL_DATA = {}


def stash_globally(o: object) -> str:
    unique_id = str(uuid.uuid1())
    _GLOBAL_DATA[unique_id] = o
    return unique_id


def unstash_globally(id: str) -> Any:
    return _GLOBAL_DATA.get(id)
