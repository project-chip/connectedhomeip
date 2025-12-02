
#include <pw_unit_test/framework.h>

#include <credentials/GroupcastDataProvider.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {
namespace app {
namespace TestGroups {

struct TestGroupcastDataProvider : public ::testing::Test
{
    static chip::TestPersistentStorageDelegate sStorage;
    static chip::Crypto::DefaultSessionKeystore sSessionKeystore;
    static Groupcast::DataProvider sProvider;

    static void SetUpTestSuite()
    {
        EXPECT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        sProvider.Initialize(&sStorage, &sSessionKeystore);
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

chip::TestPersistentStorageDelegate TestGroupcastDataProvider::sStorage;
chip::Crypto::DefaultSessionKeystore TestGroupcastDataProvider::sSessionKeystore;
Groupcast::DataProvider TestGroupcastDataProvider::sProvider;

GroupId GetGroupId(size_t n)
{
    return static_cast<GroupId>(0x100 + n);
}

void SetupGroup(Groupcast::Group & g, size_t n, uint16_t endpoints)
{
    g.group_id       = GetGroupId(n);
    g.endpoint_count = endpoints;
    g.key_id         = static_cast<KeysetId>(g.group_id * 10);
    for (size_t i = 0; (i < endpoints) && (i < Groupcast::kEndpointsMax); ++i)
    {
        g.endpoints[i] = static_cast<EndpointId>(i + 1);
    }
}

TEST_F(TestGroupcastDataProvider, TestAdd)
{
    FabricIndex fabric = 1;
    for (size_t i = 1; i <= sProvider.GetMaxMembershipCount(); ++i)
    {
        Groupcast::Group g;
        SetupGroup(g, i, Groupcast::kEndpointsMax);
        EXPECT_EQ(CHIP_NO_ERROR, sProvider.AddGroup(fabric, g));
    }
}

TEST_F(TestGroupcastDataProvider, TestIterator)
{
    FabricIndex fabric = 1;
    Groupcast::Group g, expected;
    size_t count = 0;

    Groupcast::DataProvider::GroupIterator * iter = sProvider.IterateGroups(fabric);
    ASSERT_NE(nullptr, iter);

    EXPECT_EQ(iter->Count(), sProvider.GetMaxMembershipCount());
    while (iter->Next(g))
    {
        SetupGroup(expected, ++count, Groupcast::kEndpointsMax);
        EXPECT_EQ(g.group_id, expected.group_id);
        EXPECT_EQ(g.key_id, expected.key_id);
        EXPECT_EQ(g.endpoint_count, expected.endpoint_count);
        for (size_t i = 0; i < g.endpoint_count; ++i)
        {
            EXPECT_EQ(g.endpoints[i], expected.endpoints[i]);
        }
    }
    EXPECT_EQ(count, sProvider.GetMaxMembershipCount());
    iter->Release();
}

TEST_F(TestGroupcastDataProvider, TestRemove)
{
    // Remove groups
    FabricIndex fabric   = 1;
    size_t removed_count = 0;
    for (size_t i = 1; i <= sProvider.GetMaxMembershipCount(); ++i)
    {
        GroupId gid = GetGroupId(i);
        if (0 == (gid % 3))
        {
            EXPECT_EQ(CHIP_NO_ERROR, sProvider.RemoveGroup(fabric, gid));
            removed_count++;
        }
    }

    // Verify removed
    Groupcast::DataProvider::GroupIterator * iter = sProvider.IterateGroups(fabric);
    ASSERT_NE(nullptr, iter);

    Groupcast::Group g;
    size_t expected_count = sProvider.GetMaxMembershipCount() - removed_count;
    size_t actual_count   = 0;
    EXPECT_EQ(iter->Count(), expected_count);
    while (iter->Next(g))
    {
        actual_count++;
        EXPECT_NE(0, g.group_id % 3);
        EXPECT_EQ(g.group_id * 10, g.key_id);
        EXPECT_EQ(g.endpoint_count, Groupcast::kEndpointsMax);
    }
    EXPECT_EQ(actual_count, expected_count);
    iter->Release();
}

TEST_F(TestGroupcastDataProvider, TestEditEndpoints)
{
    // Modify endpoints
    FabricIndex fabric = 1;
    GroupId target_id  = GetGroupId(3);
    Groupcast::Group g(target_id);
    EXPECT_EQ(CHIP_NO_ERROR, sProvider.GetGroup(fabric, g));
    g.endpoint_count = 3;
    for (size_t i = 0; i < g.endpoint_count; ++i)
    {
        g.endpoints[i] = static_cast<EndpointId>(i + 3);
    }
    EXPECT_EQ(CHIP_NO_ERROR, sProvider.SetEndpoints(fabric, g));

    // Verify modified
    Groupcast::DataProvider::GroupIterator * iter = sProvider.IterateGroups(fabric);
    ASSERT_NE(nullptr, iter);
    size_t modified_count = 0;
    while (iter->Next(g))
    {
        if (target_id == g.group_id)
        {
            // Modified group
            modified_count++;
            EXPECT_EQ(g.endpoint_count, 3);
            for (size_t i = 0; i < g.endpoint_count; ++i)
            {
                EXPECT_EQ(g.endpoints[i], i + 3);
            }
        }
        else
        {
            // Unmodified group
            EXPECT_EQ(g.endpoint_count, Groupcast::kEndpointsMax);
            for (size_t i = 0; i < g.endpoint_count; ++i)
            {
                EXPECT_EQ(g.endpoints[i], i + 1);
            }
        }
    }
    EXPECT_EQ(modified_count, static_cast<size_t>(1));
    iter->Release();
}

} // namespace TestGroups
} // namespace app
} // namespace chip
