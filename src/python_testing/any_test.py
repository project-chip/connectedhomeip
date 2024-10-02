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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import copy
import logging
import time

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import AttributePath, TypedAttributePath, AsyncReadTransaction
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from matter_testing_support import AttributeChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

from chip.clusters import Attribute
from global_attribute_ids import GlobalAttributeIds


import chip.clusters as Clusters
import chip.logging
import chip.native
from chip import discovery
from chip.ChipStack import ChipStack
from chip.clusters import Attribute
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.setup_payload import SetupPayload
from chip.storage import PersistentStorage
from chip.tracing import TracingContext
from global_attribute_ids import GlobalAttributeIds
from mobly import asserts, base_test, signals, utils
from mobly.config_parser import ENV_MOBLY_LOGPATH, TestRunConfig
from mobly.test_runner import TestRunner
from pics_support import read_pics_from_file


from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.FabricAdmin  # Needed before chip.CertificateAuthority
import chip.CertificateAuthority
from chip.ChipDeviceCtrl import CommissioningParameters

import pprint

'''
Category:
Functional

Description:
Validates Interaction Data Model (IDM), specifically subscription responses. Some example of tests run:
  - Subscriptions with varying MaxIntervalCeiling
  - Checks for `InvalidAction` results when subscribing to clusters and attributes without access rights
  - Checks that subscription is not established for invalid MinIntervalFloor
  - Validates that only correctly filtered data is received when a subscription is established

Full test plan link for details:
https://github.com/CHIP-Specifications/chip-test-plans/blob/master/src/interactiondatamodel.adoc#tc-idm-4-2-subscription-response-messages-from-dut-test-cases-dut_server
'''


