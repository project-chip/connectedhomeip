/**
 *
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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <app/data-model/Nullable.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

// TODO: Update the unit tests when the XML is updated to use latest cluster spec
//       - SecureState isn't present the cluster XML yet

TEST(GenericOverallStateTest, DefaultConstructor)
{
    GenericOverallState state;
    EXPECT_FALSE(state.positioning.HasValue());
    EXPECT_FALSE(state.latch.HasValue());
    EXPECT_FALSE(state.speed.HasValue());
}

TEST(GenericOverallStateTest, ParameterizedConstructor)
{
    auto positioning = Optional<DataModel::Nullable<PositioningEnum>>(MakeNullable(PositioningEnum::kFullyClosed));
    auto latch       = Optional<DataModel::Nullable<bool>>(MakeNullable(true));
    auto speed       = Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>>(MakeNullable(Globals::ThreeLevelAutoEnum::kAuto));

    GenericOverallState state(positioning, latch, speed, NullOptional);

    EXPECT_TRUE(state.positioning.HasValue());
    EXPECT_EQ(state.positioning.Value(), PositioningEnum::kFullyClosed);
    EXPECT_TRUE(state.latch.HasValue());
    EXPECT_EQ(state.latch.Value(), true);
    EXPECT_TRUE(state.speed.HasValue());
    EXPECT_EQ(state.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}

TEST(GenericOverallStateTest, EqualityOperator)
{
    auto positioning1 = Optional<DataModel::Nullable<PositioningEnum>>(MakeNullable(PositioningEnum::kFullyClosed));
    auto latching1    = Optional<DataModel::Nullable<bool>>(MakeNullable(true));

    auto positioning2 = Optional<DataModel::Nullable<PositioningEnum>>(MakeNullable(PositioningEnum::kFullyClosed));
    auto latching2    = Optional<DataModel::Nullable<bool>>(MakeNullable(true));

    GenericOverallState state1(positioning1, latching1);
    GenericOverallState state2(positioning2, latching2);

    EXPECT_TRUE(state1 == state2);

    latching2    = Optional<DataModel::Nullable<bool>>(MakeNullable(false));
    state2.latch = latching2;
    EXPECT_FALSE(state1 == state2);
}

TEST(GenericOverallStateTest, CopyConstructor)
{
    auto positioning = Optional<DataModel::Nullable<PositioningEnum>>(MakeNullable(PositioningEnum::kFullyClosed));
    auto latch       = Optional<DataModel::Nullable<bool>>(MakeNullable(true));
    auto speed       = Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>>(MakeNullable(Globals::ThreeLevelAutoEnum::kAuto));

    GenericOverallState originalState(positioning, latch, speed, NullOptional);
    GenericOverallState copiedState(originalState);

    // Modify the original object
    originalState.positioning = Optional<DataModel::Nullable<PositioningEnum>>(MakeNullable(PositioningEnum::kFullyOpened));
    originalState.latch       = Optional<DataModel::Nullable<bool>>(MakeNullable(false));
    originalState.speed =
        Optional<DataModel::Nullable<Globals::ThreeLevelAutoEnum>>(MakeNullable(Globals::ThreeLevelAutoEnum::kLow));

    // Validate that the copied object remains unchanged
    EXPECT_TRUE(copiedState.positioning.HasValue());
    EXPECT_EQ(copiedState.positioning.Value(), PositioningEnum::kFullyClosed);
    EXPECT_TRUE(copiedState.latch.HasValue());
    EXPECT_EQ(copiedState.latch.Value(), true);
    EXPECT_TRUE(copiedState.speed.HasValue());
    EXPECT_EQ(copiedState.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}

TEST(GenericOverallTargetTest, DefaultConstructor)
{
    GenericOverallTarget target;
    EXPECT_FALSE(target.position.HasValue());
    EXPECT_FALSE(target.latch.HasValue());
    EXPECT_FALSE(target.speed.HasValue());
}

TEST(GenericOverallTargetTest, ParameterizedConstructor)
{
    auto position = Optional<TargetPositionEnum>(TargetPositionEnum::kCloseInFull);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh);

    GenericOverallTarget target(position, latch, speed);

    EXPECT_TRUE(target.position.HasValue());
    EXPECT_EQ(target.position.Value(), TargetPositionEnum::kCloseInFull);
    EXPECT_TRUE(target.latch.HasValue());
    EXPECT_EQ(target.latch.Value(), true);
    EXPECT_TRUE(target.speed.HasValue());
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
}

TEST(GenericOverallTargetTest, EqualityOperator)
{
    auto position1 = Optional<TargetPositionEnum>(TargetPositionEnum::kCloseInFull);
    auto latch1    = Optional<bool>(true);

    auto position2 = Optional<TargetPositionEnum>(TargetPositionEnum::kCloseInFull);
    auto latch2    = Optional<bool>(true);

    GenericOverallTarget target1(position1, latch1);
    GenericOverallTarget target2(position2, latch2);

    EXPECT_TRUE(target1 == target2);

    latch2        = Optional<bool>(false);
    target2.latch = latch2;
    EXPECT_FALSE(target1 == target2);
}

TEST(GenericOverallTargetTest, CopyConstructor)
{
    auto position = Optional<TargetPositionEnum>(TargetPositionEnum::kCloseInFull);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh);

    GenericOverallTarget originalTarget(position, latch, speed);
    GenericOverallTarget copiedTarget(originalTarget);

    // Modify the original object
    originalTarget.position = Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull);
    originalTarget.latch    = Optional<bool>(false);
    originalTarget.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    // Validate that the copied object remains unchanged
    EXPECT_TRUE(copiedTarget.position.HasValue());
    EXPECT_EQ(copiedTarget.position.Value(), TargetPositionEnum::kCloseInFull);
    EXPECT_TRUE(copiedTarget.latch.HasValue());
    EXPECT_EQ(copiedTarget.latch.Value(), true);
    EXPECT_TRUE(copiedTarget.speed.HasValue());
    EXPECT_EQ(copiedTarget.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
}
