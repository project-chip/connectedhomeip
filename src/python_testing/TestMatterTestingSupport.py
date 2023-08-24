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

import time
import typing
from datetime import datetime, timedelta, timezone

import chip.clusters as Clusters
from chip.clusters.Types import Nullable, NullValue
from chip.tlv import uint
from matter_testing_support import (MatterBaseTest, async_test_body, compare_time, default_matter_test_main,
                                    get_wait_seconds_from_set_time, parse_pics, type_matches, utc_time_in_matter_epoch)
from mobly import asserts, signals
from TC_DeviceBasicComposition import find_tree_roots, get_all_children, parts_list_cycles, separate_endpoint_types


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
                     'lower.s.a0000=1',
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
        asserts.assert_true(self.check_pics("LOWER.S.A0000"), "PICS pased incorrectly for LOWER.S.A0000")
        asserts.assert_true(self.check_pics("SPACE.S.A0000"), "PICS parsed incorrectly for SPACE.S.A0000")
        asserts.assert_false(self.check_pics("NOT.S.A0000"), "PICS parsed incorrectly for NOT.S.A0000")
        asserts.assert_true(self.check_pics(" test.s.a0000"), "PICS checker lowercase handled incorrectly")

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
                                             Clusters.Descriptor.Attributes.DeviceTypeList: device_types_structs}
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

    def test_get_all_children(self):
        endpoints = self.create_example_topology()
        asserts.assert_equal(get_all_children(2, endpoints), [1, 3, 4, 5, 9], "Child list for ep2 is incorrect")
        asserts.assert_equal(get_all_children(6, endpoints), [7, 8], "Child list for ep6 is incorrect")
        asserts.assert_equal(get_all_children(13, endpoints), [12, 14, 15, 16], "Child list for ep13 is incorrect")
        asserts.assert_equal(get_all_children(17, endpoints), [18, 19], "Child list for ep17 is incorrect")

    def test_get_tree_roots(self):
        endpoints = self.create_example_topology()
        _, tree = separate_endpoint_types(endpoints)
        asserts.assert_equal(find_tree_roots(tree, endpoints), {2, 6, 13, 17}, "Incorrect tree root list")


if __name__ == "__main__":
    default_matter_test_main()
