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
from chip.clusters.Attribute import AttributePath, AttributeReadResult, AttributeStatus, ValueDecodeFailure
import chip.interaction_model
import asyncio

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

NODE_ID = 1
LIGHTING_ENDPOINT_ID = 1

# Ignore failures decoding these attributes (e.g. not yet implemented)
ignoreAttributeDecodeFailureList = []


def _IgnoreAttributeDecodeFailure(path):
    return str(path) in ignoreAttributeDecodeFailureList


def _AssumeAttributesDecodeSuccess(values):
    for k, v in values['Attributes'].items():
        print(f"{k} = {v}")
        if isinstance(v.Data, ValueDecodeFailure):
            if _IgnoreAttributeDecodeFailure(k):
                print(f"Ignoring attribute decode failure for path {k}")
            else:
                raise AssertionError(
                    f"Cannot decode value for path {k}, got error: '{str(v.Data.Reason)}', raw TLV data: '{v.Data.TLVValue}'")


def _AssumeEventsDecodeSuccess(values):
    print(f"Dump the events: {values} ")


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
    async def SendCommandWithTestClusterEventTrigger(cls, devCtrl, eventNumber):
        req = Clusters.TestCluster.Commands.TestEmitTestEventRequest(arg1=1)
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if not isinstance(res, Clusters.TestCluster.Commands.TestEmitTestEventResponse):
            logger.error(f"Unexpected response of type {type(res)} received.")
            raise ValueError()
        logger.info(f"Received response: {res}")
        if res.value != eventNumber:
            raise ValueError()

    @classmethod
    async def SendWriteRequest(cls, devCtrl):
        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               (0, Clusters.Basic.Attributes.NodeLabel(
                                                   "Test")),
                                               (0, Clusters.Basic.Attributes.Location(
                                                   "A loooong string"))
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
    async def TestSubscribeAttribute(cls, devCtrl):
        logger.info("Test Subscription")
        sub = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.OnOff.Attributes.OnOff)], reportInterval=(3, 10))
        updated = False

        def subUpdate(path, value):
            nonlocal updated
            logger.info(
                f"Received attribute update path {path}, New value {value}")
            updated = True
        sub.SetAttributeUpdateCallback(subUpdate)
        req = Clusters.OnOff.Commands.On()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)
        await asyncio.sleep(5)
        req = Clusters.OnOff.Commands.Off()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)
        await asyncio.sleep(5)

        if not updated:
            raise AssertionError("Did not receive updated attribute")

    @classmethod
    async def TestReadAttributeRequests(cls, devCtrl):
        '''
        Tests out various permutations of endpoint, cluster and attribute ID (with wildcards) to validate
        reads.

        With the use of cluster objects, the actual received data is validated against the data model description
        for those values, so no extra validation has to be done here in this test for the values themselves.
        '''

        logger.info("1: Reading Ex Cx Ax")
        req = [
            (0, Clusters.Basic.Attributes.VendorName),
            (0, Clusters.Basic.Attributes.ProductID),
            (0, Clusters.Basic.Attributes.HardwareVersion),
        ]
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req)
        if (len(res['Attributes']) != 3):
            raise AssertionError(
                f"Got back {len(res['Attributes'])} data items instead of 3")
        _AssumeAttributesDecodeSuccess(res)

        logger.info("2: Reading Ex Cx A*")
        req = [
            (0, Clusters.Basic),
        ]
        _AssumeAttributesDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("3: Reading E* Cx Ax")
        req = [
            Clusters.Descriptor.Attributes.ServerList
        ]
        _AssumeAttributesDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("4: Reading Ex C* A*")
        req = [
            0
        ]
        _AssumeAttributesDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("5: Reading E* Cx A*")
        req = [
            Clusters.Descriptor
        ]
        _AssumeAttributesDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("6: Reading E* C* A*")
        req = [
            '*'
        ]
        _AssumeAttributesDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("7: Reading Chunked List")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.TestCluster.Attributes.ListLongOctetString)])
        if res.get('Attributes', {}).get(AttributePath(EndpointId=1, Attribute=Clusters.TestCluster.Attributes.ListLongOctetString)).Data.value != [b'0123456789abcdef' * 32] * 4:
            raise AssertionError("Unexpected read result")

    @classmethod
    async def TestReadEventRequests(cls, devCtrl, expectEventsNum):
        logger.info("1: Reading Ex Cx Ex")
        req = [
            (1, Clusters.TestCluster.Events.TestEvent),
        ]
        res = await devCtrl.ReadEvent(nodeid=NODE_ID, events=req)
        if (len(res['Events']) != expectEventsNum):
            raise AssertionError(
                f"Got back {len(res['Events'])} event items instead of {expectEventsNum}")
        _AssumeEventsDecodeSuccess(res)
        logger.info("2: Reading Ex Cx E*")
        req = [
            (1, Clusters.TestCluster),
        ]

        res = await devCtrl.ReadEvent(nodeid=NODE_ID, events=req)
        if (len(res['Events']) != 0):
            raise AssertionError(
                f"Got back {len(res['Events'])} event items instead of 0")
        _AssumeEventsDecodeSuccess(res)

        logger.info("3: Reading Ex C* E*")
        req = [
            1
        ]
        if (len(res['Events']) != 0):
            raise AssertionError(
                f"Got back {len(res['Events'])} event items instead of 0")
        _AssumeEventsDecodeSuccess(await devCtrl.ReadEvent(nodeid=NODE_ID, events=req))

        logger.info("4: Reading E* C* E*")
        req = [
            '*'
        ]
        if (len(res['Events']) != 0):
            raise AssertionError(
                f"Got back {len(res['Events'])} event items instead of 0")
        _AssumeEventsDecodeSuccess(await devCtrl.ReadEvent(nodeid=NODE_ID, events=req))

        # TODO: Add more wildcard test for IM events.

    @classmethod
    async def RunTest(cls, devCtrl):
        try:
            cls.TestAPI()
            await cls.RoundTripTest(devCtrl)
            await cls.RoundTripTestWithBadEndpoint(devCtrl)
            await cls.SendCommandWithResponse(devCtrl)
            await cls.SendCommandWithTestClusterEventTrigger(devCtrl, 1)
            await cls.TestReadEventRequests(devCtrl, 1)
            await cls.SendCommandWithTestClusterEventTrigger(devCtrl, 2)
            await cls.TestReadEventRequests(devCtrl, 2)
            await cls.SendWriteRequest(devCtrl)
            await cls.TestReadAttributeRequests(devCtrl)
            await cls.TestSubscribeAttribute(devCtrl)
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
