/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <algorithm>
#include <array>
#include <stdio.h>

#include <lib/support/SortUtils.h>
#include <lib/support/Span.h>

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Sorting;

namespace {

struct Datum
{
    // Key is the element that takes part in sorting.
    int key;
    // The associated data is constructed to detect stable sort.
    int associated_data;

    bool operator==(const Datum & other) const
    {
        return (this == &other) || ((this->key == other.key) && (this->associated_data == other.associated_data));
    }
};

class Sorter
{
public:
    virtual ~Sorter() = default;

    void Reset() { m_compare_count = 0; }

    size_t compare_count() const { return m_compare_count; }

    virtual void Sort(chip::Span<Datum> data) = 0;

protected:
    size_t m_compare_count = 0;
};

class InsertionSorter : public Sorter
{
public:
    void Sort(chip::Span<Datum> data)
    {
        InsertionSort(data.data(), data.size(), [&](const Datum & a, const Datum & b) -> bool {
            ++m_compare_count;
            return a.key < b.key;
        });
    }
};

void DoBasicSortTest(nlTestSuite * inSuite, Sorter & sorter)
{
    Span<Datum> empty_to_sort;
    Span<Datum> empty_expected;

    std::array<Datum, 1> single_entry_to_sort{ { { 1, 100 } } };
    std::array<Datum, 1> single_entry_expected{ { { 1, 100 } } };

    std::array<Datum, 2> two_entries_to_sort{ { { 2, 200 }, { 1, 100 } } };
    std::array<Datum, 2> two_entries_expected{ { { 1, 100 }, { 2, 200 } } };

    std::array<Datum, 20> random_order_to_sort{ { { 1, 100 }, { 10, 1000 }, { 2, 200 },   { 13, 1300 }, { 4, 400 },
                                                  { 8, 800 }, { 10, 1001 }, { 6, 600 },   { 7, 700 },   { 7, 701 },
                                                  { 6, 601 }, { 6, 602 },   { 6, 603 },   { 8, 801 },   { 14, 1400 },
                                                  { 6, 604 }, { 10, 1002 }, { 12, 1200 }, { 4, 401 },   { 2, 201 } } };

    std::array<Datum, 20> random_order_expected{ { { 1, 100 },   { 2, 200 },   { 2, 201 },   { 4, 400 },   { 4, 401 },
                                                   { 6, 600 },   { 6, 601 },   { 6, 602 },   { 6, 603 },   { 6, 604 },
                                                   { 7, 700 },   { 7, 701 },   { 8, 800 },   { 8, 801 },   { 10, 1000 },
                                                   { 10, 1001 }, { 10, 1002 }, { 12, 1200 }, { 13, 1300 }, { 14, 1400 } } };

    std::array<Datum, 20> reverse_order_to_sort{ { { 20, 2000 }, { 19, 1900 }, { 18, 1800 }, { 17, 1700 }, { 16, 1600 },
                                                   { 15, 1500 }, { 14, 1400 }, { 13, 1300 }, { 12, 1200 }, { 11, 1100 },
                                                   { 10, 1000 }, { 9, 900 },   { 8, 800 },   { 7, 700 },   { 6, 600 },
                                                   { 5, 500 },   { 4, 400 },   { 3, 300 },   { 2, 200 },   { 1, 100 } } };

    std::array<Datum, 20> reverse_order_expected{ { { 1, 100 },   { 2, 200 },   { 3, 300 },   { 4, 400 },   { 5, 500 },
                                                    { 6, 600 },   { 7, 700 },   { 8, 800 },   { 9, 900 },   { 10, 1000 },
                                                    { 11, 1100 }, { 12, 1200 }, { 13, 1300 }, { 14, 1400 }, { 15, 1500 },
                                                    { 16, 1600 }, { 17, 1700 }, { 18, 1800 }, { 19, 1900 }, { 20, 2000 } } };

    std::array<Span<Datum>, 5> inputs = { { empty_to_sort, Span<Datum>(single_entry_to_sort), Span<Datum>(two_entries_to_sort),
                                            Span<Datum>(random_order_to_sort), Span<Datum>(reverse_order_to_sort) } };
    std::array<Span<Datum>, 5> expected_outs = { { empty_expected, Span<Datum>(single_entry_expected),
                                                   Span<Datum>(two_entries_expected), Span<Datum>(random_order_expected),
                                                   Span<Datum>(reverse_order_expected) } };

    for (size_t case_idx = 0; case_idx < inputs.size(); ++case_idx)
    {
        printf("Case index: %d\n", static_cast<int>(case_idx));
        auto & to_sort        = inputs[case_idx];
        const auto & expected = expected_outs[case_idx];

        sorter.Sort(to_sort);
        NL_TEST_ASSERT(inSuite, to_sort.data_equal(expected));
        if (!to_sort.data_equal(expected))
        {
            for (size_t idx = 0; idx < to_sort.size(); ++idx)
            {
                Datum sorted_item   = to_sort[idx];
                Datum expected_item = expected[idx];
                printf("Index: %d, got { %d, %d }, expected { %d, %d }\n", static_cast<int>(idx), sorted_item.key,
                       sorted_item.associated_data, expected_item.key, expected_item.associated_data);
            }
        }
        NL_TEST_ASSERT(inSuite, sorter.compare_count() <= (to_sort.size() * to_sort.size()));
        printf("Compare counts: %d\n", static_cast<int>(sorter.compare_count()));
        sorter.Reset();
    }
}

void TestBasicSort(nlTestSuite * inSuite, void * inContext)
{
    printf("Testing insertion sorter.\n");
    InsertionSorter insertion_sorter;
    DoBasicSortTest(inSuite, insertion_sorter);
}

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Basic sort tests for custom sort utilities", TestBasicSort),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestSortUtils()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Test for SortUtils",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSortUtils)
