#
#    Copyright (c) 2022 Project CHIP Authors
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

import xml.etree.ElementTree as ET
import glob
import time


class ClustersDefinitions:
    def __init__(self, clusters_dir):
        self.__clusterIdToName = {}
        self.__commandIdToName = {}
        self.__attributeIdToName = {}
        self.__globalAttributeIdToName = {}
        self.__eventIdToName = {}

        self.__commands = {}
        self.__responses = {}
        self.__attributes = {}
        self.__events = {}
        self.__structs = {}
        self.__enums = {}
        self.__bitmaps = {}

        clusters_files = glob.glob(clusters_dir + '**/*.xml', recursive=True)
        for cluster_file in clusters_files:
            tree = ET.parse(cluster_file)

            root = tree.getroot()

            structs = root.findall('struct')
            for struct in structs:
                self.__structs[struct.get('name').lower()] = struct

            enums = root.findall('enum')
            for enum in enums:
                self.__enums[enum.get('name').lower()] = enum

            bitmaps = root.findall('bitmap')
            for bitmap in bitmaps:
                self.__bitmaps[bitmap.get('name').lower()] = bitmap

            global_elements = root.find('global')
            if global_elements:
                global_attributes = global_elements.findall('attribute')

                for attribute in global_attributes:
                    attribute_side = attribute.get('side')
                    attribute_code = attribute.get('code')

                    description = attribute.find('description')
                    attribute_name = description.text if description is not None else attribute.text

                    if attribute_code.startswith('0x') or attribute_code.startswith('0X'):
                        attribute_code = int(attribute_code, base=16)
                    self.__globalAttributeIdToName[attribute_code] = attribute_name

                    if attribute_side == 'server':
                        self.__attributes[attribute_name.lower()] = attribute

            cluster = root.find('cluster')
            if not cluster:
                continue

            cluster_code = cluster.find('code').text
            cluster_name = cluster.find('name').text

            if cluster_code.startswith('0x') or cluster_code.startswith('0X'):
                cluster_code = int(cluster_code, base=16)

            self.__clusterIdToName[cluster_code] = cluster_name
            self.__commandIdToName[cluster_code] = {}
            self.__attributeIdToName[cluster_code] = {}
            self.__eventIdToName[cluster_code] = {}

            commands = cluster.findall('command')
            for command in commands:
                command_source = command.get('source')
                command_code = command.get('code')
                command_name = command.get('name')

                base = 16 if command_code.startswith('0x') or command_code.startswith('0X') else 10
                command_code = int(command_code, base=base)
                self.__commandIdToName[cluster_code][command_code] = command_name

                if command_source == 'client':
                    self.__commands[command_name.lower()] = command
                elif command_source == 'server':
                    # The name is not converted to lowercase here
                    self.__responses[command_name] = command

            attributes = cluster.findall('attribute')
            for attribute in attributes:
                attribute_side = attribute.get('side')
                attribute_code = attribute.get('code')

                description = attribute.find('description')
                attribute_name = description.text if description is not None else attribute.text

                base = 16 if attribute_code.startswith('0x') or attribute_code.startswith('0X') else 10
                attribute_code = int(attribute_code, base=base)
                self.__attributeIdToName[cluster_code][attribute_code] = attribute_name

                if attribute_side == 'server':
                    self.__attributes[attribute_name.lower()] = attribute

            events = cluster.findall('event')
            for event in events:
                event_side = event.get('side')
                event_code = event.get('code')

                description = event.find('description')
                event_name = description.text if description is not None else event.text

                base = 16 if event_code.startswith('0x') or event_code.startswith('0X') else 10
                event_code = int(event_code, base=base)
                self.__eventIdToName[cluster_code][event_code] = event_name

                if event_side == 'server':
                    self.__events[event_name.lower()] = event

    def get_cluster_name(self, cluster_id):
        return self.__clusterIdToName[cluster_id]

    def get_command_name(self, cluster_id, command_id):
        return self.__commandIdToName[cluster_id][command_id]

    def get_attribute_name(self, cluster_id, attribute_id):
        if attribute_id in self.__globalAttributeIdToName:
            return self.__globalAttributeIdToName[attribute_id]
        return self.__attributeIdToName[cluster_id][attribute_id]

    def get_event_name(self, cluster_id, event_id):
        return self.__eventIdToName[cluster_id][event_id]

    def get_response_mapping(self, command_name):
        if not command_name in self.__responses:
            return None
        response = self.__responses[command_name]

        args = response.findall('arg')

        mapping = {}
        for mapping_index, arg in enumerate(args):
            mapping[str(mapping_index)] = {'name': arg.get('name'), 'type': arg.get('type').lower()}
        return mapping

    def get_attribute_mapping(self, attribute_name):
        if not attribute_name.lower() in self.__attributes:
            return None
        attribute = self.__attributes[attribute_name.lower()]

        attribute_type = attribute.get('type')
        if attribute_type.lower() == 'array':
            attribute_type = attribute.get('entryType')

        if not self.get_type_mapping(attribute_type):
            return None

        return {'name': attribute_name.lower(), 'type': attribute_type}

    def get_event_mapping(self, event_name):
        return None

    def get_type_mapping(self, type_name):
        struct = self.__structs.get(type_name.lower())
        if struct is None:
            return None

        mapping = {}

        # FabricIndex is a special case where the framework requires it to be passed even if it is not part of the
        # requested arguments per spec and not part of the XML definition.
        if struct.get('isFabricScoped'):
            mapping[str(254)] = {'name': 'FabricIndex', 'type': 'int64u'}

        mapping_index = 0

        items = struct.findall('item')
        for item in items:
            if item.get('fieldId'):
                mapping_index = int(item.get('fieldId'))
            mapping[str(mapping_index)] = {'name': item.get('name'), 'type': item.get('type').lower()}
            mapping_index += 1

        return mapping

    def get_attribute_definition(self, attribute_name):
        attribute = self.__attributes.get(attribute_name.lower())
        if attribute is None:
            return None

        attribute_type = attribute.get('type').lower()
        if attribute_type == 'array':
            attribute_type = attribute.get('entryType').lower()
        return self.__to_base_type(attribute_type)

    def get_command_args_definition(self, command_name):
        command = self.__commands.get(command_name.lower())
        if command is None:
            return None

        return self.__args_to_dict(command.findall('arg'))

    def get_response_args_definition(self, command_name):
        command = self.__commands.get(command_name.lower())
        if command is None:
            return None

        response = self.__responses.get(command.get('response'))
        if response is None:
            return None

        return self.__args_to_dict(response.findall('arg'))

    def __args_to_dict(self, args):
        rv = {}
        for item in args:
            arg_name = item.get('name')
            arg_type = item.get('type').lower()
            rv[arg_name] = self.__to_base_type(arg_type)
        return rv

    def __to_base_type(self, item_type):
        if item_type in self.__bitmaps:
            bitmap = self.__bitmaps[item_type]
            item_type = bitmap.get('type').lower()
        elif item_type in self.__enums:
            enum = self.__enums[item_type]
            item_type = enum.get('type').lower()
        elif item_type in self.__structs:
            struct = self.__structs[item_type]
            item_type = self.__struct_to_dict(struct)
        return item_type

    def __struct_to_dict(self, struct):
        type_entry = {}
        for item in struct.findall('item'):
            item_name = item.get('name')
            item_type = item.get('type').lower()
            type_entry[item_name] = self.__to_base_type(item_type)
        return type_entry
