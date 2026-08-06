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
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleMovement type=%u", static_cast<unsigned>(type));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingWindowCovering::HandleStopMotion()
{
    ChipLogProgress(DeviceLayer, "WindowCovering: HandleStopMotion");
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
