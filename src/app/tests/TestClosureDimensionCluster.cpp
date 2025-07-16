/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>
#include <vector>

using namespace chip::app::Clusters::ClosureDimension;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

class TestDelegate : public DelegateBase
{
public:
    TestDelegate() {}

    Protocols::InteractionModel::Status HandleSetTarget(const Optional<Percent100ths> & position, const Optional<bool> & latch,
                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed)
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                   const Optional<Globals::ThreeLevelAutoEnum> & speed)
    {
        return Protocols::InteractionModel::Status::Success;
    }

    void SetManualLatching(const bool manualLatch) { isLatchManual = manualLatch; }

private:
    bool isLatchManual = false;
};

class MockMatterContext : public MatterContext
{
public:
    MockMatterContext() : MatterContext(kInvalidEndpointId) {}
    void MarkDirty(AttributeId id) override { mDirtyMarkedList.push_back(id); }
    std::vector<AttributeId> GetDirtyList() { return mDirtyMarkedList; }
    void ClearDirtyList() { mDirtyMarkedList.clear(); }
    ~MockMatterContext() = default;

private:
    // Won't handle double-marking an attribute, so don't do that in tests
    std::vector<AttributeId> mDirtyMarkedList;
};

class TestClosureDimensionClusterLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mockDelegate = TestDelegate();
        mockContext  = MockMatterContext();
        conformance  = ClusterConformance();
        logic        = std::make_unique<ClusterLogic>(mockDelegate, mockContext);

        // Add values to attributes need to be set in Init()
        initParams.modulationType       = ModulationTypeEnum::kOpacity;
        initParams.rotationAxis         = RotationAxisEnum::kBottom;
        initParams.translationDirection = TranslationDirectionEnum::kBackward;
    }

    void TearDown() override { logic.reset(); }

    TestDelegate mockDelegate;
    MockMatterContext mockContext;
    ClusterConformance conformance;
    ClusterInitParameters initParams;
    std::unique_ptr<ClusterLogic> logic;
};

bool HasAttributeChanges(std::vector<AttributeId> changes, AttributeId id)
{
    return std::find(changes.begin(), changes.end(), id) != changes.end();
}

//=========================================================================================
// Tests for conformance
//=========================================================================================

/*
    ClusterConformance Valid Function Test Case
*/
TEST_F(TestClosureDimensionClusterLogic, TestConformanceValid)
{

    // Validating if either Positioning or MotionLatching is supported. If neither are enabled, returns false.

    // Neither Positioning or MotionLatching is enabled , return false.
    EXPECT_FALSE(conformance.Valid());

    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    conformance.FeatureMap().Set(Feature::kMotionLatching);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Validating If Unit, Limitation or speed is enabled, Positioning must be enabled. Return false otherwise.

    // Speed is enabled, Positioning is not enabled. Return false.
    conformance.FeatureMap().Set(Feature::kSpeed);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Limitation is enabled, Positioning is not enabled. Return false.
    conformance.FeatureMap().Set(Feature::kLimitation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Unit is enabled, Positioning is not enabled. Return false.
    conformance.FeatureMap().Set(Feature::kUnit);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Speed is enabled, Positioning is also enabled. Return true.
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Limitation is enabled, Positioning is also enabled. Return true.
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kLimitation);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Unit is enabled, Positioning is also enabled. Return true.
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kUnit);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Validating If Translation, Rotation or Modulation is enabled, Positioning must be enabled. Return false otherwise.

    // Translation is enabled, Positioning is not enabled. Return false
    conformance.FeatureMap().Set(Feature::kTranslation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Rotation is enabled, Positioning is not enabled. Return false
    conformance.FeatureMap().Set(Feature::kRotation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Modulation is enabled, Positioning is not enabled. Return false
    conformance.FeatureMap().Set(Feature::kModulation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Translation is enabled, Positioning is enabled. Return true
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kTranslation);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Rotation is enabled, Positioning is enabled. Return true
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Modulation is enabled, Positioning is enabled. Return true
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);
    EXPECT_TRUE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // Validating Only one of Translation, Rotation or Modulation must be enabled. Return false otherwise.

    // If Positioning is enabled, all 3 Translation, Rotation and  Modulation are enabled. Return false
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kTranslation)
        .Set(Feature::kRotation)
        .Set(Feature::kModulation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // If Positioning is enabled, both Rotation and  Modulation are enabled. Return false
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation).Set(Feature::kModulation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // If Positioning is enabled, both Translation and Rotation are enabled. Return false
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kTranslation).Set(Feature::kRotation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();

    // If Positioning is enabled, both Translation and  Modulation are enabled. Return false
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kTranslation).Set(Feature::kModulation);
    EXPECT_FALSE(conformance.Valid());
    conformance.FeatureMap().ClearAll();
}

