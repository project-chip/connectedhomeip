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
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
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
using chip::Testing::IsAttributesListEqualTo;

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
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsBuilder;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, IcdManagement::Id), acceptedCommandsBuilder),
              CHIP_NO_ERROR);

    // Calculate expected accepted commands based on feature map and configuration
    bool hasStayActive = optionalCommands.Has(IcdManagement::OptionalCommands::kStayActive);

    size_t expectedAcceptedCommands = 0;
    if (hasCIP)
    {
        expectedAcceptedCommands += 2; // RegisterClient, UnregisterClient
    }
    if (hasLIT || hasStayActive)
    {
        expectedAcceptedCommands += 1; // StayActiveRequest
    }

    ASSERT_TRUE(acceptedCommandsBuilder.Size() == expectedAcceptedCommands);

    // Test generated commands list
    ReadOnlyBufferBuilder<CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kRootEndpointId, IcdManagement::Id), generatedCommandsBuilder),
              CHIP_NO_ERROR);

    // Calculate expected generated commands based on feature map and configuration
    size_t expectedGeneratedCommands = 0;
    if (hasCIP)
    {
        expectedGeneratedCommands += 1; // RegisterClientResponse
    }
    if (hasLIT || hasStayActive)
    {
        expectedGeneratedCommands += 1; // StayActiveResponse
    }

    ASSERT_TRUE(generatedCommandsBuilder.Size() == expectedGeneratedCommands);
}

} // namespace
