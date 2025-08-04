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

from abc import ABC, abstractmethod


class TestAdapter(ABC):
    """
    TestAdapter is an abstract interface that defines the set of methods an adapter
    should implement.

    Adapters are used to translate test step requests and test responses back and forth
    between the format used by the matter_yamltests package and the format used by the
    adapter target.

    Some examples of adapters includes chip-repl, chip-tool and the placeholder applications.
    """

    @abstractmethod
    def encode(self, request):
        """Encode a test step request from the matter_yamltests format to the adapter format."""
        pass

    @abstractmethod
    def decode(self, response):
        """
        Decode a test step response from the adapter format to the matter_yamltests format.

        This method returns a tuple containing both the decoded response and additional logs
        from the adapter.
        """
        pass
