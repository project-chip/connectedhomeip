#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
