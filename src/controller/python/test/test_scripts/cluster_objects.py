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

import asyncio
import logging
import pprint

import base
import chip.clusters as Clusters
import chip.exceptions
import chip.interaction_model
from chip.clusters.Attribute import (AttributePath, AttributeStatus, DataVersion, SubscriptionTransaction, TypedAttributePath,
                                     ValueDecodeFailure)

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)

NODE_ID = 1
LIGHTING_ENDPOINT_ID = 1

# Ignore failures decoding these attributes (e.g. not yet implemented)
ignoreAttributeDecodeFailureList = [
    (1, Clusters.Objects.UnitTesting,
     Clusters.Objects.UnitTesting.Attributes.GeneralErrorBoolean),
    (1, Clusters.Objects.UnitTesting,
     Clusters.Objects.UnitTesting.Attributes.ClusterErrorBoolean),
    (1, Clusters.Objects.UnitTesting,
     Clusters.Objects.UnitTesting.Attributes.FailureInt32U),
]


def _IgnoreAttributeDecodeFailure(path):
    return path in ignoreAttributeDecodeFailureList


def VerifyDecodeSuccess(values):
    pprint.pprint(values)
    for endpoint in values:
        for cluster in values[endpoint]:
            for attribute in values[endpoint][cluster]:
                v = values[endpoint][cluster][attribute]
                if (isinstance(v, ValueDecodeFailure)):
                    if _IgnoreAttributeDecodeFailure((endpoint, cluster, attribute)):
                        print(
                            f"Ignoring attribute decode failure for path {endpoint}/{attribute}")
                    else:
                        raise AssertionError(
                            f"Cannot decode value for path {endpoint}/{attribute}, "
                            f"got error: '{str(v.Reason)}', raw TLV data: '{v.TLVValue}'")

    for endpoint in values:
        for cluster in values[endpoint]:
            v = values[endpoint][cluster].get(DataVersion, None)
            print(f"EP{endpoint}/{cluster} version = {v}")
            if v is None:
                raise AssertionError(
                    f"Cannot get data version for path {endpoint}/{cluster}")


def _AssumeEventsDecodeSuccess(values):
    print(f"Dump the events: {values} ")


