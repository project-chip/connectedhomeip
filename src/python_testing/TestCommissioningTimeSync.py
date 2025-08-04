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
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts

# We don't have a good pipe between the c++ enums in CommissioningDelegate and python
# so this is hardcoded.
# I realize this is dodgy, not sure how to cross the enum from c++ to python cleanly
kCheckForMatchingFabric = 3
kConfigureUTCTime = 6
kConfigureTimeZone = 7
kConfigureDSTOffset = 8
kConfigureDefaultNTP = 9
kConfigureTrustedTimeSource = 19


class TestCommissioningTimeSync(MatterBaseTest):
    def setup_class(self):
        self.commissioner = None
        self.commissioned = False
        return super().setup_class()

    async def destroy_current_commissioner(self):
        if self.commissioner:
            if self.commissioned:
                fabricidx = await self.read_single_attribute_check_success(dev_ctrl=self.commissioner, cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)
                cmd = Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=fabricidx)
                await self.send_single_cmd(cmd=cmd)
            self.commissioner.Shutdown()
        self.commissioner = None
        self.commissioned = False

    @async_test_body
    async def teardown_test(self):
        await self.destroy_current_commissioner()
        return super().teardown_test()

    async def commission_and_base_checks(self):
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=1234, option=1)
        await self.commissioner.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)
        self.commissioned = True

        # Check the feature map - if we have a time cluster, we want UTC time to be set
        features = await self.read_single_attribute(dev_ctrl=self.default_controller, node_id=self.dut_node_id,
                                                    endpoint=0, attribute=Clusters.TimeSynchronization.Attributes.FeatureMap)
        self.supports_time = True
        if isinstance(features, Clusters.Attribute.ValueDecodeFailure):
            asserts.assert_true(isinstance(features.Reason, InteractionModelError), InteractionModelError,
                                f'Unexpected exception from reading time cluster feature map {features.Reason}')
            asserts.assert_equal(features.Reason.status, Status.UnsupportedCluster,
                                 f'Unexpected error response from reading time cluster feature map {features.Reason}')
            self.supports_time = False

        asserts.assert_equal(self.commissioner.CheckStageSuccessful(
            kConfigureUTCTime), self.supports_time, 'UTC time stage incorrect')

        if self.supports_time:
            self.supports_time_zone = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kTimeZone)
            self.supports_default_ntp = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kNTPClient)
            self.supports_trusted_time_source = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kTimeSyncClient)
        else:
            self.supports_time_zone = False
            self.supports_default_ntp = False
            self.supports_trusted_time_source = False

    async def create_commissioner(self):
        if self.commissioner:
            await self.destroy_current_commissioner()
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        self.commissioner = new_fabric_admin.NewController(nodeId=112233, useTestCommissioner=True)

        self.commissioner.ResetCommissioningParameters()
        self.commissioner.ResetTestCommissioner()

        # If the app we're testing against doesn't have a time cluster, we still want to run this
        # tests and we want it to succeed, so catch the unsupported cluster error here and ignore
        try:
            cmd = Clusters.TimeSynchronization.Commands.SetDefaultNTP(defaultNTP=NullValue)
            await self.send_single_cmd(cmd=cmd)
            cmd = Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(NullValue)
            await self.send_single_cmd(cmd=cmd)
        except InteractionModelError as e:
            if e.status == Status.UnsupportedCluster:
                pass

    async def commission_stages(self, time_zone: bool, dst: bool, default_ntp: bool, trusted_time_source: bool):
        await self.create_commissioner()

        logging.info(
            f'Running Commissioning test - time_zone: {time_zone}, dst: {dst}, default_ntp: {default_ntp}, trusted_time_source: {trusted_time_source}')

        if time_zone:
            self.commissioner.SetTimeZone(offset=3600, validAt=0)
        if dst:
            six_months = 1.577e+13  # in us
            dst_valid_until = utc_time_in_matter_epoch() + int(six_months)
            self.commissioner.SetDSTOffset(offset=3600, validStarting=0, validUntil=dst_valid_until)
        if default_ntp:
            self.commissioner.SetDefaultNTP("fe80::1")
        if trusted_time_source:
            self.commissioner.SetTrustedTimeSource(self.commissioner.nodeId, 0)

        await self.commission_and_base_checks()

        should_set_time_zone = bool(self.supports_time_zone and time_zone)
        should_set_dst = bool(self.supports_time_zone and time_zone and dst)
        should_set_default_ntp = bool(self.supports_default_ntp and default_ntp)
        should_set_trusted_time = bool(self.supports_trusted_time_source and trusted_time_source)

        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureTimeZone),
                             should_set_time_zone, 'Incorrect value for time zone stage check')
        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureDSTOffset),
                             should_set_dst, 'Incorrect value for kConfigureDSTOffset stage')
        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureDefaultNTP),
                             should_set_default_ntp, 'Incorrect value for kConfigureDefaultNTP stage')
        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureTrustedTimeSource),
                             should_set_trusted_time, 'Incorrect value for kConfigureTrustedTimeSource stage')

        if should_set_time_zone:
            received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeZone)
            expected = [Clusters.TimeSynchronization.Structs.TimeZoneStruct(offset=3600, validAt=0)]
            asserts.assert_equal(received, expected, "Time zone was not correctly set by commissioner")

        if should_set_dst:
            received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.DSTOffset)
            expected = [Clusters.TimeSynchronization.Structs.DSTOffsetStruct(
                offset=3600, validStarting=0, validUntil=dst_valid_until)]
            asserts.assert_equal(received, expected, "DST was not set correctly by the commissioner")

        if should_set_trusted_time:
            fabric_idx = await self.read_single_attribute_check_success(cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, dev_ctrl=self.commissioner)
            received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TrustedTimeSource)
            expected = Clusters.TimeSynchronization.Structs.TrustedTimeSourceStruct(
                fabricIndex=fabric_idx, nodeID=self.commissioner.nodeId, endpoint=0)
            asserts.assert_equal(received, expected, "Trusted Time source was not set properly")

        if should_set_default_ntp:
            received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.DefaultNTP)
            expected = "fe80::1"
            asserts.assert_equal(received, expected, "Default NTP was not set properly")

    @async_test_body
    async def test_CommissioningAllBasic(self):
        # We want to assess all combos (ie, all flags in the range of 0b0000 to 0b1111)
        for i in range(0, 0xF):
            time_zone = bool(i & 0x1)
            dst = bool(i & 0x2)
            default_ntp = bool(i & 0x4)
            trusted_time_source = bool(i & 0x8)
            await self.commission_stages(time_zone, dst, default_ntp, trusted_time_source)

    @async_test_body
    async def test_CommissioningPreSetValues(self):

        await self.create_commissioner()

        # If we're running this against a node that doesn't have a time cluster, this test doesn't apply
        # and the remaining cases are covered in the base case above.
        try:
            trusted_time_source = Clusters.TimeSynchronization.Structs.FabricScopedTrustedTimeSourceStruct(
                nodeID=0x5555, endpoint=0)
            cmd = Clusters.TimeSynchronization.Commands.SetTrustedTimeSource(trusted_time_source)
            await self.send_single_cmd(cmd)

            cmd = Clusters.TimeSynchronization.Commands.SetDefaultNTP("fe80::02")
            await self.send_single_cmd(cmd)
        except InteractionModelError as e:
            if e.status == Status.UnsupportedCluster:
                await self.destroy_current_commissioner()
                return

        self.commissioner.SetTimeZone(offset=3600, validAt=0)
        six_months = 1.577e+13  # in us
        self.commissioner.SetDSTOffset(offset=3600, validStarting=0, validUntil=utc_time_in_matter_epoch() + int(six_months))
        self.commissioner.SetDefaultNTP("fe80::1")
        self.commissioner.SetTrustedTimeSource(self.commissioner.nodeId, 0)

        await self.commission_and_base_checks()
        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureTimeZone),
                             self.supports_time_zone, 'Incorrect value for time zone stage check')
        asserts.assert_equal(self.commissioner.CheckStageSuccessful(kConfigureDSTOffset),
                             self.supports_time_zone, 'Incorrect value for kConfigureDSTOffset stage')
        asserts.assert_false(self.commissioner.CheckStageSuccessful(kConfigureDefaultNTP), 'kConfigureDefaultNTP incorrectly set')
        asserts.assert_false(self.commissioner.CheckStageSuccessful(
            kConfigureTrustedTimeSource), 'kConfigureTrustedTimeSource incorrectly set')

    @async_test_body
    async def test_FabricCheckStage(self):
        await self.create_commissioner()

        # This was moved into a different stage when the time sync stuff was added
        asserts.assert_equal(self.commissioner.GetFabricCheckResult(), -1, "Fabric check result is already set")
        self.commissioner.SetCheckMatchingFabric(True)
        await self.commission_and_base_checks()
        asserts.assert_true(self.commissioner.CheckStageSuccessful(
            kCheckForMatchingFabric), "Did not run check for matching fabric stage")
        asserts.assert_equal(self.commissioner.GetFabricCheckResult(), 0, "Fabric check result did not get set by pairing delegate")

        # Let's try it again with no check
        await self.create_commissioner()
        asserts.assert_equal(self.commissioner.GetFabricCheckResult(), -1, "Fabric check result is already set")
        self.commissioner.SetCheckMatchingFabric(False)
        await self.commission_and_base_checks()
        asserts.assert_false(self.commissioner.CheckStageSuccessful(
            kCheckForMatchingFabric), "Incorrectly ran check for matching fabric stage")
        asserts.assert_equal(self.commissioner.GetFabricCheckResult(), -1, "Fabric check result incorrectly set")

    @async_test_body
    async def test_TimeZoneName(self):
        await self.create_commissioner()
        self.commissioner.SetTimeZone(offset=3600, validAt=0, name="test")
        await self.commission_and_base_checks()
        asserts.assert_true(self.commissioner.CheckStageSuccessful(kConfigureTimeZone), 'Time zone was not successfully set')

        received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeZone)
        expected = [Clusters.TimeSynchronization.Structs.TimeZoneStruct(offset=3600, validAt=0, name="test")]
        asserts.assert_equal(received, expected, "Time zone was not correctly set by commissioner")

        await self.create_commissioner()
        # name is max 64 per the spec
        sixty_five_byte_string = "x" * 65
        self.commissioner.SetTimeZone(offset=3600, validAt=0, name=sixty_five_byte_string)
        await self.commission_and_base_checks()
        asserts.assert_true(self.commissioner.CheckStageSuccessful(kConfigureTimeZone), 'Time zone was not successfully set')

        received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeZone)
        expected = [Clusters.TimeSynchronization.Structs.TimeZoneStruct(offset=3600, validAt=0, name=None)]
        asserts.assert_equal(received, expected, "Commissioner did not ignore too-long name")

        await self.create_commissioner()
        # name is max 64 per the spec
        sixty_four_byte_string = "x" * 64
        self.commissioner.SetTimeZone(offset=3600, validAt=0, name=sixty_four_byte_string)
        await self.commission_and_base_checks()
        asserts.assert_true(self.commissioner.CheckStageSuccessful(kConfigureTimeZone), 'Time zone was not successfully set')

        received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.TimeZone)
        expected = [Clusters.TimeSynchronization.Structs.TimeZoneStruct(offset=3600, validAt=0, name=sixty_four_byte_string)]
        asserts.assert_equal(received, expected, "Time zone 64 byte name was not correctly set")

    @async_test_body
    async def test_DefaultNtpSize(self):
        await self.create_commissioner()
        too_long_name = "x." + "x" * 127
        self.commissioner.SetDefaultNTP(too_long_name)
        await self.commission_and_base_checks()
        asserts.assert_false(self.commissioner.CheckStageSuccessful(kConfigureDefaultNTP),
                             'Commissioner attempted to set default NTP to a too long value')

        await self.create_commissioner()
        just_fits_name = "x." + "x" * 126
        self.commissioner.SetDefaultNTP(just_fits_name)
        await self.commission_and_base_checks()
        asserts.assert_true(self.commissioner.CheckStageSuccessful(kConfigureDefaultNTP),
                            'Commissioner did not correctly set default NTP')
        received = await self.read_single_attribute_check_success(cluster=Clusters.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.DefaultNTP)
        asserts.assert_equal(received, just_fits_name, 'Commissioner incorrectly set default NTP name')


# TODO(cecille): Test - Add hooks to change the time zone response to indicate no DST is needed
# TODO(cecille): Test - Set commissioningParameters TimeZone and DST list size to > node list size to ensure they get truncated
if __name__ == "__main__":
    default_matter_test_main()
