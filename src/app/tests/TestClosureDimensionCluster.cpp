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
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

using namespace chip::app::Clusters::ClosureDimension;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

namespace {

    constexpr uint16_t kExpectedClusterRevision = 1u;

} // namespace

class TestClosureDimensionClusterLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::DeviceLayer::SetSystemLayerForTesting(nullptr);
    }
};

class TestDelegate : public DelegateBase
{
public:
    TestDelegate() {}

    Protocols::InteractionModel::Status HandleSetTarget(const Optional<Percent100ths> & position,
        const Optional<TargetLatchEnum> & latch, const Optional<Globals::ThreeLevelAutoEnum> & speed) {
            return Protocols::InteractionModel::Status::Success;
        }

    Protocols::InteractionModel::Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
        const Optional<Globals::ThreeLevelAutoEnum> & speed) {
            return Protocols::InteractionModel::Status::Success;
        }
};

class MockedMatterContext : public MatterContext
{
public:
    MockedMatterContext(EndpointId endpoint) :
        MatterContext(endpoint)
    {}
    void MarkDirty(AttributeId id) override { mDirtyMarkedList.push_back(id); }
    std::vector<AttributeId> GetDirtyList() { return mDirtyMarkedList; }
    void ClearDirtyList() { mDirtyMarkedList.clear(); }
    ~MockedMatterContext() {};

private:
    // Won't handle double-marking an attribute, so don't do that in tests
    std::vector<AttributeId> mDirtyMarkedList;
};

bool HasAttributeChanges(std::vector<AttributeId> changes, AttributeId id)
{
    return std::find(changes.begin(), changes.end(), id) != changes.end();
}

//=========================================================================================
// Tests for conformance
//=========================================================================================

// This test ensures that the Init function properly errors when the conformance is valid and passes otherwise.
TEST_F(TestClosureDimensionClusterLogic, TestConformanceValid)
{
    // Nothing on, should be valid
    ClusterConformance conformance = {
        .featureMap = 0, .supportsOverflow = false
    };
    EXPECT_TRUE(conformance.Valid());

    conformance = {
        .featureMap = 0, .supportsOverflow = true
    };
    EXPECT_FALSE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = to_underlying(Feature::kRotation),
                    .supportsOverflow         = true };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = to_underlying(Feature::kRotation),
                    .supportsOverflow         = false };
    EXPECT_FALSE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = to_underlying(Feature::kMotionLatching),
                    .supportsOverflow         = true };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = to_underlying(Feature::kMotionLatching),
                .supportsOverflow         = false };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = 66,
                    .supportsOverflow         = true };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = 66,
                    .supportsOverflow         = false };
    EXPECT_FALSE(conformance.Valid());
}

// This test ensures the Init function corectly errors if the supplied delegate does not match
// the given cluster conformance and invalid state if init is called twice.
TEST_F(TestClosureDimensionClusterLogic, TestDelegates)
{
    TestDelegate delegate;
    EndpointId endpoint = 1;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);
    ClusterConformance conformanceLevel   = { .featureMap = 255,
                                              .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformanceLevel), CHIP_NO_ERROR);
    //EXPECT_EQ(logicLevel.Init(conformanceLevel), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestClosureDimensionClusterLogic, TestWrongDelegates)
{
    TestDelegate delegate;
    EndpointId endpoint = 1;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);
    ClusterConformance conformanceNotValid = { .featureMap = 0, .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformanceNotValid), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

}

//=========================================================================================
// Tests for getters
//=========================================================================================

