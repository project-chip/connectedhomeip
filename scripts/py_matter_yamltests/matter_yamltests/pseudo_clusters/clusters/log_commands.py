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
    <name>LogCommands</name>
    <code>0xFFF1FD01</code>

    <command source="client" code="0" name="Log">
      <arg name="message" type="char_string"/>
    </command>

    <command source="client" code="1" name="UserPrompt">
      <arg name="message" type="char_string"/>
      <arg name="expectedValue" type="char_string" optional="true"/>
    </command>
</cluster>
</configurator>
'''


class LogCommands(PseudoCluster):
    name = 'LogCommands'
    definition = _DEFINITION

    async def UserPrompt(self, request):
        expected_value = None
        for value in request.arguments.get("values", []):
            if value.get('name') and 'expectedValue' in value['name']:
                expected_value = value['value']
                request.responses = [{"values": [{"name": "expectedValue", "value": expected_value}]}]

        if expected_value is not None:
            input_result = input("")
            return {"value": {"expectedValue": input_result}}

        return {}

    async def Log(self, request):
        pass
