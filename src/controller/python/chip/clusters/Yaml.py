#
#    Copyright (c) 2020 Project CHIP Authors
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

from dataclasses import dataclass, field
import typing
from chip import ChipDeviceCtrl
import sys, inspect
from chip.tlv import uint
import yaml
import enum
import stringcase
import chip.clusters as Clusters
import chip.interaction_model
import asyncio as asyncio

def FixupYamlArguments(argValues):
    ''' For some reason, instead of treating the entire
        data payload of a command as a singular struct,
        the top-level args are specified as 'name' and 'value' pairs,
        while the payload of each argument is itself correctly encapsulated.

        This fixes up this oddity to create a new key/value pair with the key
        being the value of the 'name' field, and the value being 'value' value.
    '''
    retValue = {}
    
    for item in argValues:
        retValue[item['name']] = item['value']

    return retValue

class ValidationError(Exception):
    def __init__(self, message):
        super().__init__(message)

def FixupYamlTypes(fieldValue, fieldType):
    ''' The 'from_dict' method provided by dacite requires
        the actual types of the various dict items to match those
        specified in the cluster object representation.

        However, the YAML representation when converted to a Python dictionary
        does not quite line up in terms of type (see each of the specific if branches below for
        the rationale for the necessary fix-ups).

        This function does a fix-up given a field value (as present in the YAML) and its
        matching cluster object type and returns it.
    '''
    # dictionary represents a data model struct.
    if (type(fieldValue) is dict):
        fieldDescriptors = fieldType.descriptor
        for item in fieldValue:
            try:
                # We search for a matching item in the list of field descriptors for this struct
                # and ensure we can find a field with a matching label.
                fieldDescriptor = next(x for x in fieldDescriptors.Fields if x.Label == item)
            except StopIteration as exc:
                raise ValidationError('Did not find field "' + item + '" in ' + str(fieldType)) from None

            fieldValue[item] = FixupYamlTypes(fieldValue[item], fieldDescriptor.Type) 
        return fieldValue
    # list represents a data model list
    elif(type(fieldValue) is list):
        # The field type passed in is the type of the list element and not list[T].
        for idx, item in enumerate(fieldValue):
            fieldValue[idx] = FixupYamlTypes(item, fieldType)
        return fieldValue
    # YAML conversion treats all numbers as ints. Convert to a uint type if the schema
    # type indicates so.
    elif (fieldType == uint):
        return fieldType(fieldValue)
    # YAML treats enums as ints. Convert to the typed enum class.
    elif (issubclass(fieldType, enum.Enum)):
        return fieldType(fieldValue)
    # YAML treats bytes as strings. Convert to a byte string.
    elif (fieldType == bytes and type(fieldValue) != bytes):
        return str.encode(fieldValue)
    # By default, just return the field value un-changed.
    else:
        return fieldValue

class YamlTestParser:
    ''' Parses the test YAMLs and converts to a more natural Pythonic representation that can be
        acted upon. The parser also permits execution of those tests there-after.
    '''
    @dataclass
    class InvokeTest:
        ''' Dataclass that encapsulates the details of a single YAML invoke interaction test.'''
        Label: str = ''
        RequestTypeName: str = ''
        RequestData: dict = field(default_factory=dict)
        RequestObject: None = None
        ExpectedResponseData: dict = field(default_factory=dict)
        ExpectedResponseObject: None = None
        ExpectedRawResponse: dict = field(default_factory=dict)
        Cluster: str = None
         
    Name: str
    Cluster: str
    Endpoint: int
    InvokeTestList: typing.List[InvokeTest]
    
    _RawData: dict

    def __init__(self, yamlPath: str):
        ''' Constructor that initializes the parser given a path to the YAML file.'''
        with open(yamlPath, "r") as stream:
           try:
               self._RawData = yaml.safe_load(stream)
           except yaml.YAMLError as exc:
               raise exc

        self.Name = self._RawData['name']
        self.Cluster = self._RawData['config']['cluster'].replace(' ','')
        self.Endpoint = self._RawData['config']['endpoint']
        self.InvokeTestList = []

        for item in self._RawData['tests']:
            # We only support parsing invoke interactions. As support for write/reads get added,
            # these skips will be removed.
            if (item['command'] == 'writeAttribute' or item['command'] == 'readAttribute'):
                print('Skipping test ', item['label'])
                continue

            test = self.InvokeTest(item['label'], stringcase.pascalcase(item['command']))
            test.Cluster = self.Cluster

            # Some of the tests contain 'cluster over-rides' that refer to a different
            # cluster than that specified at the top of the file.
            if (item.get('cluster')):
                test.Cluster = item.get('cluster').replace(' ', '').replace('/', '')

            test.RequestObject = eval('Clusters.' + test.Cluster + '.Commands.' + test.RequestTypeName)()

            if (item.get('arguments')):
                args = item['arguments']['values']
                test.RequestData = FixupYamlArguments(args)
                test.RequestData = FixupYamlTypes(test.RequestData, type(test.RequestObject))

                # Create a cluster object for the request from the provided YAML data.
                test.RequestObject = test.RequestObject.FromDict(test.RequestData)
            
            test.ExpectedRawResponse = item.get('response')
            if (test.ExpectedRawResponse != None and test.ExpectedRawResponse.get('values')):
                expectedCommand = 'Clusters.' + test.Cluster + '.Commands.' + stringcase.pascalcase(test.ExpectedRawResponse['command'])
                test.ExpectedResponseObject = eval(expectedCommand)
                test.ExpectedResponseData = test.ExpectedRawResponse['values']
                test.ExpectedResponseData = FixupYamlArguments(test.ExpectedResponseData)
                test.ExpectedResponseData = FixupYamlTypes(test.ExpectedResponseData, test.ExpectedResponseObject)
                test.ExpectedResponseObject = test.ExpectedResponseObject.FromDict(test.ExpectedResponseData)
                
            self.InvokeTestList.append(test)

    def ExecuteTests(self, devCtrl: ChipDeviceCtrl):
        for idx, test in enumerate(self.InvokeTestList):
            print("Test: ", idx, " -- Executing", test.Label)

            try:
                resp = asyncio.run(devCtrl.SendCommand(1, self.Endpoint, test.RequestObject))
            except chip.interaction_model.InteractionModelError:
                if (test.ExpectedRawResponse != None) and (test.ExpectedRawResponse.get('error')):
                    print("Got error, but was expected")
                else: 
                    raise

            if (test.ExpectedResponseObject != None):
                if (test.ExpectedResponseObject != resp):
                    raise AssertionError('Expected response didnt match actual')
