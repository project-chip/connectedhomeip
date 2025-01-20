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

#include <cstdint>
#include <functional>
#include <utility>

#include <pw_unit_test/framework.h>

#include <app/data-model-provider/MetadataList.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::app::DataModel;

namespace {

class TestMetadataList : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

template <typename T>
struct IdAndValue
{
    uint32_t id;
    T value;

    bool operator==(const IdAndValue & other) const
    {
        return (this == &other) || ((this->id == other.id) && (this->value == other.value));
    }
};

TEST_F(TestMetadataList, ListBuilderWorks)
{
    ListBuilder<IdAndValue<int>> list1;
    EXPECT_EQ(list1.Size(), 0u);
    EXPECT_TRUE(list1.IsEmpty());

    ASSERT_EQ(list1.EnsureAppendCapacity(3), CHIP_NO_ERROR);
    EXPECT_EQ(list1.Size(), 0u);
    EXPECT_TRUE(list1.IsEmpty());

    ASSERT_EQ(list1.EnsureAppendCapacity(2), CHIP_NO_ERROR);

    // Values can be appended until the capacity.
    EXPECT_EQ(list1.Append({ 0xA1, 111 }), CHIP_NO_ERROR);
    EXPECT_EQ(list1.Size(), 1u);

    EXPECT_EQ(list1.Append({ 0xA2, 222 }), CHIP_NO_ERROR);
    EXPECT_EQ(list1.Size(), 2u);

    // capacity is 3 because of the largest ensure
    EXPECT_EQ(list1.Append({ 0xA3, 333 }), CHIP_NO_ERROR);
    EXPECT_EQ(list1.Size(), 3u);

    EXPECT_EQ(list1.Append({ 0xA4, 444 }), CHIP_ERROR_BUFFER_TOO_SMALL);
    EXPECT_EQ(list1.Size(), 3u);

    ListBuilder<IdAndValue<int>> list2 = std::move(list1);

    // Moved-from list is "empty", un-Metadata and span is empty.
    EXPECT_EQ(list1.Size(), 0u);             // NOLINT(bugprone-use-after-move)
    EXPECT_TRUE(list1.IsEmpty());            // NOLINT(bugprone-use-after-move)
    EXPECT_TRUE(list1.TakeBuffer().empty()); // NOLINT(bugprone-use-after-move)

    // Moved-to list has storage.
    EXPECT_EQ(list2.Size(), 3u);
    EXPECT_FALSE(list2.IsEmpty());

    // A span can be obtained over the list.
    auto contents = list2.TakeBuffer();
    EXPECT_EQ(contents.size(), 3u);

    // contents takes ownersip of the list and clears it (and has no capacity)
    EXPECT_TRUE(list2.IsEmpty());
    EXPECT_EQ(list2.Append({ 1, 2 }), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t idx = 0;
    for (const auto & element : contents)
    {
        size_t oneBasedIndex = idx + 1;
        EXPECT_EQ(element.id, 0xA0u + oneBasedIndex);
        EXPECT_EQ(element.value, 111 * static_cast<int>(oneBasedIndex));
        ++idx;
    }
    EXPECT_EQ(idx, 3u);
}

TEST_F(TestMetadataList, ListBuilderConvertersWorks)
{
    {
        ListBuilder<int> list;
        std::array<int, 3> kArray{ 1, 2, 3 };
        EXPECT_EQ(list.ReferenceExisting(Span<const int>(kArray)), CHIP_NO_ERROR);

        auto list2 = std::move(list);
        EXPECT_EQ(list.Size(), 0u); // NOLINT(bugprone-use-after-move)

        auto list2Span = list2.TakeBuffer();
        EXPECT_TRUE(list2.IsEmpty()); // took over
        EXPECT_EQ(list2Span.size(), 3u);
        EXPECT_EQ(list2Span[0], 1);
        EXPECT_EQ(list2Span[1], 2);
        EXPECT_EQ(list2Span[2], 3);
    }

    {
        ListBuilder<int> list;
        std::array<int, 3> kArray{ 1, 2, 3 };
        std::array<int, 3> kArray2{ 4, 5, 6 };
        EXPECT_EQ(list.ReferenceExisting(Span<const int>(kArray)), CHIP_NO_ERROR);
        EXPECT_EQ(list.ReferenceExisting(Span<const int>(kArray2)), CHIP_NO_ERROR);

        auto list2 = std::move(list);
        EXPECT_EQ(list.Size(), 0u); // NOLINT(bugprone-use-after-move)

        auto list2Span = list2.TakeBuffer();
        EXPECT_TRUE(list2.IsEmpty()); // took over
        EXPECT_EQ(list2Span.size(), 6u);
        EXPECT_EQ(list2Span[0], 1);
        EXPECT_EQ(list2Span[1], 2);
        EXPECT_EQ(list2Span[2], 3);
        EXPECT_EQ(list2Span[3], 4);
        EXPECT_EQ(list2Span[4], 5);
        EXPECT_EQ(list2Span[5], 6);
    }

    {
        ListBuilder<int> list;

        EXPECT_EQ(list.Append(10), CHIP_ERROR_BUFFER_TOO_SMALL);
        EXPECT_EQ(list.EnsureAppendCapacity(5), CHIP_NO_ERROR);

        EXPECT_EQ(list.Append(10), CHIP_NO_ERROR);
        EXPECT_EQ(list.Append(11), CHIP_NO_ERROR);

        std::array<int, 3> kArray{ 1, 2, 3 };

        EXPECT_EQ(list.ReferenceExisting(Span<const int>(kArray)), CHIP_NO_ERROR);

        auto list2 = std::move(list);
        EXPECT_EQ(list.Size(), 0u); // NOLINT(bugprone-use-after-move)

        auto list2Span = list2.TakeBuffer();
        EXPECT_TRUE(list2.IsEmpty()); // took over
        EXPECT_EQ(list2Span.size(), 5u);
        EXPECT_EQ(list2Span[0], 10);
        EXPECT_EQ(list2Span[1], 11);
        EXPECT_EQ(list2Span[2], 1);
        EXPECT_EQ(list2Span[3], 2);
        EXPECT_EQ(list2Span[4], 3);
    }
    {
        ListBuilder<int> list;

        EXPECT_EQ(list.Append(10), CHIP_ERROR_BUFFER_TOO_SMALL);
        EXPECT_EQ(list.EnsureAppendCapacity(1), CHIP_NO_ERROR);

        EXPECT_EQ(list.Append(10), CHIP_NO_ERROR);
        EXPECT_EQ(list.Append(11), CHIP_ERROR_BUFFER_TOO_SMALL);

        std::array<int, 3> kArray{ 1, 2, 3 };

        EXPECT_EQ(list.AppendElements(Span<const int>(kArray)), CHIP_NO_ERROR);
        EXPECT_EQ(list.ReferenceExisting(Span<const int>(kArray)), CHIP_NO_ERROR);

        auto list2 = std::move(list);
        EXPECT_EQ(list.Size(), 0u); // NOLINT(bugprone-use-after-move)

        auto list2Span = list2.TakeBuffer();
        EXPECT_TRUE(list2.IsEmpty()); // took over
        EXPECT_EQ(list2Span.size(), 7u);
        EXPECT_EQ(list2Span[0], 10);
        EXPECT_EQ(list2Span[1], 1);
        EXPECT_EQ(list2Span[2], 2);
        EXPECT_EQ(list2Span[3], 3);
        EXPECT_EQ(list2Span[4], 1);
        EXPECT_EQ(list2Span[5], 2);
        EXPECT_EQ(list2Span[6], 3);
    }
}

} // namespace
