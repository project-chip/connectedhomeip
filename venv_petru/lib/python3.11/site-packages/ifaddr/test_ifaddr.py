# Copyright (C) 2015 Stefan C. Mueller

import unittest

import pytest

import ifaddr
import ifaddr.netifaces


try:
    import netifaces
except ImportError:
    skip_netifaces = True
else:
    skip_netifaces = False


class TestIfaddr(unittest.TestCase):
    """
    Unittests for :mod:`ifaddr`.

    There isn't much unit-testing that can be done without making assumptions
    on the system or mocking of operating system APIs. So this just contains
    a sanity check for the moment.
    """

    def test_get_adapters_contains_localhost(self) -> None:

        found = False
        adapters = ifaddr.get_adapters()
        for adapter in adapters:
            for ip in adapter.ips:
                if ip.ip == "127.0.0.1":
                    found = True

        self.assertTrue(found, "No adapter has IP 127.0.0.1: %s" % str(adapters))


@pytest.mark.skipif(skip_netifaces, reason='netifaces not installed')
def test_netifaces_compatibility() -> None:
    interfaces = ifaddr.netifaces.interfaces()
    assert interfaces == netifaces.interfaces()
    # TODO: implement those as well
    # for interface in interfaces:
    #     print(interface)
    #     assert ifaddr.netifaces.ifaddresses(interface) == netifaces.ifaddresses(interface)
    # assert ifaddr.netifaces.gateways() == netifaces.gateways()