// This test ensures the Init function with proper parameters should pass and reintilization should fail.
TEST_F(TestClosureDimensionClusterLogic, TestInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);

    initParams.modulationType       = ModulationTypeEnum::kOpacity;
    initParams.rotationAxis         = RotationAxisEnum::kBottom;
    initParams.translationDirection = TranslationDirectionEnum::kBackward;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));

    ClusterState state = logic->GetState();

    EXPECT_EQ(state.modulationType, ModulationTypeEnum::kOpacity);
    EXPECT_EQ(state.rotationAxis, RotationAxisEnum::kUnknownEnumValue);
    EXPECT_EQ(state.translationDirection, TranslationDirectionEnum::kUnknownEnumValue);

    // ReInit should fail
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INCORRECT_STATE);
}

// This test ensures the Init function with Invalid conformance should fail.
TEST_F(TestClosureDimensionClusterLogic, TestInvalidConformance)
{
    conformance.FeatureMap();

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    ;
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));

    ClusterState state = logic->GetState();
    EXPECT_EQ(state.modulationType, ModulationTypeEnum::kUnknownEnumValue);
    EXPECT_EQ(state.rotationAxis, RotationAxisEnum::kUnknownEnumValue);
    EXPECT_EQ(state.translationDirection, TranslationDirectionEnum::kUnknownEnumValue);
}

// This test ensures the Init function with Invalid InitParameters should fail.
TEST_F(TestClosureDimensionClusterLogic, TestInvalidInitParameters)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);

    initParams.modulationType = ModulationTypeEnum::kUnknownEnumValue;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));

    ClusterState state = logic->GetState();
    EXPECT_EQ(state.modulationType, ModulationTypeEnum::kUnknownEnumValue);
    EXPECT_EQ(state.rotationAxis, RotationAxisEnum::kUnknownEnumValue);
    EXPECT_EQ(state.translationDirection, TranslationDirectionEnum::kUnknownEnumValue);
}

//=========================================================================================
// Tests for Featuremap and Cluster Revision Attribute
//=========================================================================================

// This test ensures the get FeatureMap get the Featuremap set from conformance
TEST_F(TestClosureDimensionClusterLogic, TestFeatureMap)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));

    BitFlags<Feature> featureMap;

    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, conformance.FeatureMap());
}

// This test ensures the get ClusterRevision get the intended value
TEST_F(TestClosureDimensionClusterLogic, TestClusterRevision)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));

    uint16_t kExpectedClusterRevision = 1u;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;

    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kExpectedClusterRevision);
}

//=========================================================================================
// Tests for CurrentState Attribute
//=========================================================================================

// This test ensures that the set/get of CurrentState without Intialisation
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateWithNoIntialisation)
{
    GenericDimensionStateStruct testCurrentStateStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
}

// This test ensures that the set of CurrentState without Positioning feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if position feild is present
// - should return CHIP_NO_ERROR if position feild is abscent
TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateWithNoPositioningFeature)
{
    GenericDimensionStateStruct testCurrentStateStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    conformance.FeatureMap().Set(Feature::kMotionLatching);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testCurrentStateStruct.Set(NullOptional, Optional<bool>(false), NullOptional);
    testCurrentState.SetNonNull(testCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
}

// This test ensures that the set of CurrentState without MotionLatching feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if latch feild is present
// - should return CHIP_NO_ERROR if latch feild is abscent
TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateWithNoMotionLatchingFeature)
{
    GenericDimensionStateStruct testCurrentStateStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testCurrentStateStruct.Set(Optional<Percent100ths>(0), NullOptional,
                               Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    testCurrentState.SetNonNull(testCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
}

// This test ensures that the set of CurrentState without Speed feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if speed field is present
// - should return CHIP_NO_ERROR if speed field is abscent
TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateWithNoSpeedFeature)
{
    GenericDimensionStateStruct testCurrentStateStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testCurrentStateStruct.Set(Optional<Percent100ths>(0), Optional<bool>(false), NullOptional);
    testCurrentState.SetNonNull(testCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
}

// This test ensures that the Setter and getter for currentstate
// - sets/gets the value properly
// - error for constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestCurrentState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
    // Change values
    testCurrentStateStruct.Set(Optional<Percent100ths>(10000), Optional<bool>(true),
                               Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh));
    testCurrentState.SetNonNull(testCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
    // Change to NULL
    testCurrentState.SetNull();
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
    // Change from NULL
    testCurrentStateStruct.Set(Optional<Percent100ths>(5000), Optional<bool>(false),
                               Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium));
    testCurrentState.SetNonNull(testCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
    // Invalid position
    GenericDimensionStateStruct InvalidCurrentState{ Optional<Percent100ths>(10001), Optional<bool>(false),
                                                     Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow) };
    DataModel::Nullable<GenericDimensionStateStruct> testInvalid(InvalidCurrentState);
    EXPECT_EQ(logic->SetCurrentState(testInvalid), CHIP_ERROR_INVALID_ARGUMENT);

    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    // Invalid speed
    InvalidCurrentState.Set(Optional<Percent100ths>(10000), Optional<bool>(false),
                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue));
    testInvalid.SetNonNull(InvalidCurrentState);
    EXPECT_EQ(logic->SetCurrentState(testInvalid), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
}

