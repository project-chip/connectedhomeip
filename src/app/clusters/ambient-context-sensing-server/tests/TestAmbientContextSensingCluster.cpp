/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/clusters/ambient-context-sensing-server/ambient-context-sensing-namespace.h>
#include <app/persistence/AttributePersistence.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/AmbientContextSensing/Attributes.h>
#include <clusters/AmbientContextSensing/Metadata.h>
#include <lib/support/TimerDelegateMock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AmbientContextSensing;
using namespace chip::Testing;


using chip::app::DataModel::DecodableList;
using TagDecodable = Globals::Structs::SemanticTagStruct::DecodableType;
using AmbientContextSensingTypeDecodable =
    AmbientContextSensing::Structs::AmbientContextTypeStruct::DecodableType;
using PredictedActivityDecordable =
    AmbientContextSensing::Structs::PredictedActivityStruct::DecodableType;

namespace {

constexpr EndpointId kTestEndpointId = 1;

constexpr uint32_t g_kFeatures_all = static_cast<uint32_t>(Feature::kHumanActivity) |
        static_cast<uint32_t>(Feature::kObjectCounting) |
        static_cast<uint32_t>(Feature::kObjectIdentification) |
        static_cast<uint32_t>(Feature::kSoundIdentification) |
        static_cast<uint32_t>(Feature::kPredictedActivity);
SemanticTagType g_kACTSupportedArray[] = {
        {
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kPresence),
        },
        {
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kFall),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kChild),
        },
        {
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kObjectFall),
        },
        {
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kSnoring),
        },
    };

SemanticTagType g_kSemanticTagDetectArray[] = {
        {
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kPresence),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kChild),
        },
        {
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kObjectFall),
        },
    };

AmbientContextSensingType g_kACTDetectArray[] = {
        {
            .ambientContextSensed = DataModel::List<const SemanticTagType>(&g_kSemanticTagDetectArray[0], 1),
        },
        {
            .ambientContextSensed = DataModel::List<const SemanticTagType>(&g_kSemanticTagDetectArray[1], 1),
        },
        {
            .ambientContextSensed = DataModel::List<const SemanticTagType>(&g_kSemanticTagDetectArray[2], 1),
        },
        {
            .ambientContextSensed = DataModel::List<const SemanticTagType>(&g_kSemanticTagDetectArray[3], 1),
        },
    };

struct TestAmbientContextSensingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mMockTimerDelegate;
};

