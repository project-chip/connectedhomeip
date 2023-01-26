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

import sys
import time

from ..pseudo_cluster import PseudoCluster


class DelayCommands(PseudoCluster):
    name = 'DelayCommands'

    async def WaitForMs(self, request):
        duration_in_ms = 0

        for argument in request.arguments['values']:
            if argument['name'] == 'ms':
                duration_in_ms = argument['value']

        sys.stdout.flush()
        time.sleep(duration_in_ms / 1000)
