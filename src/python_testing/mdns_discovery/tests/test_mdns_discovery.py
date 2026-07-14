#
#    Copyright (c) 2026 Project CHIP Authors
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

import unittest
from types import SimpleNamespace

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType


class FakeMdnsDiscovery(MdnsDiscovery):
    def __init__(
        self,
        service_types: list[str],
        commissionable_services: list[SimpleNamespace],
        advertised_subtypes: list[str],
    ):
        self.service_types = service_types
        self.commissionable_services = commissionable_services
        self.advertised_subtypes = advertised_subtypes
        self.ptr_record_queries: list[list[str]] = []

    async def get_all_service_types(
        self,
        log_output: bool = False,
        discovery_timeout_sec: float = 15,
    ) -> list[str]:
        return self.service_types

    async def get_commissionable_services(
        self,
        log_output: bool = False,
        discovery_timeout_sec: float = 15,
    ) -> list[SimpleNamespace]:
        return self.commissionable_services

    async def get_ptr_records(
        self,
        service_types: list[str],
        discovery_timeout_sec: float = 15,
        log_output: bool = False,
    ) -> list[SimpleNamespace]:
        self.ptr_record_queries.append(service_types)
        return [
            SimpleNamespace(service_type=service_type)
            for service_type in service_types
            if service_type in self.advertised_subtypes
        ]


class TestGetCommissionableSubtypes(unittest.IsolatedAsyncioTestCase):
    async def test_verifies_txt_derived_subtypes_with_ptr_browse(self):
        commissionable_type = MdnsServiceType.COMMISSIONABLE.value
        service_info = SimpleNamespace(
            service_type=commissionable_type,
            txt={
                'D': '3840',
                'CM': '2',
                'VP': '65521+32769',
                'DT': '10',
            }
        )

        discovery = FakeMdnsDiscovery(
            service_types=[commissionable_type],
            commissionable_services=[service_info],
            advertised_subtypes=[
                f"_L3840._sub.{commissionable_type}",
                f"_S15._sub.{commissionable_type}",
                f"_CM._sub.{commissionable_type}",
            ],
        )

        subtypes = await discovery.get_commissionable_subtypes()

        self.assertEqual(
            subtypes,
            [
                f"_L3840._sub.{commissionable_type}",
                f"_S15._sub.{commissionable_type}",
                f"_CM._sub.{commissionable_type}",
            ]
        )
        self.assertEqual(
            discovery.ptr_record_queries,
            [[
                f"_L3840._sub.{commissionable_type}",
                f"_S15._sub.{commissionable_type}",
                f"_CM._sub.{commissionable_type}",
                f"_V65521._sub.{commissionable_type}",
                f"_T10._sub.{commissionable_type}",
            ]]
        )

    async def test_merges_enumerated_and_discovered_subtypes_without_duplicates(self):
        commissionable_type = MdnsServiceType.COMMISSIONABLE.value
        service_info = SimpleNamespace(
            service_type=f"_L15._sub.{commissionable_type}",
            txt={'D': '15', 'CM': '0'}
        )

        discovery = FakeMdnsDiscovery(
            service_types=[
                commissionable_type,
                f"_L15._sub.{commissionable_type}",
                f"_S0._sub.{commissionable_type}",
            ],
            commissionable_services=[service_info],
            advertised_subtypes=[],
        )

        subtypes = await discovery.get_commissionable_subtypes()

        self.assertEqual(
            subtypes,
            [
                f"_L15._sub.{commissionable_type}",
                f"_S0._sub.{commissionable_type}",
            ]
        )
        self.assertEqual(discovery.ptr_record_queries, [])


if __name__ == '__main__':
    unittest.main()
