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

import inspect
from typing import List

from .clusters.commissioner_commands import CommissionerCommands
from .clusters.delay_commands import DelayCommands
from .clusters.discovery_commands import DiscoveryCommands
from .clusters.equality_commands import EqualityCommands
from .clusters.log_commands import LogCommands
from .clusters.system_commands import SystemCommands
from .pseudo_cluster import PseudoCluster


class PseudoClusters:
    def __init__(self, clusters: List[PseudoCluster]):
        self.clusters = clusters

    def supports(self, request) -> bool:
        return False if self.__get_command(request) is None else True

    def is_manual_step(self, request):
        return (request.cluster == LogCommands().name and
                request.command == "UserPrompt")

    def add(self, cluster: PseudoCluster):
        self.clusters.append(cluster)

    async def execute(self, request, definitions=None):
        status = {'error': 'FAILURE'}

        command = self.__get_command(request)
        if command:
            if 'definitions' in inspect.signature(command).parameters:
                status = await command(request, definitions)
            else:
                status = await command(request)

            # If the command does not returns an error, it is considered a success.
            if status is None:
                status = {}

        return status, []

    def __get_command(self, request):
        for cluster in self.clusters:
            if request.cluster == cluster.name and getattr(cluster, request.command, None):
                return getattr(cluster, request.command)
        return None


def get_default_pseudo_clusters() -> PseudoClusters:
    clusters = [
        CommissionerCommands(),
        DelayCommands(),
        DiscoveryCommands(),
        EqualityCommands(),
        LogCommands(),
        SystemCommands()
    ]
    return PseudoClusters(clusters)
