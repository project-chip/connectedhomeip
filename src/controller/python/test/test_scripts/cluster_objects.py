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
import chip.interaction_model

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
    async def RoundTripTest(cls, devCtrl):
        req = TestObjects.OnOff.Commands.On()
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if res is not None:
            logger.error(
                f"Got {res} Response from server, but None is expected.")
            raise ValueError()

    @classmethod
    async def RoundTripTestWithBadEndpoint(cls, devCtrl):
        req = TestObjects.OnOff.Commands.On()
        try:
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=233, payload=req)
            raise ValueError(f"Failure expected")
        except chip.interaction_model.InteractionModelError as ex:
            logger.info(f"Recevied {ex} from server.")
            return

    @classmethod
    async def RunTest(cls, devCtrl):
        try:
            cls.TestAPI()
            await cls.RoundTripTest(devCtrl)
            await cls.RoundTripTestWithBadEndpoint(devCtrl)
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
