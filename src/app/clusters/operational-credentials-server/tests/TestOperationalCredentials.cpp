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

#include <app/clusters/operational-credentials-server/OperationalCredentialsCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/server/Server.h>
#include <clusters/OperationalCredentials/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace {

using namespace chip;
using namespace chip::app;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestOperationalCredentials : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestOperationalCredentials, TestAttributes)
{
    OperationalCredentialsCluster::Context context = { .fabricTable     = Server::GetInstance().GetFabricTable(),
                                                       .failSafeContext = Server::GetInstance().GetFailSafeContext(),
                                                       .sessionManager  = Server::GetInstance().GetSecureSessionManager(),
                                                       .dnssdServer     = app::DnssdServer::Instance(),
                                                       .commissioningWindowManager =
                                                           Server::GetInstance().GetCommissioningWindowManager() };
    OperationalCredentialsCluster cluster(kRootEndpointId, context);

    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            OperationalCredentials::Attributes::NOCs::kMetadataEntry,
                                            OperationalCredentials::Attributes::Fabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::SupportedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::CommissionedFabrics::kMetadataEntry,
                                            OperationalCredentials::Attributes::TrustedRootCertificates::kMetadataEntry,
                                            OperationalCredentials::Attributes::CurrentFabricIndex::kMetadataEntry,
                                        }));
}

TEST_F(TestOperationalCredentials, TestCommands)
{
    OperationalCredentialsCluster::Context context = { .fabricTable     = Server::GetInstance().GetFabricTable(),
                                                       .failSafeContext = Server::GetInstance().GetFailSafeContext(),
                                                       .sessionManager  = Server::GetInstance().GetSecureSessionManager(),
                                                       .dnssdServer     = app::DnssdServer::Instance(),
                                                       .commissioningWindowManager =
                                                           Server::GetInstance().GetCommissioningWindowManager() };
    OperationalCredentialsCluster cluster(kRootEndpointId, context);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  OperationalCredentials::Commands::AttestationRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::CertificateChainRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::CSRRequest::kMetadataEntry,
                                                  OperationalCredentials::Commands::AddNOC::kMetadataEntry,
                                                  OperationalCredentials::Commands::UpdateNOC::kMetadataEntry,
                                                  OperationalCredentials::Commands::UpdateFabricLabel::kMetadataEntry,
                                                  OperationalCredentials::Commands::RemoveFabric::kMetadataEntry,
                                                  OperationalCredentials::Commands::AddTrustedRootCertificate::kMetadataEntry,
                                                  OperationalCredentials::Commands::SetVIDVerificationStatement::kMetadataEntry,
                                                  OperationalCredentials::Commands::SignVIDVerificationRequest::kMetadataEntry,
                                              }));
}

} // namespace
