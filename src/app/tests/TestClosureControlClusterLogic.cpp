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

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>

using namespace chip;
using namespace chip::app::Clusters::ClosureControl;
using Status = chip::Protocols::InteractionModel::Status;

namespace {
// Simple mock implementation of DelegateBase
class MockDelegate : public DelegateBase
{
public:
    virtual ~MockDelegate() = default;

    Status HandleStopCommand() override { return Status::Success; }
    Status HandleMoveToCommand() override { return Status::Success; }
    Status HandleCalibrateCommand() override { return Status::Success; }
};
} // namespace

// Simple mock implementation of MatterContext
class MockMatterContext : public MatterContext
{
public:
    MockMatterContext() : MatterContext(kInvalidEndpointId) {}
    void MarkDirty(AttributeId attributeId) override
    { /* No-op for testing */
    }
};

TEST(TestClosureControlClusterLogic, InitValidConformance)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning);

    MockDelegate mockDelegate;
    MockMatterContext mockContext;
    ClusterLogic logic(mockDelegate, mockContext);

    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);
}

TEST(TestClosureControlClusterLogic, InitInvalidConformance)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed).Set(Feature::kInstantaneous);
    MockDelegate mockDelegate;
    MockMatterContext mockContext;
    ClusterLogic logic(mockDelegate, mockContext);

    EXPECT_NE(logic.Init(conformance), CHIP_NO_ERROR);
}
