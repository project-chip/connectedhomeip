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

import asyncio
import sys
import time

from ..pseudo_cluster import PseudoCluster
from .accessory_server_bridge import AccessoryServerBridge


class MockTestStep:

    def __init__(self, cluster, command, attribute, endpoint, node_id):
        self.cluster = cluster
        self.command = command
        self.attribute = attribute
        self.endpoint = endpoint
        self.node_id = node_id

        self.group_id = None
        self.is_attribute = True
        self.is_event = False
        self.arguments = None
        self.responses = None
        self.event = None

        self.min_interval = None
        self.max_interval = None
        self.keep_subscriptions = None
        self.timed_interaction_timeout_ms = None
        self.timeout = None
        self.event_number = None
        self.busy_wait_ms = None
        self.identity = None
        self.fabric_filtered = True
        self.data_version = None
        self.label = "MockTestStep"
        self.is_pics_enabled = True


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

    <command source="client" code="4" name="WaitForAttributeValue">
      <arg name="attribute" type="char_string"/>
      <arg name="expectedValue" type="any"/>
      <arg name="expectedDurationMs" type="int32u"/>
      <arg name="cluster" type="char_string"/>
      <arg name="endpoint" type="int16u"/>
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
        await asyncio.sleep(int(duration_in_ms) / 1000)

    async def WaitForMessage(self, request):
        AccessoryServerBridge.waitForMessage(request)

    async def WaitForAttributeValue(self, request, runner=None, config=None):
        if runner is None or config is None:
            raise ValueError("Runner and config are required for WaitForAttributeValue")

        args = {arg['name']: arg['value'] for arg in request.arguments['values']}
        attribute_name = args['attribute']
        expected_value = args['expectedValue']
        expected_duration_ms = args['expectedDurationMs']
        cluster_name = args['cluster']
        endpoint = args['endpoint']

        extra_duration_ms = request.get_config_value('valueWaitExtraDurationMs')
        if extra_duration_ms is None:
            extra_duration_ms = 250

        timeout_s = (expected_duration_ms + extra_duration_ms) / 1000.0
        poll_interval_s = 0.1
        start_time = time.time()

        mock_step = MockTestStep(
            cluster=cluster_name,
            command='readAttribute',
            attribute=attribute_name,
            endpoint=endpoint,
            node_id=request.node_id
        )

        history = []
        max_history = 10

        while True:
            try:
                responses, logs = await runner.run_step(mock_step, config)
                response = responses[0] if isinstance(responses, list) and responses else None
                if response is None:
                    history.append("empty response")
                elif 'error' in response:
                    history.append(f"error: {response['error']}")
                else:
                    value = response.get('value')
                    history.append(f"value: {value}")
                    if value == expected_value:
                        return
            except (AttributeError, NameError, TypeError):
                # Let programming errors (bugs in our code or test definition)
                # propagate immediately instead of timing out.
                raise
            except Exception as e:
                # We ignore intermediate errors during polling (e.g., node
                # temporarily unreachable, packet loss, or busy device) because
                # the device might recover and report the expected value before
                # the timeout expires. If the error persists, the command will
                # eventually fail with a timeout exception. We keep a history of
                # these failures to help with debugging.
                history.append(f"exception: {str(e)}")

            if len(history) > max_history:
                history.pop(0)

            if time.time() - start_time >= timeout_s:
                break

            await asyncio.sleep(poll_interval_s)

        history_str = "\n".join(f"  Attempt -{len(history)-i}: {attempt}" for i, attempt in enumerate(history))
        raise TimeoutError(f"Timeout waiting for attribute {cluster_name}.{attribute_name} to become {expected_value}.\n"
                            f"Recent history of attempts:\n{history_str}")
