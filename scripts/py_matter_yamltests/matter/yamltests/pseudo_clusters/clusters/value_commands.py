#
#    Copyright (c) 2026 Project CHIP Authors
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
    <name>ValueCommands</name>
    <code>0xFFF1FD09</code>

    <command source="client" code="0" name="UnsignedNumberValue" response="UnsignedNumberValueResponse">
      <arg name="Value" type="int64u"/>
    </command>

    <command source="server" code="254" name="UnsignedNumberValueResponse">
      <arg name="Value" type="int64u"/>
    </command>
</cluster>
</configurator>
'''


class ValueCommands(PseudoCluster):
    name = 'ValueCommands'
    definition = _DEFINITION

    async def UnsignedNumberValue(self, request):
        value = None
        for argument in request.arguments['values']:
            if argument['name'] == 'Value':
                value = argument['value']

        return {'value': {'Value': value}}
