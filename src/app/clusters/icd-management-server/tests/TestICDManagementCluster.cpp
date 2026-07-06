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

#include <app/clusters/icd-management-server/ICDManagementCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/IcdManagement/Enums.h>
#include <clusters/IcdManagement/Metadata.h>
#include <credentials/FabricTable.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace chip::app::Clusters::IcdManagement::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestIcdManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestIcdManagementCluster, TestAttributes)
{
    // Create test instances
    chip::Crypto::DefaultSessionKeystore keystore;
    FabricTable fabricTable;
    ICDConfigurationData & icdConfig = ICDConfigurationData::GetInstance();

    BitMask<IcdManagement::OptionalCommands> optionalCommands =
        BitMask<IcdManagement::OptionalCommands>(IcdManagement::OptionalCommands::kStayActive);
    BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerHint(0);

    // Since ICDManagementClusterWithCIP is under the #if CHIP_CONFIG_ENABLE_ICD_CIP, we need to test both cases.
#if CHIP_CONFIG_ENABLE_ICD_CIP
    ICDManagementClusterWithCIP cluster(kRootEndpointId, keystore, fabricTable, icdConfig,
                                        OptionalAttributeSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id),
                                        optionalCommands, userActiveModeTriggerHint, CharSpan());
#else
    ICDManagementCluster cluster(kRootEndpointId, keystore, fabricTable, icdConfig,
                                 OptionalAttributeSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id),
                                 optionalCommands, userActiveModeTriggerHint, CharSpan());
#endif

    // Calculate expected attributes based on feature map and configuration
    BitFlags<IcdManagement::Feature> featureMap = icdConfig.GetFeatureMap();
    bool hasCIP                                 = featureMap.Has(IcdManagement::Feature::kCheckInProtocolSupport);
    bool hasUAT                                 = featureMap.Has(IcdManagement::Feature::kUserActiveModeTrigger);
    bool hasLIT                                 = featureMap.Has(IcdManagement::Feature::kLongIdleTimeSupport);
    bool hasUserActiveModeTriggerInstruction =
        OptionalAttributeSet().IsSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id);

    std::vector<DataModel::AttributeEntry> expectedAttributes;

    // Add mandatory attributes
    expectedAttributes.push_back(IcdManagement::Attributes::IdleModeDuration::kMetadataEntry);
    expectedAttributes.push_back(IcdManagement::Attributes::ActiveModeDuration::kMetadataEntry);
    expectedAttributes.push_back(IcdManagement::Attributes::ActiveModeThreshold::kMetadataEntry);

    // Add optional attributes based on feature map
    if (hasCIP)
    {
        expectedAttributes.push_back(IcdManagement::Attributes::RegisteredClients::kMetadataEntry);
        expectedAttributes.push_back(IcdManagement::Attributes::ICDCounter::kMetadataEntry);
        expectedAttributes.push_back(IcdManagement::Attributes::ClientsSupportedPerFabric::kMetadataEntry);
        expectedAttributes.push_back(IcdManagement::Attributes::MaximumCheckInBackOff::kMetadataEntry);
    }

    if (hasUAT)
    {
        expectedAttributes.push_back(IcdManagement::Attributes::UserActiveModeTriggerHint::kMetadataEntry);
    }
    if (hasUserActiveModeTriggerInstruction)
    {
        expectedAttributes.push_back(IcdManagement::Attributes::UserActiveModeTriggerInstruction::kMetadataEntry);
    }
    if (hasLIT)
    {
        expectedAttributes.push_back(IcdManagement::Attributes::OperatingMode::kMetadataEntry);
    }

    ASSERT_TRUE(IsAttributesListEqualTo(cluster, expectedAttributes));

    // Test accepted commands list
    bool hasStayActive = optionalCommands.Has(IcdManagement::OptionalCommands::kStayActive);

    // Build expected accepted commands list dynamically
    if (hasCIP && (hasLIT || hasStayActive))
    {
        // Both CIP and StayActive
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      IcdManagement::Commands::RegisterClient::kMetadataEntry,
                                                      IcdManagement::Commands::UnregisterClient::kMetadataEntry,
                                                      IcdManagement::Commands::StayActiveRequest::kMetadataEntry,
                                                  }));
    }
    else if (hasCIP)
    {
        // Only CIP
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      IcdManagement::Commands::RegisterClient::kMetadataEntry,
                                                      IcdManagement::Commands::UnregisterClient::kMetadataEntry,
                                                  }));
    }
    else if (hasLIT || hasStayActive)
    {
        // Only StayActive
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      IcdManagement::Commands::StayActiveRequest::kMetadataEntry,
                                                  }));
    }
    else
    {
        // No commands
        ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));
    }

    // Test generated commands list
    if (hasCIP && (hasLIT || hasStayActive))
    {
        // Both CIP and StayActive
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       IcdManagement::Commands::RegisterClientResponse::Id,
                                                       IcdManagement::Commands::StayActiveResponse::Id,
                                                   }));
    }
    else if (hasCIP)
    {
        // Only CIP
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       IcdManagement::Commands::RegisterClientResponse::Id,
                                                   }));
    }
    else if (hasLIT || hasStayActive)
    {
        // Only StayActive
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       IcdManagement::Commands::StayActiveResponse::Id,
                                                   }));
    }
    else
    {
        // No commands
        ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, {}));
    }
}

} // namespace
