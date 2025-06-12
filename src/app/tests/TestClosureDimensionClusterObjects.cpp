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

#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/data-model/Nullable.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;

TEST(GenericCurrentStateStructTest, DefaultConstructor)
{
    GenericDimensionStateStruct currentState;
    EXPECT_FALSE(currentState.position.HasValue());
    EXPECT_FALSE(currentState.latch.HasValue());
    EXPECT_FALSE(currentState.speed.HasValue());
}

TEST(GenericCurrentStateStructTest, ParameterizedConstructor)
{
    auto position = Optional<Percent100ths>(5000);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto);

    GenericDimensionStateStruct currentState(position, latch, speed);

    EXPECT_TRUE(currentState.position.HasValue());
    EXPECT_EQ(currentState.position.Value().Value(), 5000);
    EXPECT_TRUE(currentState.latch.HasValue());
    EXPECT_EQ(currentState.latch.Value().Value(), true);
    EXPECT_TRUE(currentState.speed.HasValue());
    EXPECT_EQ(currentState.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}

TEST(GenericCurrentStateStructTest, EqualityOperator)
{
    auto position1 = Optional<Percent100ths>(6000);
    auto latch1    = Optional<bool>(true);

    auto position2 = Optional<Percent100ths>(6000);
    auto latch2    = Optional<bool>(true);

    GenericDimensionStateStruct state1(position1, latch1);
    GenericDimensionStateStruct state2(position2, latch2);

    EXPECT_TRUE(state1 == state2);

    latch2       = Optional<bool>(false);
    state2.latch = latch2;
    EXPECT_FALSE(state1 == state2);
}

TEST(GenericCurrentStateStructTest, InequalityOperator)
{
    auto position1 = Optional<Percent100ths>(6000);
    auto latch1    = Optional<bool>(true);

    auto position2 = Optional<Percent100ths>(6000);
    auto latch2    = Optional<bool>(false);

    GenericDimensionStateStruct state1(position1, latch1);
    GenericDimensionStateStruct state2(position2, latch2);

    EXPECT_TRUE(state1 != state2);

    latch2       = Optional<bool>(true);
    state2.latch = latch2;
    EXPECT_FALSE(state1 != state2);
}

TEST(GenericCurrentStateStructTest, CopyConstructor)
{
    auto position = Optional<Percent100ths>(1000);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto);

    GenericDimensionStateStruct originalState(position, latch, speed);
    GenericDimensionStateStruct copiedState(originalState);

    // Modify the original object
    originalState.position = Optional<Percent100ths>(2000);
    originalState.latch    = Optional<bool>(false);
    originalState.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    // Validate that the copied object remains unchanged
    EXPECT_TRUE(copiedState.position.HasValue());
    EXPECT_EQ(copiedState.position.Value().Value(), 1000);
    EXPECT_TRUE(copiedState.latch.HasValue());
    EXPECT_EQ(copiedState.latch.Value().Value(), true);
    EXPECT_TRUE(copiedState.speed.HasValue());
    EXPECT_EQ(copiedState.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}

TEST(GenericCurrentStateStructTest, Set)
{
    auto position = Optional<Percent100ths>(1000);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto);

    GenericDimensionStateStruct originalState;
    EXPECT_FALSE(originalState.position.HasValue());
    EXPECT_FALSE(originalState.latch.HasValue());
    EXPECT_FALSE(originalState.speed.HasValue());

    originalState.Set(position, latch, speed);

    // Validate that the set
    EXPECT_TRUE(originalState.position.HasValue());
    EXPECT_EQ(originalState.position.Value().Value(), 1000);
    EXPECT_TRUE(originalState.latch.HasValue());
    EXPECT_EQ(originalState.latch.Value().Value(), true);
    EXPECT_TRUE(originalState.speed.HasValue());
    EXPECT_EQ(originalState.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}

TEST(GenericTargetStructTest, DefaultConstructor)
{
    GenericDimensionStateStruct target;
    EXPECT_FALSE(target.position.HasValue());
    EXPECT_FALSE(target.latch.HasValue());
    EXPECT_FALSE(target.speed.HasValue());
}

TEST(GenericTargetStructTest, ParameterizedConstructor)
{
    auto position = Optional<Percent100ths>(5000);
    auto latch    = Optional<bool>(false);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh);

    GenericDimensionStateStruct target(position, latch, speed);

    EXPECT_TRUE(target.position.HasValue());
    EXPECT_EQ(target.position.Value().Value(), 5000);
    EXPECT_TRUE(target.latch.HasValue());
    EXPECT_EQ(target.latch.Value().Value(), false);
    EXPECT_TRUE(target.speed.HasValue());
    EXPECT_EQ(target.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
}

TEST(GenericTargetStructTest, EqualityOperator)
{
    auto position1 = Optional<Percent100ths>(1000);
    auto latch1    = Optional<bool>(true);

    auto position2 = Optional<Percent100ths>(1000);
    auto latch2    = Optional<bool>(true);

    GenericDimensionStateStruct target1(position1, latch1);
    GenericDimensionStateStruct target2(position2, latch2);

    EXPECT_TRUE(target1 == target2);

    latch2        = Optional<bool>(false);
    target2.latch = latch2;
    EXPECT_FALSE(target1 == target2);
}

TEST(GenericTargetStructTest, InequalityOperator)
{
    auto position1 = Optional<Percent100ths>(1000);
    auto latch1    = Optional<bool>(true);

    auto position2 = Optional<Percent100ths>(1000);
    auto latch2    = Optional<bool>(false);

    GenericDimensionStateStruct target1(position1, latch1);
    GenericDimensionStateStruct target2(position2, latch2);

    EXPECT_TRUE(target1 != target2);

    latch2        = Optional<bool>(true);
    target2.latch = latch2;
    EXPECT_FALSE(target1 != target2);
}

TEST(GenericTargetStructTest, CopyConstructor)
{
    auto position = Optional<Percent100ths>(1000);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh);

    GenericDimensionStateStruct originalTarget(position, latch, speed);
    GenericDimensionStateStruct copiedTarget(originalTarget);

    // Modify the original object
    originalTarget.position = Optional<Percent100ths>(2000);
    originalTarget.latch    = Optional<bool>(false);
    originalTarget.speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kLow);

    // Validate that the copied object remains unchanged
    EXPECT_TRUE(copiedTarget.position.HasValue());
    EXPECT_EQ(copiedTarget.position.Value().Value(), 1000);
    EXPECT_TRUE(copiedTarget.latch.HasValue());
    EXPECT_EQ(copiedTarget.latch.Value().Value(), true);
    EXPECT_TRUE(copiedTarget.speed.HasValue());
    EXPECT_EQ(copiedTarget.speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);
}

TEST(GenericTargetStructTest, Set)
{
    auto position = Optional<Percent100ths>(1000);
    auto latch    = Optional<bool>(true);
    auto speed    = Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kAuto);

    GenericDimensionStateStruct originalState;
    EXPECT_FALSE(originalState.position.HasValue());
    EXPECT_FALSE(originalState.latch.HasValue());
    EXPECT_FALSE(originalState.speed.HasValue());

    originalState.Set(position, latch, speed);

    // Validate that the set
    EXPECT_TRUE(originalState.position.HasValue());
    EXPECT_EQ(originalState.position.Value().Value(), 1000);
    EXPECT_TRUE(originalState.latch.HasValue());
    EXPECT_EQ(originalState.latch.Value().Value(), true);
    EXPECT_TRUE(originalState.speed.HasValue());
    EXPECT_EQ(originalState.speed.Value(), Globals::ThreeLevelAutoEnum::kAuto);
}
