#!/usr/bin/env -S python3 -B
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

import logging
import os
import sys
import unittest

from TC_IDM_10_7 import TC_IDM_10_7

import matter.clusters as Clusters

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))


def make_descriptor_cluster(parts_list=None):
    if parts_list is None:
        parts_list = []
    return {
        Clusters.Descriptor.Attributes.PartsList: parts_list
    }


def make_ep0(include_network_commissioning=False, parts_list=None):
    ep0 = {
        Clusters.Descriptor: make_descriptor_cluster(parts_list),
        Clusters.AccessControl: {},
        Clusters.BasicInformation: {},
        Clusters.GeneralCommissioning: {},
        Clusters.OperationalCredentials: {},
    }

    if include_network_commissioning:
        ep0[Clusters.NetworkCommissioning] = {}

    return ep0


class TestTCIDM107MandatoryClustersPresence(unittest.TestCase):

    def setUp(self):
        self.tc = TC_IDM_10_7.__new__(TC_IDM_10_7)

        self.pics = {}
        self.pics['MCORE.DD.QR'] = False
        self.pics['MCORE.DD.MANUAL_PC'] = False
        self.pics['MCORE.DD.NFC'] = True

        self.tc.check_pics = lambda key: self.pics.get(key, False)

    def assert_missing_ep0_cluster_fails(self, cluster):
        ep0 = make_ep0(include_network_commissioning=False, parts_list=[1])
        del ep0[cluster]

        self.tc.endpoints = {
            0: ep0,
            1: {Clusters.Descriptor: make_descriptor_cluster([])}
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_success_without_network_commissioning_requirement(self):
        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1, 2]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
            2: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertTrue(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_endpoints_missing(self):
        if hasattr(self.tc, 'endpoints'):
            delattr(self.tc, 'endpoints')

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_endpoints_is_none(self):
        self.tc.endpoints = None

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_endpoint_0_missing(self):
        self.tc.endpoints = {
            1: {Clusters.Descriptor: make_descriptor_cluster([])}
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_descriptor_missing_on_ep0(self):
        self.assert_missing_ep0_cluster_fails(Clusters.Descriptor)

    def test_failure_when_access_control_missing_on_ep0(self):
        self.assert_missing_ep0_cluster_fails(Clusters.AccessControl)

    def test_failure_when_basic_information_missing_on_ep0(self):
        self.assert_missing_ep0_cluster_fails(Clusters.BasicInformation)

    def test_failure_when_general_commissioning_missing_on_ep0(self):
        self.assert_missing_ep0_cluster_fails(Clusters.GeneralCommissioning)

    def test_failure_when_operational_credentials_missing_on_ep0(self):
        self.assert_missing_ep0_cluster_fails(Clusters.OperationalCredentials)

    def test_failure_when_network_commissioning_required_by_wifi_but_missing(self):
        self.pics['MCORE.COM.WIFI'] = True

        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_success_when_network_commissioning_required_by_wifi_and_present(self):
        self.pics['MCORE.COM.WIFI'] = True

        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=True, parts_list=[1]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertTrue(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_network_commissioning_required_by_thread_but_missing(self):
        self.pics['MCORE.COM.THR'] = True

        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_success_when_network_commissioning_required_by_thread_and_present(self):
        self.pics['MCORE.COM.THR'] = True

        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=True, parts_list=[1]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertTrue(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_endpoint_in_parts_list_is_missing(self):
        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1, 2]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_failure_when_child_endpoint_missing_descriptor(self):
        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1]),
            1: {Clusters.BasicInformation: {}},
        }

        self.assertFalse(self.tc._check_mandatory_clusters_presence())

    def test_success_when_parts_list_missing_defaults_to_empty(self):
        self.tc.endpoints = {
            0: {
                Clusters.Descriptor: {},
                Clusters.AccessControl: {},
                Clusters.BasicInformation: {},
                Clusters.GeneralCommissioning: {},
                Clusters.OperationalCredentials: {},
            }
        }

        self.assertTrue(self.tc._check_mandatory_clusters_presence())

    def test_success_when_parts_list_contains_endpoint_0(self):
        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[0, 1]),
            1: {Clusters.Descriptor: make_descriptor_cluster([])},
        }

        self.assertTrue(self.tc._check_mandatory_clusters_presence())

    def test_failure_on_exception(self):
        self.tc.endpoints = {
            0: make_ep0(include_network_commissioning=False, parts_list=[1])
        }

        def raise_exception(_):
            raise RuntimeError("Injected test exception")

        self.tc.check_pics = raise_exception

        self.assertFalse(self.tc._check_mandatory_clusters_presence())


if __name__ == "__main__":
    unittest.main()