TEST_F(TestAmbientContextSensingCluster, TestAttributes)
{
    chip::Testing::TestServerClusterContext context;
    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all)) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Read the revision
    uint16_t clusterRevision;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, AmbientContextSensing::kRevision);

    // Read the feature map
    uint32_t featureVal;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureVal), CHIP_NO_ERROR);
    BitMask<AmbientContextSensing::Feature> featureMap = featureVal;
    EXPECT_TRUE(featureMap.Has(Feature::kHumanActivity));
    EXPECT_TRUE(featureMap.Has(Feature::kObjectCounting));
    EXPECT_TRUE(featureMap.Has(Feature::kObjectIdentification));
    EXPECT_TRUE(featureMap.Has(Feature::kSoundIdentification));
    EXPECT_TRUE(featureMap.Has(Feature::kPredictedActivity));

    // Read the attributes
    bool eventDetected;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, eventDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(eventDetected);
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectIdentified::Id, eventDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(eventDetected);
    EXPECT_EQ(tester.ReadAttribute(Attributes::AudioContextDetected::Id, eventDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(eventDetected);

    ASSERT_TRUE(Testing::IsAttributesListEqualTo(cluster, {
                                                              Attributes::HumanActivityDetected::kMetadataEntry,
                                                              Attributes::ObjectIdentified::kMetadataEntry,
                                                              Attributes::AudioContextDetected::kMetadataEntry,
                                                              Attributes::AmbientContextType::kMetadataEntry,
                                                              Attributes::AmbientContextTypeSupported::kMetadataEntry,
                                                              Attributes::ObjectCountReached::kMetadataEntry,
                                                              Attributes::ObjectCountConfig::kMetadataEntry,
                                                              Attributes::SimultaneousDetectionLimit::kMetadataEntry,
                                                              Attributes::HoldTime::kMetadataEntry,
                                                              Attributes::HoldTimeLimits::kMetadataEntry,
                                                              Attributes::PredictedActivity::kMetadataEntry,
                                                          }));
    { // Read AmbientContextTypeSupported attribute, whose type is: list[SemanticTagStruct]

        DecodableList<TagDecodable> out;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextTypeSupported::Id, out), CHIP_NO_ERROR);
        size_t size = 0;
        EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
        EXPECT_EQ(size, 0u);
    }

    bool objectCountReached;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountReached::Id, objectCountReached), CHIP_NO_ERROR);
    EXPECT_FALSE(objectCountReached);

    uint16_t objectCount;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCount::Id, objectCount), Protocols::InteractionModel::Status::UnsupportedAttribute);

    uint8_t sDetectLimit;
    EXPECT_EQ(tester.ReadAttribute(Attributes::SimultaneousDetectionLimit::Id, sDetectLimit), CHIP_NO_ERROR);
    EXPECT_EQ(sDetectLimit, kDefaultSimultaneousDetectionLimit);

    uint16_t holdTime;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, kDefaultHoldTimeDefault);

    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimit;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTimeLimits::Id, holdTimeLimit), CHIP_NO_ERROR);
    EXPECT_EQ(holdTimeLimit.holdTimeMin, kDefaultHoldTimeMin);
    EXPECT_EQ(holdTimeLimit.holdTimeMax, kDefaultHoldTimeMax);
    EXPECT_EQ(holdTimeLimit.holdTimeDefault, kDefaultHoldTimeDefault);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestAmbientContextTypeSupported)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint32_t kFeatures = static_cast<uint32_t>(Feature::kHumanActivity) |
        static_cast<uint32_t>(Feature::kObjectCounting) |
        static_cast<uint32_t>(Feature::kObjectIdentification) |
        static_cast<uint32_t>(Feature::kPredictedActivity);
    SemanticTagType kACTSupported_all[] = {
        {
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kPresence),
        },
        {
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kFall),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult),
        },
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kChild),
        },
        // Duplicated one
        {
            .namespaceID = kNamespaceIdentifiedObject,
            .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult),
        },
        // Not enabled in feature-map
        {
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kObjectFall),
        },
        {
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kSnoring),
        },
    };
    SemanticTagType kACTSupported_dup[] = {
        kACTSupported_all[0],
        kACTSupported_all[1],
        kACTSupported_all[2],
        kACTSupported_all[3],
        // the duplicate one 
        kACTSupported_all[3],
    };
    SemanticTagType kACTSupported_unsupport[] = {
        kACTSupported_all[0],
        kACTSupported_all[1],
        kACTSupported_all[2],
        kACTSupported_all[3],
        // the unsupported ones
        kACTSupported_all[4],
        kACTSupported_all[5],
    };

    SemanticTagType kACTSupported[] = {
        kACTSupported_all[0],
        kACTSupported_all[1],
        kACTSupported_all[2],
        kACTSupported_all[3],
    };

    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(kFeatures))
        .WithAmbientContextSupported(chip::Span<const SemanticTagType>(kACTSupported_dup))
    };

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Read the AmbientContextTypeSupported.
    DecodableList<TagDecodable> out;
    EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextTypeSupported::Id, out), CHIP_NO_ERROR);
    size_t size = 0;
    EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
    // Exp: No supported type is saved, since there are errors in kACTSupported_dup
    EXPECT_EQ(size, 0u);

    // Push the unsupported one
    std::vector<SemanticTagType> ACTypeList_unsupport(
        std::begin(kACTSupported_unsupport), std::end(kACTSupported_unsupport));
    EXPECT_EQ(cluster.SetAmbientContextTypeSupported(ACTypeList_unsupport), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextTypeSupported::Id, out), CHIP_NO_ERROR);
    EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
    EXPECT_EQ(size, 0u);

    // Push the correct one
    std::vector<SemanticTagType> ACTypeList(
        std::begin(kACTSupported), std::end(kACTSupported));
    EXPECT_EQ(cluster.SetAmbientContextTypeSupported(ACTypeList), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextTypeSupported::Id, out), CHIP_NO_ERROR);
    EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
    EXPECT_EQ(size, 4u);

    // Set the unsupported event. Exp: Discarded
    {
        AmbientContextSensingType detectEvent;
        std::vector<SemanticTagType> tags;
        tags.push_back(SemanticTagType{
            .namespaceID = kNamespaceIdentifiedSound,
            .tag         = static_cast<uint8_t>(TagIdentifiedSound::kObjectFall),
        });
        detectEvent.ambientContextSensed = chip::app::DataModel::List<const SemanticTagType>(tags.data(), tags.size());
        EXPECT_EQ(cluster.AddDetection(detectEvent), CHIP_ERROR_INCORRECT_STATE);
    }
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestHoldTimeAttribute)
{
    chip::Testing::TestServerClusterContext context;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                        .holdTimeMax     = 200,
                                                                                        .holdTimeDefault = 100 };
    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithHoldTime(100, holdTimeLimitsConfig,
                                                                                                     mMockTimerDelegate) };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Verify that we can set a valid hold time via direct method call
    EXPECT_EQ(cluster.SetHoldTime(150), CHIP_NO_ERROR);
    uint16_t holdTime;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 150);
    EXPECT_EQ(cluster.GetHoldTime(), 150);

    // Verify that setting the same value returns NoOp
    EXPECT_EQ(cluster.SetHoldTime(150), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    // Verify that we cannot set a hold time less than the minimum
    EXPECT_EQ(cluster.SetHoldTime(5), Protocols::InteractionModel::Status::ConstraintError);

    // Verify that we cannot set a hold time greater than the maximum
    EXPECT_EQ(cluster.SetHoldTime(250), Protocols::InteractionModel::Status::ConstraintError);

    // Verify that we can write a valid hold time via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(180)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
    EXPECT_EQ(holdTime, 180);
    EXPECT_EQ(cluster.GetHoldTime(), 180);

    // Verify that we cannot write a hold time less than the minimum via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(5)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Verify that we cannot write a hold time greater than the maximum via WriteAttribute
    EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, static_cast<uint16_t>(250)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Give the new HoldTimeLimitation whose range is different. The HoldTime should be reset
    holdTimeLimitsConfig.holdTimeMin     = 201;
    holdTimeLimitsConfig.holdTimeMax     = 300;
    holdTimeLimitsConfig.holdTimeDefault = 250;
    cluster.SetHoldTimeLimits(holdTimeLimitsConfig);
    EXPECT_EQ(cluster.GetHoldTime(), holdTimeLimitsConfig.holdTimeDefault);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestHoldTimePersistence)
{
    chip::Testing::TestServerClusterContext context;
    constexpr uint16_t kDefaultHoldTime                                        = 100;
    constexpr uint16_t kNewHoldTime                                            = 150;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = kDefaultHoldTime };

    // 1. Create a cluster. On startup, it should store the default hold time.
    {
        AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        uint16_t holdTime = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kDefaultHoldTime);
        EXPECT_EQ(cluster.GetHoldTime(), kDefaultHoldTime);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // 2. Write a new value to the attribute. This should update the value in persistence.
    {
        AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR); // Startup will load the default value again
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(tester.WriteAttribute(Attributes::HoldTime::Id, kNewHoldTime), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetHoldTime(), kNewHoldTime);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // 3. Create a new cluster instance. It should load the new value from persistence on startup.
    {
        AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithHoldTime(
            kDefaultHoldTime, holdTimeLimitsConfig, mMockTimerDelegate) };

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        uint16_t holdTime = 0;
        EXPECT_EQ(tester.ReadAttribute(Attributes::HoldTime::Id, holdTime), CHIP_NO_ERROR);
        EXPECT_EQ(holdTime, kNewHoldTime);
        EXPECT_EQ(cluster.GetHoldTime(), kNewHoldTime);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAmbientContextSensingCluster, TestAmbientContextDetect)
{
    chip::Testing::TestServerClusterContext context;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = 100 };

    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
        .WithAmbientContextSupported(chip::Span<const SemanticTagType>(g_kACTSupportedArray))
        .WithHoldTime(kDefaultHoldTimeDefault, holdTimeLimitsConfig, mMockTimerDelegate)
    };

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Read the AmbientContextTypeSupported.
    {
        DecodableList<TagDecodable> out;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextTypeSupported::Id, out), CHIP_NO_ERROR);
        size_t size = 0;
        EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
        EXPECT_EQ(size, 6u);
    }

    // Read the detection
    bool isDetected;
    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(isDetected);
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectIdentified::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(isDetected);
    EXPECT_EQ(tester.ReadAttribute(Attributes::AudioContextDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(isDetected);

    // Add a detection event
    {
        AmbientContextSensingType detectEvent;
        std::vector<SemanticTagType> tags;
        tags.push_back(SemanticTagType{
            .namespaceID = kNamespaceIdentifiedHumanActivity,
            .tag         = static_cast<uint8_t>(TagIdentifiedHumanActivity::kFall),
        });
        detectEvent.ambientContextSensed = chip::app::DataModel::List<const SemanticTagType>(tags.data(), tags.size());
        EXPECT_EQ(cluster.AddDetection(detectEvent), CHIP_NO_ERROR);
    }

    // Check if it's detected. Expect: Detected, & timer running
    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_TRUE(isDetected);
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // Fetch the detection info
    {
        DecodableList<AmbientContextSensingTypeDecodable> out;
        EXPECT_EQ(tester.ReadAttribute(Attributes::AmbientContextType::Id, out), CHIP_NO_ERROR);
        size_t size = 0;
        EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
        EXPECT_EQ(size, 1u);

        auto it = out.begin();
        while (it.Next())
        {
            const auto & ACType = it.GetValue().ambientContextSensed;
            size_t tagCount;
            EXPECT_EQ(ACType.ComputeSize(&tagCount), CHIP_NO_ERROR);
            EXPECT_EQ(tagCount, 1u);

            auto it_e = ACType.begin();
            while (it_e.Next())
            {
                const auto & tag = it_e.GetValue();
                EXPECT_TRUE((tag.namespaceID == kNamespaceIdentifiedHumanActivity) && (tag.tag == static_cast<uint8_t>(TagIdentifiedHumanActivity::kFall)));
            }
        }
    }

    // Advance clock by more than the hold time
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(kDefaultHoldTimeDefault + 1));

    // Check again. Expect: Not-detected
    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(isDetected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestSimultaneousDetectLimit)
{
    chip::Testing::TestServerClusterContext context;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin     = 10,
                                                                                   .holdTimeMax     = 200,
                                                                                   .holdTimeDefault = 100 };

    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
        .WithAmbientContextSupported(chip::Span<const SemanticTagType>(g_kACTSupportedArray))
        .WithHoldTime(kDefaultHoldTimeDefault, holdTimeLimitsConfig, mMockTimerDelegate)
    };

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Read SimultaneousDetectionLimit attribute: Exp: kDefaultSimultaneousDetectionLimit;
    bool isDetected;
    uint8_t simultaneousDetectLimit;
    EXPECT_EQ(tester.ReadAttribute(Attributes::SimultaneousDetectionLimit::Id, simultaneousDetectLimit), CHIP_NO_ERROR);
    EXPECT_EQ(simultaneousDetectLimit, kDefaultSimultaneousDetectionLimit);

    // Write a bigger SimultaneousDetectionLimit
    uint8_t newSDL = simultaneousDetectLimit + 1;
    EXPECT_EQ(tester.WriteAttribute(Attributes::SimultaneousDetectionLimit::Id, newSDL), CHIP_NO_ERROR);

    // Add Detection
    EXPECT_EQ(cluster.AddDetection(g_kACTDetectArray[0]), CHIP_NO_ERROR);
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_EQ(cluster.AddDetection(g_kACTDetectArray[1]), CHIP_NO_ERROR);
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(1));
    EXPECT_EQ(cluster.AddDetection(g_kACTDetectArray[2]), CHIP_NO_ERROR);

    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_TRUE(isDetected);

    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectIdentified::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_TRUE(isDetected);

    constexpr uint8_t kTestSimultaneousDetectionLimit = 2;

    // Reduce the simultaneous detection limitation to the value which is less than the current detection number
    EXPECT_EQ(tester.WriteAttribute(Attributes::SimultaneousDetectionLimit::Id, kTestSimultaneousDetectionLimit), CHIP_NO_ERROR);
    // Re-write it again. Exp: kWriteSuccessNoOp
    EXPECT_EQ(tester.WriteAttribute(Attributes::SimultaneousDetectionLimit::Id, kTestSimultaneousDetectionLimit), DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

    // Read back the simultaneous detection limitation, exp: kTestSimultaneousDetectionLimit
    EXPECT_EQ(tester.ReadAttribute(Attributes::SimultaneousDetectionLimit::Id, simultaneousDetectLimit), CHIP_NO_ERROR);
    EXPECT_EQ(simultaneousDetectLimit, kTestSimultaneousDetectionLimit);

    // Check the AmbientContextType. exp: the 1st one is removed
    EXPECT_EQ(tester.ReadAttribute(Attributes::HumanActivityDetected::Id, isDetected), CHIP_NO_ERROR);
    EXPECT_FALSE(isDetected);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestObjectCount)
{
    constexpr uint16_t kTestObjCntThreshold = 10;
    constexpr uint16_t kTestObjectCount     = 11;

    chip::Testing::TestServerClusterContext context;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10,
                                                                                    .holdTimeMax     = 200,
                                                                                    .holdTimeDefault = kDefaultHoldTimeDefault};
    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
            .WithHoldTime(kDefaultHoldTimeDefault, holdTimeLimitsConfig, mMockTimerDelegate)
        };

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    // Set the ObjectCountConfig attribute
    ObjectCountConfigType defObjCountCfg = {
        .countingObject = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult)
            },
        .objectCountThreshold = kTestObjCntThreshold,
    };
    EXPECT_EQ(cluster.SetObjectCountConfig(defObjCountCfg), Protocols::InteractionModel::Status::ConstraintError);
    // Set the supported list
    std::vector<SemanticTagType> ACTypeList(
            std::begin(g_kACTSupportedArray), std::end(g_kACTSupportedArray));
    EXPECT_EQ(cluster.SetAmbientContextTypeSupported(ACTypeList), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(Attributes::ObjectCountConfig::Id, defObjCountCfg), Protocols::InteractionModel::Status::Success);
    // Read back the ObjectCountConfig attribute and check if it's the new value
    ObjectCountConfigType rdObjCountCfg;
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountConfig::Id, rdObjCountCfg), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(rdObjCountCfg.countingObject.namespaceID, defObjCountCfg.countingObject.namespaceID);
    EXPECT_EQ(rdObjCountCfg.countingObject.tag, defObjCountCfg.countingObject.tag);
    EXPECT_EQ(rdObjCountCfg.objectCountThreshold, defObjCountCfg.objectCountThreshold);

    bool objCntReached;
    uint16_t objCount;
    // Read ObjectCountReached attribute, exp: false
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountReached::Id, objCntReached), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(objCntReached);
    
    // Read ObjectCount attribute, exp: unsupported_attribute
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCount::Id, objCount), Protocols::InteractionModel::Status::UnsupportedAttribute);

    // Set ObjectCount attribute which is smaller than ObjectCountThreshold
    EXPECT_EQ(cluster.SetObjectCount(kMinObjectCount), CHIP_NO_ERROR);

    // Read ObjectCountReached attribute, exp: false
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountReached::Id, objCntReached), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(objCntReached);

    // Read ObjectCount attribute, exp: unsupported_attribute
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCount::Id, objCount), Protocols::InteractionModel::Status::UnsupportedAttribute);

    // Set ObjectCount attribute which is bigger than ObjectCountThreshold
    EXPECT_EQ(cluster.SetObjectCount(kTestObjectCount), CHIP_NO_ERROR);
    EXPECT_TRUE(mMockTimerDelegate.IsTimerActive(&cluster));

    // Read ObjectCountReached attribute, exp: true
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountReached::Id, objCntReached), Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(objCntReached);

    // Read ObjectCount attribute, exp: the value to be set
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCount::Id, objCount), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(objCount, kTestObjectCount);

    // Advance clock by more than the hold time
    mMockTimerDelegate.AdvanceClock(System::Clock::Seconds16(kDefaultHoldTimeDefault + 1));

    // Read ObjectCountReached attribute, exp: false
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountReached::Id, objCntReached), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(objCntReached);


    // Read ObjectCount attribute, exp: unsupported_attribute 
    EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCount::Id, objCount), Protocols::InteractionModel::Status::UnsupportedAttribute);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestAmbientContextSensingCluster, TestObjCntThresholdPersistence)
{
    constexpr uint16_t kTestObjCntThreshold = 10;
    chip::Testing::TestServerClusterContext context;

    // 1. Create a cluster. Read the ObjectCountConfig. ObjectCountThreshold == kDefaultCountThreshold
    {
        AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10,
                                                                                    .holdTimeMax     = 200,
                                                                                    .holdTimeDefault = kDefaultHoldTimeDefault};
        AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
            .WithHoldTime(kDefaultHoldTimeDefault, holdTimeLimitsConfig, mMockTimerDelegate)
        };

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        // Set the supported list
        std::vector<SemanticTagType> ACTypeList(
            std::begin(g_kACTSupportedArray), std::end(g_kACTSupportedArray));
        EXPECT_EQ(cluster.SetAmbientContextTypeSupported(ACTypeList), CHIP_NO_ERROR);

        // Set the ObjectCountConfig attribute
        ObjectCountConfigType defObjCountCfg = {
            .countingObject = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kAdult)
            },
            .objectCountThreshold = kTestObjCntThreshold,
        };

        // Read back the ObjectCountConfig attribute. ObjectCountConfig.objectCountThreshold should be the default value
        ObjectCountConfigType rdObjCountCfg;
        EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountConfig::Id, rdObjCountCfg), Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(rdObjCountCfg.objectCountThreshold, kDefaultCountThreshold);

        // Write the new ObjectCountConfig attribute
        EXPECT_EQ(tester.WriteAttribute(Attributes::ObjectCountConfig::Id, defObjCountCfg), Protocols::InteractionModel::Status::Success);

        // Read back the ObjectCountConfig attribute. ObjectCountConfig.objectCountThreshold should be the new value
        EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountConfig::Id, rdObjCountCfg), Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(rdObjCountCfg.objectCountThreshold, defObjCountCfg.objectCountThreshold);
	    
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // 2. Create a new cluster and read the ObjectCountConfig. ObjectCountThreshold should be the new value which is loaded from the persistence storage
    {
        AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimitsConfig = { .holdTimeMin = 10,
                                                                                    .holdTimeMax     = 200,
                                                                                    .holdTimeDefault = kDefaultHoldTimeDefault};
        AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
            .WithHoldTime(kDefaultHoldTimeDefault, holdTimeLimitsConfig, mMockTimerDelegate)
        };

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);
        // Set the supported list
        std::vector<SemanticTagType> ACTypeList(std::begin(g_kACTSupportedArray), std::end(g_kACTSupportedArray));
        EXPECT_EQ(cluster.SetAmbientContextTypeSupported(ACTypeList), CHIP_NO_ERROR);

        ObjectCountConfigType rdObjCountCfg;
        EXPECT_EQ(tester.ReadAttribute(Attributes::ObjectCountConfig::Id, rdObjCountCfg), Protocols::InteractionModel::Status::Success);
        EXPECT_EQ(rdObjCountCfg.objectCountThreshold, kTestObjCntThreshold);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestAmbientContextSensingCluster, TestPredictActivity)
{
    chip::Testing::TestServerClusterContext context;
    AmbientContextSensingCluster cluster{ AmbientContextSensingCluster::Config{ kTestEndpointId }.WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))};

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    const uint32_t now = std::chrono::duration_cast<System::Clock::Seconds16>(mMockTimerDelegate.GetCurrentMonotonicTimestamp()).count();
    const DataModel::List<const SemanticTagType> kACTList(g_kSemanticTagDetectArray, std::size(g_kSemanticTagDetectArray));

    std::vector<PredictedActivityType> predictActivity {
            {
                .startTimestamp = now + 10,
                .endTimestamp = now + 20,
                .ambientContextType = MakeOptional(kACTList),
                .crowdDetected = MakeOptional(false),
                .crowdCount = MakeOptional(1u),
                .confidence = 100,
            },
        };
    
    EXPECT_EQ(cluster.SetPredictedActivity(predictActivity), CHIP_NO_ERROR);

    // Readback PredictedActivity
    DecodableList<PredictedActivityDecordable> out;
    EXPECT_EQ(tester.ReadAttribute(Attributes::PredictedActivity::Id, out), CHIP_NO_ERROR);
    size_t size = 0;
    EXPECT_EQ(out.ComputeSize(&size), CHIP_NO_ERROR);
    EXPECT_EQ(size, 1u);

    auto it = out.begin();
    while (it.Next())
    {
        const auto & ACTypeList = it.GetValue().ambientContextType.Value();
        auto it_e = ACTypeList.begin();
        auto i = 0;
        while (it_e.Next())
        {
            const auto & ACType = it_e.GetValue();
            EXPECT_TRUE((ACType.namespaceID == g_kSemanticTagDetectArray[i].namespaceID) && (ACType.tag == g_kSemanticTagDetectArray[i].tag));
            i++;
        }
        EXPECT_EQ(it.GetValue().startTimestamp, (now + 10));
        EXPECT_EQ(it.GetValue().endTimestamp, (now + 20));
        EXPECT_EQ(it.GetValue().crowdDetected.Value(), false);
        EXPECT_EQ(it.GetValue().crowdCount.Value(), 1);
        EXPECT_EQ(it.GetValue().confidence, 100);
    }
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
