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

from ..pseudo_cluster import PseudoCluster
from .accessory_server_bridge import AccessoryServerBridge


class SystemCommands(PseudoCluster):
    name = 'SystemCommands'

    async def Start(self, request):
        AccessoryServerBridge.start(request)

    async def Stop(self, request):
        AccessoryServerBridge.stop(request)

    async def Reboot(self, request):
        AccessoryServerBridge.reboot(request)

    async def FactoryReset(self, request):
        AccessoryServerBridge.factoryReset(request)

    async def CreateOtaImage(self, request):
        AccessoryServerBridge.createOtaImage(request)

    async def CompareFiles(self, request):
        AccessoryServerBridge.compareFiles(request)
