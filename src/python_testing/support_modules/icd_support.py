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

import logging
from enum import IntEnum

import matter.clusters as Clusters

log = logging.getLogger(__name__)

# Base for ICD test-event triggers: (IcdManagement cluster ID << 48).
_ICD_CLUSTER_CODE = Clusters.Objects.IcdManagement.id << 48


class ICDTestEventTriggerOperations(IntEnum):
    """
    Mirrors ICDTestEventTriggerEvent from ICDManager.cpp.
    Values are derived from the SDK cluster ID to stay in sync automatically.
    """
    kAddActiveModeReq = _ICD_CLUSTER_CODE | 0x0000_00000001
    kRemoveActiveModeReq = _ICD_CLUSTER_CODE | 0x0000_00000002
    kInvalidateHalfCounterValues = _ICD_CLUSTER_CODE | 0x0000_00000003
    kInvalidateAllCounterValues = _ICD_CLUSTER_CODE | 0x0000_00000004
    kForceMaximumCheckInBackOffState = _ICD_CLUSTER_CODE | 0x0000_00000005
    kDSLSForceSitMode = _ICD_CLUSTER_CODE | 0x0000_00000006
    kDSLSWithdrawSitMode = _ICD_CLUSTER_CODE | 0x0000_00000007
