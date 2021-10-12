#
#    Copyright (c) 2021 Project CHIP Authors
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


from chip.clusters import TestObjects
import logging
from chip.interaction_model import exceptions as IMExceptions

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

NODE_ID = 1
LIGHTING_ENDPOINT_ID = 1


class ClusterObjectTests:
    @classmethod
    def TestAPI(cls):
        if TestObjects.OnOff.id != 6:
            raise ValueError()
        if TestObjects.OnOff.Commands.Off.command_id != 0:
            raise ValueError()
        if TestObjects.OnOff.Commands.Off.cluster_id != 6:
            raise ValueError()
        if TestObjects.OnOff.Commands.On.command_id != 1:
            raise ValueError()
        if TestObjects.OnOff.Commands.On.cluster_id != 6:
            raise ValueError()

    @classmethod
    async def RoundTripTest(cls):
        req = TestObjects.OnOff.Commands.On()
        res = await req.send(nodeId=NODE_ID, endpointId=LIGHTING_ENDPOINT_ID)
        if res is not None:
            logger.error(
                f"Got {res} Response from server, but None is expected.")
            raise ValueError()

    @classmethod
    async def RoundTripTestWithBadEndpoint(cls):
        req = TestObjects.OnOff.Commands.On()
        try:
            await req.send(nodeId=NODE_ID, endpointId=233)
            raise ValueError(f"Failure expected")
        except IMExceptions.InteractionModelError as ex:
            logger.info(f"Recevied {ex} from server.")
            return

    @classmethod
    async def RunTest(cls):
        try:
            cls.TestAPI()
            await cls.RoundTripTest()
            await cls.RoundTripTestWithBadEndpoint()
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
