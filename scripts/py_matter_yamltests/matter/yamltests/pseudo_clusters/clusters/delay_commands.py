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
from dataclasses import dataclass
from typing import Any

from ..pseudo_cluster import PseudoCluster
from .accessory_server_bridge import AccessoryServerBridge


@dataclass
class MockTestStep:
    """Mock of TestStep used to execute read attribute actions dynamically.

    This lightweight class simulates a parsed YAML test step. It is passed to
    `runner.run_step` during WaitForAttributeValue polling to execute standard
    attribute reads without needing to parse a full YAML structure.

    Properties are initialized to defaults that match typical read steps. New
    properties should be added to this mock if they are accessed by runner
    adapters (e.g., in their `encode` methods) during action compilation.
    """
    cluster: str
    command: str
    attribute: str
    endpoint: int
    node_id: int

    group_id: int | None = None
    is_attribute: bool = True
    is_event: bool = False
    arguments: Any | None = None
    responses: Any | None = None
    event: str | None = None

    min_interval: int | None = None
    max_interval: int | None = None
    keep_subscriptions: bool | None = None
    timed_interaction_timeout_ms: int | None = None
    timeout: int | None = None
    event_number: int | None = None
    busy_wait_ms: int | None = None
    identity: str | None = None
    fabric_filtered: bool = True
    data_version: int | None = None
    label: str = "MockTestStep"
    is_pics_enabled: bool = True


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
        """Polls an attribute until it reaches the expected value or times out.

        This method dynamically constructs a read attribute step and executes it
        repeatedly every 100ms. It ignores intermediate network or device errors
        during polling, as the device might recover before the timeout.

        Args:
            request: The TestStep containing arguments:
                - attribute: Name of the attribute to read.
                - expectedValue: The target value to wait for.
                - expectedDurationMs: Nominal transition time in milliseconds.
                - cluster: Name of the cluster.
                - endpoint: Endpoint ID.
            runner: The TestRunner instance to execute the read step.
            config: The TestRunnerConfig configuration.

        Raises:
            ValueError: If runner or config are not provided.
            TimeoutError: If the expected value is not reached within the timeout
                (expectedDurationMs + valueWaitExtraDurationMs).
        """
        if runner is None or config is None:
            raise ValueError("Runner and config are required for WaitForAttributeValue")

        args = {arg['name']: arg['value'] for arg in request.arguments['values']}
        attribute_name = args['attribute']
        expected_value = args['expectedValue']
        expected_duration_ms = args['expectedDurationMs']
        cluster_name = args['cluster']
        endpoint = args['endpoint']

        extra_duration_ms = request.get_config_value('valueWaitExtraDurationMs', 250)

        timeout_s = (expected_duration_ms + extra_duration_ms) / 1000.0
        poll_interval_s = 0.1
        start_time = time.monotonic()

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
            elapsed_s = time.monotonic() - start_time
            try:
                responses, _ = await runner.run_step(mock_step, config)
                if isinstance(responses, list) and responses:
                    response = responses[0]
                elif isinstance(responses, dict):
                    response = responses
                else:
                    response = None

                if response is None:
                    history.append(f"[{elapsed_s:.2f}s] empty response")
                elif 'error' in response:
                    history.append(f"[{elapsed_s:.2f}s] error: {response['error']}")
                else:
                    value = response.get('value')
                    history.append(f"[{elapsed_s:.2f}s] value: {value}")
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
                history.append(f"[{elapsed_s:.2f}s] exception: {str(e)}")

            if len(history) > max_history:
                history.pop(0)

            if time.monotonic() - start_time >= timeout_s:
                break

            await asyncio.sleep(poll_interval_s)

        history_str = "\n".join(f"  {attempt}" for attempt in history)
        raise TimeoutError(f"Timeout waiting for attribute {cluster_name}.{attribute_name} to become {expected_value}.\n"
                           f"Recent history of attempts:\n{history_str}")
