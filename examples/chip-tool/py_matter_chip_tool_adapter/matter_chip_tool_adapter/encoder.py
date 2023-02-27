# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import base64
import json
import re

_ALIASES = {
    'CommissionerCommands': {
        'alias': 'pairing',
        'commands': {
            'PairWithCode': {
                'alias': 'code',
                'arguments': {
                    'nodeId': 'node-id',
                    'discoverOnce': 'discover-once',
                },
                'has_destination': False,
                'has_endpoint': False,
            },
            'GetCommissionerNodeId': {
                'has_destination': False,
                'has_endpoint': False,
            }
        }
    },

    'DelayCommands': {
        'alias': 'delay',
        'commands': {
            'WaitForCommissionee': {
                'arguments': {
                    'expireExistingSession': 'expire-existing-session',
                },
                'has_destination': False,
                'has_endpoint': False,
            }
        }
    },

    'DiscoveryCommands': {
        'alias': 'discover',
        'commands': {
            'FindCommissionable': {
                'alias': 'commissionables',
                'has_destination': False,
                'has_endpoint': False,
            },
            'FindCommissionableByShortDiscriminator': {
                'has_destination': False,
                'has_endpoint': False,
            },
            'FindCommissionableByLongDiscriminator': {
                'has_destination': False,
                'has_endpoint': False,
            },
            'FindCommissionableByVendorId': {
                'has_destination': False,
                'has_endpoint': False,
            },
            'FindCommissionableByDeviceType': {
                'has_destination': False,
                'has_endpoint': False,
            },
            'FindCommissionableByCommissioningMode': {
                'has_destination': False,
                'has_endpoint': False,
            },
        }
    }
}

_GLOBAL_ALIASES = {
    'readAttribute': 'read',
    'writeAttribute': 'write',
    'subscribeAttribute': 'subscribe',
    'readEvent': 'read-event',
    'subscribeEvent': 'subscribe-event',
}


