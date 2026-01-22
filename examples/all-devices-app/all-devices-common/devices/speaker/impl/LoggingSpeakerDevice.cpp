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
#include <platform/DefaultTimerDelegate.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
DefaultTimerDelegate sTimerDelegate;
} // namespace

LoggingSpeakerDevice::LoggingSpeakerDevice() : SpeakerDevice(*this, *this, sTimerDelegate) {}

LoggingSpeakerDevice::~LoggingSpeakerDevice() {}

// LevelControlDelegate

void LoggingSpeakerDevice::OnLevelChanged(uint8_t value)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: Volume set to %u", value);
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

void LoggingSpeakerDevice::SetOnOff(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: Request to %s Speaker (Mute/Unmute)", on ? "Unmute" : "Mute");
    // Forward to OnOff Cluster
    LogErrorOnFailure(OnOffCluster().SetOnOff(on));
}

bool LoggingSpeakerDevice::GetOnOff()
{
    return OnOffCluster().GetOnOff();
}

// OnOffDelegate

void LoggingSpeakerDevice::OnOffStartup(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: OnOff Startup: %d", on);
    // Usually no action needed unless we need to sync LC state immediately?
}

void LoggingSpeakerDevice::OnOnOffChanged(bool on)
{
    ChipLogProgress(AppServer, "LoggingSpeakerDevice: Speaker %s (Mute State Changed)", on ? "Unmuted" : "Muted");
    // Forward to Level Control Cluster to handle "Effect of On/Off Commands on CurrentLevel"
    LevelControlCluster().OnOffChanged(on);
}

} // namespace app
} // namespace chip
