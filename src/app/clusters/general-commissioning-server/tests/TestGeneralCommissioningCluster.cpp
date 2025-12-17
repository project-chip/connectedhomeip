/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app/clusters/general-commissioning-server/GeneralCommissioningCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/server/Server.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <clusters/GeneralCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <platform/DeviceControlServer.h>
#include <platform/NetworkCommissioning.h>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestGeneralCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

GeneralCommissioningCluster::Context CreateStandardContext()
{
    return {
        .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(), //
        .configurationManager       = DeviceLayer::ConfigurationMgr(),                       //
        .deviceControlServer        = DeviceLayer::DeviceControlServer::DeviceControlSvr(),  //
        .fabricTable                = Server::GetInstance().GetFabricTable(),                //
        .failsafeContext            = Server::GetInstance().GetFailSafeContext(),            //
        .platformManager            = DeviceLayer::PlatformMgr(),                            //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    };
}

TEST_F(TestGeneralCommissioningCluster, TestAttributes)
{
    // test without optional attributes
    {
        GeneralCommissioningCluster cluster(CreateStandardContext(), GeneralCommissioningCluster::OptionalAttributes(0));

        std::vector<AttributeEntry> expectedAttributes = {
            Breadcrumb::kMetadataEntry,         BasicCommissioningInfo::kMetadataEntry,       RegulatoryConfig::kMetadataEntry,
            LocationCapability::kMetadataEntry, SupportsConcurrentConnection::kMetadataEntry,
        };

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        // This is both define AND feature map dependent. The feature map is hardcoded
        // based on compile-time defines, so we check the define directly.
        expectedAttributes.insert(expectedAttributes.end(),
                                  {
                                      TCAcceptedVersion::kMetadataEntry,
                                      TCMinRequiredVersion::kMetadataEntry,
                                      TCAcknowledgements::kMetadataEntry,
                                      TCAcknowledgementsRequired::kMetadataEntry,
                                      TCUpdateDeadline::kMetadataEntry,
                                  });
#endif

        ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
    }

    // test with optional attributes
    {
        GeneralCommissioningCluster cluster(
            CreateStandardContext(), GeneralCommissioningCluster::OptionalAttributes().Set<IsCommissioningWithoutPower::Id>());

        std::vector<AttributeEntry> expectedAttributes = {
            Breadcrumb::kMetadataEntry,         BasicCommissioningInfo::kMetadataEntry,       RegulatoryConfig::kMetadataEntry,
            LocationCapability::kMetadataEntry, SupportsConcurrentConnection::kMetadataEntry,
        };

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        // This is both define AND feature map dependent. The feature map is hardcoded
        // based on compile-time defines, so we check the define directly.
        expectedAttributes.insert(expectedAttributes.end(),
                                  {
                                      TCAcceptedVersion::kMetadataEntry,
                                      TCMinRequiredVersion::kMetadataEntry,
                                      TCAcknowledgements::kMetadataEntry,
                                      TCAcknowledgementsRequired::kMetadataEntry,
                                      TCUpdateDeadline::kMetadataEntry,
                                  });
#endif

        expectedAttributes.push_back(IsCommissioningWithoutPower::kMetadataEntry);

        ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));
    }
}

TEST_F(TestGeneralCommissioningCluster, TestAcceptedCommands)
{
    GeneralCommissioningCluster cluster(CreateStandardContext(), GeneralCommissioningCluster::OptionalAttributes());

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::ArmFailSafe::kMetadataEntry,
                                                  Commands::SetRegulatoryConfig::kMetadataEntry,
                                                  Commands::CommissioningComplete::kMetadataEntry,
                                                  Commands::SetTCAcknowledgements::kMetadataEntry,
                                              }));
#else
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::ArmFailSafe::kMetadataEntry,
                                                  Commands::SetRegulatoryConfig::kMetadataEntry,
                                                  Commands::CommissioningComplete::kMetadataEntry,
                                              }));
#endif
}

} // namespace
