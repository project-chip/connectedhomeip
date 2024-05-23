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
import time

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

# We don't have a good pipe between the c++ enums in CommissioningDelegate and python
# so this is hardcoded.
# I realize this is dodgy, not sure how to cross the enum from c++ to python cleanly
kConfigureUTCTime = 6
kConfigureTimeZone = 7
kConfigureDSTOffset = 8
kConfigureDefaultNTP = 9
kConfigureTrustedTimeSource = 19

# NOTE: all of these tests require a specific app setup. Please see TestTimeSyncTrustedTimeSourceRunner.py


class TestTestTimeSyncTrustedTimeSource(MatterBaseTest):
    # This test needs to be run against an app that has previously been commissioned, has been reset
    # but not factory reset, and which has been started with the --simulate-no-internal-time flag.
    # This test should be run using the provided "TestTimeSyncTrustedTimeSourceRunner.py" script
    @async_test_body
    async def test_SimulateNoInternalTime(self):
        ret = await self.read_single_attribute_check_success(
            cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.UTCTime)
        asserts.assert_equal(ret, NullValue, "Non-null value returned for time")

    @async_test_body
    async def test_HaveInternalTime(self):
        ret = await self.read_single_attribute_check_success(
            cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.UTCTime)
        asserts.assert_not_equal(ret, NullValue, "Null value returned for time")

    @async_test_body
    async def test_SetupTimeSourceACL(self):
        # We just want to append to this list
        ac = Clusters.AccessControl
        acl = await self.read_single_attribute_check_success(cluster=ac, attribute=ac.Attributes.Acl)
        new_acl_entry = ac.Structs.AccessControlEntryStruct(privilege=ac.Enums.AccessControlEntryPrivilegeEnum.kView,
                                                            authMode=ac.Enums.AccessControlEntryAuthModeEnum.kCase,
                                                            subjects=NullValue, targets=[ac.Structs.AccessControlTargetStruct(
                                                                cluster=Clusters.TimeSynchronization.id)]
                                                            )
        acl.append(new_acl_entry)
        await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, ac.Attributes.Acl(acl))])

    async def ReadFromTrustedTimeSource(self):
        # Give the node a couple of seconds to reach out and set itself up
        # TODO: Subscribe to granularity instead.
        time.sleep(6)
        ret = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.UTCTime)
        asserts.assert_not_equal(ret, NullValue, "Returned time is null")
        ret = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.Granularity)
        asserts.assert_not_equal(ret, Clusters.TimeSynchronization.Enums.GranularityEnum.kNoTimeGranularity,
                                 "Returned Granularity is kNoTimeGranularity")
        # TODO: needs to be gated on the optional attribute
        ret = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeSource)
        asserts.assert_equal(ret, Clusters.TimeSynchronization.Enums.TimeSourceEnum.kNodeTimeCluster,
                             "Returned time source is incorrect")

    @async_test_body
    async def test_SetAndReadFromTrustedTimeSource(self):
        asserts.assert_true('trusted_time_source' in self.matter_test_config.global_test_params,
                            "trusted_time_source must be included on the command line in "
                            "the --int-arg flag as trusted_time_source:<nodeId>")
        trusted_time_source = Clusters.TimeSynchronization.Structs.FabricScopedTrustedTimeSourceStruct(
            nodeID=self.matter_test_config.global_test_params["trusted_time_source"], endpoint=0)
        cmd = Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(trustedTimeSource=trusted_time_source)
        await self.send_single_cmd(cmd)

        await self.ReadFromTrustedTimeSource()

    @async_test_body
    async def test_ReadFromTrustedTimeSource(self):
        await self.ReadFromTrustedTimeSource()


if __name__ == "__main__":
    default_matter_test_main()