// This test ensures that all getters are properly implemented and return valid values after successful initialization.
TEST_F(TestClosureDimensionClusterLogic, TestGetAttributesAllFeatures)
{
    TestDelegate delegate;
    MockedMatterContext context(1);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericCurrentStateStruct currentState;
    GenericTargetStruct target;
    Percent100ths resolution;
    Percent100ths stepValue;
    ClosureUnitEnum unit;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange;
    Structs::RangePercent100thsStruct::Type limitRange;
    TranslationDirectionEnum translationDirection;
    RotationAxisEnum rotationAxis;
    OverflowEnum overflow;
    ModulationTypeEnum modulationType;
    Attributes::FeatureMap::TypeInfo::Type featureMap;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position, DataModel::NullNullable);
    EXPECT_EQ(target.latch, DataModel::NullNullable);
    EXPECT_EQ(target.speed, DataModel::NullNullable);

    EXPECT_EQ(logic->GetResolution(resolution), CHIP_NO_ERROR);
    EXPECT_EQ(resolution, 1);

    EXPECT_EQ(logic->GetStepValue(stepValue), CHIP_NO_ERROR);
    EXPECT_EQ(stepValue, 1);

    EXPECT_EQ(logic->GetUnit(unit), CHIP_NO_ERROR);
    EXPECT_EQ(unit, ClosureUnitEnum::kUnknownEnumValue);

    EXPECT_EQ(logic->GetUnitRange(unitRange), CHIP_NO_ERROR);
    EXPECT_EQ(unitRange.IsNull(), true);

    EXPECT_EQ(logic->GetLimitRange(limitRange), CHIP_NO_ERROR);
    EXPECT_EQ(limitRange.min, 0);
    EXPECT_EQ(limitRange.max, 0);

    EXPECT_EQ(logic->GetTranslationDirection(translationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(translationDirection, TranslationDirectionEnum::kUnknownEnumValue);

    EXPECT_EQ(logic->GetRotationAxis(rotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(rotationAxis, RotationAxisEnum::kUnknownEnumValue);

    EXPECT_EQ(logic->GetOverflow(overflow), CHIP_NO_ERROR);
    EXPECT_EQ(overflow, OverflowEnum::kUnknownEnumValue);

    EXPECT_EQ(logic->GetModulationType(modulationType), CHIP_NO_ERROR);
    EXPECT_EQ(modulationType, ModulationTypeEnum::kUnknownEnumValue);

    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, conformance.featureMap);

    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kExpectedClusterRevision);
}

// This test ensures that attributes that are not supported by the conformance properly return errors
// and attributes that are supported return values properly.
TEST_F(TestClosureDimensionClusterLogic, TestSetAttributesNoFeatures)
{
    TestDelegate delegate;
    MockedMatterContext context(1);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = {
        .featureMap = 0, .supportsOverflow = false
    };
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericCurrentStateStruct currentState;
    GenericTargetStruct target;
    Percent100ths resolution;
    Percent100ths stepValue;
    ClosureUnitEnum unit;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange;
    Structs::RangePercent100thsStruct::Type limitRange;
    TranslationDirectionEnum translationDirection;
    RotationAxisEnum rotationAxis;
    OverflowEnum overflow;
    ModulationTypeEnum modulationType;
    Attributes::FeatureMap::TypeInfo::Type featureMap;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position, DataModel::NullNullable);
    EXPECT_EQ(target.latch, DataModel::NullNullable);
    EXPECT_EQ(target.speed, DataModel::NullNullable);

    EXPECT_EQ(logic->GetResolution(resolution), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetStepValue(stepValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetUnit(unit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetUnitRange(unitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetLimitRange(limitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetTranslationDirection(translationDirection), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetRotationAxis(rotationAxis), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetOverflow(overflow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetModulationType(modulationType), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, conformance.featureMap);

    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kExpectedClusterRevision);
}

// TODO: This test ensures that all attribute getters return the given starting state values before changes.
/*
TEST_F(TestClosureDimensionClusterLogic, TestGetAttributesStartingState)
{
    TestDelegate delegate;
    TestPersistentStorageDelegate storageDelegate;
    MockedMatterContext context(0, storageDelegate);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance     = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                           .supportsDefaultOpenLevel = true,
                                           .supportsValveFault       = true,
                                           .supportsLevelStep        = true };
    ClusterInitParameters initialState = {
        .currentState = DataModel::MakeNullable(ValveStateEnum::kTransitioning),
        .currentLevel = DataModel::MakeNullable(static_cast<Percent>(50u)),
        .valveFault   = BitMask<ValveFaultBitmap>(ValveFaultBitmap::kLeaking),
        .levelStep    = 2u,
    };
    EXPECT_EQ(logic->Init(conformance, initialState), CHIP_NO_ERROR);

    ClusterState state;
    state.currentState = initialState.currentState;
    state.currentLevel = initialState.currentLevel;
    state.valveFault   = initialState.valveFault;
    state.levelStep    = initialState.levelStep;

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<EpochUs> valEpochUsNullable;
    DataModel::Nullable<ValveStateEnum> valEnumNullable;
    DataModel::Nullable<Percent> valPercentNullable;
    Percent valPercent;
    uint8_t val8;
    BitMask<ValveFaultBitmap> valBitmap;
    Attributes::FeatureMap::TypeInfo::Type featureMap;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;

    EXPECT_EQ(logic->GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.openDuration);

    EXPECT_EQ(logic->GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.defaultOpenDuration);

    EXPECT_EQ(logic->GetAutoCloseTime(valEpochUsNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEpochUsNullable, state.autoCloseTime);

    EXPECT_EQ(logic->GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.remainingDuration.value());

    EXPECT_EQ(logic->GetCurrentState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, state.currentState);

    EXPECT_EQ(logic->GetTargetState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, state.targetState);

    EXPECT_EQ(logic->GetCurrentLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, state.currentLevel);

    EXPECT_EQ(logic->GetTargetLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, state.targetLevel);

    EXPECT_EQ(logic->GetDefaultOpenLevel(valPercent), CHIP_NO_ERROR);
    EXPECT_EQ(valPercent, state.defaultOpenLevel);

    EXPECT_EQ(logic->GetValveFault(valBitmap), CHIP_NO_ERROR);
    EXPECT_EQ(valBitmap, state.valveFault);

    EXPECT_EQ(logic->GetLevelStep(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, state.levelStep);

    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, conformance.featureMap);

    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kExpectedClusterRevision);
}
*/

// This test ensures that all attribute getter functions properly error on an uninitialized cluster.
TEST_F(TestClosureDimensionClusterLogic, TestGetAttributesUninitialized)
{
    TestDelegate delegate;
    MockedMatterContext context(1);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct currentState;
    GenericTargetStruct target;
    Percent100ths resolution;
    Percent100ths stepValue;
    ClosureUnitEnum unit;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange;
    Structs::RangePercent100thsStruct::Type limitRange;
    TranslationDirectionEnum translationDirection;
    RotationAxisEnum rotationAxis;
    OverflowEnum overflow;
    ModulationTypeEnum modulationType;
    Attributes::FeatureMap::TypeInfo::Type featureMap;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetTarget(target), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetResolution(resolution), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetStepValue(stepValue), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetUnit(unit), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetUnitRange(unitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetLimitRange(limitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetTranslationDirection(translationDirection), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetRotationAxis(rotationAxis), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetOverflow(overflow), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetModulationType(modulationType), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_ERROR_INCORRECT_STATE);
}

//=========================================================================================
// Tests for setters
//=========================================================================================


// This test ensures that all attribute setter functions properly error on an uninitialized cluster.
TEST_F(TestClosureDimensionClusterLogic, TestSetAttributesUninitialized)
{
    TestDelegate delegate;
    MockedMatterContext context(1);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct currentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct target{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    Percent100ths resolution = 1;
    Percent100ths stepValue = 1;
    ClosureUnitEnum unit = ClosureUnitEnum::kDegree;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange{};
    Structs::RangePercent100thsStruct::Type limitRange{};
    TranslationDirectionEnum translationDirection = TranslationDirectionEnum::kBackward;
    RotationAxisEnum rotationAxis = RotationAxisEnum::kBottom;
    OverflowEnum overflow = OverflowEnum::kBottomInside;
    ModulationTypeEnum modulationType = ModulationTypeEnum::kOpacity;

    EXPECT_EQ(logic->SetCurrentState(currentState), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetTarget(target), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetResolution(resolution), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetUnit(unit), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetUnitRange(unitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetTranslationDirection(translationDirection), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetRotationAxis(rotationAxis), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetOverflow(overflow), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->SetModulationType(modulationType), CHIP_ERROR_INCORRECT_STATE);
}

// This test ensures that attributes that are not supported by the conformance properly return errors
// and attributes that are supported return values properly.
TEST_F(TestClosureDimensionClusterLogic, TestGetAttributesNoFeatures)
{
    TestDelegate delegate;
    MockedMatterContext context(1);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = {
        .featureMap = 0, .supportsOverflow = false
    };
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericCurrentStateStruct testcurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericCurrentStateStruct currentState;
    GenericTargetStruct testtarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct target;
    Percent100ths resolution = 1;
    Percent100ths stepValue = 1;
    ClosureUnitEnum unit = ClosureUnitEnum::kDegree;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange{};
    Structs::RangePercent100thsStruct::Type limitRange{};
    TranslationDirectionEnum translationDirection = TranslationDirectionEnum::kBackward;
    RotationAxisEnum rotationAxis = RotationAxisEnum::kBottom;
    OverflowEnum overflow = OverflowEnum::kBottomInside;
    ModulationTypeEnum modulationType = ModulationTypeEnum::kOpacity;

    //As Feature conformance is not present, Current state should not be set
    EXPECT_EQ(logic->SetCurrentState(testcurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, NullOptional);
    EXPECT_EQ(currentState.latching, NullOptional);
    EXPECT_EQ(currentState.speed, NullOptional);

    //As Feature conformance is not present, Target should not be set
    EXPECT_EQ(logic->SetTarget(testtarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position, NullOptional);
    EXPECT_EQ(target.latch, NullOptional);
    EXPECT_EQ(target.speed, NullOptional);

    EXPECT_EQ(logic->SetResolution(resolution), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetUnit(unit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetUnitRange(unitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetTranslationDirection(translationDirection), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetRotationAxis(rotationAxis), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetOverflow(overflow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic->SetModulationType(modulationType), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestSetCurrentStateValues)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct testCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericCurrentStateStruct currentState;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    //Change values
    testCurrentState.Set(Optional<Percent100ths>(10000),
    Optional<LatchingEnum>(LatchingEnum::kLatchedAndSecured),
    Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh));
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    //Invlaid position
    GenericCurrentStateStruct InvalidCurrentState{Optional<Percent100ths>(10001),
        Optional<LatchingEnum>(LatchingEnum::kLatchedButNotSecured),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)};
    EXPECT_EQ(logic->SetCurrentState(InvalidCurrentState), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState, testCurrentState);

    //Invalid latching
    InvalidCurrentState.Set(Optional<Percent100ths>(10000),
        Optional<LatchingEnum>(LatchingEnum::kUnknownEnumValue),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow));
    EXPECT_EQ(logic->SetCurrentState(InvalidCurrentState), CHIP_ERROR_INVALID_ARGUMENT);
     // Ensure the value wasn't changed
     EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
     EXPECT_EQ(currentState, testCurrentState);

    //Invalid speed
    InvalidCurrentState.Set(Optional<Percent100ths>(10000),
        Optional<LatchingEnum>(LatchingEnum::kLatchedButNotSecured),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue));
    EXPECT_EQ(logic->SetCurrentState(InvalidCurrentState), CHIP_ERROR_INVALID_ARGUMENT);
     // Ensure the value wasn't changed
     EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
     EXPECT_EQ(currentState, testCurrentState);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetCurrentStateOnlyPosition)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct testCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericCurrentStateStruct currentState;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 1,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    // check only the position is set
    EXPECT_EQ(currentState.position, testCurrentState.position);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetCurrentStateOnlylatching)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct testCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericCurrentStateStruct currentState;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 2,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    // check only the latching is set
    EXPECT_EQ(currentState.latching, testCurrentState.latching);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetCurrentStateOnlyspeed)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericCurrentStateStruct testCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericCurrentStateStruct currentState;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 16,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
    EXPECT_EQ(currentState.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetCurrentState(testCurrentState), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetCurrentState(currentState), CHIP_NO_ERROR);
    // check only the speed is set
    EXPECT_EQ(currentState.speed, testCurrentState.speed);
    EXPECT_EQ(currentState.position, DataModel::NullNullable);
    EXPECT_EQ(currentState.latching, DataModel::NullNullable);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestSetTargetValues)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericTargetStruct testTarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct Target;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target, testTarget);

    //Change values
    testTarget.Set(Optional<Percent100ths>(10000),
    Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
    Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh));
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target, testTarget);

    //Invlaid position
    GenericTargetStruct InvalidTarget{Optional<Percent100ths>(10001),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)};
    EXPECT_EQ(logic->SetTarget(InvalidTarget), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target, testTarget);

    //Invalid latch
    InvalidTarget.Set(Optional<Percent100ths>(100),
    Optional<TargetLatchEnum>(TargetLatchEnum::kUnknownEnumValue),
    Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto));
    EXPECT_EQ(logic->SetTarget(InvalidTarget), CHIP_ERROR_INVALID_ARGUMENT);
     // Ensure the value wasn't changed
     EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
     EXPECT_EQ(Target, testTarget);

    //Invalid speed
    InvalidTarget.Set(Optional<Percent100ths>(100),
    Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
    Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue));
    EXPECT_EQ(logic->SetTarget(InvalidTarget), CHIP_ERROR_INVALID_ARGUMENT);
     // Ensure the value wasn't changed
     EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
     EXPECT_EQ(Target, testTarget);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetTargetOnlyPosition)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericTargetStruct testTarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct Target;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 1,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    // check only the position is set
    EXPECT_EQ(Target.position, testTarget.position);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetTargetOnlylatch)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericTargetStruct testTarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct Target;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 2,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);

    //set Values
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    // check only the latch is set
    EXPECT_EQ(static_cast<uint32_t>(Target.latch.Value()), static_cast<uint32_t>(testTarget.latch.Value()));
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);
}

