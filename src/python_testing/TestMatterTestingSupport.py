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

import os
import time
import typing
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.testing.matter_testing import (MatterBaseTest, async_test_body, default_matter_test_main, parse_matter_test_args,
                                         type_matches)
from chip.testing.pics import parse_pics, parse_pics_xml
from chip.testing.taglist_and_topology_test import (TagProblem, create_device_type_list_for_root, create_device_type_lists,
                                                    find_tag_list_problems, find_tree_roots, flat_list_ok, get_all_children,
                                                    get_direct_children_of_root, parts_list_cycles, separate_endpoint_types)
from chip.testing.timeoperations import compare_time, get_wait_seconds_from_set_time, utc_time_in_matter_epoch
from chip.tlv import uint
from mobly import asserts, signals


def get_raw_type_list():
    test = Clusters.UnitTesting
    struct = test.Structs.SimpleStruct()
    struct_type = test.Structs.SimpleStruct
    null_opt_struct = test.Structs.NullablesAndOptionalsStruct()
    null_opt_struct_type = test.Structs.NullablesAndOptionalsStruct
    double_nested_struct_list = test.Structs.DoubleNestedStructList()
    double_nested_struct_list_type = test.Structs.DoubleNestedStructList
    list_of_uints = [0, 1]
    list_of_uints_type = typing.List[uint]
    list_of_structs = [struct, struct]
    list_of_structs_type = typing.List[struct_type]
    list_of_double_nested_struct_list = [double_nested_struct_list, double_nested_struct_list]
    list_of_double_nested_struct_list_type = typing.List[double_nested_struct_list_type]

    # Create a list with all the types and a list of the values that should match for that type
    vals = {uint: [1],
            str: ["str"],
            struct_type: [struct],
            null_opt_struct_type: [null_opt_struct],
            double_nested_struct_list_type: [double_nested_struct_list],
            list_of_uints_type: [list_of_uints],
            list_of_structs_type: [list_of_structs],
            list_of_double_nested_struct_list_type: [list_of_double_nested_struct_list]}
    return vals


def test_type_matching_for_type(test_type, test_nullable: bool = False, test_optional: bool = False):
    vals = get_raw_type_list()

    if test_nullable and test_optional:
        match_type = typing.Union[Nullable, None, test_type]
    elif test_nullable:
        match_type = typing.Union[Nullable, test_type]
    elif test_optional:
        match_type = typing.Optional[test_type]
    else:
        match_type = test_type

    true_list = vals[test_type]
    if test_nullable:
        true_list.append(NullValue)
    if test_optional:
        true_list.append(None)

    del vals[test_type]

    # true_list is all the values that should match with the test type
    for i in true_list:
        asserts.assert_true(type_matches(i, match_type), "{} type checking failure".format(test_type))

    # try every value in every type in the remaining dict - they should all fail
    for v in vals.values():
        for i in v:
            asserts.assert_false(type_matches(i, match_type), "{} falsely matched to type {}".format(i, match_type))

    # Test the nullables or optionals that aren't supposed to work
    if not test_nullable:
        asserts.assert_false(type_matches(NullValue, match_type), "NullValue falsely matched to {}".format(match_type))

    if not test_optional:
        asserts.assert_false(type_matches(None, match_type), "None falsely matched to {}".format(match_type))


def run_all_match_tests_for_type(test_type):
    test_type_matching_for_type(test_type=test_type)
    test_type_matching_for_type(test_type=test_type, test_nullable=True)
    test_type_matching_for_type(test_type=test_type, test_optional=True)
    test_type_matching_for_type(test_type=test_type, test_nullable=True, test_optional=True)


