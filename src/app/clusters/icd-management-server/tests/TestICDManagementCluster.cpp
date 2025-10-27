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
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
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

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace chip::app::Clusters::IcdManagement::Attributes;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestIcdManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestIcdManagementCluster, TestAttributes)
{
    // Create test instances
    TestPersistentStorageDelegate storage;
    chip::Crypto::DefaultSessionKeystore keystore;
    FabricTable fabricTable;
    ICDConfigurationData & icdConfig = ICDConfigurationData::GetInstance();

    BitMask<IcdManagement::OptionalCommands> optionalCommands =
        BitMask<IcdManagement::OptionalCommands>(IcdManagement::OptionalCommands::kStayActive);
    BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerHint(0);

#if CHIP_CONFIG_ENABLE_ICD_CIP
    ICDManagementClusterWithCIP cluster(kRootEndpointId, storage, keystore, fabricTable, icdConfig,
                                        OptionalAttributeSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id),
                                        optionalCommands, userActiveModeTriggerHint, CharSpan());
#else
    ICDManagementCluster cluster(kRootEndpointId, storage, keystore, fabricTable, icdConfig,
                                 OptionalAttributeSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id),
                                 optionalCommands, userActiveModeTriggerHint, CharSpan());
#endif

    // Test attribute list
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, IcdManagement::Id), attributesBuilder), CHIP_NO_ERROR);

    // Calculate expected attributes based on feature map and configuration
    BitFlags<IcdManagement::Feature> featureMap = icdConfig.GetFeatureMap();
    bool hasCIP                                 = featureMap.Has(IcdManagement::Feature::kCheckInProtocolSupport);
    bool hasUAT                                 = featureMap.Has(IcdManagement::Feature::kUserActiveModeTrigger);
    bool hasLIT                                 = featureMap.Has(IcdManagement::Feature::kLongIdleTimeSupport);
    bool hasUserActiveModeTriggerInstruction =
        OptionalAttributeSet().IsSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

    // Add mandatory attributes
    ASSERT_EQ(expectedBuilder.AppendElements({ IcdManagement::Attributes::IdleModeDuration::kMetadataEntry,
                                               IcdManagement::Attributes::ActiveModeDuration::kMetadataEntry,
                                               IcdManagement::Attributes::ActiveModeThreshold::kMetadataEntry }),
              CHIP_NO_ERROR);

    // Add optional attributes based on feature map
    if (hasCIP)
    {
        ASSERT_EQ(expectedBuilder.AppendElements({ IcdManagement::Attributes::RegisteredClients::kMetadataEntry,
                                                   IcdManagement::Attributes::ICDCounter::kMetadataEntry,
                                                   IcdManagement::Attributes::ClientsSupportedPerFabric::kMetadataEntry,
                                                   IcdManagement::Attributes::MaximumCheckInBackOff::kMetadataEntry }),
                  CHIP_NO_ERROR);
    }

    if (hasUAT)
    {
        ASSERT_EQ(expectedBuilder.EnsureAppendCapacity(1), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.Append(IcdManagement::Attributes::UserActiveModeTriggerHint::kMetadataEntry), CHIP_NO_ERROR);
    }
    if (hasUserActiveModeTriggerInstruction)
    {
        ASSERT_EQ(expectedBuilder.EnsureAppendCapacity(1), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.Append(IcdManagement::Attributes::UserActiveModeTriggerInstruction::kMetadataEntry),
                  CHIP_NO_ERROR);
    }
    if (hasLIT)
    {
        ASSERT_EQ(expectedBuilder.EnsureAppendCapacity(1), CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.Append(IcdManagement::Attributes::OperatingMode::kMetadataEntry), CHIP_NO_ERROR);
    }

    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));

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