// This test ensures that the Set function
// - checks conformance properly
TEST_F(TestClosureDimensionClusterLogic, TestSetTargetOnlyspeed)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    GenericTargetStruct testTarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct Target;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 16,
                                       .supportsOverflow = false };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
    EXPECT_EQ(Target.speed, DataModel::NullNullable);

    // set Values
    EXPECT_EQ(logic->SetTarget(testTarget), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTarget(Target), CHIP_NO_ERROR);
    // check only the speed is set
    EXPECT_EQ(static_cast<uint32_t>(Target.speed.Value()),static_cast<uint32_t>(testTarget.speed.Value()));
    EXPECT_EQ(Target.position, DataModel::NullNullable);
    EXPECT_EQ(Target.latch, DataModel::NullNullable);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestSetResolution)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    Percent100ths testResolution = 10;
    Percent100ths Resolution;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, 1);

    // set Values
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);

    //Change values
    testResolution = 100;
    EXPECT_EQ(logic->SetResolution(testResolution), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);

    //Invlaid position
    Percent100ths invalidResolution = 10001;
    EXPECT_EQ(logic->SetResolution(invalidResolution), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetResolution(Resolution), CHIP_NO_ERROR);
    EXPECT_EQ(Resolution, testResolution);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestStepValue)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    Percent100ths testStepValue = 10;
    Percent100ths StepValue;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetStepValue(testStepValue), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, 1);

    // set Values
    EXPECT_EQ(logic->SetStepValue(testStepValue), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, testStepValue);

    //Change values
    testStepValue = 100;
    EXPECT_EQ(logic->SetStepValue(testStepValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, testStepValue);

    //Invlaid position
    Percent100ths invalidStepValue = 10001;
    EXPECT_EQ(logic->SetStepValue(invalidStepValue), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetStepValue(StepValue), CHIP_NO_ERROR);
    EXPECT_EQ(StepValue, testStepValue);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestUnit)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    ClosureUnitEnum testUnit = ClosureUnitEnum::kDegree;
    ClosureUnitEnum Unit;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, ClosureUnitEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);

    //Change values
    testUnit = ClosureUnitEnum::kMillimeter;
    EXPECT_EQ(logic->SetUnit(testUnit), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);

    //Invlaid position
    ClosureUnitEnum invalidUnit = ClosureUnitEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetUnit(invalidUnit), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetUnit(Unit), CHIP_NO_ERROR);
    EXPECT_EQ(Unit, testUnit);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestUnitRange)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    Structs::UnitRangeStruct::Type tUnitRange = { .min = 0, .max = 10000};
    Structs::UnitRangeStruct::Type newUnitRange = { .min = 10, .max = 100};
    Structs::UnitRangeStruct::Type invalidUnitRange = { .min = 1000, .max = 10};
    DataModel::Nullable<Structs::UnitRangeStruct::Type> testUnitRange{tUnitRange};
    DataModel::Nullable<Structs::UnitRangeStruct::Type> UnitRange;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.IsNull(),true);

    // set Values
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.Value().min, testUnitRange.Value().min);
    EXPECT_EQ(UnitRange.Value().max, testUnitRange.Value().max);

    //Change values
    testUnitRange = DataModel::Nullable<Structs::UnitRangeStruct::Type>(newUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.Value().min, testUnitRange.Value().min);
    EXPECT_EQ(UnitRange.Value().max, testUnitRange.Value().max);

    //Change to Null
    testUnitRange.SetNull();
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetUnitRange(UnitRange), CHIP_NO_ERROR);
    EXPECT_EQ(UnitRange.IsNull(), true);

    //Invalid Values
    testUnitRange = DataModel::Nullable<Structs::UnitRangeStruct::Type>(invalidUnitRange);
    EXPECT_EQ(logic->SetUnitRange(testUnitRange), CHIP_ERROR_INVALID_ARGUMENT);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestLimitRange)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    Structs::RangePercent100thsStruct::Type testLimitRange = { .min = 0, .max = 10000};
    Structs::RangePercent100thsStruct::Type LimitRange;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min,0);
    EXPECT_EQ(LimitRange.max,0);

    // set Values
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min, testLimitRange.min);
    EXPECT_EQ(LimitRange.max, testLimitRange.max);

    //Change values
    testLimitRange = { .min = 10, .max = 100};
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetLimitRange(LimitRange), CHIP_NO_ERROR);
    EXPECT_EQ(LimitRange.min, testLimitRange.min);
    EXPECT_EQ(LimitRange.max, testLimitRange.max);

    //Invalid Values
    testLimitRange = { .min = 10001, .max = 100};
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    testLimitRange = { .min = 100, .max = 10001};
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);
    testLimitRange = { .min = 10000, .max = 0};
    EXPECT_EQ(logic->SetLimitRange(testLimitRange), CHIP_ERROR_INVALID_ARGUMENT);


}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestTranslationDirection)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    TranslationDirectionEnum testTranslationDirection = TranslationDirectionEnum::kBackward;
    TranslationDirectionEnum TranslationDirection;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetTranslationDirection(testTranslationDirection), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetTranslationDirection(TranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(TranslationDirection, TranslationDirectionEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetTranslationDirection(testTranslationDirection), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetTranslationDirection(TranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(TranslationDirection, testTranslationDirection);

    //Change values
    testTranslationDirection = TranslationDirectionEnum::kCeilingCenteredSymmetry;
    EXPECT_EQ(logic->SetTranslationDirection(testTranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetTranslationDirection(TranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(TranslationDirection, testTranslationDirection);

    //Invlaid position
    TranslationDirectionEnum invalidTranslationDirection = TranslationDirectionEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetTranslationDirection(invalidTranslationDirection), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetTranslationDirection(TranslationDirection), CHIP_NO_ERROR);
    EXPECT_EQ(TranslationDirection, testTranslationDirection);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestRotationAxis)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    RotationAxisEnum testRotationAxis = RotationAxisEnum::kBottom;
    RotationAxisEnum RotationAxis;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetRotationAxis(testRotationAxis), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetRotationAxis(RotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(RotationAxis, RotationAxisEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetRotationAxis(testRotationAxis), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetRotationAxis(RotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(RotationAxis, testRotationAxis);

    //Change values
    testRotationAxis = RotationAxisEnum::kCenteredHorizontal;
    EXPECT_EQ(logic->SetRotationAxis(testRotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetRotationAxis(RotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(RotationAxis, testRotationAxis);

    //Invlaid position
    RotationAxisEnum invalidRotationAxis = RotationAxisEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetRotationAxis(invalidRotationAxis), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetRotationAxis(RotationAxis), CHIP_NO_ERROR);
    EXPECT_EQ(RotationAxis, testRotationAxis);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestOverflow)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    OverflowEnum testOverflow = OverflowEnum::kBottomInside;
    OverflowEnum Overflow;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, OverflowEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);

    //Change values
    testOverflow = OverflowEnum::kBottomOutside;
    EXPECT_EQ(logic->SetOverflow(testOverflow), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);

    //Invlaid position
    OverflowEnum invalidOverflow = OverflowEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetOverflow(invalidOverflow), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetOverflow(Overflow), CHIP_NO_ERROR);
    EXPECT_EQ(Overflow, testOverflow);
}

// This test ensures that the Set function
// - sets the value properly including
// - constraints checks
TEST_F(TestClosureDimensionClusterLogic, TestModulationType)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);


    ModulationTypeEnum testModulationType = ModulationTypeEnum::kOpacity;
    ModulationTypeEnum ModulationType;

    // Setting this value before initialization should fail
    EXPECT_EQ(logic->SetModulationType(testModulationType), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true };

    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetModulationType(ModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(ModulationType, ModulationTypeEnum::kUnknownEnumValue);

    // set Values
    EXPECT_EQ(logic->SetModulationType(testModulationType), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic->GetModulationType(ModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(ModulationType, testModulationType);

    //Change values
    testModulationType = ModulationTypeEnum::kSlatsOpenwork;
    EXPECT_EQ(logic->SetModulationType(testModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetModulationType(ModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(ModulationType, testModulationType);

    //Invlaid position
    ModulationTypeEnum invalidModulationType = ModulationTypeEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->SetModulationType(invalidModulationType), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure the value wasn't changed
    EXPECT_EQ(logic->GetModulationType(ModulationType), CHIP_NO_ERROR);
    EXPECT_EQ(ModulationType, testModulationType);
}

//=========================================================================================
// Tests for command parameters
//=========================================================================================

// This test ensures Handle set Target command executes as expected. Tests:
// - Return Invalid Command if all arguments are null.
// - Return InvalidInState if CurrentState is unknown.
// - Return constrainError if arguments value are out of bounds
// - Return success if mismatch between arguments and conformance.
// - Null and value are both accepted when the parameter is supplied in the command field.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommand)
{

    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);


    GenericCurrentStateStruct currentState;
    GenericTargetStruct target;

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10), Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)), Protocols::InteractionModel::Status::InvalidInState);


    GenericCurrentStateStruct setCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);


    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(NullOptional, NullOptional, NullOptional), Protocols::InteractionModel::Status::InvalidCommand);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    //Check Target and current state after command
    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kLatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10001), Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kLatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10000), Optional<TargetLatchEnum>(TargetLatchEnum::kUnknownEnumValue),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kLatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(10000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue)), Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kLatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

}

