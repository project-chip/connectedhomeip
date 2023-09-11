#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

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
