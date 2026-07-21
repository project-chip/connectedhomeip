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
#include <device/types/speaker/impl/LoggingSpeaker.h>
#include <inttypes.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

LoggingSpeaker::LoggingSpeaker(const Context & context) : Speaker(*this, *this, context.timerDelegate) {}

LoggingSpeaker::~LoggingSpeaker() {}

// LevelControlDelegate

void LoggingSpeaker::OnLevelChanged(uint8_t value)
{
    uint8_t min  = LevelControlCluster().GetMinLevel();
    uint8_t max  = LevelControlCluster().GetMaxLevel();
    uint32_t pct = (max > min) ? (static_cast<uint32_t>(value - min) * 100) / (max - min) : 0;
    ChipLogProgress(AppServer, "LoggingSpeaker: Volume set to %u (%" PRIu32 "%%)", value, pct);
}

void LoggingSpeaker::OnOptionsChanged(BitMask<Clusters::LevelControl::OptionsBitmap> value)
{
    ChipLogProgress(AppServer, "LoggingSpeaker: Options changed to 0x%02X", value.Raw());
}

void LoggingSpeaker::OnOnLevelChanged(DataModel::Nullable<uint8_t> value)
{
    if (!value.IsNull())
    {
        ChipLogProgress(AppServer, "LoggingSpeaker: OnLevel changed to %u", value.Value());
    }
    else
    {
        ChipLogProgress(AppServer, "LoggingSpeaker: OnLevel changed to NULL");
    }
}

void LoggingSpeaker::OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> value)
{
    if (!value.IsNull())
    {
        ChipLogProgress(AppServer, "LoggingSpeaker: DefaultMoveRate changed to %u", value.Value());
    }
    else
    {
        ChipLogProgress(AppServer, "LoggingSpeaker: DefaultMoveRate changed to NULL");
    }
}

// OnOffDelegate

void LoggingSpeaker::OnOffStartup(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeaker: OnOffStartup() - Speaker %s", on ? "Unmuted" : "Muted");
}

void LoggingSpeaker::OnOnOffChanged(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeaker: OnOnOffChanged() - Speaker %s", on ? "Unmuted" : "Muted");
}

} // namespace app
} // namespace chip
