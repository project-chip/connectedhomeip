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

import sys
import time

from ..pseudo_cluster import PseudoCluster
from .accessory_server_bridge import AccessoryServerBridge

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>DelayCommands</name>
    <code>0xFFF1FD02</code>

    <command source="client" code="0" name="WaitForCommissioning">
    </command>

    <command source="client" code="1" name="WaitForCommissionee">
      <arg name="nodeId" type="node_id"/>
      <arg name="expireExistingSession" type="bool" optional="true"/>
    </command>

    <command source="client" code="2" name="WaitForMs">
      <arg name="ms" type="int16u"/>
    </command>

    <command source="client" code="3" name="WaitForMessage">
      <arg name="registerKey" type="char_string"/>
      <arg name="message" type="char_string"/>
      <arg name="timeoutInSeconds" type="int16u" optional="true"/>
    </command>
</cluster>
</configurator>
'''


class DelayCommands(PseudoCluster):
    name = 'DelayCommands'
    definition = _DEFINITION

    async def WaitForMs(self, request):
        duration_in_ms = 0

        for argument in request.arguments['values']:
            if argument['name'] == 'ms':
                duration_in_ms = argument['value']

        sys.stdout.flush()
        time.sleep(int(duration_in_ms) / 1000)

    async def WaitForMessage(self, request):
        AccessoryServerBridge.waitForMessage(request)
