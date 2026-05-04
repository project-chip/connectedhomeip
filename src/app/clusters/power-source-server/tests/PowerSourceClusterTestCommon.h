/*
 *    Copyright (c) 2026 Project CHIP Authors
 */

#pragma once

#include <pw_unit_test/framework.h>

#include <app/clusters/power-source-server/PowerSourceCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/support/TimerDelegateMock.h>

namespace chip::app::Clusters::PowerSource::TestSupport {

struct TestBase : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    template<typename AttrTypeInfo>
    void ReadAttribute(chip::Testing::ClusterTester & tester)
    {
        typename AttrTypeInfo::DecodableType value{};
        EXPECT_EQ(tester.ReadAttribute(AttrTypeInfo::GetAttributeId(), value), CHIP_NO_ERROR);
    }

    template<typename AttrTypeInfo>
    void TestStringAttributeReadLength(chip::Testing::ClusterTester & tester)
    {
        typename AttrTypeInfo::DecodableType value{};
        ASSERT_EQ(tester.ReadAttribute(AttrTypeInfo::GetAttributeId(), value), CHIP_NO_ERROR);
        EXPECT_LE(value.size(), AttrTypeInfo::MaxLength());
    }

    template<typename ClusterType>
    void TestOrderPersistence(typename ClusterType::ConfigType & config)
    {
        uint8_t testOrder = 3;
        {
            ClusterType cluster(config);
            ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

            EXPECT_EQ(cluster.SetOrder(testOrder), CHIP_NO_ERROR);
            EXPECT_EQ(cluster.GetOrder(), testOrder);

            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }

        // test that order is persisted
        {
            ClusterType cluster(config);
            ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

            chip::Testing::ClusterTester tester(cluster);
            uint8_t readOrder{};
            ASSERT_EQ(tester.ReadAttribute(PowerSource::Attributes::Order::Id, readOrder), CHIP_NO_ERROR);
            EXPECT_EQ(readOrder, testOrder);
            EXPECT_EQ(cluster.GetOrder(), testOrder);

            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }

        // test that setting orderAttributeFetchFromPersistentStorageDuringStartup to false keeps the order value from persisting
        {
            uint8_t testOrderNewValue = 123;
            config.orderAttributeFetchFromPersistentStorageDuringStartup = false;
            config.order = testOrderNewValue;
            ClusterType cluster(config);

            // this will not get the value from persistent storage.
            ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

            chip::Testing::ClusterTester tester(cluster);
            uint8_t readOrder{};
            ASSERT_EQ(tester.ReadAttribute(PowerSource::Attributes::Order::Id, readOrder), CHIP_NO_ERROR);
            EXPECT_NE(readOrder, testOrder); // should not be the old value
            EXPECT_EQ(readOrder, testOrderNewValue);

            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }

    chip::Testing::TestServerClusterContext testContext;
    chip::TimerDelegateMock timerDelegate;
    EndpointId kTestEndpointId = 1;
};

} // namespace chip::app::Clusters::PowerSource::TestSupport
