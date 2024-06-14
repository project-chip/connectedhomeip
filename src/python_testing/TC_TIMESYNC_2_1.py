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

import ipaddress
from datetime import timedelta

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, utc_time_in_matter_epoch
from mobly import asserts


class TC_TIMESYNC_2_1(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_TIMESYNC_2_1(self) -> list[str]:
        return ["TIMESYNC.S"]

    @async_test_body
    async def test_TC_TIMESYNC_2_1(self):
        # Establishing features support
        TSCfeat = self.wait_for_user_input(prompt_msg="Is TSC feature supported? (y or n)\n", input_msg="Is TSC feature supported? (y or n)\n", prompt_msg_placeholder="y or n")
        NTPCfeat = self.wait_for_user_input(prompt_msg="Is NTPC feature supported? (y or n)\n", input_msg="Is NTPC feature supported? (y or n)\n", prompt_msg_placeholder="y or n", default_value='n')
        NTPSfeat = self.wait_for_user_input(prompt_msg="Is NTPS feature supported? (y or n)\n", input_msg="Is NTPS feature supported? (y or n)\n", prompt_msg_placeholder="y or n", default_value='n')
        TZfeat = self.wait_for_user_input(prompt_msg="Is TZ feature supported? (y or n)\n", input_msg="Is TZ feature supported? (y or n)\n", prompt_msg_placeholder="y or n")

        endpoint = self.user_params.get("endpoint", 0)

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes
        
        self.print_step(2, "Read Granularity attribute")
        granularity_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.Granularity)
        asserts.assert_less(granularity_dut, Clusters.TimeSynchronization.Enums.GranularityEnum.kUnknownEnumValue,
                            "Granularity is not in valid range")

        self.print_step(3, "Read TimeSource")
        time_source = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeSource)
        asserts.assert_less(time_source, Clusters.TimeSynchronization.Enums.TimeSourceEnum.kUnknownEnumValue,
                            "TimeSource is not in valid range")

        self.print_step(4, "Read TrustedTimeSource")
        if TSCfeat == 'y':
            trusted_time_source = await self.read_ts_attribute_expect_success(endpoint=endpoint,
                                                                                attribute=attributes.TrustedTimeSource)
            if trusted_time_source is not NullValue:
                asserts.assert_less_equal(trusted_time_source.fabricIndex, 0xFE,
                                            "FabricIndex for the TrustedTimeSource is out of range")
                asserts.assert_greater_equal(trusted_time_source.fabricIndex, 1,
                                                "FabricIndex for the TrustedTimeSource is out of range")

        
        self.print_step(5, "Read DefaultNTP")
        if NTPCfeat == 'y':
            default_ntp = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultNTP)
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
        if TZfeat == 'y':
            tz_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeZone)
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
        if TZfeat == 'y':
            dst_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.DSTOffset)
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
        utc_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
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
        if TZfeat == 'y':
            utc_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.UTCTime)
            local_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.LocalTime)
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
        if TZfeat == 'y':
            tz_db_dut = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeZoneDatabase)
            asserts.assert_less(tz_db_dut, Clusters.TimeSynchronization.Enums.TimeZoneDatabaseEnum.kUnknownEnumValue,
                                "TimeZoneDatabase is not in valid range")

        self.print_step(11, "Read NTPServerAvailable")
        if NTPSfeat == 'y':
            # bool typechecking happens in the test read functions, so all we need to do here is do the read
            await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.NTPServerAvailable)
        
        self.print_step(12, "Read TimeZoneListMaxSize")
        if TZfeat == 'y':
            size = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.TimeZoneListMaxSize)
            asserts.assert_greater_equal(size, 1, "TimeZoneListMaxSize must be at least 1")
            asserts.assert_less_equal(size, 2, "TimeZoneListMaxSize must be max 2")

        self.print_step(13, "Read DSTOffsetListMaxSize")
        if TZfeat == 'y':
            size = await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.DSTOffsetListMaxSize)
            asserts.assert_greater_equal(size, 1, "DSTOffsetListMaxSize must be at least 1")

        self.print_step(14, "Read SupportsDNSResolve")
        # bool typechecking happens in the test read functions, so all we need to do here is do the read
        if NTPCfeat == 'y':
            await self.read_ts_attribute_expect_success(endpoint=endpoint, attribute=attributes.SupportsDNSResolve)

if __name__ == "__main__":
    default_matter_test_main()
