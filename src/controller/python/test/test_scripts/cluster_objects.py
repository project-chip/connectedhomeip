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
import chip.exceptions
import logging
from chip.clusters.Attribute import AttributePath, AttributeReadResult, AttributeStatus, ValueDecodeFailure, TypedAttributePath, SubscriptionTransaction
import chip.interaction_model
import asyncio
import time

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

NODE_ID = 1
LIGHTING_ENDPOINT_ID = 1

# Ignore failures decoding these attributes (e.g. not yet implemented)
ignoreAttributeDecodeFailureList = []


def _IgnoreAttributeDecodeFailure(path):
    return path in ignoreAttributeDecodeFailureList


def VerifyDecodeSuccess(values):
    for endpoint in values:
        for cluster in values[endpoint]:
            for attribute in values[endpoint][cluster]:
                v = values[endpoint][cluster][attribute]
                print(f"EP{endpoint}/{attribute} = {v}")
                if (isinstance(v, ValueDecodeFailure)):
                    if _IgnoreAttributeDecodeFailure((endpoint, cluster, attribute)):
                        print(
                            f"Ignoring attribute decode failure for path {endpoint}/{attribute}")
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
    async def SendWriteRequest(cls, devCtrl):
        logger.info("1: Trivial writes (multiple attributes)")
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

        logger.info(f"Received WriteResponse: {res}")
        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Write returned unexpected result.")

        logger.info("2: Write chunked list")
        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[(1, Clusters.TestCluster.Attributes.ListLongOctetString([b"0123456789abcdef" * 32] * 5))])
        expectedRes = [
            AttributeStatus(Path=AttributePath(
                EndpointId=1, Attribute=Clusters.TestCluster.Attributes.ListLongOctetString), Status=chip.interaction_model.Status.Success),
        ]

        logger.info(f"Received WriteResponse: {res}")
        if res != expectedRes:
            logger.error(f"Expect {expectedRes} got {res}")
            raise AssertionError("Write returned unexpected result.")

    @classmethod
    async def TestSubscribeAttribute(cls, devCtrl):
        logger.info("Test Subscription")
        sub = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.OnOff.Attributes.OnOff)], reportInterval=(3, 10))
        updated = False

        def subUpdate(path: TypedAttributePath, transaction: SubscriptionTransaction):
            nonlocal updated
            value = transaction.GetAttribute(path)
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

        sub.Shutdown()

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
        if ((0 not in res) or (Clusters.Basic not in res[0]) or (len(res[0][Clusters.Basic]) != 3)):
            raise AssertionError(
                f"Got back {len(res)} data items instead of 3")
        VerifyDecodeSuccess(res)

        logger.info("2: Reading Ex Cx A*")
        req = [
            (0, Clusters.Basic),
        ]
        VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("3: Reading E* Cx Ax")
        req = [
            Clusters.Descriptor.Attributes.ServerList
        ]
        VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("4: Reading Ex C* A*")
        req = [
            0
        ]
        VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        logger.info("5: Reading E* Cx A*")
        req = [
            Clusters.Descriptor
        ]
        VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        # TODO: #13750 Reading OperationalCredentials::FabricLists attribute may crash the server, skip this test temporarily.
        # logger.info("6: Reading E* C* A*")
        # req = [
        #     '*'
        # ]
        # VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        # res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req, returnClusterObject=True)
        # logger.info(
        #     f"Basic Cluster - Label: {res[0][Clusters.Basic].productLabel}")
        # logger.info(
        #     f"Test Cluster - Struct: {res[1][Clusters.TestCluster].structAttr}")
        # logger.info(f"Test Cluster: {res[1][Clusters.TestCluster]}")

        logger.info("7: Reading Chunked List")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.TestCluster.Attributes.ListLongOctetString)])
        if res[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListLongOctetString] != [b'0123456789abcdef' * 32] * 4:
            raise AssertionError("Unexpected read result")

        logger.info("*: Getting current fabric index")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        fabricIndex = res[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]

        logger.info("8: Read without fabric filter")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.TestCluster.Attributes.ListFabricScoped)], fabricFiltered=False)
        if len(res[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]) <= 1:
            raise AssertionError("Expect more elements in the response")

        logger.info("9: Read with fabric filter")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.TestCluster.Attributes.ListFabricScoped)], fabricFiltered=True)
        if len(res[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped]) != 1:
            raise AssertionError("Expect exact one element in the response")
        if res[1][Clusters.TestCluster][Clusters.TestCluster.Attributes.ListFabricScoped][0].fabricIndex != fabricIndex:
            raise AssertionError(
                "Expect the fabric index matches the one current reading")

    async def TriggerAndWaitForEvents(cls, devCtrl, req):
        # We trigger sending an event a couple of times just to be safe.
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=Clusters.TestCluster.Commands.TestEmitTestEventRequest())
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=Clusters.TestCluster.Commands.TestEmitTestEventRequest())
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=Clusters.TestCluster.Commands.TestEmitTestEventRequest())
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=Clusters.TestCluster.Commands.TestEmitTestFabricScopedEventRequest(arg1=0))
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=Clusters.TestCluster.Commands.TestEmitTestFabricScopedEventRequest(arg1=1))
        # Events may take some time to flush, so wait for about 10s or so to get some events.
        for i in range(0, 10):
            print("Reading out events..")
            res = await devCtrl.ReadEvent(nodeid=NODE_ID, events=req)
            if (len(res) != 0):
                break

            time.sleep(1)

        if (len(res) == 0):
            raise AssertionError("Got no events back")

    @classmethod
    async def TestReadEventRequests(cls, devCtrl, expectEventsNum):
        logger.info("1: Reading Ex Cx Ex")
        req = [
            (1, Clusters.TestCluster.Events.TestEvent),
        ]

        await cls.TriggerAndWaitForEvents(cls, devCtrl, req)

        logger.info("2: Reading Ex Cx E*")
        req = [
            (1, Clusters.TestCluster),
        ]

        await cls.TriggerAndWaitForEvents(cls, devCtrl, req)

        logger.info("3: Reading Ex C* E*")
        req = [
            1
        ]

        await cls.TriggerAndWaitForEvents(cls, devCtrl, req)

        logger.info("4: Reading E* C* E*")
        req = [
            '*'
        ]

        await cls.TriggerAndWaitForEvents(cls, devCtrl, req)

        # TODO: Add more wildcard test for IM events.

    @classmethod
    async def TestTimedRequest(cls, devCtrl):
        logger.info("1: Send Timed Command Request")
        req = Clusters.TestCluster.Commands.TimedInvokeRequest()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req, timedRequestTimeoutMs=1000)

        logger.info("2: Send Timed Write Request")
        await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                     attributes=[
                                         (1, Clusters.TestCluster.Attributes.TimedWriteBoolean(
                                             True)),
                                     ],
                                     timedRequestTimeoutMs=1000)

        logger.info("3: Send Timed Command Request -- Timeout")
        try:
            req = Clusters.TestCluster.Commands.TimedInvokeRequest()
            # 10ms is a pretty short timeout, RTT is 400ms in simulated network on CI, so this test should fail.
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req, timedRequestTimeoutMs=10)
            raise AssertionError("Timeout expected!")
        except chip.exceptions.ChipStackException:
            pass

        logger.info("4: Send Timed Write Request -- Timeout")
        try:
            await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                         attributes=[
                                             (1, Clusters.TestCluster.Attributes.TimedWriteBoolean(
                                                 True)),
                                         ],
                                         timedRequestTimeoutMs=10)
            raise AssertionError("Timeout expected!")
        except chip.exceptions.ChipStackException:
            pass

        logger.info(
            "5: Sending TestCluster-TimedInvokeRequest without timedRequestTimeoutMs should be rejected")
        try:
            req = Clusters.TestCluster.Commands.TimedInvokeRequest()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)
            raise AssertionError("The command invoke should be rejected.")
        except ValueError:
            pass

        logger.info(
            "6: Writing TestCluster-TimedWriteBoolean without timedRequestTimeoutMs should be rejected")
        try:
            await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                         attributes=[
                                             (1, Clusters.TestCluster.Attributes.TimedWriteBoolean(
                                                 True)),
                                         ])
            raise AssertionError("The write request should be rejected.")
        except ValueError:
            pass

    @classmethod
    async def RunTest(cls, devCtrl):
        try:
            cls.TestAPI()
            await cls.RoundTripTest(devCtrl)
            await cls.RoundTripTestWithBadEndpoint(devCtrl)
            await cls.SendCommandWithResponse(devCtrl)
            await cls.TestReadEventRequests(devCtrl, 1)
            await cls.TestReadAttributeRequests(devCtrl)
            await cls.TestSubscribeAttribute(devCtrl)
            # Note: Write will change some attribute values, always put it after read tests
            await cls.SendWriteRequest(devCtrl)
            await cls.TestTimedRequest(devCtrl)
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