//=========================================================================================
// Tests for Target Attribute
//=========================================================================================

// This test ensures that the set/get of Target without Intialisation
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestTargetWithNoIntialisation)
{
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    DataModel::Nullable<GenericDimensionStateStruct> target;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->GetTargetState(target), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// This test ensures that the set of Target without Positioning feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if position feild is present
// - should return CHIP_NO_ERROR if position feild is abscent
TEST_F(TestClosureDimensionClusterLogic, TestTargetWithNoPositioningFeature)
{
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    DataModel::Nullable<GenericDimensionStateStruct> target;

    conformance.FeatureMap().Set(Feature::kMotionLatching);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testTargetStruct.Set(NullOptional, Optional<bool>(false), NullOptional);
    testTarget.SetNonNull(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// This test ensures that the set of Target without MotionLatching feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if latch feild is present
// - should return CHIP_NO_ERROR if latch feild is abscent
TEST_F(TestClosureDimensionClusterLogic, TestTargetWithNoMotionLatchingFeature)
{
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    DataModel::Nullable<GenericDimensionStateStruct> target;

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testTargetStruct.Set(Optional<Percent100ths>(0), NullOptional,
                         Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    testTarget.SetNonNull(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// This test ensures that the set of Target without Speed feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if speed field is present
// - should return CHIP_NO_ERROR if speed field is abscent
TEST_F(TestClosureDimensionClusterLogic, TestTargetWithNoSpeedFeature)
{
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    DataModel::Nullable<GenericDimensionStateStruct> target;

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    // Get default value
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);

    // Set current state will with position field without positioning
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Set currentstate without position
    //  we set speed aslo NULL as speed feature is positioning optional.
    testTargetStruct.Set(Optional<Percent100ths>(0), Optional<bool>(false), NullOptional);
    testTarget.SetNonNull(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// This test ensures that the Setter and getter for currentstate
// - sets/gets value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestTarget)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };

    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    DataModel::Nullable<GenericDimensionStateStruct> target;

    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    mockContext.ClearDirtyList();
    // Change values
    testTargetStruct.Set(Optional<Percent100ths>(10000), Optional<bool>(true),
                         Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh));
    testTarget.SetNonNull(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    mockContext.ClearDirtyList();
    // Change to NULL
    testTarget.SetNull();
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    mockContext.ClearDirtyList();
    // set NULL value again
    testTarget.SetNull();
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, DataModel::NullNullable);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    mockContext.ClearDirtyList();
    // Change from NULL
    testTargetStruct.Set(Optional<Percent100ths>(5000), Optional<bool>(false),
                         Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium));
    testTarget.SetNonNull(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    mockContext.ClearDirtyList();
    // Invalid position
    GenericDimensionStateStruct InvalidTarget{ Optional<Percent100ths>(10001), Optional<bool>(false),
                                               Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow) };
    DataModel::Nullable<GenericDimensionStateStruct> testInvalid(InvalidTarget);
    EXPECT_EQ(logic->SetTargetState(testInvalid), CHIP_ERROR_INVALID_ARGUMENT);

    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Invalid speed
    InvalidTarget.Set(Optional<Percent100ths>(10000), Optional<bool>(false),
                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue));
    testInvalid.SetNonNull(InvalidTarget);
    EXPECT_EQ(logic->SetTargetState(testInvalid), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target, testTarget);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

//=========================================================================================
// Tests for Resolution Attribute
//=========================================================================================

// This test ensures that the set/get of Target without Intialisation
// - should return CHIP_ERROR_INCORRECT_STATE
TEST_F(TestClosureDimensionClusterLogic, TestResolutionWithNoIntialisation)
{
    Percent100ths testResolution = 100;
    Percent100ths resolution;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetResolution(resolution), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Resolution::Id));
}

// This test ensures that the Resolution without Positioning feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if position feild is present
TEST_F(TestClosureDimensionClusterLogic, TestResolutionWithNoPositioningFeature)
{
    conformance.FeatureMap().Set(Feature::kMotionLatching);

    Percent100ths testResolution = 100;
    Percent100ths resolution;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetResolution(resolution), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Resolution::Id));
}