// This test ensures Handle set Target command executes as expected. Tests:
// - Return success if mismatch between arguments and conformance.
TEST_F(TestClosureDimensionClusterLogic, TestHandleSetTargetCommandConformace)
{

    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    ClusterConformance conformance = { .featureMap = 255,
                                       .supportsOverflow = true};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericCurrentStateStruct setCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    conformance = { .featureMap = 0,
                    .supportsOverflow = false};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    conformance = { .featureMap = 1,
        .supportsOverflow = false};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    conformance = { .featureMap = 3,
        .supportsOverflow = false};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleSetTargetCommand(Optional<Percent100ths>(1000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::Success);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
}


// This test ensures Handle set Target command executes as expected. Tests:
// - Exit if HandleSetTarget is called without Device Intilialization.
// TEST_F(TestClosureDimensionClusterLogic, TestHandleStepTarget)
// {

//     TestDelegate delegate;
//     EndpointId endpoint = 0;
//     MockedMatterContext context(endpoint);
//     auto logic = std::make_unique<ClusterLogic>(delegate, context);


//     EXPECT_DEATH(logic->HandleStepTarget(Optional<Percent100ths>(1000), Optional<TargetLatchEnum>(TargetLatchEnum::kLatch),
//     Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)), Protocols::InteractionModel::Status::Success);

// }

// This test ensures Handle Step command executes as expected. Tests:
// - Return InvalidInState if CurrentState is unknown.
// - Return constrainError if arguments value are out of bounds
TEST_F(TestClosureDimensionClusterLogic, TestHandleStepCommand)
{
    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    ClusterConformance conformance = { .featureMap = 247,
                                       .supportsOverflow = true};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::InvalidInState);

    GenericCurrentStateStruct currentState;
    GenericTargetStruct target;
    Percent100ths stepValue = 10;

    GenericCurrentStateStruct setCurrentState{
        Optional<Percent100ths>(0),
        Optional<LatchingEnum>(LatchingEnum::kNotLatched),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    GenericTargetStruct settarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
        };
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetTarget(settarget), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_NO_ERROR);

    //Check Target and current state after command
    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(100));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(100);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 65535, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(10000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kMedium);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(10000);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(9900));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(9900);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 65535, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(0));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(0);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);


    //Test with Limiting Feature enabled
    conformance = { .featureMap = 255,
        .supportsOverflow = true};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    Structs::RangePercent100thsStruct::Type limitRange = { .min = 1000, .max = 9000};
    EXPECT_EQ(logic->SetLimitRange(limitRange), CHIP_NO_ERROR);
    setCurrentState.position = Optional<Percent100ths>(1000);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetStepValue(stepValue), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10 , Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1100));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(1100);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 65535, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kMedium)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(9000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kMedium);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(9000);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(8900));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kLow);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(8900);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kDecrease, 65535, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(1000));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
    //As delegate is not implemented, we are updating current state here
    setCurrentState.position = Optional<Percent100ths>(1000);
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);



    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kUnknownEnumValue, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 0, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kUnknownEnumValue)),
        Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

}

