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

import asyncio
import unittest
from unittest.mock import AsyncMock, Mock

from mdns_discovery.mdns_async_service_info import AddressResolverIPv6, MdnsAsyncServiceInfo
from zeroconf import DNSText, ServiceInfo, current_time_millis
from zeroconf._record_update import RecordUpdate
from zeroconf.const import _CLASS_IN, _TYPE_SRV, _TYPE_TXT

SERVICE_TYPE = "_matter._tcp.local."
SERVICE_NAME = f"0123456789ABCDEF-0000000000000001.{SERVICE_TYPE}"


class TestMdnsAsyncServiceInfo(unittest.TestCase):
    def setUp(self):
        self.info = MdnsAsyncServiceInfo(name=SERVICE_NAME, type_=SERVICE_TYPE)

    def test_txt_only_requires_txt(self):
        self.info._query_record_types = {_TYPE_TXT}
        self.assertFalse(self.info._is_complete)
        self.info.text = b"\x04IC=1"
        self.assertTrue(self.info._is_complete)
        self.assertIsNone(self.info.server)
        self.assertEqual(self.info.addresses, [])

    def test_empty_txt_payload(self):
        self.info._query_record_types = {_TYPE_TXT}
        self.info.text = b"\x00"
        self.assertTrue(self.info._is_complete)

    def test_srv_only_requires_server_and_port(self):
        self.info._query_record_types = {_TYPE_SRV}
        self.assertFalse(self.info._is_complete)
        self.info.server = "device.local."
        self.assertFalse(self.info._is_complete)
        self.info.port = 5540
        self.assertTrue(self.info._is_complete)
        self.assertEqual(self.info.addresses, [])
        self.assertEqual(self.info.text, b"")

    def test_srv_zero_port(self):
        self.info._query_record_types = {_TYPE_SRV}
        self.info.server = "device.local."
        self.info.port = 0
        self.assertTrue(self.info._is_complete)

    def test_full_service_preserves_base_completion(self):
        for addresses in ([], [b"\xc0\x00\x02\x01"], [b"\x00" * 15 + b"\x01"]):
            for properties in (None, b"\x00", b"\x04IC=1"):
                with self.subTest(addresses=addresses, properties=properties):
                    info = MdnsAsyncServiceInfo(name=SERVICE_NAME, type_=SERVICE_TYPE)
                    base = ServiceInfo(SERVICE_TYPE, SERVICE_NAME)
                    for service_info in (info, base):
                        service_info.addresses = addresses
                        if properties is not None:
                            record = DNSText(SERVICE_NAME, _TYPE_TXT, _CLASS_IN, 120, properties)
                            service_info.async_update_records(Mock(), current_time_millis(), [RecordUpdate(record, None)])
                    self.assertEqual(info._is_complete, base._is_complete)

    def test_ipv6_resolver_still_requires_ipv6(self):
        info = AddressResolverIPv6("device.local.")
        self.assertFalse(info._is_complete)
        info.addresses = [b"\xc0\x00\x02\x01"]
        self.assertFalse(info._is_complete)
        info.addresses = [b"\x00" * 15 + b"\x01"]
        self.assertTrue(info._is_complete)


class TestMdnsAsyncRequest(unittest.IsolatedAsyncioTestCase):
    async def test_txt_only_response_completes_request(self):
        info = MdnsAsyncServiceInfo(name=SERVICE_NAME, type_=SERVICE_TYPE)
        info._query_record_types = {_TYPE_TXT}
        zc = Mock(started=True, loop=asyncio.get_running_loop())

        async def receive_txt(*args):
            record = DNSText(SERVICE_NAME, _TYPE_TXT, _CLASS_IN, 120, b"\x04IC=1")
            info.async_update_records(zc, current_time_millis(), [RecordUpdate(record, None)])

        info.async_wait = AsyncMock(side_effect=receive_txt)
        self.assertTrue(await info.async_request(zc, timeout_ms=1000))
        zc.async_send.assert_called()
        self.assertEqual(info.decoded_properties, {"IC": "1"})
        self.assertEqual(info.addresses, [])
        self.assertIsNone(info.server)
        zc.async_remove_listener.assert_called_once_with(info)
        for call in zc.async_send.call_args_list:
            self.assertEqual({question.type for question in call.args[0].questions}, {_TYPE_TXT})

    async def test_missing_txt_times_out(self):
        info = MdnsAsyncServiceInfo(name=SERVICE_NAME, type_=SERVICE_TYPE)
        info._query_record_types = {_TYPE_TXT}
        zc = Mock(started=True, loop=asyncio.get_running_loop())
        self.assertFalse(await info.async_request(zc, timeout_ms=0))
        zc.async_remove_listener.assert_called_once_with(info)


if __name__ == "__main__":
    unittest.main()
