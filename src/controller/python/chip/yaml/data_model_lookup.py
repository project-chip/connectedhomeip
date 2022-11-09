#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

from abc import ABC, abstractmethod
import chip.clusters as Clusters


class DataModelLookup(ABC):
    @abstractmethod
    def get_cluster(self, cluster: str):
        pass

    @abstractmethod
    def get_command(self, cluster: str, command: str):
        pass

    @abstractmethod
    def get_attribute(self, cluster: str, attribute: str):
        pass


class PreDefinedDataModelLookup(DataModelLookup):
    def get_cluster(self, cluster: str):
        try:
            return getattr(Clusters, cluster, None)
        except AttributeError:
            return None

    def get_command(self, cluster: str, command: str):
        try:
            commands = getattr(Clusters, cluster, None).Commands
            return getattr(commands, command, None)
        except AttributeError:
            return None

    def get_attribute(self, cluster: str, attribute: str):
        try:
            attributes = getattr(Clusters, cluster, None).Attributes
            return getattr(attributes, attribute, None)
        except AttributeError:
            return None