// This test ensures that the Setter and getter of Resolution can
// - set/get value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestResolution)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    Percent100ths testResolution = 10;
    Percent100ths Resolution;

    // get default value
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, 1);

    // change Value
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Resolution::Id));

    // get changed Value
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);

    mockContext.ClearDirtyList();
    // Invalid Resolution
    Percent100ths invalidResolution = 10001;
    EXPECT_EQ(logic->SetResolution(invalidResolution), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Resolution::Id));
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);

    mockContext.ClearDirtyList();
    // Invalid Resolution
    invalidResolution = 0;
    EXPECT_EQ(logic->SetResolution(invalidResolution), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Resolution::Id));
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);
}

//=========================================================================================
// Tests for StepValue Attribute
//=========================================================================================

// This test ensures that the set/get of StepValue without Intialisation
// - should return CHIP_ERROR_INCORRECT_STATE
TEST_F(TestClosureDimensionClusterLogic, TestStepValueWithNoIntialisation)
{
    Percent100ths testStepValue = 100;
    Percent100ths stepValue;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetResolution(stepValue), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetResolution(testStepValue), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));
}

// This test ensures that the StepValue without Positioning feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestStepValueWithNoPositioningFeature)
{
    conformance.FeatureMap().Set(Feature::kMotionLatching);

    Percent100ths testStepValue = 100;
    Percent100ths stepValue;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetResolution(stepValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->SetResolution(testStepValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));
}

// This test ensures that the Setter and getter of StepValue can
// - set/get value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestStepValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);

    initParams.modulationType       = ModulationTypeEnum::kOpacity;
    initParams.rotationAxis         = RotationAxisEnum::kBottom;
    initParams.translationDirection = TranslationDirectionEnum::kBackward;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    Percent100ths testStepValue = 10;
    Percent100ths StepValue;

    // Check Default value
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, 1);

    // set Value
    EXPECT_EQ(logic->SetStepValue(testStepValue), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, testStepValue);

    mockContext.ClearDirtyList();
    // Change values
    testStepValue = 100;
    EXPECT_EQ(logic->SetStepValue(testStepValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, testStepValue);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));

    mockContext.ClearDirtyList();
    // Invalid StepValue
    Percent100ths invalidStepValue = 10001;
    EXPECT_EQ(logic->SetStepValue(invalidStepValue), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));
    EXPECT_EQ(StepValue, testStepValue);

    mockContext.ClearDirtyList();
    // StepValue not Multiple of Resolution
    invalidStepValue         = 45;
    Percent100ths resolution = 10;
    EXPECT_EQ(logic->SetResolution(resolution), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetStepValue(invalidStepValue), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::StepValue::Id));
    EXPECT_EQ(StepValue, testStepValue);
}

//=========================================================================================
// Tests for Unit Attribute
//=========================================================================================

// This test ensures that the set/get of Unit without Intialisation
// - should return CHIP_ERROR_INCORRECT_STATE
TEST_F(TestClosureDimensionClusterLogic, TestUnitWithNoIntialisation)
{
    ClosureUnitEnum testUnit = ClosureUnitEnum::kDegree;
    ClosureUnitEnum unit;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetUnit(unit), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Unit::Id));
}

// This test ensures that the Unit without Unit feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestUnitWithNoUnitFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);

    ClosureUnitEnum testUnit = ClosureUnitEnum::kDegree;
    ClosureUnitEnum unit;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetUnit(unit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Unit::Id));
}

// This test ensures that the Setter and getter of Unit can
// - set/get value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestUnit)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kUnit);

    initParams.modulationType       = ModulationTypeEnum::kOpacity;
    initParams.rotationAxis         = RotationAxisEnum::kBottom;
    initParams.translationDirection = TranslationDirectionEnum::kBackward;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    ClosureUnitEnum testUnit = ClosureUnitEnum::kDegree;
    ClosureUnitEnum Unit;

    // Default value
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, ClosureUnitEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Unit::Id));
    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);

    mockContext.ClearDirtyList();
    // Change values
    testUnit = ClosureUnitEnum::kMillimeter;
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Unit::Id));

    mockContext.ClearDirtyList();
    // Invalid Unit
    ClosureUnitEnum invalidUnit = ClosureUnitEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetUnit(invalidUnit), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Unit::Id));
}

//=========================================================================================
// Tests for UnitRange Attribute
//=========================================================================================

// This test ensures that the set/get of UnitRange without Intialisation
// - should return CHIP_ERROR_INCORRECT_STATE
TEST_F(TestClosureDimensionClusterLogic, TestUnitRangeWithNoIntialisation)
{
    Structs::UnitRangeStruct::Type tUnitRange = { .min = 0, .max = 10000 };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> testUnitRange{ tUnitRange };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetUnitRange(unitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));
}

// This test ensures that the UnitRange without Unit feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestUnitRangeWithNoUnitFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);

    Structs::UnitRangeStruct::Type tUnitRange = { .min = 0, .max = 10000 };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> testUnitRange{ tUnitRange };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetUnitRange(unitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));
}

