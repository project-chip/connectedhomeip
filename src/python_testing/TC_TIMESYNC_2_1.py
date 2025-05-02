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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import ipaddress
from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import (MatterBaseTest, default_matter_test_main, has_attribute, has_cluster,
                                         run_if_endpoint_matches)
from chip.testing.timeoperations import utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_1(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=None, cluster=cluster, attribute=attribute)

    @run_if_endpoint_matches(has_cluster(Clusters.TimeSynchronization) and has_attribute(Clusters.TimeSynchronization.Attributes.TimeSource))
    async def test_TC_TIMESYNC_2_1(self):
        attributes = Clusters.TimeSynchronization.Attributes
        features = await self.read_ts_attribute_expect_success(attribute=attributes.FeatureMap)

        self.supports_time_zone = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kTimeZone)
        self.supports_ntpc = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kNTPClient)
        self.supports_ntps = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kNTPServer)
        self.supports_trusted_time_source = bool(features & Clusters.TimeSynchronization.Bitmaps.Feature.kTimeSyncClient)

        timesync_attr_list = attributes.AttributeList
        attribute_list = await self.read_ts_attribute_expect_success(attribute=timesync_attr_list)
        timesource_attr_id = attributes.TimeSource.attribute_id

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read Granularity attribute")
        granularity_dut = await self.read_ts_attribute_expect_success(attribute=attributes.Granularity)
        asserts.assert_less(granularity_dut, Clusters.TimeSynchronization.Enums.GranularityEnum.kUnknownEnumValue,
                            "Granularity is not in valid range")

        self.print_step(3, "Read TimeSource")
        if timesource_attr_id in attribute_list:
            time_source = await self.read_ts_attribute_expect_success(attribute=attributes.TimeSource)
            asserts.assert_less(time_source, Clusters.TimeSynchronization.Enums.TimeSourceEnum.kUnknownEnumValue,
                                "TimeSource is not in valid range")

        self.print_step(4, "Read TrustedTimeSource")
        if self.supports_trusted_time_source:
            trusted_time_source = await self.read_ts_attribute_expect_success(attribute=attributes.TrustedTimeSource)
            if trusted_time_source is not NullValue:
                asserts.assert_less_equal(trusted_time_source.fabricIndex, 0xFE,
                                          "FabricIndex for the TrustedTimeSource is out of range")
                asserts.assert_greater_equal(trusted_time_source.fabricIndex, 1,
                                             "FabricIndex for the TrustedTimeSource is out of range")

        self.print_step(5, "Read DefaultNTP")
        if self.supports_ntpc:
            default_ntp = await self.read_ts_attribute_expect_success(attribute=attributes.DefaultNTP)
            if default_ntp is not NullValue:
                asserts.assert_less_equal(len(default_ntp), 128, "DefaultNTP length must be less than 128")
                # Assume this is a valid web address if it has at least one . in the name
                is_web_addr = default_ntp.find('.') != -1
                try:
                    ipaddress.IPv6Address(default_ntp)
                    is_ip_addr = True
                except ipaddress.AddressValueError:
                    is_ip_addr = False
                    pass
                asserts.assert_true(is_web_addr or is_ip_addr, "Returned DefaultNTP value is not a IP address or web address")

        self.print_step(6, "Read TimeZone")
        if self.supports_time_zone:
            tz_dut = await self.read_ts_attribute_expect_success(attribute=attributes.TimeZone)
            asserts.assert_greater_equal(len(tz_dut), 1, "TimeZone must have at least one entry in the list")
            asserts.assert_less_equal(len(tz_dut), 2, "TimeZone may have a maximum of two entries in the list")
            for entry in tz_dut:
                asserts.assert_greater_equal(entry.offset, -43200, "TimeZone offset is out of range")
                asserts.assert_less_equal(entry.offset, 50400, "TimeZone offset is out of range")
                if entry.name:
                    asserts.assert_less_equal(len(entry.name), 64, "TimeZone name is too long")

            asserts.assert_equal(tz_dut[0].validAt, 0, "TimeZone list first entry must have a 0 ValidAt time")
            if len(tz_dut) > 1:
                asserts.assert_not_equal(tz_dut[1].validAt, 0, "TimeZone list second entry must have a non-zero ValidAt time")

        self.print_step(7, "Read DSTOffset")
        if self.supports_time_zone:
            dst_dut = await self.read_ts_attribute_expect_success(attribute=attributes.DSTOffset)
            last_valid_until = -1
            last_valid_starting = -1
            for dst in dst_dut:
                asserts.assert_greater(dst.validStarting, last_valid_starting,
                                       "DSTOffset list must be sorted by ValidStarting time")
                last_valid_starting = dst.validStarting
                asserts.assert_greater_equal(dst.validStarting, last_valid_until,
                                             "DSTOffset list must have every ValidStarting > ValidUntil of the previous entry")
                last_valid_until = dst.validUntil
                if dst.validUntil is NullValue or dst.validUntil is None:
                    asserts.assert_equal(dst, dst_dut[-1], "DSTOffset list must have Null ValidUntil at the end")

        self.print_step(8, "Read UTCTime")
        utc_dut = await self.read_ts_attribute_expect_success(attribute=attributes.UTCTime)
        if utc_dut is NullValue:
            asserts.assert_equal(granularity_dut, Clusters.TimeSynchronization.Enums.GranularityEnum.kNoTimeGranularity)
        else:
            asserts.assert_not_equal(granularity_dut, Clusters.TimeSynchronization.Enums.GranularityEnum.kNoTimeGranularity)
            if granularity_dut is Clusters.TimeSynchronization.Enums.GranularityEnum.kMinutesGranularity:
                toleranace = timedelta(minutes=10)
            else:
                toleranace = timedelta(minutes=1)
            delta_us = abs(utc_dut - utc_time_in_matter_epoch())
            delta = timedelta(microseconds=delta_us)
            asserts.assert_less_equal(delta, toleranace, "UTC time is not within tolerance of TH")

        self.print_step(9, "Read LocalTime")
        if self.supports_time_zone:
            utc_dut = await self.read_ts_attribute_expect_success(attribute=attributes.UTCTime)
            local_dut = await self.read_ts_attribute_expect_success(attribute=attributes.LocalTime)
            if utc_dut is NullValue:
                asserts.assert_true(local_dut is NullValue, "LocalTime must be Null if UTC time is Null")
            elif len(dst_dut) == 0:
                asserts.assert_true(local_dut is NullValue, "LocalTime must be Null if the DST table is empty")
            else:
                local_calculated = utc_dut + dst_dut[0].offset + tz_dut[0].offset
                delta_us = abs(local_dut, local_calculated)
                delta = timedelta(microseconds=delta_us)
                toleranace = timedelta(minutes=1)
                asserts.assert_less_equal(delta, toleranace, "Local time caluclation is not within tolerance of calculated value")

        self.print_step(10, "Read TimeZoneDatabase")
        if self.supports_time_zone:
            tz_db_dut = await self.read_ts_attribute_expect_success(attribute=attributes.TimeZoneDatabase)
            asserts.assert_less(tz_db_dut, Clusters.TimeSynchronization.Enums.TimeZoneDatabaseEnum.kUnknownEnumValue,
                                "TimeZoneDatabase is not in valid range")

        self.print_step(11, "Read NTPServerAvailable")
        if self.supports_ntps:
            # bool typechecking happens in the test read functions, so all we need to do here is do the read
            await self.read_ts_attribute_expect_success(attribute=attributes.NTPServerAvailable)

        self.print_step(12, "Read TimeZoneListMaxSize")
        if self.supports_time_zone:
            size = await self.read_ts_attribute_expect_success(attribute=attributes.TimeZoneListMaxSize)
            asserts.assert_greater_equal(size, 1, "TimeZoneListMaxSize must be at least 1")
            asserts.assert_less_equal(size, 2, "TimeZoneListMaxSize must be max 2")

        self.print_step(13, "Read DSTOffsetListMaxSize")
        if self.supports_time_zone:
            size = await self.read_ts_attribute_expect_success(attribute=attributes.DSTOffsetListMaxSize)
            asserts.assert_greater_equal(size, 1, "DSTOffsetListMaxSize must be at least 1")

        self.print_step(14, "Read SupportsDNSResolve")
        # bool typechecking happens in the test read functions, so all we need to do here is do the read
        if self.supports_ntpc:
            await self.read_ts_attribute_expect_success(attribute=attributes.SupportsDNSResolve)


if __name__ == "__main__":
    default_matter_test_main()
