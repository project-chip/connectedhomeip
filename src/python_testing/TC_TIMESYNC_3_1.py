#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TIMESYNC_3_1(MatterBaseTest):

    @async_test_body
    async def test_TC_TIMESYNC_3_1(self):
        self.print_step(1, "Wildcard read of time sync cluster")
        utc_time_attr = Clusters.TimeSynchronization.Attributes.UTCTime
        ret = await self.default_controller.ReadAttribute(nodeid=self.dut_node_id, attributes=[(utc_time_attr)])
        asserts.assert_equal(len(ret.keys()), 1, 'More than one time cluster found on the node')
        asserts.assert_equal(next(iter(ret)), 0, 'Time cluster found on non-root endpoint')

        self.print_step(2, "Wildcard read of descriptor cluster")
        server_list_attr = Clusters.Descriptor.Attributes.ServerList
        ret = await self.default_controller.ReadAttribute(nodeid=self.dut_node_id, attributes=[(server_list_attr)])
        print(ret)
        for endpoint, servers in ret.items():
            server_list = servers[Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]
            if endpoint == 0:
                asserts.assert_true(Clusters.TimeSynchronization.id in server_list,
                                    'Time cluster id is not listed in server list for root endpoint')
            else:
                asserts.assert_false(Clusters.TimeSynchronization.id in server_list,
                                     f'Time cluster id is incorrectly listed in the server list for ep {endpoint}')


if __name__ == "__main__":
    default_matter_test_main()
