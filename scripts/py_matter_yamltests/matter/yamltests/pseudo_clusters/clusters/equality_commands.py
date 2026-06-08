#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

from ..pseudo_cluster import PseudoCluster

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>EqualityCommands</name>
    <code>0xFFF1FD08</code>

    <command source="client" code="0" name="BooleanEquals" response="EqualityResponse">
      <arg name="Value1" type="boolean"/>
      <arg name="Value2" type="boolean"/>
    </command>

    <command source="client" code="1" name="SignedNumberEquals" response="EqualityResponse">
      <arg name="Value1" type="int64s"/>
      <arg name="Value2" type="int64s"/>
    </command>

    <command source="client" code="2" name="UnsignedNumberEquals" response="EqualityResponse">
      <arg name="Value1" type="int64u"/>
      <arg name="Value2" type="int64u"/>
    </command>

    <command source="server" code="254" name="EqualityResponse">
      <arg name="Equals" type="bool"/>
    </command>
</cluster>
</configurator>
'''


def Compare(request):
    value1 = None
    value2 = None

    for argument in request.arguments['values']:
        if argument['name'] == 'Value1':
            value1 = argument['value']
        elif argument['name'] == 'Value2':
            value2 = argument['value']

    return value1 == value2


class EqualityCommands(PseudoCluster):
    name = 'EqualityCommands'
    definition = _DEFINITION

    async def BooleanEquals(self, request):
        return {'value': {'Equals': Compare(request)}}

    async def SignedNumberEquals(self, request):
        return {'value': {'Equals': Compare(request)}}

    async def UnsignedNumberEquals(self, request):
        return {'value': {'Equals': Compare(request)}}
