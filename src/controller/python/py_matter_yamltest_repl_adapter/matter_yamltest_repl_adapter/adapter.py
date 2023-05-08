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

from chip.yaml.runner import ReplTestRunner
from matter_yamltests.adapter import TestAdapter


class Adapter(TestAdapter):
    def __init__(self, specifications):
        self._adapter = ReplTestRunner(specifications, None, None)

    def encode(self, request):
        return self._adapter.encode(request)

    def decode(self, response):
        # TODO We should provide more meaningful logs here, but to adhere to
        # abstract function definition we do need to return list here.
        logs = []
        decoded_response = self._adapter.decode(response)
        if len(decoded_response) == 0:
            decoded_response = [{}]
        return decoded_response, logs