// This test ensures that the Setter and getter of UnitRange can
// - set/get value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestUnitRange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kUnit);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    Structs::UnitRangeStruct::Type tUnitRange       = { .min = 0, .max = 10000 };
    Structs::UnitRangeStruct::Type newUnitRange     = { .min = 10, .max = 100 };
    Structs::UnitRangeStruct::Type invalidUnitRange = { .min = 1000, .max = 10 };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> testUnitRange{ tUnitRange };
    DataModel::Nullable<Structs::UnitRangeStruct::Type> UnitRange;

    // Default Value
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.IsNull(), true);

    // set Values
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.Value().min, testUnitRange.Value().min);
    EXPECT_EQ(UnitRange.Value().max, testUnitRange.Value().max);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Change values
    testUnitRange = DataModel::Nullable<Structs::UnitRangeStruct::Type>(newUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.Value().min, testUnitRange.Value().min);
    EXPECT_EQ(UnitRange.Value().max, testUnitRange.Value().max);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Change to Null, when present UnitRange is not NULL
    testUnitRange.SetNull();
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.IsNull(), true);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Change to Null, when present UnitRange is NULL
    testUnitRange.SetNull();
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.IsNull(), true);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // valid degree Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    newUnitRange.max = 90;
    newUnitRange.min = -90;
    testUnitRange    = DataModel::Nullable<Structs::UnitRangeStruct::Type>(newUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid Values
    testUnitRange = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid millimeter Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kMillimeter), CHIP_NO_ERROR);
    invalidUnitRange.max = 0;
    invalidUnitRange.min = 10000;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid millimeter Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kMillimeter), CHIP_NO_ERROR);
    invalidUnitRange.max = 10000;
    invalidUnitRange.min = -1;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid millimeter Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kMillimeter), CHIP_NO_ERROR);
    invalidUnitRange.max = -1;
    invalidUnitRange.min = 1000;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid degree Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    invalidUnitRange.max = 390;
    invalidUnitRange.min = 0;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid degree Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    invalidUnitRange.max = -390;
    invalidUnitRange.min = 0;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid degree range
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    invalidUnitRange.max = -180;
    invalidUnitRange.min = 270;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid degree Values
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    invalidUnitRange.max = 90;
    invalidUnitRange.min = 180;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid degree range
    EXPECT_EQ(logic->SetUnit(ClosureUnitEnum::kDegree), CHIP_NO_ERROR);
    invalidUnitRange.max = 190;
    invalidUnitRange.min = -180;
    testUnitRange        = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));
}

//=========================================================================================
// Tests for LimitRange Attribute
//=========================================================================================

// This test ensures that the set/get of LimitRange without Intialisation
// - should return CHIP_ERROR_INCORRECT_STATE
TEST_F(TestClosureDimensionClusterLogic, TestLimitRangeWithNoIntialisation)
{
    Structs::RangePercent100thsStruct::Type testLimitRange = { .min = 0, .max = 10000 };
    Structs::RangePercent100thsStruct::Type limitRange;

    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetLimitRange(limitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));
}

// This test ensures that the LimitRange without Limitation feature
// - should return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestLimitRangeWithNoUnitFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);

    Structs::RangePercent100thsStruct::Type testLimitRange = { .min = 0, .max = 10000 };
    Structs::RangePercent100thsStruct::Type LimitRange;

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::UnitRange::Id));
}

// This test ensures that the Setter and getter of LimitRange can
// - set/get value.
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestLimitRange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kLimitation);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    Structs::RangePercent100thsStruct::Type testLimitRange = { .min = 0, .max = 10000 };
    Structs::RangePercent100thsStruct::Type LimitRange;

    // Default Values
    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min, 0);
    EXPECT_EQ(LimitRange.max, 0);

    mockContext.ClearDirtyList();
    // set Values
    Percent100ths resolution = 1;
    EXPECT_EQ(logic->SetResolution(resolution), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min, testLimitRange.min);
    EXPECT_EQ(LimitRange.max, testLimitRange.max);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));

    mockContext.ClearDirtyList();
    // Change values
    testLimitRange = { .min = 10, .max = 100 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min, testLimitRange.min);
    EXPECT_EQ(LimitRange.max, testLimitRange.max);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));

    mockContext.ClearDirtyList();
    // Invalid Values
    testLimitRange = { .min = 10001, .max = 100 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));
    testLimitRange = { .min = 100, .max = 10001 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));
    testLimitRange = { .min = 10000, .max = 0 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));
    resolution = 10;
    EXPECT_EQ(logic->SetResolution(resolution), CHIP_NO_ERROR);
    // LimitValue not multiple of resolution
    testLimitRange = { .min = 45, .max = 100 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));
    testLimitRange = { .min = 40, .max = 105 };
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::LimitRange::Id));
}

