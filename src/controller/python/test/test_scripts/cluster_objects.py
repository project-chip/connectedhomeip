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


import chip.clusters as Clusters
import logging
from chip.clusters.Attribute import AttributePath, AttributeReadResult, AttributeStatus
import chip.interaction_model

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

NODE_ID = 1
LIGHTING_ENDPOINT_ID = 1


class ClusterObjectTests:
    @classmethod
    def TestAPI(cls):
        if Clusters.OnOff.id != 6:
            raise ValueError()
        if Clusters.OnOff.Commands.Off.command_id != 0:
            raise ValueError()
        if Clusters.OnOff.Commands.Off.cluster_id != 6:
            raise ValueError()
        if Clusters.OnOff.Commands.On.command_id != 1:
            raise ValueError()
        if Clusters.OnOff.Commands.On.cluster_id != 6:
            raise ValueError()

    @classmethod
    async def RoundTripTest(cls, devCtrl):
        req = Clusters.OnOff.Commands.On()
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if res is not None:
            logger.error(
                f"Got {res} Response from server, but None is expected.")
            raise ValueError()

    @classmethod
    async def RoundTripTestWithBadEndpoint(cls, devCtrl):
        req = Clusters.OnOff.Commands.On()
        try:
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=233, payload=req)
            raise ValueError(f"Failure expected")
        except chip.interaction_model.InteractionModelError as ex:
            logger.info(f"Recevied {ex} from server.")
            return

    @classmethod
    async def SendCommandWithResponse(cls, devCtrl):
        req = Clusters.TestCluster.Commands.TestAddArguments(arg1=2, arg2=3)
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if not isinstance(res, Clusters.TestCluster.Commands.TestAddArgumentsResponse):
            logger.error(f"Unexpected response of type {type(res)} received.")
            raise ValueError()
        logger.info(f"Received response: {res}")
        if res.returnValue != 5:
            raise ValueError()

    @classmethod
    async def SendWriteRequest(cls, devCtrl):
        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               Clusters.Attribute.AttributeWriteRequest(
                                                   EndpointId=0, Attribute=Clusters.Basic.Attributes.UserLabel, Data="Test"),
                                               Clusters.Attribute.AttributeWriteRequest(
                                                   EndpointId=0, Attribute=Clusters.Basic.Attributes.Location, Data="A loooong string")
                                           ])
        expectedRes = [
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                            AttributeId=5), Status=chip.interaction_model.Status.Success),
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                            AttributeId=6), Status=chip.interaction_model.Status.InvalidValue)
        ]

        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Read returned unexpected result.")

    @classmethod
    async def SendReadRequest(cls, devCtrl):
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
                                          attributes=[
                                              (0, Clusters.Basic.Attributes.VendorName),
                                              (0, Clusters.Basic.Attributes.VendorID),
                                              (0, Clusters.Basic.Attributes.ProductName),
                                              (0, Clusters.Basic.Attributes.ProductID),
                                              (0, Clusters.Basic.Attributes.UserLabel),
                                              (0, Clusters.Basic.Attributes.Location),
                                              (0, Clusters.Basic.Attributes.HardwareVersion),
                                              (0, Clusters.Basic.Attributes.HardwareVersionString),
                                              (0, Clusters.Basic.Attributes.SoftwareVersion),
                                              (0, Clusters.Basic.Attributes.SoftwareVersionString),
                                          ])
        expectedRes = [
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=1), Status=0, Data='TEST_VENDOR'),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=2), Status=0, Data=9050),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=3), Status=0, Data='TEST_PRODUCT'),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=4), Status=0, Data=65279),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=5), Status=0, Data='Test'),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=6), Status=0, Data=''),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=7), Status=0, Data=0),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=8), Status=0, Data='TEST_VERSION'),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=9), Status=0, Data=0),
            AttributeReadResult(Path=AttributePath(
                EndpointId=0, ClusterId=40, AttributeId=10), Status=0, Data='prerelease')
        ]

        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Read returned unexpected result.")

    @classmethod
    async def RunTest(cls, devCtrl):
        try:
            cls.TestAPI()
            await cls.RoundTripTest(devCtrl)
            await cls.RoundTripTestWithBadEndpoint(devCtrl)
            await cls.SendCommandWithResponse(devCtrl)
            await cls.SendWriteRequest(devCtrl)
            await cls.SendReadRequest(devCtrl)
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