@base.test_set
class ClusterObjectTests:
    @classmethod
    @base.test_case
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
    @base.test_case
    async def TestCommandRoundTrip(cls, devCtrl):
        req = Clusters.OnOff.Commands.On()
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if res is not None:
            logger.error(
                f"Got {res} Response from server, but None is expected.")
            raise ValueError()

    @classmethod
    @base.test_case
    async def TestCommandRoundTripWithBadEndpoint(cls, devCtrl):
        req = Clusters.OnOff.Commands.On()
        try:
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=233, payload=req)
            raise ValueError("Failure expected")
        except chip.interaction_model.InteractionModelError as ex:
            logger.info(f"Recevied {ex} from server.")
            return

    @classmethod
    @base.test_case
    async def TestCommandWithResponse(cls, devCtrl):
        req = Clusters.UnitTesting.Commands.TestAddArguments(arg1=2, arg2=3)
        res = await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=LIGHTING_ENDPOINT_ID, payload=req)
        if not isinstance(res, Clusters.UnitTesting.Commands.TestAddArgumentsResponse):
            logger.error(f"Unexpected response of type {type(res)} received.")
            raise ValueError()
        logger.info(f"Received response: {res}")
        if res.returnValue != 5:
            raise ValueError()

    @classmethod
    @base.test_case
    async def TestReadWriteOnlyAttribute(cls, devCtrl):
        logger.info("Test wildcard read of attributes containing write-only attribute")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(Clusters.UnitTesting)])
        if Clusters.UnitTesting.Attributes.WriteOnlyInt8u in res[1][Clusters.UnitTesting]:
            raise AssertionError("Received un-expected WriteOnlyInt8u attribute in TestCluster")

    @ classmethod
    @ base.test_case
    async def TestWriteRequest(cls, devCtrl):
        logger.info("1: Trivial writes (multiple attributes)")
        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               (0, Clusters.BasicInformation.Attributes.NodeLabel(
                                                   "Test")),
                                               (0, Clusters.BasicInformation.Attributes.Location(
                                                   "A loooong string"))
                                           ])
        expectedRes = [
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                                               AttributeId=5), Status=chip.interaction_model.Status.Success),
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                                               AttributeId=6), Status=chip.interaction_model.Status.ConstraintError)
        ]

        logger.info(f"Received WriteResponse: {res}")
        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Write returned unexpected result.")

        logger.info("2: Write chunked list")
        res = await devCtrl.WriteAttribute(
            nodeid=NODE_ID,
            attributes=[
                (1, Clusters.UnitTesting.Attributes.ListLongOctetString([b"0123456789abcdef" * 32] * 5))
            ]
        )
        expectedRes = [
            AttributeStatus(Path=AttributePath.from_attribute(
                EndpointId=1,
                Attribute=Clusters.UnitTesting.Attributes.ListLongOctetString), Status=chip.interaction_model.Status.Success),
        ]

        logger.info(f"Received WriteResponse: {res}")
        if res != expectedRes:
            logger.error(f"Expect {expectedRes} got {res}")
            raise AssertionError("Write returned unexpected result.")

    @ classmethod
    @ base.test_case
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

    @ classmethod
    @ base.test_case
    async def TestAttributeCacheAttributeView(cls, devCtrl):
        logger.info("Test AttributeCache Attribute-View")
        sub: SubscriptionTransaction = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.OnOff.Attributes.OnOff)], returnClusterObject=False, reportInterval=(3, 10))

        event = asyncio.Event()

        def subUpdate(path: TypedAttributePath, transaction: SubscriptionTransaction):
            event.set()

        sub.SetAttributeUpdateCallback(subUpdate)

        try:
            req = Clusters.OnOff.Commands.On()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

            await asyncio.wait_for(event.wait(), timeout=11)

            data = sub.GetAttributes()
            if (data[1][Clusters.OnOff][Clusters.OnOff.Attributes.OnOff] != 1):
                raise ValueError("Current On/Off state should be 1")

            event.clear()

            req = Clusters.OnOff.Commands.Off()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

            await asyncio.wait_for(event.wait(), timeout=11)

            data = sub.GetAttributes()
            if (data[1][Clusters.OnOff][Clusters.OnOff.Attributes.OnOff] != 0):
                raise ValueError("Current On/Off state should be 0")

        except TimeoutError:
            raise AssertionError("Did not receive updated attribute")
        finally:
            sub.Shutdown()

    @ classmethod
    @ base.test_case
    async def TestAttributeCacheClusterView(cls, devCtrl):
        logger.info("Test AttributeCache Cluster-View")
        sub: SubscriptionTransaction = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.OnOff.Attributes.OnOff)], returnClusterObject=True, reportInterval=(3, 10))

        event = asyncio.Event()

        def subUpdate(path: TypedAttributePath, transaction: SubscriptionTransaction):
            event.set()

        sub.SetAttributeUpdateCallback(subUpdate)

        try:
            req = Clusters.OnOff.Commands.On()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

            await asyncio.wait_for(event.wait(), timeout=11)

            data = sub.GetAttributes()
            cluster: Clusters.OnOff = data[1][Clusters.OnOff]
            if (not cluster.onOff):
                raise ValueError("Current On/Off state should be True")

            event.clear()

            req = Clusters.OnOff.Commands.Off()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

            await asyncio.wait_for(event.wait(), timeout=11)

            data = sub.GetAttributes()
            cluster: Clusters.OnOff = data[1][Clusters.OnOff]
            if (cluster.onOff):
                raise ValueError("Current On/Off state should be False")

        except TimeoutError:
            raise AssertionError("Did not receive updated attribute")
        finally:
            sub.Shutdown()

    @ classmethod
    @ base.test_case
    async def TestSubscribeZeroMinInterval(cls, devCtrl):
        '''
        This validates receiving subscription reports for two attributes at a time in quick succession after
        issuing a command that results in attribute side-effects. Specifically, it relies on the fact that the second attribute
        is changed in a different execution context than the first. This ensures that we pick-up the first
        attribute change and generate a notification, and validating that shortly after that,
        we generate a second report for the second change.

        This is done using subscriptions with a min reporting interval of 0 to ensure timely notification of the above.
        An On() command is sent to the OnOff cluster
        which should simultaneously set the state to On as well as set the level to 254.
        '''
        logger.info("Test Subscription With MinInterval of 0")
        sub = await devCtrl.ReadAttribute(nodeid=NODE_ID,
                                          attributes=[Clusters.OnOff, Clusters.LevelControl], reportInterval=(0, 60))

        logger.info("Sending off command")

        req = Clusters.OnOff.Commands.Off()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

        logger.info("Sending on command")

        req = Clusters.OnOff.Commands.On()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)

        # Wait for the report containing both attributes to arrive to us.
        await asyncio.sleep(2)

        logger.info("Checking read back value is indeed 254")

        data = sub.GetAttributes()
        if (data[1][Clusters.LevelControl][Clusters.LevelControl.Attributes.CurrentLevel] != 254):
            raise ValueError("Current Level should have been 254")

        sub.Shutdown()

    @ classmethod
    @ base.test_case
    async def TestReadAttributeRequests(cls, devCtrl):
        '''
        Tests out various permutations of endpoint, cluster and attribute ID (with wildcards) to validate
        reads.

        With the use of cluster objects, the actual received data is validated against the data model description
        for those values, so no extra validation has to be done here in this test for the values themselves.
        '''

        logger.info("1: Reading Ex Cx Ax")
        req = [
            (0, Clusters.BasicInformation.Attributes.VendorName),
            (0, Clusters.BasicInformation.Attributes.ProductID),
            (0, Clusters.BasicInformation.Attributes.HardwareVersion),
        ]
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req)
        if ((0 not in res) or (Clusters.BasicInformation not in res[0]) or (len(res[0][Clusters.BasicInformation]) != 4)):
            # 3 attribute data + DataVersion
            raise AssertionError(
                f"Got back {len(res)} data items instead of 3")
        VerifyDecodeSuccess(res)

        logger.info("2: Reading Ex Cx A*")
        req = [
            (0, Clusters.BasicInformation),
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

        logger.info("6: Reading E* C* A*")
        req = [
            '*'
        ]
        VerifyDecodeSuccess(await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req))

        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req, returnClusterObject=True)
        logger.info(
            f"Basic Cluster - Label: {res[0][Clusters.BasicInformation].productLabel}")
        # TestCluster will be ValueDecodeError here, so we comment out the log below.
        # Values are not expected to be ValueDecodeError for real clusters.
        # logger.info(
        #    f"Test Cluster - Struct: {res[1][Clusters.UnitTesting].structAttr}")
        logger.info(f"Test Cluster: {res[1][Clusters.UnitTesting]}")

        logger.info("7: Reading Chunked List")
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=[(1, Clusters.UnitTesting.Attributes.ListLongOctetString)])
        if res[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListLongOctetString] != [b'0123456789abcdef' * 32] * 4:
            raise AssertionError("Unexpected read result")

        # Note: ListFabricScoped is an empty list for now. We should re-enable this test after we make it return expected data.
        # logger.info("*: Getting current fabric index")
        # res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
        #                                   attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        # fabricIndex = res[0][Clusters.OperationalCredentials][Clusters.OperationalCredentials.Attributes.CurrentFabricIndex]
        #
        # logger.info("8: Read without fabric filter")
        # res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
        #                                   attributes=[(1, Clusters.UnitTesting.Attributes.ListFabricScoped)],
        #                                                fabricFiltered=False)
        # if len(res[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]) == 1:
        #     raise AssertionError("Expect more elements in the response")
        # logger.info("9: Read with fabric filter")
        # res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
        #                                   attributes=[(1, Clusters.UnitTesting.Attributes.ListFabricScoped)], fabricFiltered=True)
        # if len(res[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped]) != 1:
        #     raise AssertionError("Expect exact one element in the response")
        # if res[1][Clusters.UnitTesting][Clusters.UnitTesting.Attributes.ListFabricScoped][0].fabricIndex != fabricIndex:
        #     raise AssertionError(
        #         "Expect the fabric index matches the one current reading")

    @ classmethod
    async def _TriggerEvent(cls, devCtrl):
        # We trigger sending an event a couple of times just to be safe.
        await devCtrl.SendCommand(nodeid=NODE_ID,
                                  endpoint=1, payload=Clusters.UnitTesting.Commands.TestEmitTestEventRequest())
        await devCtrl.SendCommand(nodeid=NODE_ID,
                                  endpoint=1, payload=Clusters.UnitTesting.Commands.TestEmitTestEventRequest())
        return await devCtrl.SendCommand(nodeid=NODE_ID,
                                         endpoint=1, payload=Clusters.UnitTesting.Commands.TestEmitTestEventRequest())

    @ classmethod
    async def _RetryForContent(cls, request, until, retryCount=10, intervalSeconds=1):
        for i in range(retryCount):
            logger.info(f"Attempt {i + 1}/{retryCount}")
            res = await request()
            if until(res):
                return res
            asyncio.sleep(1)
        raise AssertionError("condition is not met")

    @ classmethod
    async def TriggerAndWaitForEvents(cls, devCtrl, req):
        await cls._TriggerEvent(devCtrl)
        await cls._RetryForContent(request=lambda: devCtrl.ReadEvent(nodeid=NODE_ID, events=req), until=lambda res: res != 0)

    @ classmethod
    async def TriggerAndWaitForEventsWithFilter(cls, devCtrl, req):
        response = await cls._TriggerEvent(devCtrl)
        current_event_filter = response.value

        def validate_got_expected_event(events):
            number_of_events = len(events)
            if number_of_events != 1:
                return False

            parsed_event_number = events[0].Header.EventNumber
            if parsed_event_number != current_event_filter:
                return False
            return True

        await cls._RetryForContent(request=lambda: devCtrl.ReadEvent(
            nodeid=NODE_ID,
            events=req,
            eventNumberFilter=current_event_filter
        ), until=validate_got_expected_event)

        def validate_got_no_event(events):
            return len(events) == 0

        await cls._RetryForContent(request=lambda: devCtrl.ReadEvent(
            nodeid=NODE_ID,
            events=req,
            eventNumberFilter=(current_event_filter + 1)
        ), until=validate_got_no_event)

    @ classmethod
    @ base.test_case
    async def TestGenerateUndefinedFabricScopedEventRequests(cls, devCtrl):
        logger.info("Running TestGenerateUndefinedFabricScopedEventRequests")
        try:
            res = await devCtrl.SendCommand(nodeid=NODE_ID,
                                            endpoint=1,
                                            payload=Clusters.UnitTesting.Commands.TestEmitTestFabricScopedEventRequest(arg1=0))
            raise ValueError("Unexpected Failure")
        except chip.interaction_model.InteractionModelError as ex:
            logger.info(f"Recevied {ex} from server.")
        res = await devCtrl.ReadEvent(nodeid=NODE_ID, events=[
            (1, Clusters.UnitTesting.Events.TestFabricScopedEvent, 0),
        ])
        logger.info(f"return result is {res}")
        if len(res) != 0:
            raise AssertionError("failure: not expect to receive fabric-scoped event when fabric is undefined")
        else:
            logger.info("TestGenerateUndefinedFabricScopedEventRequests: Success")

    @ classmethod
    @ base.test_case
    async def TestReadEventRequests(cls, devCtrl, expectEventsNum):
        logger.info("1: Reading Ex Cx Ex")
        req = [
            (1, Clusters.UnitTesting.Events.TestEvent, 0),
        ]

        await cls.TriggerAndWaitForEvents(devCtrl, req)

        logger.info("2: Reading Ex Cx E*")
        req = [
            (1, Clusters.UnitTesting, 0),
        ]

        await cls.TriggerAndWaitForEvents(devCtrl, req)

        logger.info("3: Reading Ex C* E*")
        req = [
            1
        ]

        await cls.TriggerAndWaitForEvents(devCtrl, req)

        logger.info("4: Reading E* C* E*")
        req = [
            '*'
        ]

        await cls.TriggerAndWaitForEvents(devCtrl, req)

        logger.info("5: Reading Ex Cx E* Urgency")
        req = [
            (1, Clusters.UnitTesting, 1),
        ]

        await cls.TriggerAndWaitForEvents(devCtrl, req)

        logger.info("6: Reading Ex Cx Ex, with filter")
        req = [
            (1, Clusters.UnitTesting.Events.TestEvent, 0),
        ]
        await cls.TriggerAndWaitForEventsWithFilter(devCtrl, req)

        # TODO: Add more wildcard test for IM events.

    @ classmethod
    @ base.test_case
    async def TestTimedRequest(cls, devCtrl):
        logger.info("1: Send Timed Command Request")
        req = Clusters.UnitTesting.Commands.TimedInvokeRequest()
        await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req, timedRequestTimeoutMs=1000)

        logger.info("2: Send Timed Write Request")
        await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                     attributes=[
                                         (1, Clusters.UnitTesting.Attributes.TimedWriteBoolean(
                                             True)),
                                     ],
                                     timedRequestTimeoutMs=1000)

        logger.info(
            "3: Sending TestCluster-TimedInvokeRequest without timedRequestTimeoutMs should be rejected")
        try:
            req = Clusters.UnitTesting.Commands.TimedInvokeRequest()
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req)
            raise AssertionError("The command invoke should be rejected.")
        except chip.interaction_model.InteractionModelError as ex:
            if ex.status != chip.interaction_model.Status.NeedsTimedInteraction:
                raise AssertionError("The command invoke was expected to error with NeedsTimedInteraction.")

        logger.info(
            "4: Writing TestCluster-TimedWriteBoolean without timedRequestTimeoutMs should be rejected")
        try:
            await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                         attributes=[
                                             (1, Clusters.UnitTesting.Attributes.TimedWriteBoolean(
                                                 True)),
                                         ])
            raise AssertionError("The write request should be rejected.")
        except chip.interaction_model.InteractionModelError as ex:
            if ex.status != chip.interaction_model.Status.NeedsTimedInteraction:
                raise AssertionError("The write attribute was expected to error with NeedsTimedInteraction.")

    @ classmethod
    @ base.test_case
    async def TestTimedRequestTimeout(cls, devCtrl):
        logger.info("1: Send Timed Command Request -- Timeout")
        try:
            req = Clusters.UnitTesting.Commands.TimedInvokeRequest()
            # 10ms is a pretty short timeout, RTT is 400ms in simulated network on CI, so this test should fail.
            await devCtrl.SendCommand(nodeid=NODE_ID, endpoint=1, payload=req, timedRequestTimeoutMs=1)
            raise AssertionError("Timeout expected!")
        except chip.exceptions.ChipStackException:
            pass

        logger.info("2: Send Timed Write Request -- Timeout")
        try:
            await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                         attributes=[
                                             (1, Clusters.UnitTesting.Attributes.TimedWriteBoolean(
                                                 True)),
                                         ],
                                         timedRequestTimeoutMs=1)
            raise AssertionError("Timeout expected!")
        except chip.exceptions.ChipStackException:
            pass

    @ classmethod
    @ base.test_case
    async def TestReadWriteAttributeRequestsWithVersion(cls, devCtrl):
        logger.info("TestReadWriteAttributeRequestsWithVersion")
        req = [
            (0, Clusters.BasicInformation.Attributes.VendorName)
        ]
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID, attributes=req)
        VerifyDecodeSuccess(res)
        data_version = res[0][Clusters.BasicInformation][DataVersion]

        logger.info(res)
        logger.info(data_version)

        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               (0, Clusters.BasicInformation.Attributes.NodeLabel(
                                                   "Test"))
                                           ])
        expectedRes = [
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                                               AttributeId=5), Status=chip.interaction_model.Status.Success),
        ]

        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Write returned unexpected result.")

        req = [
            (0, Clusters.BasicInformation.Attributes.VendorName),
        ]
        res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
                                          attributes=req, dataVersionFilters=[(0, Clusters.BasicInformation, data_version)])
        VerifyDecodeSuccess(res)
        new_data_version = res[0][Clusters.BasicInformation][DataVersion]
        if (data_version + 1) != new_data_version:
            raise AssertionError("Version mistmatch happens.")

        res = await devCtrl.ReadAttribute(nodeid=NODE_ID,
                                          attributes=req, dataVersionFilters=[(0, Clusters.BasicInformation, new_data_version)])
        VerifyDecodeSuccess(res)

        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               (0, Clusters.BasicInformation.Attributes.NodeLabel(
                                                   "Test"), new_data_version)
                                           ])

        expectedRes = [
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                                               AttributeId=5), Status=chip.interaction_model.Status.Success),
        ]

        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Write returned unexpected result.")

        res = await devCtrl.WriteAttribute(nodeid=NODE_ID,
                                           attributes=[
                                               (0, Clusters.BasicInformation.Attributes.NodeLabel(
                                                   "Test"), new_data_version)
                                           ])

        expectedRes = [
            AttributeStatus(Path=AttributePath(EndpointId=0, ClusterId=40,
                                               AttributeId=5), Status=chip.interaction_model.Status.DataVersionMismatch),
        ]

        if res != expectedRes:
            for i in range(len(res)):
                if res[i] != expectedRes[i]:
                    logger.error(
                        f"Item {i} is not expected, expect {expectedRes[i]} got {res[i]}")
            raise AssertionError("Write returned unexpected result.")

    @ classmethod
    @ base.test_case
    async def TestMixedReadAttributeAndEvents(cls, devCtrl):
        def attributePathPossibilities():
            yield ('Ex Cx Ax', [
                (0, Clusters.BasicInformation.Attributes.VendorName),
                (0, Clusters.BasicInformation.Attributes.ProductID),
                (0, Clusters.BasicInformation.Attributes.HardwareVersion),
            ])
            yield ('Ex Cx A*', [(0, Clusters.BasicInformation)])
            yield ('E* Cx A*', [Clusters.Descriptor.Attributes.ServerList])
            yield ('E* A* A*', ['*'])

        def eventPathPossibilities():
            yield ('Ex Cx Ex', [(1, Clusters.UnitTesting.Events.TestEvent, 0)])
            yield ('Ex Cx E*', [(1, Clusters.UnitTesting, 0)])
            yield ('Ex C* E*', [1])
            yield ('E* C* E*', ['*'])
            yield ('Ex Cx E* Urgent', [(1, Clusters.UnitTesting, 1)])

        testCount = 0

        for attributes in attributePathPossibilities():
            for events in eventPathPossibilities():
                logging.info(
                    f"{testCount}: Reading mixed Attributes({attributes[0]}) Events({events[0]})")
                await cls._TriggerEvent(devCtrl)
                res = await cls._RetryForContent(request=lambda: devCtrl.Read(
                    nodeid=NODE_ID,
                    attributes=attributes[1],
                    events=events[1]), until=lambda res: res != 0)
                VerifyDecodeSuccess(res.attributes)

    @ classmethod
    async def RunTest(cls, devCtrl):
        try:
            cls.TestAPI()
            await cls.TestReadWriteOnlyAttribute(devCtrl)
            await cls.TestCommandRoundTrip(devCtrl)
            await cls.TestCommandRoundTripWithBadEndpoint(devCtrl)
            await cls.TestCommandWithResponse(devCtrl)
            await cls.TestReadEventRequests(devCtrl, 1)
            await cls.TestReadWriteAttributeRequestsWithVersion(devCtrl)
            await cls.TestReadAttributeRequests(devCtrl)
            await cls.TestSubscribeZeroMinInterval(devCtrl)
            await cls.TestSubscribeAttribute(devCtrl)
            await cls.TestAttributeCacheAttributeView(devCtrl)
            await cls.TestAttributeCacheClusterView(devCtrl)
            await cls.TestMixedReadAttributeAndEvents(devCtrl)
            # Note: Write will change some attribute values, always put it after read tests
            await cls.TestWriteRequest(devCtrl)
            await cls.TestTimedRequest(devCtrl)
            await cls.TestTimedRequestTimeout(devCtrl)
            await cls.TestGenerateUndefinedFabricScopedEventRequests(devCtrl)
        except Exception as ex:
            logger.error(
                f"Unexpected error occurred when running tests: {ex}")
            logger.exception(ex)
            return False
        return True
