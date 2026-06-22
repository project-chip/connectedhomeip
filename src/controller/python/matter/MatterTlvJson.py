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

import builtins
import ctypes
import json
import typing
from ctypes import CDLL, POINTER, c_char_p, c_size_t, c_ubyte

from .clusters import ClusterObjects as ClusterObjects
from .clusters.Attribute import AttributeCache, AttributePath, ValueDecodeFailure
from .tlv import TLVReader


class TLVJsonConverter():
    ''' Converter class used to convert MatterJsonTlv attribute wildcard dump files into an AttributeCache.

        Attribute wildcard dump files can be generated as a side effect of basic composition tests, or
        by running the IDM-12.1 cerification test case. The file contains a representation of this
        data in MatterTlvJson format. This class is used to convert this data back into a python-based
        representation.

        More information on the MatterTlvJson format can be found here:
        https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/jsontlv/README.md
    '''

    def __init__(self, name: str = ''):
        self._ChipStack = builtins.chipStack  # type: ignore[attr-defined]  # 'chipStack' is dynamically added to builtins

        self._dmLib = CDLL(self._ChipStack.LocateChipDLL())

        self._dmLib.pychip_JsonToTlv.argtypes = [c_char_p, POINTER(c_ubyte), c_size_t]
        self._dmLib.pychip_JsonToTlv.restype = c_size_t

    def _attribute_to_tlv(self, json_string: str) -> bytearray:
        ''' Converts the MatterJsonTlv for one attribute into TLV that can be parsed and put into the cache.'''
        # We don't currently have a way to size this properly, but we know attributes need to fit into 1 MTU.
        size = 1280
        buf = bytearray(size)
        encoded_bytes = self._dmLib.pychip_JsonToTlv(json_string.encode("utf-8"), (ctypes.c_ubyte * size).from_buffer(buf), size)
        return buf[:encoded_bytes]

    def convert_dump_to_cache(self, json_tlv: typing.Any) -> AttributeCache:
        ''' Converts a json object containing the MatterJsonTlv dump of an entire device into an AttributeCache object.
            Input:
              json_tlv: json loaded from from the dump file.
            Returns:
              AttributeCache with the data from the json_string
        '''
        cache = AttributeCache()
        for endpoint_id_str, endpoint in json_tlv.items():
            endpoint_id = int(endpoint_id_str, 0)
            for cluster_id_and_type_str, cluster in endpoint.items():
                cluster_id_str, _ = cluster_id_and_type_str.split(':', 2)
                cluster_id = int(cluster_id_str)
                for attribute_id_and_type_str, attribute in cluster.items():
                    attribute_id_str, _ = attribute_id_and_type_str.split(':', 2)
                    attribute_id = int(attribute_id_str)
                    json_str = json.dumps({attribute_id_and_type_str: attribute}, indent=2)
                    tmp = self._attribute_to_tlv(json_str)
                    path = AttributePath(EndpointId=endpoint_id, ClusterId=cluster_id, AttributeId=attribute_id)
                    # Each of these attributes contains only one item
                    try:
                        tlvData = next(iter(TLVReader(tmp).get().get("Any", {}).values()))
                    except StopIteration:
                        # no data, this is a value decode error
                        tlvData = ValueDecodeFailure()
                    cache.UpdateTLV(path=path, dataVersion=0, data=tlvData)
                    cache.GetUpdatedAttributeCache()
        return cache
