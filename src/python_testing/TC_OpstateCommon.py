#
#    Copyright (c) 2023 Project CHIP Authors
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

import logging

import chip.clusters as Clusters
from mobly import asserts
from dataclasses import dataclass
from matter_testing_support import TestStep

@dataclass
class TestInfo:
    pics_name: str
    cluster: Clusters

@dataclass
class TestPlanConfig:
    cluster_revision: int
    feature_map: int


class TC_OVENOPSTATE_BASE():
    def __init__(self, implementer=None, test_info=None, test_plan_config=None):
        asserts.assert_true(implementer != None,
                            "You shall define the instance of Operational State Test Case implementer")

        asserts.assert_true(test_info != None,
                            "You shall define the test info!")

        asserts.assert_true(test_plan_config != None,
                            "You shall define the test plan config!")

        self.implementer = implementer
        self.test_info = test_info
        self.test_plan_config = test_plan_config

    async def read_and_expect_value(self, endpoint, attribute, expected_value):
        logging.info("\t##### Read %s" % attribute)
        attr_value = await self.implementer.read_single_attribute_check_success(
                                endpoint=endpoint,
                                cluster=self.test_info.cluster,
                                attribute=attribute)
        logging.info("\t## %s: %s" % (attribute, attr_value))

        asserts.assert_equal(attr_value, expected_value,
                             "Current value not according expected. Expected %d, received %d!" %
                             (expected_value, attr_value))

    async def read_and_expect_array_contains(self, endpoint, attribute, expected_contains):
        logging.info("\t##### Read %s" % attribute)
        attr_value = await self.implementer.read_single_attribute_check_success(
                                endpoint=endpoint,
                                cluster=self.test_info.cluster,
                                attribute=attribute)
        attr_value.sort()
        expected_contains.sort()

        logging.info("\t## Current value: [%s]" % ','.join(map(str, attr_value)))
        logging.info("\t## Expected value: [%s]" % ','.join(map(str, expected_contains)))

        for item in expected_contains:
            if item not in attr_value:
                asserts.fail("""Entry (%s), not found! The returned value SHALL include all the entries: [%s]!""" %
                             (item, ','.join(map(str, expected_contains))))

    def steps_TC_OPSTATE_BASE_1_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Commissioning, already done", is_commissioning=True),
                 TestStep(2, "TH reads from the DUT the ClusterRevision attribute"),
                 TestStep(3, "TH reads from the DUT the FeatureMap attribute"),
                 TestStep(4, "TH reads from the DUT the AttributeList attribute"),
                 TestStep(5, "TH reads from the DUT the EventList attribute"),
                 TestStep(6, "TH reads from the DUT the AcceptedCommandList attribute"),
                 TestStep(7, "TH reads from the DUT the GeneratedCommandList attribute")
                 ]
        return steps

    async def test_TC_OPSTATE_BASE_1_1(self, endpoint=1):
        cluster = self.test_info.cluster
        attribute = cluster.Attributes
        events = cluster.Events
        commands = cluster.Commands

        # STEP 1: Commission DUT to TH (can be skipped if done in a preceding test)
        self.implementer.step(1)

        # STEP 2: TH reads from the DUT the ClusterRevision attribute
        self.implementer.step(2)
        await self.read_and_expect_value(endpoint=endpoint,
                              attribute=attribute.ClusterRevision,
                              expected_value=self.test_plan_config.cluster_revision)

        # STEP 3: TH reads from the DUT the FeatureMap attribute
        self.implementer.step(3)
        await self.read_and_expect_value(endpoint=endpoint,
                              attribute=attribute.FeatureMap,
                              expected_value=self.test_plan_config.feature_map)

        # STEP 4: TH reads from the DUT the AttributeList attribute
        self.implementer.step(4)
        expected_value = [
            attribute.PhaseList.attribute_id,
            attribute.CurrentPhase.attribute_id,
            attribute.OperationalStateList.attribute_id,
            attribute.OperationalState.attribute_id,
            attribute.OperationalError.attribute_id,
            attribute.GeneratedCommandList.attribute_id,
            attribute.AcceptedCommandList.attribute_id,
            attribute.AttributeList.attribute_id,
            attribute.FeatureMap.attribute_id,
            attribute.ClusterRevision.attribute_id
        ]

        if self.implementer.check_pics(("%s.S.A0002" % self.test_info.pics_name)):
            expected_value.append(attribute.CountdownTime.attribute_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attribute.AttributeList,
                              expected_contains=expected_value)

        # STEP 5: TH reads from the DUT the EventList attribute
        self.implementer.step(5)
        if self.implementer.pics_guard(self.implementer.check_pics("PICS_EVENT_LIST_ENABLED")):
            expected_value = [
                events.OperationalError.event_id,
            ]

            if self.implementer.check_pics(("%s.S.E01" % self.test_info.pics_name)):
                expected_value.append(events.OperationCompletion.event_id)

            await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attribute.EventList,
                              expected_contains=expected_value)

        # STEP 6: TH reads from the DUT the AcceptedCommandList attribute
        self.implementer.step(6)
        expected_value = []

        if self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_name)):
            expected_value.append(commands.Pause.command_id)

        if self.implementer.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_name)):
            expected_value.append(commands.Stop.command_id)

        if self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_name)):
            expected_value.append(commands.Start.command_id)

        if self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_name)):
            expected_value.append(commands.Resume.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attribute.AcceptedCommandList,
                              expected_contains=expected_value)

        # STEP 7: TH reads from the DUT the AcceptedCommandList attribute
        self.implementer.step(7)
        expected_value = []

        if self.implementer.check_pics(("%s.S.C00.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C01.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C02.Rsp" % self.test_info.pics_name)) or \
            self.implementer.check_pics(("%s.S.C03.Rsp" % self.test_info.pics_name)):
            expected_value.append(commands.OperationalCommandResponse.command_id)

        await self.read_and_expect_array_contains(endpoint=endpoint,
                              attribute=attribute.GeneratedCommandList,
                              expected_contains=expected_value)

