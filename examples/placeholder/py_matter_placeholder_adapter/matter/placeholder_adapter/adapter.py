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

# These constants represent the vocabulary used for the incoming JSON.
_CLUSTER_ID = 'clusterId'
_ENDPOINT_ID = 'endpointId'
_COMMAND_ID = 'commandId'
_ATTRIBUTE_ID = 'attributeId'
_WAIT_TYPE = 'waitType'

# These constants represent the vocabulary used for the outgoing data.
_CLUSTER = 'cluster'
_ENDPOINT = 'endpoint'
_COMMAND = 'command'
_ATTRIBUTE = 'attribute'
_WAIT_FOR = 'wait_for'


class Adapter:
    def __init__(self, definitions):
        self.encoder = PlaceholderEncoder()
        self.decoder = PlaceholderDecoder(definitions)

    def encode(self, request):
        return self.encoder.encode(request)

    def decode(self, response):
        return self.decoder.decode(response)


class PlaceholderEncoder:
    def encode(self, request):
        cluster = request.cluster
        command = request.command
        if cluster is None or command is None:
            return ''

        return request.command


class PlaceholderDecoder:
    def __init__(self, definitions):
        self.__definitions = definitions

    def decode(self, payload):
        payloads, logs = self.__get_payload_content(payload)
        json_response = payloads[0]

        decoded_response = {}

        for key, value in json_response.items():
            if key == _CLUSTER_ID:
                decoded_response[_CLUSTER] = self.__definitions.get_cluster_name(
                    value)
            elif key == _ENDPOINT_ID:
                decoded_response[_ENDPOINT] = value
            elif key == _COMMAND_ID:
                clusterId = json_response[_CLUSTER_ID]
                decoded_response[_COMMAND] = self.__definitions.get_response_name(
                    clusterId, value)
                if not decoded_response[_COMMAND]:
                    decoded_response[_COMMAND] = self.__definitions.get_command_name(clusterId, value)
            elif key == _ATTRIBUTE_ID:
                clusterId = json_response[_CLUSTER_ID]
                decoded_response[_ATTRIBUTE] = self.__definitions.get_attribute_name(
                    clusterId, value)
            elif key == _WAIT_TYPE:
                decoded_response[_WAIT_FOR] = value
            else:
                # Raise an error since the other fields probably needs to be translated too.
                raise KeyError(f'Error: field "{key}" in unsupported')

        return decoded_response, logs

    def __get_payload_content(self, payload):
        json_payload = json.loads(payload)
        logs = list(map(MatterLog, json_payload.get('logs')))
        results = json_payload.get('results')
        return results, logs


class MatterLog:
    def __init__(self, log):
        self.module = log['module']
        self.level = log['category']

        base64_message = log["message"].encode('utf-8')
        decoded_message_bytes = base64.b64decode(base64_message)
        self.message = decoded_message_bytes.decode('utf-8')
