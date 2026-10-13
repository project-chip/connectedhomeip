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

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes::FeatureMap {

Status Get(EndpointId, uint32_t * value)
{
    *value = 0;
    return Status::Success;
}

} // namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes::FeatureMap

namespace {

TEST(TestPumpConfigurationAndControlCluster, RejectsUndefinedControlModeWithinAttributeRange)
{
    uint8_t value = 6;
    ConcreteAttributePath path(1, PumpConfigurationAndControl::Id, PumpConfigurationAndControl::Attributes::ControlMode::Id);

    EXPECT_EQ(MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback(path, ZCL_ENUM8_ATTRIBUTE_TYPE,
                                                                                        sizeof(value), &value),
              Status::ConstraintError);
}

} // namespace
