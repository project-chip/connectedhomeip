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

#include <device/capabilities/dimmable-load/impl/LoggingDimmableDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

// LevelControlDelegate

void LoggingDimmableDelegate::OnLevelChanged(uint8_t level)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnLevelChanged() -> %u", level);
}

void LoggingDimmableDelegate::OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options)
{
    ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnOptionsChanged() -> 0x%02X", options.Raw());
}

void LoggingDimmableDelegate::OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel)
{
    if (!onLevel.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnOnLevelChanged() -> %u", onLevel.Value());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnOnLevelChanged() -> NULL");
    }
}

void LoggingDimmableDelegate::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate)
{
    if (!defaultMoveRate.IsNull())
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnDefaultMoveRateChanged() -> %u", defaultMoveRate.Value());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "LoggingDimmableDelegate::OnDefaultMoveRateChanged() -> NULL");
    }
}

} // namespace app
} // namespace chip
