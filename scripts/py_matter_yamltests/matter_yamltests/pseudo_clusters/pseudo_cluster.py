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

from abc import ABC, abstractproperty


class PseudoCluster(ABC):
    """
    PseudoCluster is an abstract interface that custom pseudo clusters
    should inherit from.

    The interface expose a name property that is used while looking
    where to dispatch the test step to.

    The implementation should then expose methods where the name match
    the name used in the YAML test file.

    For example, the 'CustomCommands' pseudo cluster can be implemented as:

    class CustomCommand(PseudoCluster):
        name = 'CustomCommands'

        async def MyCustomMethod(self, request):
            pass

        async def MyCustomMethod(self, request):
            pass

    It can then be called from any test step as:

    - label: "Call a custom method"
      cluster: "CustomCommands"
      command: "MyCustomMethod"
      arguments:
          values:
              - name: "MyCustomParameter"
                value: "this_is_a_custom_value"
    """

    @abstractproperty
    def name(self):
        pass
