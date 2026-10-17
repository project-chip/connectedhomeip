# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
from types import SimpleNamespace

from matter.testing.linux.wifi import WpaSupplicantMock


class TestWpaSupplicantScan(unittest.IsolatedAsyncioTestCase):
    async def test_scan_results_describe_the_commissioned_network(self):
        interface = WpaSupplicantMock.WpaInterface(SimpleNamespace(ssid="MatterAP"), 0)
        self.assertEqual(await interface.BSSs.get_async(), [interface.bss.path])
        self.assertEqual(await interface.bss.SSID.get_async(), b"MatterAP")
        self.assertEqual(len(await interface.bss.BSSID.get_async()), 6)
        self.assertEqual(await interface.bss.Frequency.get_async(), 2412)
        self.assertEqual(await interface.bss.RSN.get_async(), {"KeyMgmt": ("as", ["wpa-psk"])})
        self.assertEqual(await interface.network.Properties.get_async(), {"ssid": ("s", '"MatterAP"')})

    async def test_select_and_remove_network_clear_connected_state(self):
        for remove_all in (False, True):
            with self.subTest(remove_all=remove_all):
                interface = WpaSupplicantMock.WpaInterface(SimpleNamespace(ssid="MatterAP"), 0)
                self.assertFalse(await interface.network.Enabled.get_async())
                await interface.SelectNetwork(interface.network.path)
                self.assertTrue(await interface.network.Enabled.get_async())
                self.assertEqual(await interface.CurrentNetwork.get_async(), interface.network.path)
                if remove_all:
                    await interface.RemoveAllNetworks()
                else:
                    await interface.RemoveNetwork(interface.network.path)
                self.assertFalse(await interface.network.Enabled.get_async())
                self.assertEqual(await interface.CurrentNetwork.get_async(), "/")


if __name__ == "__main__":
    unittest.main()
