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
#include <devices/speaker/impl/LoggingSpeakerDevice.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

LoggingSpeakerDevice::LoggingSpeakerDevice(const Context & context) : SpeakerDevice(*this, *this, context.timerDelegate) {}

LoggingSpeakerDevice::~LoggingSpeakerDevice() {}

// LevelControlDelegate

void LoggingSpeakerDevice::OnLevelChanged(uint8_t value)
{
    uint8_t min  = LevelControlCluster().GetMinLevel();
    uint8_t max  = LevelControlCluster().GetMaxLevel();
    uint32_t pct = (max > min) ? (static_cast<uint32_t>(value - min) * 100) / (max - min) : 0;
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: Volume set to %u (%u%%)", value, pct);
}

void LoggingSpeakerDevice::OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> value)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: Options changed to 0x%02X", value.Raw());
}

void LoggingSpeakerDevice::OnOnLevelChanged(DataModel::Nullable<uint8_t> value)
{
    if (!value.IsNull())
    {
        ChipLogProgress(AppServer, "LoggingSpeakerDevice: OnLevel changed to %u", value.Value());
    }
    else
    {
        ChipLogProgress(AppServer, "LoggingSpeakerDevice: OnLevel changed to NULL");
    }
}

void LoggingSpeakerDevice::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> value)
{
    if (!value.IsNull())
    {
        ChipLogProgress(AppServer, "LoggingSpeakerDevice: DefaultMoveRate changed to %u", value.Value());
    }
    else
    {
        ChipLogProgress(AppServer, "LoggingSpeakerDevice: DefaultMoveRate changed to NULL");
    }
}

// OnOffDelegate

void LoggingSpeakerDevice::OnOffStartup(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: OnOffStartup() - Speaker %s", on ? "Unmuted" : "Muted");
}

void LoggingSpeakerDevice::OnOnOffChanged(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: OnOnOffChanged() - Speaker %s", on ? "Unmuted" : "Muted");
}

} // namespace app
} // namespace chip
