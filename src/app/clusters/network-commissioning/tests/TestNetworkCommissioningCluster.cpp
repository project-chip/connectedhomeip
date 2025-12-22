/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app/AttributePathParams.h>
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/GeneralCommissioning/Attributes.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Metadata.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/NetworkCommissioning.h>
#include <vector>

#include "FakeDrivers.h"

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning::Attributes;

using chip::app::AttributeValueDecoder;
using chip::app::ClusterShutdownType;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::ClusterTester;
using chip::Testing::IsAttributesListEqualTo;

class NoopBreadcrumbTracker : public BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t v) override {}
};
// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestNetworkCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestNetworkCommissioningCluster, TestAttributes)
{
    NoopBreadcrumbTracker tracker;
    {
        Testing::FakeWiFiDriver fakeWifiDriver;

        NetworkCommissioningCluster cluster(kRootEndpointId, &fakeWifiDriver, tracker);

        // NOTE: this is AWKWARD: we pass in a wifi driver, yet attributes are still depending
        //       on device enabling. Ideally we should not allow compiling odd things at all.
        //       For now keep the logic as inherited from previous implementation.
        std::vector<AttributeEntry> expectedAttributes;
        expectedAttributes.push_back(MaxNetworks::kMetadataEntry);
        expectedAttributes.push_back(Networks::kMetadataEntry);
        expectedAttributes.push_back(InterfaceEnabled::kMetadataEntry);
        expectedAttributes.push_back(LastNetworkingStatus::kMetadataEntry);
        expectedAttributes.push_back(LastNetworkID::kMetadataEntry);
        expectedAttributes.push_back(LastConnectErrorValue::kMetadataEntry);

#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        expectedAttributes.push_back(ScanMaxTimeSeconds::kMetadataEntry);
        expectedAttributes.push_back(ConnectMaxTimeSeconds::kMetadataEntry);
        expectedAttributes.push_back(SupportedWiFiBands::kMetadataEntry);
#endif

        ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
    }

    // TODO: more tests for ethernet and thread should be added
}

TEST_F(TestNetworkCommissioningCluster, TestNotifyOnEnableInterface)
{
    Testing::FakeWiFiDriver fakeWifiDriver;
    NoopBreadcrumbTracker tracker;
    NetworkCommissioningCluster cluster(kRootEndpointId, &fakeWifiDriver, tracker);

    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    {
        // no notification if enable fails
        tester.GetDirtyList().clear();
        fakeWifiDriver.SetEnabledAllowed(false);
        ASSERT_FALSE(tester.WriteAttribute(InterfaceEnabled::Id, false).IsSuccess());
        ASSERT_TRUE(tester.GetDirtyList().empty());
    }

    {
        // Receive a notification if enable succeeds
        tester.GetDirtyList().clear();
        fakeWifiDriver.SetEnabledAllowed(true);
        ASSERT_TRUE(tester.WriteAttribute(InterfaceEnabled::Id, true).IsSuccess());
        ASSERT_EQ(tester.GetDirtyList().size(), 1u);
        ASSERT_EQ(tester.GetDirtyList()[0],
                  app::AttributePathParams(kRootEndpointId, NetworkCommissioning::Id, InterfaceEnabled::Id));
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
