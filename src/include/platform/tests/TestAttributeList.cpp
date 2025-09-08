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
    EXPECT_EQ(list.size(), (size_t) 0);
    EXPECT_EQ(list.begin(), list.end());

    // This will abort/die
    // list[0];
}

// Test adding elements and overflow
TEST_F(TestAttributeList, TestAddAndOverflow)
{
    AttributeList<int, 5> list;

    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), (size_t) 1);

    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), (size_t) 2);

    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), (size_t) 3);

    EXPECT_EQ(list.add(40), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), (size_t) 4);

    EXPECT_EQ(list.add(50), CHIP_NO_ERROR);
    EXPECT_EQ(list.size(), (size_t) 5);

    EXPECT_EQ(list.add(60), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(list.size(), (size_t) 5);
}

// Test operator[]
TEST_F(TestAttributeList, TestIndexing)
{
    AttributeList<int, 5> list;

    EXPECT_EQ(list.add(10), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(20), CHIP_NO_ERROR);
    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);

    ASSERT_EQ(list.size(), (size_t) 3);

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
    EXPECT_NE(list.begin(), list.end());

    EXPECT_EQ(list.add(30), CHIP_NO_ERROR);
    EXPECT_NE(list.begin(), list.end());

    ASSERT_EQ(list.size(), (size_t) 3);

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

} // namespace DeviceLayer
} // namespace chip