// This test ensures Handle set Target command executes as expected. Tests:
// - Return UnsupportedCommand if positioning feature is not supported
// - ignore Speed if speed feature is not supported
TEST_F(TestClosureDimensionClusterLogic, TestHandleStepCommandConformace)
{

    TestDelegate delegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint);
    auto logic = std::make_unique<ClusterLogic>(delegate, context);

    ClusterConformance conformance = { .featureMap = 255,
        .supportsOverflow = true};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericCurrentStateStruct setCurrentState{
    Optional<Percent100ths>(0),
    Optional<LatchingEnum>(LatchingEnum::kNotLatched),
    Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
    };
    GenericTargetStruct settarget{
        Optional<Percent100ths>(0),
        Optional<TargetLatchEnum>(TargetLatchEnum::kUnlatch),
        Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto)
    };
    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);

    conformance = { .featureMap = 0,
                                       .supportsOverflow = false};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    GenericTargetStruct target;

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::UnsupportedCommand);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Current::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));

    conformance = { .featureMap = 3,
        .supportsOverflow = false};
    EXPECT_EQ(logic->Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCurrentState(setCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetTarget(settarget), CHIP_NO_ERROR);

    context.ClearDirtyList();
    EXPECT_EQ(logic->HandleStepCommand(StepDirectionEnum::kIncrease, 10, Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
        Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(logic->GetTarget(target), CHIP_NO_ERROR);
    EXPECT_EQ(target.position.Value(),static_cast<unsigned short>(10));
    EXPECT_EQ(target.latch.Value(), TargetLatchEnum::kUnlatch);
    EXPECT_EQ(target.speed,NullOptional);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::Target::Id));
}


} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