//=========================================================================================
// Tests for TranslationDirection Attribute
//=========================================================================================

// This test ensures that the Setter and getter of TranslationDirection can
// - set/get value.
TEST_F(TestClosureDimensionClusterLogic, TestTranslationDirection)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kTranslation);

    initParams.translationDirection = TranslationDirectionEnum::kBackward;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    TranslationDirectionEnum TranslationDirection;

    EXPECT_EQ(logic->GetTranslationDirection(TranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(TranslationDirection, TranslationDirectionEnum::kBackward);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));
}

// This test ensures that the Setter and getter of TranslationDirection can
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestTranslationDirectionInvalidValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kTranslation);

    initParams.translationDirection = TranslationDirectionEnum::kUnknownEnumValue;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TranslationDirection::Id));
}

//=========================================================================================
// Tests for RotationAxis Attribute
//=========================================================================================

// This test ensures that the Setter and getter of RotationAxis can
// - set/get value.
TEST_F(TestClosureDimensionClusterLogic, TestRotationAxis)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);

    initParams.rotationAxis = RotationAxisEnum::kCenteredHorizontal;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    RotationAxisEnum RotationAxis;

    EXPECT_EQ(logic->GetRotationAxis(RotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(RotationAxis, RotationAxisEnum::kCenteredHorizontal);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
}

// This test ensures that the Setter and getter of RotationAxis can
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestRotationAxisInvalidValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);

    initParams.rotationAxis = RotationAxisEnum::kUnknownEnumValue;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::RotationAxis::Id));
}

//=========================================================================================
// Tests for ModulationType Attribute
//=========================================================================================

// This test ensures that the Setter and getter of ModulationType can
// - set/get value.
TEST_F(TestClosureDimensionClusterLogic, TestModulationType)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);

    initParams.modulationType = ModulationTypeEnum::kSlatsOpenwork;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    ModulationTypeEnum ModulationType;

    EXPECT_EQ(logic->GetModulationType(ModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(ModulationType, ModulationTypeEnum::kSlatsOpenwork);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
}

// This test ensures that the Setter and getter of ModulationType can
// - return CHIP_ERROR_INVALID_ARGUMENT constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestModulationTypeInvalidValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);

    initParams.modulationType = ModulationTypeEnum::kUnknownEnumValue;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::ModulationType::Id));
}

//=========================================================================================
// Tests for Overflow Attribute
//=========================================================================================

// This test ensures that the Setter and getter of Overflow can
// - set/get value.
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestOverflow)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);

    initParams.rotationAxis = RotationAxisEnum::kBottom;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    OverflowEnum testOverflow = OverflowEnum::kBottomInside;
    OverflowEnum Overflow;

    // Default value
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, OverflowEnum::kUnknownEnumValue);

    mockContext.ClearDirtyList();
    // set Values
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Change values
    testOverflow = OverflowEnum::kBottomOutside;
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Invalid Overflow
    OverflowEnum invalidOverflow = OverflowEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetOverflow(invalidOverflow), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestRotationOverflowDependency)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kRotation);

    initParams.rotationAxis = RotationAxisEnum::kCenteredHorizontal;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    OverflowEnum testOverflow = OverflowEnum::kBottomInside;
    OverflowEnum Overflow;

    // Default value
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, OverflowEnum::kUnknownEnumValue);

    mockContext.ClearDirtyList();
    // set and get Values
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Change values
    testOverflow = OverflowEnum::kTopOutside;
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Invalid Overflow
    OverflowEnum invalidOverflow = OverflowEnum::kInside;
    EXPECT_EQ(logic->SetOverflow(invalidOverflow), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Invalid Overflow
    invalidOverflow = OverflowEnum::kOutside;
    EXPECT_EQ(logic->SetOverflow(invalidOverflow), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));

    mockContext.ClearDirtyList();
    // Invalid Overflow
    invalidOverflow = OverflowEnum::kNoOverflow;
    EXPECT_EQ(logic->SetOverflow(invalidOverflow), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));
}

