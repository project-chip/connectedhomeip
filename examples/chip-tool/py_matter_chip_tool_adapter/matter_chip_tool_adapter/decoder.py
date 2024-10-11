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

# These constants represent the vocabulary used for the incoming JSON.
_CLUSTER_ID = 'clusterId'
_ENDPOINT_ID = 'endpointId'
_RESPONSE_ID = 'commandId'
_ATTRIBUTE_ID = 'attributeId'
_EVENT_ID = 'eventId'

# These constants represent the vocabulary used for the outgoing data.
_CLUSTER = 'cluster'
_ENDPOINT = 'endpoint'
_RESPONSE = 'command'
_ATTRIBUTE = 'attribute'
_EVENT = 'event'


# These constants represent the common vocabulary between input and output.
_ERROR = 'error'
_CLUSTER_ERROR = 'clusterError'
_VALUE = 'value'
_DATA_VERSION = 'dataVersion'
_EVENT_NUMBER = 'eventNumber'

# FabricIndex is a special case where the field is added as a struct field by the SDK
# if needed but is not part of the XML definition of the struct.
# These constants are used to map from the field code (254) to the field name if such
# a field is used for a fabric scoped struct.
_FABRIC_INDEX_FIELD_CODE = '254'
_FABRIC_INDEX_FIELD_NAME = 'FabricIndex'
_FABRIC_INDEX_FIELD_NAME_DARWIN = 'fabricIndex'
_FABRIC_INDEX_FIELD_TYPE = 'int8u'


class Decoder:
    """
    This class implement decoding a test step response from the adapter format to the
    matter_yamltests format.
    """

    def __init__(self, specifications):
        self.__specs = specifications
        self.__converter = Converter(specifications)

    def decode(self, payload):
        payload, logs = self.__get_payload_content(payload)
        payload = self.__translate_names(payload)
        payload = self.__converter.convert(payload)

        if len(payload) == 0:
            payload = [{}]
        elif len(payload) > 1 and payload[-1] == {'error': 'FAILURE'}:
            payload = payload[:-1]
        return payload, logs

    def __translate_names(self, payloads):
        translated_payloads = []
        specs = self.__specs

        for payload in payloads:
            translated_payload = {}
            for key, value in payload.items():
                if key == _CLUSTER_ID:
                    key = _CLUSTER
                    value = specs.get_cluster_name(value)
                elif key == _ENDPOINT_ID:
                    key = _ENDPOINT
                elif key == _RESPONSE_ID:
                    key = _RESPONSE
                    value = specs.get_response_name(
                        payload[_CLUSTER_ID], value)
                elif key == _ATTRIBUTE_ID:
                    key = _ATTRIBUTE
                    value = specs.get_attribute_name(
                        payload[_CLUSTER_ID], value)
                elif key == _EVENT_ID:
                    key = _EVENT
                    value = specs.get_event_name(payload[_CLUSTER_ID], value)
                elif key == _VALUE or key == _ERROR or key == _CLUSTER_ERROR or key == _DATA_VERSION or key == _EVENT_NUMBER:
                    pass
                else:
                    # Raise an error since the other fields probably needs to be translated too.
                    raise KeyError(f'Error: field "{key}" not supported')

                if value is None and (key == _CLUSTER or key == _RESPONSE or key == _ATTRIBUTE or key == _EVENT) and _ERROR not in payload:
                    # If the definition for this cluster/command/attribute/event is missing, there is not
                    # much we can do to convert the response to the proper format. It usually indicates that
                    # the cluster definition is missing something. So we just raise an exception to tell the
                    # user something is wrong and the cluster definition needs to be updated.
                    # The only exception being when the definition can not be found but the returned payload
                    # contains an error. It could be because the payload is for an unknown cluster/command/attribute/event
                    # in which case we obviously don't have a definition for it.
                    cluster_code = hex(payload[_CLUSTER_ID])
                    if key == _CLUSTER:
                        raise KeyError(
                            f'Error: The cluster ({cluster_code}) definition can not be found. Please update the cluster definition.')
                    else:
                        value_code = hex(payload[key + 'Id'])
                        raise KeyError(
                            f'Error: The cluster ({cluster_code}) {key} ({value_code}) definition can not be found. Please update the cluster definition.')

                translated_payload[key] = value
            translated_payloads.append(translated_payload)

        return translated_payloads

    def __get_payload_content(self, payload):
        json_payload = json.loads(payload)
        results = json_payload.get('results')
        logs = MatterLog.decode_logs(json_payload.get('logs'))
        return results, logs


class MatterLog:
    def __init__(self, log):
        self.module = log['module']
        self.level = log['category']

        base64_message = log["message"].encode('utf-8')
        decoded_message_bytes = base64.b64decode(base64_message)
        # TODO We do assume utf-8 encoding is used, it may not be true though.
        self.message = decoded_message_bytes.decode('utf-8', 'replace')

    def decode_logs(logs):
        return list(map(MatterLog, logs))


