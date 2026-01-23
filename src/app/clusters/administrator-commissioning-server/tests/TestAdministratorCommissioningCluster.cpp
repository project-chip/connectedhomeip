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
#include <app/tests/AppTestContext.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/AdministratorCommissioning/Enums.h>
#include <clusters/AdministratorCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestAdministratorCommissioningCluster : public chip::Testing::AppContext
{
};

TEST_F(TestAdministratorCommissioningCluster, TestAttributes)
{
    {
        AdministratorCommissioningCluster cluster(kRootEndpointId, {});

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                AdministratorCommissioning::Attributes::WindowStatus::kMetadataEntry,
                                                AdministratorCommissioning::Attributes::AdminFabricIndex::kMetadataEntry,
                                                AdministratorCommissioning::Attributes::AdminVendorId::kMetadataEntry,
                                            }));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId,
                                                                              BitFlags<Feature>{ Feature::kBasic });

        ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                AdministratorCommissioning::Attributes::WindowStatus::kMetadataEntry,
                                                AdministratorCommissioning::Attributes::AdminFabricIndex::kMetadataEntry,
                                                AdministratorCommissioning::Attributes::AdminVendorId::kMetadataEntry,
                                            }));
    }
}

TEST_F(TestAdministratorCommissioningCluster, TestCommands)
{
    {
        AdministratorCommissioningCluster cluster(kRootEndpointId, {});

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                                                      AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                                                  }));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId, BitFlags<Feature>{});

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                                                      AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                                                  }));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId,
                                                                              BitFlags<Feature>{ Feature::kBasic });

        EXPECT_TRUE(
            IsAcceptedCommandsListEqualTo(cluster,
                                          {
                                              AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                                              AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                                              AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::kMetadataEntry,
                                          }));
    }
}
class ScopedFailSafeDisarm
{
public:
    explicit ScopedFailSafeDisarm(FailSafeContext & ctx) : mCtx(ctx) {}
    ~ScopedFailSafeDisarm() { mCtx.DisarmFailSafe(); }

private:
    FailSafeContext & mCtx;
};

// This test ensures that calling RevokeCommissioning does NOT expire the fail-safe if it is NOT held by a PASE session. Which
// proves that the RevokeCommissioning command does not interfere with fail-safes that are not related to commissioning.
TEST_F(TestAdministratorCommissioningCluster, TestRevokeCommissioningDoesNotExpireFailSafeIfNotHeldByPASE)
{
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();
    ScopedFailSafeDisarm disarmFailSafe(failSafeContext);

    // Arming the fail-safe outside of the commissioning context
    ASSERT_SUCCESS(failSafeContext.ArmFailSafe(kUndefinedFabricIndex, System::Clock::Seconds16(60)));
    ASSERT_TRUE(failSafeContext.IsFailSafeArmed());

    AdministratorCommissioningLogic logic;
    AdministratorCommissioning::Commands::RevokeCommissioning::DecodableType unused;

    // RevokeCommissioning attempts to expire the fail-safe (when it is held by a PASE session) regardless of the commissioning
    // window state; therefore, for the sake of the test it is acceptable for it to return StatusCode::kWindowNotOpen instead of
    // Status::Success.
    ASSERT_EQ(logic.RevokeCommissioning(unused),
              chip::Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCode::kWindowNotOpen));

    // Ensure that the fail-safe is still armed
    // RevokeCommissioning should NOT expire the fail-safe since it is not held by a PASE session
    ASSERT_TRUE(failSafeContext.IsFailSafeArmed());
}
} // namespace
