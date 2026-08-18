/*
 *
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
#include <device/types/window-covering/impl/LoggingWindowCovering.h>
#include <inttypes.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

LoggingWindowCovering::LoggingWindowCovering(const Context & context) : WindowCovering(*this, *this, context) {}

void LoggingWindowCovering::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnIdentifyStart");
}

void LoggingWindowCovering::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnIdentifyStop");
}

void LoggingWindowCovering::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: OnTriggerEffect");
}

CHIP_ERROR LoggingWindowCovering::HandleMovement(Clusters::WindowCovering::WindowCoveringType type)
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleMovement type=%" PRIu16, static_cast<unsigned>(type));
    auto & cluster = WindowCoveringCluster();
    if (type == Clusters::WindowCovering::WindowCoveringType::Lift)
    {
        auto target = cluster.GetTargetPositionLiftPercent100ths();
        if (!target.IsNull())
        {
            cluster.SetCurrentPositionLiftPercent100ths(target);
        }
    }
    else if (type == Clusters::WindowCovering::WindowCoveringType::Tilt)
    {
        auto target = cluster.GetTargetPositionTiltPercent100ths();
        if (!target.IsNull())
        {
            cluster.SetCurrentPositionTiltPercent100ths(target);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingWindowCovering::HandleStopMotion()
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleStopMotion");
    return CHIP_NO_ERROR;
}

void LoggingWindowCovering::OnTargetPositionLiftChanged(DataModel::Nullable<Percent100ths> newTargetLift)
{
    if (newTargetLift.IsNull())
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionLiftChanged -> NULL");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionLiftChanged -> %" PRIu16, newTargetLift.Value());
    }
}

void LoggingWindowCovering::OnTargetPositionTiltChanged(DataModel::Nullable<Percent100ths> newTargetTilt)
{
    if (newTargetTilt.IsNull())
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionTiltChanged -> NULL");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "WindowCovering: OnTargetPositionTiltChanged -> %" PRIu16, newTargetTilt.Value());
    }
}

} // namespace app
} // namespace chip
