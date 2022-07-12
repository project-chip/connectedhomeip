'''
   Copyright (c) 2021 Project CHIP Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
'''

import construct
import ctypes
import chip.native
import chip.exceptions
import typing
from dataclasses import dataclass
from enum import IntEnum
import builtins

''' These are ctypes equivalents of C structs defined in af-types.h
    and in attribute-metadata.h. These are used to compose cluster descriptor
    metadata tables that are in turn passed directly into the SDK when registering
    endpoints
'''


class EmberAfAttributeMetadata(ctypes.Structure):
    _fields_ = [
        ('attributeId', ctypes.c_uint32),
        ('attributeType', ctypes.c_uint8)
    ]


class EmberAfCluster(ctypes.Structure):
    _fields_ = [
        ('clusterId', ctypes.c_uint32),
        ('attributeCount', ctypes.c_uint16),
        ('attributes', ctypes.POINTER(EmberAfAttributeMetadata))
    ]


class EmberAfEndpointType(ctypes.Structure):
    _fields_ = [
        ('clusterCount', ctypes.c_uint8),
        ('cluster', ctypes.POINTER(EmberAfCluster))
    ]
