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
import chip.clusters.ClusterObjects as ClusterObjects
import chip.clusters.Objects as Clusters
from .AfTypes import *
import builtins
import inspect
import logging

_AttributeGetterCbFunct = ctypes.CFUNCTYPE(
        None, ctypes.py_object, ctypes.c_uint16, ctypes.c_uint32, ctypes.c_uint32, ctypes.POINTER(ctypes.c_char), ctypes.POINTER(ctypes.c_uint16))

@_AttributeGetterCbFunct
def AttributeGetterCallback(serverObj, endpointId: int, clusterId: int, attributeId: int, value: bytes, size):
    size[0] = 0

    endpointState = serverObj.attributeState[endpointId]
    for cluster in endpointState:
        if (cluster.id == clusterId):
            clusterState = endpointState[cluster]

            for attribute in clusterState:
                if (attribute.attribute_id == attributeId):
                    tlvValue = attribute.ToTLV(None, clusterState[attribute])
                    
                    count = 0
                    for idx, byte in enumerate(tlvValue):
                        value[idx] = byte
                        count = count + 1
                    
                    size[0] = count
                    return

class Server():
    ''' Class that handles the management of the server side of a Python REPL instance.
    '''
    _handle = chip.native.GetLibraryHandle()

    attributeState = {}
    endpointConfig = {}

    '''
      This has to be in alignment with the value for CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
      in CHIPPlatfornConfig.h
    '''
    maxEndpoints = 20

    def __init__(self):
        res = self._handle.pychip_Server_InitializeServer()
        if res != 0:
            raise self.ErrorToException(res)

        self._handle.pychip_Server_RegisterAttributeGetterCallback(ctypes.py_object(self), AttributeGetterCallback)

    def SetCommissioningParams(self, discriminator: int = 3840):
        ''' Updates the commissioning parameters and re-advertises those details over mDNS
        '''
        curLogLevel = logging.getLogger().level
        logging.getLogger().setLevel(logging.INFO)
    
        res = self._handle.pychip_Server_SetCommissioningParams(discriminator)
        if res != 0:
            raise self.ErrorToException(res)
    
        logging.getLogger().setLevel(curLogLevel)

    def FinalizeEndpoint(self, endpointId):
        ''' Finalizes the supported attributes on a given endpoint and commits that the endpoint to the SDK. At this point,
            the endpoint is live. 

            This looks at the value of an attribute to see if it has a value of 'None' (which is the default for an optional value) to decide 
            if it is indeed implemented or not. If the value is 'None', it will signal that attribute as an optional attribute that isn't implemented to the SDK.
        '''
        if (endpointId not in self.attributeState):
            raise ValueError(f"{endpointId} not in endpoint list")

        endpoint = self.attributeState[endpointId]

        ''' Start creating the EmberAfEndpointType object that in turn, contains all the metadata information
            describing the clusters and their attributes. These types match those defined in their equivalent C++ header
            files (see AfTypes.py for more information), and are used to create the metadata tables needed for endpoint registration.
        '''
        c_emberAfEndpointType = EmberAfEndpointType()
        c_emberAfEndpointType.clusterCount = len(endpoint)
        c_emberAfEndpointType.cluster = (EmberAfCluster * c_emberAfEndpointType.clusterCount)()

        i = 0
        for cluster in endpoint:
            attributes = endpoint[cluster]

            c_cluster = c_emberAfEndpointType.cluster[i]

            c_cluster.clusterId = cluster.id
            c_cluster.attributes = (EmberAfAttributeMetadata * len(attributes))()

            j = 0
            for attribute in attributes:
                if (attributes[attribute] == None):
                    continue

                c_metadata = c_cluster.attributes[j]
                c_metadata.attributeId = attribute.attribute_id
                c_metadata.attributeType = 1

                j = j + 1

            c_cluster.attributeCount = j
            i = i + 1

        print(f"Finalizing Endpoint {endpointId} with Endpoint Index = {self.endpointConfig[endpointId]['endpointIndex']}")

        c_deviceTypeList = (ctypes.c_uint32 * len(self.endpointConfig[endpointId]['deviceTypeList']))(*self.endpointConfig[endpointId]['deviceTypeList'])

        self._handle.pychip_Server_RegisterEndpoint.restype = ctypes.c_uint32

        ''' Register the endpoint with the SDK. This eventually calls emberAfSetDynamicEndpoint to register the endpoint.
            It also registers an AttributeAccessInterface instance for each cluster instance that serves as an adapter to intercept
            attribute reads and route them to the AttributeGetterCallback above to extract TLV from the attributeState map that stores
            the totality of all attribute state in the Python layer.
        '''
        builtins.chipStack.Call(
                lambda: self._handle.pychip_Server_RegisterEndpoint(ctypes.c_uint16(endpointId), ctypes.c_uint16(self.endpointConfig[endpointId]['endpointIndex']), ctypes.byref(c_emberAfEndpointType),
                    ctypes.pointer(c_deviceTypeList), 
                    ctypes.c_uint16(len(self.endpointConfig[endpointId]['deviceTypeList'])))
        )

    def RemoveEndpoint(self, endpointId):
        ''' Remove and de-register a previously registered endpoint.
        '''
        if (endpointId not in self.attributeState):
            raise ValueError(f"{endpointId} not already present on this node")

        builtins.chipStack.Call(
                lambda: self._handle.pychip_Server_RemoveEndpoint(ctypes.c_uint16(self.endpointConfig[endpointId]['endpointIndex']))
        )

        self.attributeState.pop(endpointId)

    def CreateEndpoint(self, endpointId: int, clusterList: typing.List[ClusterObjects.Cluster], deviceTypeList: typing.List[int] = []):
        ''' Create a new endpoint given an endpoint ID, a list of clusters that should exist on that endpoint and the list of device types supported
            on this endpoint.

            The clusterList is expected to contain a list of ClusterObjects picked from the generated Cluster Objects set.

            E.g

            CreateEndpoint(10, [ Clusters.OnOff, Clusters.Basic ] )
        '''
        if (endpointId in self.attributeState):
            raise ValueError(f"{endpointId} already exists on this node!")

        if (endpointId == 0):
            raise ValueError("EP0 is reserved by the SDK for the root node!")

        candidateEndpointIndex = None
        for targetEndpointIndex in range(0, self.maxEndpoints - 1):
            occupied = False
            for endpoint in self.endpointConfig:
                if self.endpointConfig[endpoint]['endpointIndex'] == targetEndpointIndex:
                    occupied = True
                    break

            if (occupied == False):
               candidateEndpointIndex = targetEndpointIndex
               break

        if (candidateEndpointIndex is None):
            raise ValueError(f"Exceeded the maximum number of dynamic endpoints of {self.maxEndpoints}!")

        print(f"Selected index of {candidateEndpointIndex}..")

        self.endpointConfig[endpointId] = {'endpointIndex': targetEndpointIndex, 'deviceTypeList': deviceTypeList}
        self.attributeState[endpointId] = {}

        if Clusters.Descriptor not in clusterList:
            clusterList.append(Clusters.Descriptor)

        for cluster in clusterList:
            self.attributeState[endpointId][cluster] = {}

            for attributeName, attribute in inspect.getmembers(cluster.Attributes):
                if inspect.isclass(attribute):
                    base_classes = inspect.getmro(attribute)

                    matched = [
                        value for value in base_classes if 'ClusterAttributeDescriptor' in str(value)]
                    if (matched == []):
                        continue

                    self.attributeState[endpointId][cluster][attribute] = attribute().value
        
    def GetAttribute(self, endpointId = int, cluster = ClusterObjects.Cluster, attribute = ClusterObjects.ClusterObjectDescriptor):
        ''' Returns the value of an attribute in the attribute store.
        '''
        if (endpointId not in self.attributeState):
            raise ValueError(f"{endpointId} not in the cache!")

        if (cluster not in self.attributeState[endpointId]):
            raise ValueError(f"{cluster} not in the cache at {endpointId} endpoint!")

        return self.attributeState[endpointId][cluster][attribute]

    def GetAllAttributes(self):
        ''' Returns the value of all attributes in the store.
        '''
        
        return self.attributeState

    def SetAttribute(self, endpointId = int, cluster = ClusterObjects.Cluster, attribute = ClusterObjects.ClusterAttributeDescriptor, value = typing.Any):
        ''' Sets the value of an attribute in the store. It will also mark it as dirty, triggering any report generation to fire as well.
        '''
       
        self.attributeState[endpointId][cluster][attribute] = value 

        builtins.chipStack.Call(
                lambda: self._handle.pychip_Server_SetDirty(ctypes.c_uint16(endpointId), ctypes.c_uint32(cluster.id), ctypes.c_uint32(attribute.attribute_id))
        )
