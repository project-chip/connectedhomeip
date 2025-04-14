#
#    Copyright (c) 2025 Project CHIP Authors
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

from loguru import logger

from ..pseudo_cluster import PseudoCluster

_DEFINITION = '''<?xml version="1.0"?>
<configurator>
<cluster>
    <name>WebRTC</name>
    <code>0xFFF1FD20</code>

    <command source="client" code="0" name="UserPromptSdp" response="UserPromptSdpResponse">
      <arg name="promptMessage" type="char_string"/>
    </command>

    <command source="client" code="1" name="VerifyVideoStream">
    </command>

    <command source="server" code="1" name="UserPromptSdpResponse">
      <arg name="offerSdp" type="char_string"/>
    </command>
</cluster>
</configurator>
'''


def _get_arg_value(request, item_name: str, default_value="Please enter SDP from TH Logs"):
    logger.info("LOGGER")
    if request.arguments:
        values = request.arguments['values']
        for item in values:
            name = item['name']
            value = item['value']
            if name == item_name:
                return value

    return default_value


class WebRTC(PseudoCluster):
    name = 'WebRTC'
    definition = _DEFINITION

    async def UserPromptSdp(self, request):
        pass

    async def VerifyVideoStream(self, request):
        pass
