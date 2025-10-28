#include <pw_unit_test/framework.h>

#include <platform/AttributeList.h>

namespace chip {
namespace DeviceLayer {

class TestAttributeList : public ::testing::Test
{
};

// Test an empty list
TEST_F(TestAttributeList, TestEmpty)
{
    AttributeList<int, 5> list;
    EXPECT_EQ(list.size(), 0u);
    EXPECT_EQ(list.begin(), list.end());

    // This will abort/die
    // list[0];
}

// Test adding elements and overflow
TEST_F(TestAttributeList, TestAddAndOverflow)
{
    AttributeList<int, 5> list;

    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), 1u);

    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), 2u);

    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), 3u);

    EXPECT_EQ(list.add(40), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), 4u);

    EXPECT_EQ(list.add(50), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), 5u);

    EXPECT_EQ(list.add(60), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(list.size(), 5u);
}

// Test operator[]
TEST_F(TestAttributeList, TestIndexing)
{
    AttributeList<int, 5> list;

    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);

    ASSERT_EQ(list.size(), 3u);

    EXPECT_EQ(list[0], 10);
    EXPECT_EQ(list[1], 20);
    EXPECT_EQ(list[2], 30);

    // This will abort/die
    // list[3];
}

// Test the iterator methods
TEST_F(TestAttributeList, TestIterator)
{
    AttributeList<int, 5> list;

    EXPECT_EQ(list.begin(), list.end());

    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_NE(list.begin(), list.end());

    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);

    ASSERT_EQ(list.size(), 3u);

    auto it = list.begin();
    EXPECT_EQ(*it, 10);

    ++it;
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 20);

    ++it;
    EXPECT_EQ(*it, 30);

    ++it;
    EXPECT_EQ(it, list.end());
};

// Test range-based for loop
TEST_F(TestAttributeList, TestRangeBasedFor)
{
    AttributeList<int, 5> list;
    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);

    int expected_values[] = { 10, 20, 30 };
    int i                 = 0;
    for (const auto & val : list)
    {
        ASSERT_LT(i, 3);
        EXPECT_EQ(val, expected_values[i]);
        i++;
    }
    EXPECT_EQ(i, 3);
}

// Test iterators from different lists
TEST_F(TestAttributeList, TestIteratorFromDifferentLists)
{
    AttributeList<int, 5> list1;
    AttributeList<int, 5> list2;

    // Both iterators will have index 0, but point to different lists.
    // They should not be equal.
    EXPECT_NE(list1.begin(), list2.begin());

    EXPECT_EQ(list1.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list2.add(20), CHIP_NO_ERROR);

    // Both iterators will have index 0, but point to different lists.
    EXPECT_NE(list1.begin(), list2.begin());
}

} // namespace DeviceLayer
} // namespace chip