// This test ensures that the Set function
// Default value
// set fails with CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
TEST_F(TestClosureDimensionClusterLogic, TestUnsupportedOverflow)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kModulation);

    initParams.modulationType = ModulationTypeEnum::kOpacity;

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    OverflowEnum testOverflow = OverflowEnum::kBottomInside;
    OverflowEnum overflow;
    ClusterState state;

    // Default value
    EXPECT_EQ(logic->GetOverflow(overflow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    state = logic->GetState();
    EXPECT_EQ(state.overflow, OverflowEnum::kUnknownEnumValue);

    mockContext.ClearDirtyList();
    // set Values
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // Ensure the value is unaccessible via the API
    EXPECT_EQ(logic->GetOverflow(overflow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    state = logic->GetState();
    EXPECT_EQ(state.overflow, OverflowEnum::kUnknownEnumValue);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::Overflow::Id));
}

//=========================================================================================
// Tests for command parameters
//=========================================================================================

using namespace Protocols::InteractionModel;

// This test ensures Handle set Target command executes as expected.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand)
{

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    DataModel::Nullable<GenericDimensionStateStruct> target;
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    // Validating SetTargetState with no arguments
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, NullOptional, NullOptional), Status::InvalidCommand);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with invalid position
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10001), NullOptional, NullOptional), Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with manual latch
    BitFlags<LatchControlModesBitmap> latchControlModes;
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, Optional<bool>(true), NullOptional), Status::InvalidInState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, Optional<bool>(false), NullOptional), Status::InvalidInState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    // Validating SetTargetState with invalid speed
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, NullOptional,
                                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue)),
              Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with unknown current state
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<bool>(true),
                                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with proper arguments
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(0), Optional<bool>(false),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<bool>(true),
                                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value().Value(), 1000);
    EXPECT_EQ(target.Value().latch.Value().Value(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with only position arguments
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10000), NullOptional, NullOptional), Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value().Value(), 10000);
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.HasValue(), true);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// This test ensures Handle set Target command executes as expected.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommandWithLimitation)
{

    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kLimitation);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    DataModel::Nullable<GenericDimensionStateStruct> target;
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);

    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);
    Structs::RangePercent100thsStruct::Type limitRange = { .min = 1000, .max = 9000 };
    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_NO_ERROR);

    // Validating SetTargetState with position greater than Limit.Max
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(false),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10000), Optional<bool>(true),
                                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value().Value(), 9000);
    EXPECT_EQ(target.Value().latch.Value().Value(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating SetTargetState with position less the limit.Min
    setCurrentStateStruct.Set(Optional<Percent100ths>(9000), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(0), Optional<bool>(true),
                                            Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value().Value(), 1000);
    EXPECT_EQ(target.Value().latch.Value().Value(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
}

// HandleSetTarget command should return InvalidInState when CurrentState is latched and SetTarget command changes position without
// latch.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand_ChangePositionWithoutIncomingLatchWhenLatched)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(true),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), NullOptional, NullOptional), Status::InvalidInState);
}

// HandleSetTarget command should return InvalidInState when CurrentState is latched and position is changed.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand_ChangePositionWhenLatched)
{

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(true),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<bool>(true), NullOptional),
              Status::InvalidInState);
}

// HandleSetTarget command should return Success when CurrentState is latched and only speed is changed.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand_ChangeSpeedWhenLatched)
{

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(true),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, NullOptional, Optional(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);
    DataModel::Nullable<GenericDimensionStateStruct> target;
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_FALSE(target.IsNull());
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
}

// HandleSetTarget command should return Success when CurrentState is latched and only latch is changed.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand_ChangeLatchWhenLatched)
{

    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(true),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, Optional<bool>(false), NullOptional), Status::Success);
    DataModel::Nullable<GenericDimensionStateStruct> target;
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_FALSE(target.IsNull());
    EXPECT_EQ(target.Value().latch.Value(), false);
}

// This test ensures Handle Step command executes as expected. Tests:
// - Return InvalidInState if CurrentState is unknown.
// - Return constrainError if arguments value are out of bounds
TEST_F(TestClosureDimensionClusterLogic, TestHandleStepCommand)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    DataModel::Nullable<GenericDimensionStateStruct> target;

    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);
    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);

    // Validating Step with Invalid direction
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kUnknownEnumValue, 1, NullOptional), Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating Step with invalid steps
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 0, NullOptional), Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating Step with invalid speed
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 1,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue)),
              Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    // Validating Step with unknown current state
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 1,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));

    Percent100ths stepValue = 10;
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_NO_ERROR);

    // Validating Step increase with proper arguments
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(0), Optional<bool>(false),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(100));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(100), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    // Validating Step increase which target will be >10000
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 65535,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(10000));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kMedium);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(10000), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    // Validating Step decrease
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 10,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(9900));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(9900), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    // Validating Step decrease which target will be <0
    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 65535,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(0));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(0), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
}

// This test ensures Handle Step command executes as expected. Tests:
// - Return InvalidInState if CurrentState is unknown.
// - Return constrainError if arguments value are out of bounds
TEST_F(TestClosureDimensionClusterLogic, TestHandleStepCommandWithLimitation)
{
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kLimitation);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    DataModel::Nullable<GenericDimensionStateStruct> target;
    GenericDimensionStateStruct testTargetStruct{ Optional<Percent100ths>(0), Optional<bool>(false),
                                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto) };
    DataModel::Nullable<GenericDimensionStateStruct> testTarget(testTargetStruct);

    EXPECT_EQ(logic->SetTargetState(testTarget), CHIP_NO_ERROR);

    Percent100ths stepValue = 10;
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_NO_ERROR);

    // Validating Step with proper arguments
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(false),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    Structs::RangePercent100thsStruct::Type limitRange = { .min = 1000, .max = 9000 };
    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(1100));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(1100), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 65535,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(9000));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kMedium);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(9000), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 10,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(8900));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(8900), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 65535,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)),
              Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTargetState(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.Value().position.Value(), static_cast<unsigned short>(1000));
    EXPECT_EQ(target.Value().latch.HasValue(), true);
    EXPECT_EQ(target.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::TargetState::Id));
    // As delegate is not implemented, we are updating current state here
    setCurrentStateStruct.Set(Optional<Percent100ths>(1000), Optional<bool>(false),
                              Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
}