class TestMatterTestingSupport(MatterBaseTest):
    @async_test_body
    async def test_matter_epoch_time(self):
        # Matter epoch should return zero
        ret = utc_time_in_matter_epoch(datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc))
        asserts.assert_equal(ret, 0, "UTC epoch returned non-zero value")

        # Jan 2 is exactly 1 day after Jan 1
        ret = utc_time_in_matter_epoch(datetime(2000, 1, 2, 0, 0, 0, 0, timezone.utc))
        expected_delay = timedelta(days=1)
        actual_delay = timedelta(microseconds=ret)
        asserts.assert_equal(expected_delay, actual_delay, "Calculation for Jan 2 date is incorrect")

        # There's a catch 22 for knowing the current time, but we can check that it's
        # going up, and that it's larger than when I wrote the test
        # Check that the returned value is larger than the test writing date
        writing_date = utc_time_in_matter_epoch(datetime(2023, 5, 5, 0, 0, 0, 0, timezone.utc))
        current_date = utc_time_in_matter_epoch()
        asserts.assert_greater(current_date, writing_date, "Calculation for current date is smaller than writing date")

        # Check that the time is going up
        last_date = current_date
        current_date = utc_time_in_matter_epoch()
        asserts.assert_greater(current_date, last_date, "Time does not appear to be incrementing")

    @async_test_body
    async def test_type_checking(self):
        vals = get_raw_type_list()
        for k in vals.keys():
            run_all_match_tests_for_type(k)

    @async_test_body
    async def test_pics_support(self):
        pics_list = ['TEST.S.A0000=1',
                     'TEST.S.A0001=0',
                     'TEST.S.A000a=1'
                     '',
                     ' ',
                     '# comment',
                     ' # comment',
                     ' SPACE.S.A0000 = 1']
        pics = parse_pics(pics_list)
        # force the parsed pics here to be in the config so we can check the check_pics function
        self.matter_test_config.pics = pics

        asserts.assert_true(self.check_pics("TEST.S.A0000"), "PICS parsed incorrectly for TEST.S.A0000")
        asserts.assert_false(self.check_pics("TEST.S.A0001"), "PICS parsed incorrectly for TEST.S.A0001")
        asserts.assert_true(self.check_pics("TEST.S.A000a"), "PICS parsed incorrectly for TEST.S.A000a")
        asserts.assert_true(self.check_pics("SPACE.S.A0000"), "PICS parsed incorrectly for SPACE.S.A0000")
        asserts.assert_false(self.check_pics("NOT.S.A0000"), "PICS parsed incorrectly for NOT.S.A0000")

        # invalid pics file should throw a value error
        pics_list.append("BAD.S.A000=5")
        try:
            pics = parse_pics(pics_list)
            asserts.assert_false(True, "PICS parser did not throw an error as expected")
        except ValueError:
            pass

    def test_time_compare_function(self):
        # only offset, exact match
        compare_time(received=1000, offset=timedelta(microseconds=1000), utc=0, tolerance=timedelta())
        # only utc, exact match
        compare_time(received=1000, offset=timedelta(), utc=1000, tolerance=timedelta())
        # both, exact match
        compare_time(received=2000, offset=timedelta(microseconds=1000), utc=1000, tolerance=timedelta())
        # both, negative offset
        compare_time(received=0, offset=timedelta(microseconds=-1000), utc=1000, tolerance=timedelta())

        # Exact match, within delta, both
        compare_time(received=2000, offset=timedelta(microseconds=1000), utc=1000, tolerance=timedelta(seconds=5))

        # Just inside tolerance
        compare_time(received=1001, offset=timedelta(), utc=2000, tolerance=timedelta(microseconds=1000))

        # Just outside tolerance
        try:
            compare_time(received=999, offset=timedelta(), utc=2000, tolerance=timedelta(microseconds=1000))
            asserts.fail("Expected failure case for time just outside of the tolerance failed")
        except signals.TestFailure:
            pass

        # everything in the seconds range
        compare_time(received=timedelta(seconds=3600).total_seconds() * 1000000,
                     offset=timedelta(seconds=3605), utc=0, tolerance=timedelta(seconds=5))

    def test_get_wait_time_function(self):
        th_utc = utc_time_in_matter_epoch()
        secs = get_wait_seconds_from_set_time(th_utc, 5)
        asserts.assert_equal(secs, 5)
        # If we've pass less than a second, we still want to wait 5
        time.sleep(0.5)
        secs = get_wait_seconds_from_set_time(th_utc, 5)
        asserts.assert_equal(secs, 5)

        time.sleep(0.5)
        secs = get_wait_seconds_from_set_time(th_utc, 5)
        asserts.assert_equal(secs, 4)
        secs = get_wait_seconds_from_set_time(th_utc, 15)
        asserts.assert_equal(secs, 14)

    def create_example_topology(self):
        """Creates a limited example of a wildcard read that contains only the descriptor cluster parts list and device types"""
        def create_endpoint(parts_list: list[uint], device_types: list[uint]):
            endpoint = {}
            device_types_structs = []
            for device_type in device_types:
                device_types_structs.append(Clusters.Descriptor.Structs.DeviceTypeStruct(deviceType=device_type, revision=1))
            endpoint[Clusters.Descriptor] = {Clusters.Descriptor.Attributes.PartsList: parts_list,
                                             Clusters.Descriptor.Attributes.DeviceTypeList: device_types_structs,
                                             Clusters.Descriptor.Attributes.FeatureMap: 0}
            return endpoint

        endpoints = {}
        # Root node is 0
        # We have two trees in the root node and two trees in the aggregator
        # 2 - 1
        #   - 3 - 4
        #       - 5 - 9
        # 6 - 7
        #   - 8
        # 10
        # 11 (aggregator - all remaining are under it)
        # 13 - 12
        #    - 14 - 15
        #         - 16
        # 17 - 18
        #    - 19
        # 20
        # 21
        endpoints[0] = create_endpoint([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21], [22])
        endpoints[1] = create_endpoint([], [1])  # Just using a random device id, as long as it's not the aggregator it's fine
        endpoints[2] = create_endpoint([1, 3], [1])
        endpoints[3] = create_endpoint([4, 5], [1])
        endpoints[4] = create_endpoint([], [1])
        endpoints[5] = create_endpoint([9], [1])
        endpoints[6] = create_endpoint([7, 8], [1])
        endpoints[7] = create_endpoint([], [1])
        endpoints[8] = create_endpoint([], [1])
        endpoints[9] = create_endpoint([], [1])
        endpoints[10] = create_endpoint([], [1])
        endpoints[11] = create_endpoint([12, 13, 14, 15, 16, 17, 18, 19, 20, 21], [0xe])  # aggregator device type
        endpoints[12] = create_endpoint([], [1])
        endpoints[13] = create_endpoint([12, 14], [1])
        endpoints[14] = create_endpoint([15, 16], [1])
        endpoints[15] = create_endpoint([], [1])
        endpoints[16] = create_endpoint([], [1])
        endpoints[17] = create_endpoint([18, 19], [1])
        endpoints[18] = create_endpoint([], [1])
        endpoints[19] = create_endpoint([], [1])
        endpoints[20] = create_endpoint([], [1])
        endpoints[21] = create_endpoint([], [1])

        return endpoints

    def test_cycle_detection_and_splitting(self):
        # Example topology has no cycles
        endpoints = self.create_example_topology()
        flat, tree = separate_endpoint_types(endpoints)
        asserts.assert_equal(len(flat), len(set(flat)), "Duplicate endpoints found in flat list")
        asserts.assert_equal(len(tree), len(set(tree)), "Duplicate endpoints found in tree list")
        asserts.assert_equal(set(flat), {11}, "Aggregator node not found in list")
        asserts.assert_equal(set(tree), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21})

        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(len(cycles), 0, "Found cycles in the example tree")

        # Add in several cycles and make sure we detect them all
        # ep 10 refers back to itself (0 level cycle) on 10
        endpoints[10][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(10)
        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(cycles, [10])
        endpoints[10][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].remove(10)
        print(endpoints[10])

        # ep 4 refers back to 3 (1 level cycle) on 3 (will include 2, 3 and 4 in the cycles list)
        endpoints[4][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(3)
        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(cycles, [2, 3, 4])
        endpoints[4][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].remove(3)

        # ep 16 refers back to 13 (2 level cycle) on 13 (will include 13, 14 and 16 in cycles)
        endpoints[16][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(13)
        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(cycles, [13, 14, 16])
        endpoints[16][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].remove(13)

        # ep 9 refers back to 2 (3 level cycle) on 2 (includes 2, 3, 5, and 9)
        endpoints[9][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(2)
        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(cycles, [2, 3, 5, 9])
        endpoints[9][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].remove(2)

        # make sure we get them all
        endpoints[10][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(10)
        endpoints[4][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(3)
        endpoints[16][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(13)
        endpoints[9][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(2)
        cycles = parts_list_cycles(tree, endpoints)
        asserts.assert_equal(cycles, [2, 3, 4, 5, 9, 10, 13, 14, 16])

    def test_flat_list(self):
        endpoints = self.create_example_topology()
        # check the aggregator endpoint to ensure it's ok - aggregator is on 11
        asserts.assert_true(flat_list_ok(11, endpoints), "Incorrect failure on flat list")
        # Remove one of the sub-children endpoints from the parts list - it should fail
        endpoints[11][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].remove(14)
        asserts.assert_false(flat_list_ok(11, endpoints), "Incorrect pass on flat list missing a part list entry")

    def test_get_all_children(self):
        endpoints = self.create_example_topology()
        asserts.assert_equal(get_all_children(2, endpoints), {1, 3, 4, 5, 9}, "Child list for ep2 is incorrect")
        asserts.assert_equal(get_all_children(6, endpoints), {7, 8}, "Child list for ep6 is incorrect")
        asserts.assert_equal(get_all_children(13, endpoints), {12, 14, 15, 16}, "Child list for ep13 is incorrect")
        asserts.assert_equal(get_all_children(17, endpoints), {18, 19}, "Child list for ep17 is incorrect")

    def test_get_tree_roots(self):
        endpoints = self.create_example_topology()
        _, tree = separate_endpoint_types(endpoints)
        asserts.assert_equal(find_tree_roots(tree, endpoints), {2, 6, 13, 17}, "Incorrect tree root list")

    def test_tag_list_problems(self):
        # Right now, the whole endpoint list uses the same device id except for ep11, which is an aggregator
        # The relevant trees are
        # 2 - 1
        #   - 3 - 4
        #       - 5 - 9
        #
        # 6 - 7
        #   - 8
        #
        # 13 - 12
        #    - 14 - 15
        #         - 16
        #
        # 17 - 18
        #    - 19

        endpoints = self.create_example_topology()
        # First test, everything in every tree has the same device type, so the device lists
        # should contain all the device endpoints
        _, tree = separate_endpoint_types(endpoints)
        roots = find_tree_roots(tree, endpoints)
        device_types = create_device_type_lists(roots, endpoints)
        asserts.assert_equal(set(roots), set(device_types.keys()), "Device types list does not match roots list")
        for root in roots:
            asserts.assert_equal({1}, set(device_types[root].keys()), "Unexpected device type found in device type list")

        asserts.assert_equal(device_types[2][1], {2, 1, 3, 4, 5, 9}, "device type list for ep 2 is incorrect")
        asserts.assert_equal(device_types[6][1], {6, 7, 8}, "device type list for ep 6 is incorrect")
        asserts.assert_equal(device_types[13][1], {13, 12, 14, 15, 16}, "device type list for ep 13 is incorrect")
        asserts.assert_equal(device_types[17][1], {17, 18, 19}, "device type list for ep 17 is incorrect")

        # every single one of these should have the same problem - they have no tags
        problems = find_tag_list_problems(roots, device_types, endpoints)
        expected_problems = {2, 1, 3, 4, 5, 9, 6, 7, 8, 13, 12, 14, 15, 16, 17, 18, 19}
        asserts.assert_equal(set(problems.keys()), expected_problems, "Incorrect set of tag problems")
        for root in roots:
            eps = get_all_children(root, endpoints)
            eps.add(root)
            for ep in eps:
                expected_problem = TagProblem(root=root, missing_attribute=True,
                                              missing_feature=True, duplicates=set(eps), same_tag=set())
                asserts.assert_equal(problems[ep], expected_problem, f"Incorrect problem for ep {ep}")

        # Add the feature for every endpoint, but not the attribute
        for ep in expected_problems:
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.FeatureMap] = 1
        problems = find_tag_list_problems(roots, device_types, endpoints)
        for root in roots:
            eps = get_all_children(root, endpoints)
            eps.add(root)
            for ep in eps:
                expected_problem = TagProblem(root=root, missing_attribute=True,
                                              missing_feature=False, duplicates=set(eps), same_tag=set())
                asserts.assert_equal(problems[ep], expected_problem, f"Incorrect problem for ep {ep}")

        # Add empty tag lists
        for ep in expected_problems:
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = []
        problems = find_tag_list_problems(roots, device_types, endpoints)
        for root in roots:
            eps = get_all_children(root, endpoints)
            eps.add(root)
            for ep in eps:
                expected_problem = TagProblem(root=root, missing_attribute=True,
                                              missing_feature=False, duplicates=set(eps), same_tag=set())
                asserts.assert_equal(problems[ep], expected_problem, f"Incorrect problem for ep {ep}")

        # Add a tag list on every one of these, but make it the same tag
        tag = Clusters.Descriptor.Structs.SemanticTagStruct()
        for ep in expected_problems:
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag]
        problems = find_tag_list_problems(roots, device_types, endpoints)
        for root in roots:
            eps = get_all_children(root, endpoints)
            eps.add(root)
            for ep in eps:
                expected_problem = TagProblem(root=root, missing_attribute=False,
                                              missing_feature=False, duplicates=set(eps), same_tag=set(eps))
                asserts.assert_equal(problems[ep], expected_problem, f"Incorrect problem for ep {ep}")

        # swap out all the tags lists so they're all different - we should get no problems
        for ep in expected_problems:
            tag = Clusters.Descriptor.Structs.SemanticTagStruct(tag=ep)
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag]
        problems = find_tag_list_problems(roots, device_types, endpoints)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # Remove all the feature maps, we should get all errors again
        for ep in expected_problems:
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.FeatureMap] = 0
        problems = find_tag_list_problems(roots, device_types, endpoints)
        for root in roots:
            eps = get_all_children(root, endpoints)
            eps.add(root)
            for ep in eps:
                expected_problem = TagProblem(root=root, missing_attribute=False,
                                              missing_feature=True, duplicates=set(eps))
                asserts.assert_equal(problems[ep], expected_problem, f"Incorrect problem for ep {ep}")

        # Create a simple two-tree system where everything is OK, but the tags are the same between the trees (should be ok)
        # 1 (dt 1) - 2 (dt 2) - tag 2
        #          - 3 (dt 2) - tag 3
        # 4 (dt 1) - 5 (dt 2) - tag 2
        #          - 6 (dt 2) - tag 3
        desc_dt2_tag2 = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                         Clusters.Descriptor.Attributes.PartsList: [],
                         Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(2, 1)],
                         Clusters.Descriptor.Attributes.TagList: [Clusters.Descriptor.Structs.SemanticTagStruct(tag=2)]
                         }
        desc_dt2_tag3 = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                         Clusters.Descriptor.Attributes.PartsList: [],
                         Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(2, 1)],
                         Clusters.Descriptor.Attributes.TagList: [Clusters.Descriptor.Structs.SemanticTagStruct(tag=3)]
                         }
        desc_ep1 = {Clusters.Descriptor.Attributes.FeatureMap: 0,
                    Clusters.Descriptor.Attributes.PartsList: [2, 3],
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(1, 1)],
                    }
        desc_ep4 = {Clusters.Descriptor.Attributes.FeatureMap: 0,
                    Clusters.Descriptor.Attributes.PartsList: [5, 6],
                    Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(1, 1)],
                    }
        new_endpoints = {}
        new_endpoints[1] = {Clusters.Descriptor: desc_ep1}
        new_endpoints[2] = {Clusters.Descriptor: desc_dt2_tag2}
        new_endpoints[3] = {Clusters.Descriptor: desc_dt2_tag3}
        new_endpoints[4] = {Clusters.Descriptor: desc_ep4}
        new_endpoints[5] = {Clusters.Descriptor: desc_dt2_tag2}
        new_endpoints[6] = {Clusters.Descriptor: desc_dt2_tag3}

        _, tree = separate_endpoint_types(new_endpoints)
        roots = find_tree_roots(tree, new_endpoints)
        device_types = create_device_type_lists(roots, new_endpoints)

        problems = find_tag_list_problems(roots, device_types, new_endpoints)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # Create a simple tree where ONE of the tags in the set matches, but not the other - should be no problems
        # 1 (dt 1) - 2 (dt 2) - tag 2,3
        #          - 3 (dt 2) - tag 2,4
        desc_dt2_tag23 = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                          Clusters.Descriptor.Attributes.PartsList: [],
                          Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(2, 1)],
                          Clusters.Descriptor.Attributes.TagList: [Clusters.Descriptor.Structs.SemanticTagStruct(
                              tag=2), Clusters.Descriptor.Structs.SemanticTagStruct(tag=3)]
                          }
        desc_dt2_tag24 = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                          Clusters.Descriptor.Attributes.PartsList: [],
                          Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(2, 1)],
                          Clusters.Descriptor.Attributes.TagList: [Clusters.Descriptor.Structs.SemanticTagStruct(
                              tag=2), Clusters.Descriptor.Structs.SemanticTagStruct(tag=4)]
                          }
        simple = {}
        simple[1] = {Clusters.Descriptor: desc_ep1}
        simple[2] = {Clusters.Descriptor: desc_dt2_tag23}
        simple[3] = {Clusters.Descriptor: desc_dt2_tag24}

        _, tree = separate_endpoint_types(simple)
        roots = find_tree_roots(tree, simple)
        device_types = create_device_type_lists(roots, simple)

        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # now both match, but the ordering is different - this SHOULD be a problem
        desc_dt2_tag32 = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                          Clusters.Descriptor.Attributes.PartsList: [],
                          Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(2, 1)],
                          Clusters.Descriptor.Attributes.TagList: [Clusters.Descriptor.Structs.SemanticTagStruct(
                              tag=3), Clusters.Descriptor.Structs.SemanticTagStruct(tag=2)]
                          }
        simple[3] = {Clusters.Descriptor: desc_dt2_tag32}

        problems = find_tag_list_problems(roots, device_types, simple)
        # expect this problem reported on both 2 and 3 endpoints
        expected_problem = TagProblem(root=1, missing_attribute=False, missing_feature=False, duplicates={2, 3}, same_tag={2, 3})
        asserts.assert_true(2 in problems.keys(), "Missing problem report for ep2")
        asserts.assert_true(3 in problems.keys(), "Missing problem report for ep3")
        asserts.assert_equal(problems[2], expected_problem, "Problem report for simple EP2 is not as expected")
        asserts.assert_equal(problems[3], expected_problem, "Problem report for simple EP3 is not as expected")

        # Let's check that we're correctly checking all the pieces of the tag
        # Different mfgcode
        simple[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            Clusters.Descriptor.Structs.SemanticTagStruct(mfgCode=1)]
        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [Clusters.Descriptor.Structs.SemanticTagStruct()]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            Clusters.Descriptor.Structs.SemanticTagStruct(mfgCode=2)]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # Different namespace ids
        simple[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            Clusters.Descriptor.Structs.SemanticTagStruct(namespaceID=1)]
        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [Clusters.Descriptor.Structs.SemanticTagStruct()]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # Different labels
        simple[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            Clusters.Descriptor.Structs.SemanticTagStruct(label="test")]
        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [Clusters.Descriptor.Structs.SemanticTagStruct()]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
            Clusters.Descriptor.Structs.SemanticTagStruct(label="test1")]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # One tag list is a subset of the other - this should pass
        tag1 = Clusters.Descriptor.Structs.SemanticTagStruct(tag=1)
        tag2 = Clusters.Descriptor.Structs.SemanticTagStruct(tag=2)
        tag3 = Clusters.Descriptor.Structs.SemanticTagStruct(tag=3)

        simple[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag1, tag2]
        simple[3][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag1, tag2, tag3]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

        # Tags with mfg tags
        tag_mfg = Clusters.Descriptor.Structs.SemanticTagStruct(mfgCode=0xFFF1, label="test")
        tag_label = Clusters.Descriptor.Structs.SemanticTagStruct(tag=1, label="test")
        simple[1][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag1, tag_mfg]
        simple[2][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [tag1, tag_label]
        problems = find_tag_list_problems(roots, device_types, simple)
        asserts.assert_equal(len(problems), 0, "Unexpected problems found in list")

    def test_root_node_tag_list_functions(self):
        # Example topology - see comment above for the layout.
        # There are 4 direct children of root 0
        # node 2, node 6 and node 10 all have device ID 1
        # node 11 is an aggregator
        endpoints = self.create_example_topology()
        expected = {2, 6, 10, 11}
        direct = get_direct_children_of_root(endpoints)
        asserts.assert_equal(expected, direct, 'Incorrect list of direct children returned from root')

        # add a new child endpoint that's an aggregator on EP 20
        aggregator_desc = {Clusters.Descriptor.Attributes.FeatureMap: 1,
                           Clusters.Descriptor.Attributes.PartsList: [],
                           Clusters.Descriptor.Attributes.DeviceTypeList: [Clusters.Descriptor.Structs.DeviceTypeStruct(0xe)],
                           }
        endpoints[22] = {Clusters.Descriptor: aggregator_desc}
        endpoints[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.PartsList].append(22)
        expected.add(22)
        direct = get_direct_children_of_root(endpoints)
        asserts.assert_equal(expected, direct, 'Incorrect list of direct children returned from root')

        device_type_list = create_device_type_list_for_root(direct, endpoints)
        asserts.assert_equal(len(device_type_list), 2, 'Incorrect number of device types returned in root device type list')
        expected_device_types = {1, 0xe}
        asserts.assert_equal(set(device_type_list.keys()), expected_device_types, 'Unexpected device type list returned')
        expected_eps_dt1 = {2, 6, 10}
        asserts.assert_equal(set(device_type_list[1]), expected_eps_dt1, 'Unexpected endpoint list for DT1')
        expected_eps_dte = {11, 22}
        asserts.assert_equal(set(device_type_list[0xe]), expected_eps_dte, 'Unexpected endpoint list for DT 0xe')

        problems = find_tag_list_problems(roots=[0], device_types={0: device_type_list}, endpoint_dict=endpoints)

        # NONE of the endpoints currently have tags, so they should ALL be reported as having problems
        expected_problems = {2, 6, 10, 11, 22}
        asserts.assert_equal(set(problems.keys()), expected_problems, "Unexpected problem list returned for root node")

        # Let's add correct tags to everything and make sure we get no problems reported.
        # the various problems are tested individually in the above test case, so the intent is to ensure this also
        # works for the root
        for ep in expected_problems:
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.TagList] = [
                Clusters.Descriptor.Structs.SemanticTagStruct(namespaceID=ep)]
            endpoints[ep][Clusters.Descriptor][Clusters.Descriptor.Attributes.FeatureMap] = 1
        problems = find_tag_list_problems(roots=[0], device_types={0: device_type_list}, endpoint_dict=endpoints)
        asserts.assert_equal(len(problems.keys()), 0, 'Unexpected problems found in root endpoint')

    def pics_assert(self, pics: str, support: bool):
        asserts.assert_equal(self.check_pics(pics), support,
                             f'Unexpected PICS value for {pics} - expected {support}, got {self.check_pics(pics)}')

    def test_xml_pics(self):
        script_dir = os.path.dirname(os.path.realpath(__file__))
        with open(f'{script_dir}/test_testing/example_pics_xml_basic_info.xml') as f:
            pics = parse_pics_xml(f.read())
        print(pics)
        # force the parsed pics here to be in the config so we can check the check_pics function
        self.matter_test_config.pics = pics
        self.pics_assert('BINFO.S', True)
        self.pics_assert('BINFO.S.A0000', True)
        self.pics_assert('BINFO.S.A0001', True)
        self.pics_assert('BINFO.S.A0002', True)
        self.pics_assert('BINFO.S.A0003', True)
        self.pics_assert('BINFO.S.A0004', True)
        self.pics_assert('BINFO.S.A0005', True)
        self.pics_assert('BINFO.S.A0006', True)
        self.pics_assert('BINFO.S.A0007', True)
        self.pics_assert('BINFO.S.A0008', True)
        self.pics_assert('BINFO.S.A0009', True)
        self.pics_assert('BINFO.S.A000a', True)
        self.pics_assert('BINFO.S.A000b', True)
        self.pics_assert('BINFO.S.A000c', True)
        self.pics_assert('BINFO.S.A000d', True)
        self.pics_assert('BINFO.S.A000e', True)
        self.pics_assert('BINFO.S.A000f', True)
        self.pics_assert('BINFO.S.A0010', True)
        self.pics_assert('BINFO.S.A0011', False)
        self.pics_assert('BINFO.S.A0012', True)
        self.pics_assert('BINFO.S.A0013', False)
        self.pics_assert('BINFO.S.A0014', False)
        self.pics_assert('PICSDOESNOTEXIST', False)

    def test_parse_matter_test_args(self):
        args = [
            # Verify that it is possible to pass multiple test cases at once
            "--tests", "TC_1", "TC_2",
            # Verify that values are appended to a single argument
            "--int-arg", "PIXIT.TEST.DEC:42",
            "--int-arg", "PIXIT.TEST.HEX:0x1234",
            # Verify that multiple values can be passed for a single argument
            "--string-arg", "PIXIT.TEST.STR.MULTI.1:foo", "PIXIT.TEST.STR.MULTI.2:bar",
            # Verify JSON parsing
            "--json-arg", "PIXIT.TEST.JSON:{\"key\":\"value\"}",
        ]

        parsed = parse_matter_test_args(args)
        asserts.assert_equal(parsed.tests, ["TC_1", "TC_2"])
        asserts.assert_equal(parsed.global_test_params.get("PIXIT.TEST.DEC"), 42)
        asserts.assert_equal(parsed.global_test_params.get("PIXIT.TEST.HEX"), 0x1234)
        asserts.assert_equal(parsed.global_test_params.get("PIXIT.TEST.STR.MULTI.1"), "foo")
        asserts.assert_equal(parsed.global_test_params.get("PIXIT.TEST.STR.MULTI.2"), "bar")
        asserts.assert_equal(parsed.global_test_params.get("PIXIT.TEST.JSON"), {"key": "value"})


if __name__ == "__main__":
    default_matter_test_main()