class any_test(MatterBaseTest):
    
    def stringify_keys(self, d):
        """Recursively converts keys in the dictionary to string format."""
        if isinstance(d, dict):
            return {str(key): self.stringify_keys(value) for key, value in d.items()}
        elif isinstance(d, list):
            return [self.stringify_keys(item) for item in d]
        else:
            return d

    def pretty_print_unconventional_json(self, data):
        """Pretty prints the unconventional JSON-like structure."""
        processed_data = self.stringify_keys(data)
        print('\n\n\n')
        pprint.pprint(processed_data, indent=64, width=100)
        print('\n\n\n')

    def steps_any_test(self):
        return [TestStep(0, "Some action", "Some check") ]

    ROOT_NODE_ENDPOINT_ID = 0

    async def get_descriptor_server_list(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.ServerList
        )

    async def get_descriptor_parts_list(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.Descriptor,
            attribute=Clusters.Descriptor.Attributes.PartsList
        )

    async def get_idle_mode_duration_sec(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        return await self.read_single_attribute_check_success(
            endpoint=ep,
            dev_ctrl=ctrl,
            cluster=Clusters.IcdManagement,
            attribute=Clusters.IcdManagement.Attributes.IdleModeDuration
        )

    @staticmethod
    def verify_attribute_exists(sub, cluster, attribute, ep=ROOT_NODE_ENDPOINT_ID):
        sub_attrs = sub
        if isinstance(sub, Clusters.Attribute.SubscriptionTransaction):
            sub_attrs = sub.GetAttributes()

        asserts.assert_true(ep in sub_attrs, "Must have read endpoint %s data" % ep)
        asserts.assert_true(cluster in sub_attrs[ep], "Must have read %s cluster data" % cluster.__name__)
        asserts.assert_true(attribute in sub_attrs[ep][cluster],
                            "Must have read back attribute %s" % attribute.__name__)

    @staticmethod
    def get_typed_attribute_path(attribute, ep=ROOT_NODE_ENDPOINT_ID):
        return TypedAttributePath(
            Path=AttributePath.from_attribute(
                EndpointId=ep,
                Attribute=attribute
            )
        )

    async def write_dut_acl(self, ctrl, acl, ep=ROOT_NODE_ENDPOINT_ID):
        result = await ctrl.WriteAttribute(self.dut_node_id, [(ep, Clusters.AccessControl.Attributes.Acl(acl))])
        asserts.assert_equal(result[ep].Status, Status.Success, "ACL write failed")

    async def get_dut_acl(self, ctrl, ep=ROOT_NODE_ENDPOINT_ID):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(ep, Clusters.AccessControl.Attributes.Acl)],
            keepSubscriptions=False,
            fabricFiltered=True
        )

        acl_list = sub[ep][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]

        return acl_list

    async def add_ace_to_dut_acl(self, ctrl, ace, dut_acl_original):
        dut_acl = copy.deepcopy(dut_acl_original)
        dut_acl.append(ace)
        await self.write_dut_acl(ctrl=ctrl, acl=dut_acl)

    @staticmethod
    def is_valid_uint32_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFFFFFF

    @staticmethod
    def is_valid_uint16_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFF

    @async_test_body
    async def test_any_test(self):

        # Test setup
        cluster_rev_attr = Clusters.BasicInformation.Attributes.ClusterRevision
        cluster_rev_attr_typed_path = self.get_typed_attribute_path(cluster_rev_attr)
        node_label_attr = Clusters.BasicInformation.Attributes.NodeLabel
        node_label_attr_path = [(0, node_label_attr)]
        subscription_max_interval_publisher_limit_sec = 0
        INVALID_ACTION_ERROR_CODE = 0x580

        # Controller 1 setup
        # Subscriber/client with admin access to the DUT
        # Will write ACL for controller 2 and validate success/error codes
        CR1: ChipDeviceController = self.default_controller

        # Original DUT ACL used for reseting the ACL on some steps
        dut_acl_original = await self.get_dut_acl(CR1)

        # Controller 2 setup
        # Subscriber/client with limited access to the DUT
        # Will validate error status codes
        fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        CR2_nodeid = self.matter_test_config.controller_node_id + 1
        CR2: ChipDeviceController = fabric_admin.NewController(
            nodeId=CR2_nodeid,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
        )

        # *** Step 0 ***
        # CR1 reads the ServerList attribute from the Descriptor cluster on EP0. If the ICDManagement cluster ID
        # (70,0x46) is present, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration and
        # min_interval_floor_s to 0, otherwise, set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = 60 mins and
        # min_interval_floor_s to 3.
        self.step(0)




















        attributes = [
            # (Clusters.Descriptor),
            # (Clusters.NetworkCommissioning.Attributes.FeatureMap),
            (Clusters.NetworkCommissioning),
            # Attribute.AttributePath(None, None, GlobalAttributeIds.ATTRIBUTE_LIST_ID),
            # Attribute.AttributePath(None, None, GlobalAttributeIds.FEATURE_MAP_ID),
            # Attribute.AttributePath(None, None, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID),
        ]

        wildcard: AsyncReadTransaction.ReadResponse = await CR1.Read(
            nodeid=self.dut_node_id,
            attributes=attributes
        )

        maps = str(wildcard.attributes.keys().mapping)
        self.pretty_print_unconventional_json(maps)


















        # # Reads the ServerList attribute
        # ep0_servers = await self.get_descriptor_server_list(CR1)

        # # Check if ep0_servers contains the ICD Management cluster ID (0x0046)
        # if Clusters.IcdManagement.id in ep0_servers:
        #     # Read the IdleModeDuration attribute value from the DUT
        #     logging.info(
        #         "CR1 reads from the DUT the IdleModeDuration attribute and sets SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC = IdleModeDuration")

        #     idleModeDuration = await self.get_idle_mode_duration_sec(CR1)
        #     subscription_max_interval_publisher_limit_sec = idleModeDuration
        #     min_interval_floor_sec = 0
        # else:
        #     # Defaulting SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to 60 minutes
        #     subscription_max_interval_publisher_limit_sec = 60 * 60
        #     min_interval_floor_sec = 3

        # asserts.assert_greater_equal(subscription_max_interval_publisher_limit_sec, 1,
        #                              "SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC must be at least 1")

        # logging.info(
        #     f"Set SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT_SEC to {subscription_max_interval_publisher_limit_sec} seconds")

if __name__ == "__main__":
    default_matter_test_main()