TEST_F(TestClosureDimensionClusterLogic, TestHandleStepCommandWhenLatched)
{
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kLimitation);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    DataModel::Nullable<GenericDimensionStateStruct> currentState;
    Percent100ths stepValue = 10;
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_NO_ERROR);

    // Validating Step with proper arguments
    GenericDimensionStateStruct setCurrentStateStruct(Optional<Percent100ths>(1000), Optional<bool>(true),
                                                      Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    currentState.SetNonNull(setCurrentStateStruct);
    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_NO_ERROR);
    Structs::RangePercent100thsStruct::Type limitRange = { .min = 1000, .max = 9000 };
    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_NO_ERROR);

    mockContext.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10,
                                       Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
              Protocols::InteractionModel::Status::InvalidInState);
}

//=========================================================================================
// Tests for CurrentState Attribute Quiet Reporting
//=========================================================================================

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingPositionNonNulltoNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set position to 1000
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set position to null
    testCurrentState.Value().position.Value().SetNull();
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingPositionNulltoNonNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(DataModel::NullNullable);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set position as null
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set position to NonNull(1000)
    testCurrentState.Value().position.SetValue(DataModel::MakeNullable<chip::Percent100ths>(chip::Percent100ths(1000)));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingPositionChangeValueWithoutDelay)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set position as 1000
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set position to 2000
    testCurrentState.Value().position.SetValue(DataModel::MakeNullable<chip::Percent100ths>(chip::Percent100ths(2000)));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingPositionChangeValueWithDelay)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set position as 1000
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    chip::test_utils::SleepMillis(2000); // Sleep for 2 seconds

    // set position to 2000
    testCurrentState.Value().position.SetValue(DataModel::MakeNullable<chip::Percent100ths>(chip::Percent100ths(2000)));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();

    chip::test_utils::SleepMillis(2000); // Sleep for 2 seconds

    // set position to 3000
    testCurrentState.Value().position.SetValue(DataModel::MakeNullable<chip::Percent100ths>(chip::Percent100ths(3000)));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_FALSE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();

    chip::test_utils::SleepMillis(2000); // Sleep for 2 seconds

    // set position to 4000
    testCurrentState.Value().position.SetValue(DataModel::MakeNullable<chip::Percent100ths>(chip::Percent100ths(4000)));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingLatchNulltoValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(DataModel::NullNullable);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set latch as null
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set latch to false
    testCurrentState.Value().latch.SetValue(DataModel::MakeNullable<bool>(false));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingLatchValuetoNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set latch as false
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set latch to null
    testCurrentState.Value().latch.Value().SetNull();
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingLatchValueChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set latch as false
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set latch to true
    testCurrentState.Value().latch.Value().SetNonNull(true);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingSpeedNullOptionaltoValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = NullOptional;

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set speed as nulloptional
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set speed to low
    testCurrentState.Value().speed.SetValue(Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingSpeedValuetoNullOptional)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set speed as low
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set latch to NullOptional
    testCurrentState.Value().speed.ClearValue();
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

TEST_F(TestClosureDimensionClusterLogic, TestCurrentStateQuietReportingSpeedValueChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);

    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    mockContext.ClearDirtyList();

    GenericDimensionStateStruct testCurrentStateStruct;
    testCurrentStateStruct.position = Optional<DataModel::Nullable<chip::Percent100ths>>(1000);
    testCurrentStateStruct.latch    = Optional<DataModel::Nullable<bool>>(false);
    testCurrentStateStruct.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    DataModel::Nullable<GenericDimensionStateStruct> testCurrentState(testCurrentStateStruct);
    DataModel::Nullable<GenericDimensionStateStruct> currentState;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, DataModel::NullNullable);

    mockContext.ClearDirtyList();

    // set speed as low
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    mockContext.ClearDirtyList();

    // set speed to medium
    testCurrentState.Value().speed.SetValue(Globals::ThreeLevelAutoEnum::kMedium);
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);
    EXPECT_TRUE(HasAttributeChanges(mockContext.GetDirtyList(), Attributes::CurrentState::Id));

    mockContext.ClearDirtyList();
}

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
