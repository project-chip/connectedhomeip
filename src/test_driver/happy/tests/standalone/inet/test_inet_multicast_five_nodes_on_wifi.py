#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2016-2017 Nest Labs, Inc.
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

##
#    @file
#       Calls CHIP Inet Multicast test among sender and receiver nodes.
#

import itertools
import os
import unittest
import set_test_path

from happy.Utils import *
import happy.HappyNodeList

import ChipStateLoad
import ChipStateUnload
import ChipUtilities
import ChipInetMulticast


class test_chip_inet_multicast_five_nodes_on_wifi(unittest.TestCase):
    def setUp(self):
        self.using_lwip = False

        self.topology_file = os.path.join(os.path.dirname(os.path.realpath(
            __file__)), "../../../topologies/standalone/five_nodes_on_wifi.json")
        self.interface = "wlan0"
        self.tap = None

        self.show_strace = False

        options = ChipStateLoad.option()
        options["quiet"] = True
        options["json_file"] = self.topology_file

        setup_network = ChipStateLoad.ChipStateLoad(options)
        ret = setup_network.run()

    def tearDown(self):
        """cleaning up"""
        options = ChipStateUnload.option()
        options["quiet"] = True
        options["json_file"] = self.topology_file

        teardown_network = ChipStateUnload.ChipStateUnload(options)
        teardown_network.run()

    def test_chip_inet_multicast(self):
        options = happy.HappyNodeList.option()
        options["quiet"] = True

        # This test validates UDP over IPv6 with one sender and four receivers.
        # Each receiver varies in the number of multicast groups it participates
        # in.
        #
        # The ipv4-local-addr configuration key-value pairs are only
        # used for LwIP hosted OS topologies where a network tap
        # device is configured.

        configuration = {
            'sender': {
                '00-WifiNode-TxFourMulticastAddresses': {
                    'groups': [
                        {
                            'id': 1,
                            'send': 1,
                            'receive': 0
                        },
                        {
                            'id': 2,
                            'send': 2,
                            'receive': 0
                        },
                        {
                            'id': 3,
                            'send': 3,
                            'receive': 0
                        },
                        {
                            'id': 4,
                            'send': 5,
                            'receive': 0
                        },
                    ],
                    'tap-ipv4-local-addr': "192.168.1.0"
                }
            },
            'receivers': {
                '01-WifiNode-RxOneMulticastAddress': {
                    'groups': [
                         {
                             'id': 1,
                             'send': 0,
                             'receive': 1
                         }
                    ],
                    'tap-ipv4-local-addr': "192.168.1.1"
                },
                '02-WifiNode-RxTwoMulticastAddresses': {
                    'groups': [
                        {
                            'id': 1,
                            'send': 0,
                            'receive': 1
                        },
                        {
                            'id': 2,
                            'send': 0,
                            'receive': 2
                        }
                    ],
                    'tap-ipv4-local-addr': "192.168.1.2"
                },
                '03-WifiNode-RxThreeMulticastAddresses': {
                    'groups': [
                        {
                            'id': 1,
                            'send': 0,
                            'receive': 1
                        },
                        {
                            'id': 2,
                            'send': 0,
                            'receive': 2
                        },
                        {
                            'id': 3,
                            'send': 0,
                            'receive': 3
                        }
                    ],
                    'tap-ipv4-local-addr': "192.168.1.3"
                },
                '04-WifiNode-RxFourMulticastAddresses': {
                    'groups': [
                        {
                            'id': 1,
                            'send': 0,
                            'receive': 1
                        },
                        {
                            'id': 2,
                            'send': 0,
                            'receive': 2
                        },
                        {
                            'id': 3,
                            'send': 0,
                            'receive': 3
                        },
                        {
                            'id': 4,
                            'send': 0,
                            'receive': 5
                        }
                    ],
                    'tap-ipv4-local-addr': "192.168.1.4"
                }
            }
        }

        # Topology-independent test parameters:
        TEST_TRANSPORT = "udp"
        TEST_IP_NETWORK = "6"

        # Run the test.
        value, data = self.__run_inet_multicast_test(
            configuration, self.interface, TEST_IP_NETWORK, TEST_TRANSPORT)

        # Process and report the results.
        self.__process_result(
            configuration, self.interface, TEST_IP_NETWORK, TEST_TRANSPORT, value, data)

    def __process_result(self, configuration, interface, network, transport, value, data):
        nodes = len(configuration['sender']) + len(configuration['receivers'])

        print("Inet multicast test using %sIPv%s w/ device interface: %s (w/%s LwIP) with %u nodes:" % ("UDP/" if transport ==
                                                                                                        "udp" else "", network, "<none>" if interface == None else interface, "" if self.using_lwip else "o", nodes))

        if value:
            print("PASSED")
        else:
            print("FAILED")
            raise ValueError("Chip Inet Multicast Test Failed")

    def __run_inet_multicast_test(self, configuration, interface, network, transport):
        """ Run Inet Multicast test on configured topology
        The default interval is 1 s (1000 ms). This is a good
        default for interactive test operation; however, for
        automated continuous integration, we prefer it to run much
        faster. Consequently, use 250 ms as the interval.
        """

        options = ChipInetMulticast.option()
        options["quiet"] = False
        options["configuration"] = configuration
        options["interface"] = interface
        options["ipversion"] = network
        options["transport"] = transport
        options["interval"] = str(250)
        options["tap"] = self.tap

        chip_inet_multicast = ChipInetMulticast.ChipInetMulticast(options)
        ret = chip_inet_multicast.run()

        value = ret.Value()
        data = ret.Data()

        return value, data


if __name__ == "__main__":
    ChipUtilities.run_unittest()