class Encoder:
    """
    This class converts the names from the YAML tests to the chip-tool equivalent.
    """

    def __init__(self, specifications):
        self.__specs = specifications

    def encode(self, request):
        cluster = self.__get_cluster_name(request)
        command, command_specifier = self.__get_command_name(request)

        if command == 'wait-for-report':
            return ''

        arguments = self.__get_arguments(request)
        base64_arguments = base64.b64encode(
            (f'{{ {arguments} }}').encode('utf-8')).decode('utf-8')

        payload = f'"cluster": "{cluster}", "command": "{command}", "arguments" : "base64:{base64_arguments}"'
        if command_specifier:
            payload += f', "command_specifier": "{command_specifier}"'
        return f'json:{{ {payload} }}'

    def __get_cluster_name(self, request):
        return self.__get_alias(request.cluster) or self.__format_cluster_name(request.cluster)

    def __get_command_name(self, request):
        command_name = self.__get_alias(
            request.cluster, request.command) or self.__format_command_name(request.command)

        # 'readAttribute' is converted to 'read attr-name', 'writeAttribute' is converted to 'write attr-name',
        # 'readEvent' is converted to 'read event-name', etc.
        if request.is_attribute:
            command_specifier = self.__format_command_name(request.attribute)
            # chip-tool exposes writable attribute under the "write" command, but for non-writable
            # attributes, those appear under the "force-write" command.
            if command_name == 'write':
                attribute = self.__specs.get_attribute_by_name(
                    request.cluster, request.attribute)
                if attribute and not attribute.is_writable:
                    command_name = 'force-write'
        elif request.is_event:
            command_specifier = self.__format_command_name(request.event)
        else:
            command_specifier = None

        return command_name, command_specifier

    def __get_arguments(self, request):
        arguments = ''
        arguments = self.__maybe_add_destination(arguments, request)
        arguments = self.__maybe_add_endpoint(arguments, request)
        arguments = self.__maybe_add_command_arguments(arguments, request)
        arguments = self.__maybe_add(
            arguments, request.min_interval, "min-interval")
        arguments = self.__maybe_add(
            arguments, request.max_interval, "max-interval")
        arguments = self.__maybe_add(arguments, request.timed_interaction_timeout_ms,
                                     "timedInteractionTimeoutMs")
        arguments = self.__maybe_add(
            arguments, request.event_number, "event-min")
        arguments = self.__maybe_add(
            arguments, request.busy_wait_ms, "busyWaitForMs")
        arguments = self.__maybe_add(
            arguments, request.identity, "commissioner-name")
        arguments = self.__maybe_add(arguments, request.fabric_filtered,
                                     "fabric-filtered")

        return arguments

    def __maybe_add_destination(self, rv, request):
        if not self._supports_destination(request):
            return rv

        destination_argument_name = 'destination-id'
        destination_argument_value = None

        if request.group_id:
            destination_argument_value = hex(
                0xffffffffffff0000 | int(request.group_id))
        elif request.node_id:
            destination_argument_value = hex(request.node_id)
        else:
            destination_argument_value = None

        if rv:
            rv += ', '
        rv += f'"{destination_argument_name}": "{destination_argument_value}"'
        return rv

    def __maybe_add_endpoint(self, rv, request):
        if not self._supports_endpoint(request):
            return rv

        endpoint_argument_name = 'endpoint-id-ignored-for-group-commands'
        endpoint_argument_value = request.endpoint

        if (request.is_attribute and not request.command == "writeAttribute") or request.is_event:
            endpoint_argument_name = 'endpoint-ids'

        if rv:
            rv += ', '
        rv += f'"{endpoint_argument_name}": "{endpoint_argument_value}"'
        return rv

    def __maybe_add_command_arguments(self, rv, request):
        if request.arguments is None:
            return rv

        for entry in request.arguments['values']:
            name = self.__get_argument_name(request, entry)
            value = self.__encode_value(entry['value'])
            if rv:
                rv += ', '
            rv += f'"{name}":{value}'

        return rv

    def __get_argument_name(self, request, entry):
        cluster_name = request.cluster
        command_name = request.command
        argument_name = entry.get('name')

        if request.is_attribute:
            if command_name == 'writeAttribute':
                argument_name = 'attribute-values'
            else:
                argument_name = 'value'

        return self.__get_alias(cluster_name, command_name, argument_name) or argument_name

    def __maybe_add(self, rv, value, name):
        if value is None:
            return rv

        if rv:
            rv += ', '
        rv += f'"{name}":"{value}"'
        return rv

    def __encode_value(self, value):
        value = self.__encode_octet_strings(value)
        value = self.__lower_camel_case_member_fields(value)
        return self.__convert_to_json_string(value)

    def __encode_octet_strings(self, value):
        if isinstance(value, list):
            value = [self.__encode_octet_strings(entry) for entry in value]
        elif isinstance(value, dict):
            value = {key: self.__encode_octet_strings(
                value[key]) for key in value}
        elif isinstance(value, bytes):
            value = 'hex:' + ''.join("{:02x}".format(c) for c in value)
        return value

    def __lower_camel_case_member_fields(self, value):
        if isinstance(value, list):
            value = [self.__lower_camel_case_member_fields(
                entry) for entry in value]
        elif isinstance(value, dict):
            value = {self.__to_lower_camel_case(
                key): self.__lower_camel_case_member_fields(value[key]) for key in value}
        return value

    def __convert_to_json_string(self, value):
        is_str = isinstance(value, str)
        value = json.dumps(value)
        if not is_str:
            value = value.replace("\"", "\\\"")
            value = f'"{value}"'

        return value

    def __to_lower_camel_case(self, name):
        return name[:1].lower() + name[1:]

    def __format_cluster_name(self, name):
        return name.lower().replace(' ', '').replace('/', '').lower()

    def __format_command_name(self, name):
        if name is None:
            return name

        return re.sub(r'([a-z])([A-Z])', r'\1-\2', name).replace(' ', '-').replace(':', '-').replace('/', '').replace('_', '-').lower()

    def __get_alias(self, cluster_name: str, command_name: str = None, argument_name: str = None):
        if argument_name is None and command_name in _GLOBAL_ALIASES:
            return _GLOBAL_ALIASES.get(command_name)

        aliases = _ALIASES.get(cluster_name)
        if aliases is None:
            return None

        if command_name is None:
            return aliases.get('alias')

        aliases = aliases.get('commands')
        if aliases is None or aliases.get(command_name) is None:
            return None

        aliases = aliases.get(command_name)
        if argument_name is None:
            return aliases.get('alias')

        aliases = aliases.get('arguments')
        if aliases is None or aliases.get(argument_name) is None:
            return None

        return aliases.get(argument_name)

    def _supports_endpoint(self, request):
        return self._has_support(request, 'has_endpoint')

    def _supports_destination(self, request):
        return self._has_support(request, 'has_destination')

    def _has_support(self, request, feature_name):
        aliases = _ALIASES.get(request.cluster)
        if aliases is None:
            return True

        aliases = aliases.get('commands')
        if aliases is None:
            return True

        aliases = aliases.get(request.command)
        if aliases is None:
            return True

        return aliases.get(feature_name, True)