class Converter():
    """
    This class converts between the JSON representation used by chip-tool to transmit
    information and the response format expected by the test suite.

    There is not much differences and ideally we won't have to do any conversion.
    For example chip-tool could do the field name mapping directly instead of relying on
    the adapter, or floats can be converted to the right format directly. But in the
    meantime the conversion is done here.
    """

    def __init__(self, specifications):
        self.__specs = specifications
        self.__converters = [
            StructFieldsNameConverter(),
            FloatConverter(),
            OctetStringConverter()
        ]

    def convert(self, payloads):
        return [self._convert(payload) for payload in payloads]

    def _convert(self, rv):
        if _VALUE not in rv or _CLUSTER not in rv:
            return rv

        if _RESPONSE in rv:
            out_value = self.__convert_command(rv)
        elif _ATTRIBUTE in rv:
            out_value = self.__convert_attribute(rv)
        elif _EVENT in rv:
            out_value = self.__convert_event(rv)
        else:
            out_value = rv[_VALUE]

        rv[_VALUE] = out_value
        return rv

    def __convert_command(self, rv):
        specs = self.__specs
        cluster_name = rv[_CLUSTER]
        response_name = rv[_RESPONSE]
        value = rv[_VALUE]

        response = specs.get_response_by_name(cluster_name, response_name)
        if not response:
            raise KeyError(f'Error: response "{response_name}" not found.')

        typename = response.name
        array = False
        return self.__run(value, cluster_name, typename, array)

    def __convert_attribute(self, rv):
        specs = self.__specs
        cluster_name = rv[_CLUSTER]
        attribute_name = rv[_ATTRIBUTE]
        value = rv[_VALUE]

        attribute = specs.get_attribute_by_name(cluster_name, attribute_name)
        if not attribute:
            raise KeyError(f'Error: attribute "{attribute_name}" not found.')

        typename = attribute.definition.data_type.name
        array = attribute.definition.is_list
        return self.__run(value, cluster_name, typename, array)

    def __convert_event(self, rv):
        specs = self.__specs
        cluster_name = rv[_CLUSTER]
        event_name = rv[_EVENT]
        value = rv[_VALUE]

        event = specs.get_event_by_name(cluster_name, event_name)
        if not event:
            raise KeyError(f'Error: event "{event_name}" not found.')

        typename = event.name
        array = False
        return self.__run(value, cluster_name, typename, array)

    def __run(self, value, cluster_name: str, typename: str, array: bool):
        for converter in self.__converters:
            value = converter.run(self.__specs, value,
                                  cluster_name, typename, array)
        return value


class BaseConverter:
    def run(self, specs, value, cluster_name: str, typename: str, array: bool):
        if isinstance(value, dict) and not array:
            struct = specs.get_struct_by_name(
                cluster_name, typename) or specs.get_event_by_name(cluster_name, typename)
            for field in struct.fields:
                field_name = field.name
                field_type = field.data_type.name
                field_array = field.is_list
                if field_name in value:
                    value[field_name] = self.run(
                        specs, value[field_name], cluster_name, field_type, field_array)
        elif isinstance(value, list) and array:
            value = [self.run(specs, v, cluster_name, typename, False)
                     for v in value]
        elif value is not None:
            value = self.maybe_convert(typename.lower(), value)

        return value

    def maybe_convert(self, typename: str, value):
        return value


class FloatConverter(BaseConverter):
    """
    Jsoncpp stores floats as double.
    For float values that are just stored as an approximation it ends up with
    a different output than expected when reading them back
    """

    def maybe_convert(self, typename, value):
        if typename == 'single':
            float_representation = float("%.16f" % value)
            value = float('%g' % float_representation)
        return value


class OctetStringConverter(BaseConverter):
    def maybe_convert(self, typename, value):
        if typename == 'octet_string' or typename == 'long_octet_string':
            if value == '':
                value = bytes()
            elif value.startswith('base64:'):
                value = base64.b64decode(value.removeprefix('base64:'))

        return value


class StructFieldsNameConverter():
    """
    Converts fields identifiers to the field names specified in the cluster definition.
    """

    def run(self, specs, value, cluster_name: str, typename: str, array: bool):
        if isinstance(value, dict) and not array:
            struct = specs.get_struct_by_name(
                cluster_name, typename) or specs.get_event_by_name(cluster_name, typename)
            for field in struct.fields:
                field_code = field.code
                field_name = field.name
                field_type = field.data_type.name
                field_array = field.is_list

                provided_field_name = field_name

                if str(field_code) in value:
                    # chip-tool returns the field code as an integer but the test suite expects
                    # a field name.
                    # To not confuse the test suite, the field code is replaced by its field name
                    # equivalent and then removed.
                    provided_field_name = str(field_code)
                else:
                    # darwin-framework-tool returns the field name but with a different casing than
                    # what the test suite expects.
                    # To not confuse the test suite, the field name is replaced by its field name
                    # equivalent from the spec and then removed.

                    if field_name == 'Description' and 'descriptionString' in value:
                        # "Description" is returned as "descriptionString" since 'description' is a reserved keyword
                        # and can not be exposed in an objc struct.
                        provided_field_name = 'descriptionString'

                    # If field_name starts with a sequence of capital letters lowercase all but the last one.
                    provided_field_name = re.sub(
                        '^([A-Z]+)([A-Z])',
                        lambda m: m.group(1).lower() + m.group(2),
                        provided_field_name
                    )

                    # All field names in darwin-framework-tool start with a lowercase letter.
                    provided_field_name = provided_field_name[0].lower(
                    ) + provided_field_name[1:]

                if provided_field_name in value:
                    value[field_name] = self.run(
                        specs,
                        value[provided_field_name],
                        cluster_name,
                        field_type,
                        field_array
                    )
                    if provided_field_name != field_name:
                        del value[provided_field_name]

            if specs.is_fabric_scoped(struct):
                if _FABRIC_INDEX_FIELD_CODE in value:
                    key_name = _FABRIC_INDEX_FIELD_CODE
                elif _FABRIC_INDEX_FIELD_NAME_DARWIN in value:
                    key_name = _FABRIC_INDEX_FIELD_NAME_DARWIN

                value[_FABRIC_INDEX_FIELD_NAME] = self.run(
                    specs,
                    value[key_name],
                    cluster_name,
                    _FABRIC_INDEX_FIELD_TYPE,
                    False)
                del value[key_name]

        elif isinstance(value, list) and array:
            value = [self.run(specs, v, cluster_name, typename, False)
                     for v in value]

        return value
