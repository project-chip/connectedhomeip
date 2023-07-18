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


if __name__ == "__main__":
    default_matter_test_main()
