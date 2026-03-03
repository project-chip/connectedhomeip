#
#    Copyright (c) 2026 Project CHIP Authors
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

"""
Support module for ICD test modules containing shared functionality.
"""

import asyncio
import logging
import random
from dataclasses import dataclass
from datetime import datetime, timedelta
from enum import IntEnum
from typing import Optional

from mdns_discovery import mdns_discovery
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.commissioning import CustomCommissioningParameters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest

log = logging.getLogger(__name__)

class ICDTestEventTriggerOperations(IntEnum):
    """
    Copy of ICDTestEventTriggerEvent from ICDManager.cpp.
    Both enum classes must use the same values to maintain compatibility.
    """
    kAddActiveModeReq = 0x0046000000000001
    kRemoveActiveModeReq = 0x0046000000000002
