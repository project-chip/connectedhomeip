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


from matter_yamltests.pseudo_clusters.pseudo_cluster import PseudoCluster

_VALUE_ARGUMENT_NAME = 'Value'
_CLUSTERS_ARGUMENT_NAME = 'Clusters'


class wildcard_response_extractor_cluster(PseudoCluster):
    name = 'WildcardResponseExtractorCluster'

    async def GetDefaultEndPointForClusters(self, request, definitions):
        entries = self.__get_argument(request, _VALUE_ARGUMENT_NAME)
        clusters = self.__get_argument(request, _CLUSTERS_ARGUMENT_NAME)
        if entries is None or clusters is None:
            return {'error': 'INVALID_ARGUMENT'}

        results = {}

        for cluster in clusters:
            cluster_id = definitions.get_cluster_id_by_name(cluster)
            results[cluster] = None

            for entry in entries:
                server_list = entry.get('value')
                if cluster_id in server_list:
                    results[cluster] = entry.get('endpoint')
                    break

        return {'value': results}

    async def GetUnsupportedCluster(self, request):
        entries = self.__get_argument(request, _VALUE_ARGUMENT_NAME)
        if entries is None:
            return {'error': 'INVALID_ARGUMENT'}

        cluster_ids = []
        for entry in entries:
            server_list = entry.get('value')
            for cluster_id in server_list:
                if cluster_id not in cluster_ids:
                    cluster_ids.append(cluster_id)

        unsupported_cluster = None
        for cluster_code in range(0xFFFFFFFF):
            if cluster_code not in cluster_ids:
                unsupported_cluster = f'{cluster_code:#0{10}x}'
                break

        return {'value': {'UnsupportedCluster': unsupported_cluster}}

    async def GetUnsupportedCommand(self, request):
        entries = self.__get_argument(request, _VALUE_ARGUMENT_NAME)
        if entries is None:
            return {'error': 'INVALID_ARGUMENT'}

        command_ids = []
        for entry in entries:
            commands_list = entry.get('value')
            for command_id in commands_list:
                if command_id not in command_ids:
                    command_ids.append(command_id)

        unsupported_command = None
        for command_code in range(0xFFFFFFFF):
            if command_code not in command_ids:
                unsupported_command = f'{command_code:#0{10}x}'
                break

        return {'value': {'UnsupportedCommand': unsupported_command}}

    async def GetUnsupportedEndPoint(self, request):
        entries = self.__get_argument(request, _VALUE_ARGUMENT_NAME)
        if entries is None:
            return {'error': 'INVALID_ARGUMENT'}

        endpoint_ids = []
        for entry in entries:
            parts_list = entry.get('value')
            for endpoint_id in parts_list:
                if endpoint_id not in endpoint_ids:
                    endpoint_ids.append(endpoint_id)

            # Add the endpoint id of the response if needed.
            endpoint_id = entry.get('endpoint')
            if endpoint_id not in endpoint_ids:
                endpoint_ids.append(endpoint_id)

        unsupported_endpoint = None
        for endpoint_code in range(0xFFFF):
            if endpoint_code not in endpoint_ids:
                unsupported_endpoint = endpoint_code
                break

        return {'value': {'UnsupportedEndPoint': unsupported_endpoint}}

    def __get_argument(self, request, argument_name):
        arguments = request.arguments.get('values')
        if arguments is None:
            return None

        if type(arguments) is not list:
            return None

        for argument in arguments:
            name = argument.get('name')
            value = argument.get('value')
            if name is None or value is None:
                return None

            if name == argument_name:
                return value

        return None
