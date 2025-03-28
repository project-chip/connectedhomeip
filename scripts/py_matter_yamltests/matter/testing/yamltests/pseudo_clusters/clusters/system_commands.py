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

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>SystemCommands</name>
    <code>0xFFF1FD03</code>

    <command source="client" code="0" name="Start">
      <arg name="registerKey" type="char_string" optional="true"/>
      <arg name="discriminator" type="int16u" optional="true"/>
      <arg name="port" type="int16u" optional="true"/>
      <arg name="minCommissioningTimeout" type="int16u" optional="true"/>
      <arg name="kvs" type="char_string" optional="true"/>
      <arg name="filepath" type="char_string" optional="true"/>
      <arg name="otaDownloadPath" type="char_string" optional="true"/>
      <arg name="endUserSupportLogPath" type="char_string" optional="true"/>
      <arg name="networkDiagnosticsLogPath" type="char_string" optional="true"/>
      <arg name="crashLogPath" type="char_string" optional="true"/>
      <arg name="traceDecode" type="int8u" optional="true"/>
    </command>

    <command source="client" code="1" name="Stop">
      <arg name="registerKey" type="char_string" optional="true"/>
    </command>

    <command source="client" code="2" name="Reboot">
      <arg name="registerKey" type="char_string" optional="true"/>
    </command>

    <command source="client" code="3" name="FactoryReset">
      <arg name="registerKey" type="char_string" optional="true"/>
    </command>

    <command source="client" code="4" name="CreateOtaImage">
      <arg name="otaImageFilePath" type="char_string"/>
      <arg name="rawImageFilePath" type="char_string"/>
      <arg name="rawImageContent" type="char_string"/>
    </command>

    <command source="client" code="5" name="CompareFiles">
      <arg name="file1" type="char_string"/>
      <arg name="file2" type="char_string"/>
    </command>

    <command source="client" code="6" name="CreateFile">
      <arg name="filePath" type="char_string"/>
      <arg name="fileContent" type="char_string"/>
    </command>

    <command source="client" code="7" name="DeleteFile">
      <arg name="filePath" type="char_string"/>
    </command>

</cluster>
</configurator>
'''


class SystemCommands(PseudoCluster):
    name = 'SystemCommands'
    definition = _DEFINITION

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

    async def CreateFile(self, request):
        AccessoryServerBridge.createFile(request)

    async def DeleteFile(self, request):
        AccessoryServerBridge.deleteFile(request)
