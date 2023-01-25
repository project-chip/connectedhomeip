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

from .clusters.delay_commands import DelayCommands
from .clusters.log_commands import LogCommands
from .clusters.system_commands import SystemCommands
from .pseudo_cluster import PseudoCluster


class PseudoClusters:
    def __init__(self, clusters: list[PseudoCluster]):
        self.__clusters = clusters

    def supports(self, request) -> bool:
        return False if self.__get_command(request) is None else True

    async def execute(self, request):
        status = {'error': 'FAILURE'}

        command = self.__get_command(request)
        if command:
            status = await command(request)
            # If the command does not returns an error, it is considered a success.
            if status == None:
                status = {}

        return status, []

    def __get_command(self, request):
        for cluster in self.__clusters:
            if request.cluster == cluster.name and getattr(cluster, request.command, None):
                return getattr(cluster, request.command)
        return None


def get_default_pseudo_clusters() -> PseudoClusters:
    clusters = [
        DelayCommands(),
        LogCommands(),
        SystemCommands()
    ]
    return PseudoClusters(clusters)
