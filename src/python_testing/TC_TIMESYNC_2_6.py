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

import typing

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TIMESYNC_2_6(MatterBaseTest):
    async def read_ts_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def send_set_default_ntp_cmd(self, ntp: typing.Union[Nullable, str]) -> None:
        await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDefaultNTP(defaultNTP=ntp))

    async def send_set_default_ntp_cmd_expect_error(self, ntp: typing.Union[Nullable, str], error: Status) -> None:
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.TimeSynchronization.Commands.SetDefaultNTP(defaultNTP=ntp))
            asserts.assert_true(False, "Unexpected SetDefaultNTP command success")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, error, "Unexpected error returned")
            pass

    def pics_TC_TIMESYNC_2_6(self) -> list[str]:
        return ["TIMESYNC.S.F01"]

    @async_test_body
    async def test_TC_TIMESYNC_2_6(self):

        # Time sync is required to be on endpoint 0 if it is present
        self.endpoint = 0

        self.print_step(0, "Commissioning, already done")
        attributes = Clusters.TimeSynchronization.Attributes
        default_ntp_attr = attributes.DefaultNTP

        self.print_step(1, "Read SupportsDNSResolve attribute")
        dns_resolve_supported = await self.read_ts_attribute_expect_success(attributes.SupportsDNSResolve)

        self.print_step(2, "Clear DefaultNTP")
        await self.send_set_default_ntp_cmd(ntp=NullValue)

        self.print_step(3, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP not cleared correctly")

        self.print_step(4, "Set DefaultNTP to resolvable host name")
        ntp = "time.nist.gov"
        if dns_resolve_supported:
            await self.send_set_default_ntp_cmd(ntp=ntp)
        else:
            await self.send_set_default_ntp_cmd_expect_error(ntp=ntp, error=Status.InvalidCommand)

        self.print_step(5, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        expected = ntp if dns_resolve_supported else NullValue
        asserts.assert_equal(attr, expected, "Unexpected DefaultNTP value")

        self.print_step(6, "Clear DefaultNTP")
        await self.send_set_default_ntp_cmd(ntp=NullValue)

        self.print_step(7, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP not cleared correctly")

        self.print_step(8, "Set DefaultNTP to RandomString")
        await self.send_set_default_ntp_cmd_expect_error(ntp="RandomString", error=Status.InvalidCommand)

        self.print_step(9, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP incorrectly set")

        self.print_step(10, "Set DefaultNTP to malformed IPv6")
        await self.send_set_default_ntp_cmd_expect_error(ntp="fe80:1", error=Status.InvalidCommand)

        self.print_step(11, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP incorrectly set")

        self.print_step(12, "Set DefaultNTP to malformed IPv6")
        await self.send_set_default_ntp_cmd_expect_error(ntp="fz80::1", error=Status.InvalidCommand)

        self.print_step(13, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP incorrectly set")

        self.print_step(14, "Set DefaultNTP to good IPv6")
        ntp = "fe80::1"
        await self.send_set_default_ntp_cmd(ntp=ntp)

        self.print_step(15, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, ntp, "DefaultNTP mismatch")

        self.print_step(16, "Clear DefaultNTP")
        await self.send_set_default_ntp_cmd(ntp=NullValue)

        self.print_step(17, "Read DefaultNTP")
        attr = await self.read_ts_attribute_expect_success(default_ntp_attr)
        asserts.assert_equal(attr, NullValue, "DefaultNTP not cleared correctly")


if __name__ == "__main__":
    default_matter_test_main()
