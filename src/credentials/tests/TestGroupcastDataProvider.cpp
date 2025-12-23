
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
        EXPECT_EQ(sProvider.Initialize(&sStorage, &sSessionKeystore), CHIP_NO_ERROR);
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

void SetupGroup(Groupcast::GroupData & g, size_t n, uint16_t endpoints)
{
    g.groupID       = GetGroupId(n);
    g.keyID         = static_cast<KeysetId>(g.groupID * 10);
    g.endpointCount = endpoints;
    for (size_t i = 0; (i < endpoints) && (i < Groupcast::kMaxEndpointsPerGroup); ++i)
    {
        g.endpoints[i] = static_cast<EndpointId>(i + 1);
    }
}

TEST_F(TestGroupcastDataProvider, TestAdd)
{
    FabricIndex fabric = 1;
    for (size_t i = 1; i <= sProvider.GetMaxMembershipCount(); ++i)
    {
        Groupcast::GroupData g;
        SetupGroup(g, i, Groupcast::kMaxEndpointsPerGroup);
        EXPECT_EQ(CHIP_NO_ERROR, sProvider.SetGroup(fabric, g));
    }
}

TEST_F(TestGroupcastDataProvider, TestIterator)
{
    FabricIndex fabric = 1;
    Groupcast::GroupData g, expected;
    size_t count = 0;

    auto result = sProvider.IterateGroups(fabric, [&](auto & iter) -> CHIP_ERROR {
        EXPECT_EQ(iter.Count(), sProvider.GetMaxMembershipCount());
        while (iter.Next(g))
        {
            SetupGroup(expected, ++count, Groupcast::kMaxEndpointsPerGroup);
            EXPECT_EQ(g.groupID, expected.groupID);
            EXPECT_EQ(g.keyID, expected.keyID);
            EXPECT_EQ(g.endpointCount, expected.endpointCount);
            for (size_t i = 0; i < g.endpointCount; ++i)
            {
                EXPECT_EQ(g.endpoints[i], expected.endpoints[i]);
            }
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(CHIP_NO_ERROR, result);
    EXPECT_EQ(count, sProvider.GetMaxMembershipCount());
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
    Groupcast::GroupData g;
    size_t expected_count = sProvider.GetMaxMembershipCount() - removed_count;
    size_t actual_count   = 0;
    auto result           = sProvider.IterateGroups(fabric, [&](auto & iter) -> CHIP_ERROR {
        EXPECT_EQ(iter.Count(), expected_count);
        while (iter.Next(g))
        {
            actual_count++;
            EXPECT_NE(0, g.groupID % 3);
            EXPECT_EQ(g.groupID * 10, g.keyID);
            EXPECT_EQ(g.endpointCount, Groupcast::kMaxEndpointsPerGroup);
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(CHIP_NO_ERROR, result);
    EXPECT_EQ(actual_count, expected_count);
}

TEST_F(TestGroupcastDataProvider, TestEditEndpoints)
{
    // Modify endpoints
    FabricIndex fabric = 1;
    GroupId target_id  = GetGroupId(3);
    Groupcast::GroupData g(target_id);
    EXPECT_EQ(CHIP_NO_ERROR, sProvider.GetGroup(fabric, g));
    g.endpointCount = 3;
    for (size_t i = 0; i < g.endpointCount; ++i)
    {
        g.endpoints[i] = static_cast<EndpointId>(i + 3);
    }
    EXPECT_EQ(CHIP_NO_ERROR, sProvider.SetEndpoints(fabric, g));

    // Verify modified
    size_t modified_count = 0;
    auto result           = sProvider.IterateGroups(fabric, [&](auto & iter) -> CHIP_ERROR {
        while (iter.Next(g))
        {
            if (target_id == g.groupID)
            {
                // Modified group
                modified_count++;
                EXPECT_EQ(g.endpointCount, 3);
                for (size_t i = 0; i < g.endpointCount; ++i)
                {
                    EXPECT_EQ(g.endpoints[i], i + 3);
                }
            }
            else
            {
                // Unmodified group
                EXPECT_EQ(g.endpointCount, Groupcast::kMaxEndpointsPerGroup);
                for (size_t i = 0; i < g.endpointCount; ++i)
                {
                    EXPECT_EQ(g.endpoints[i], i + 1);
                }
            }
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(CHIP_NO_ERROR, result);
    EXPECT_EQ(modified_count, static_cast<size_t>(1));
}

} // namespace TestGroups
} // namespace app
} // namespace chip
