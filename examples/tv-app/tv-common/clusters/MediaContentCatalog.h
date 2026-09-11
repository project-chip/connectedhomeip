/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <clusters/MediaPlayback/CommandIds.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace MediaContentCatalog {

// Scoped to this namespace rather than the top level so the header does not leak it.
using namespace chip::literals;

// Live content cannot be stepped through or seeked, so it offers fewer playback commands
// than on-demand content does.
inline constexpr chip::CommandId kLiveCommands[] = {
    chip::app::Clusters::MediaPlayback::Commands::Play::Id,
    chip::app::Clusters::MediaPlayback::Commands::Pause::Id,
    chip::app::Clusters::MediaPlayback::Commands::Stop::Id,
};

inline constexpr chip::CommandId kOnDemandCommands[] = {
    chip::app::Clusters::MediaPlayback::Commands::Play::Id,        chip::app::Clusters::MediaPlayback::Commands::Pause::Id,
    chip::app::Clusters::MediaPlayback::Commands::Stop::Id,        chip::app::Clusters::MediaPlayback::Commands::Next::Id,
    chip::app::Clusters::MediaPlayback::Commands::Previous::Id,    chip::app::Clusters::MediaPlayback::Commands::Rewind::Id,
    chip::app::Clusters::MediaPlayback::Commands::FastForward::Id, chip::app::Clusters::MediaPlayback::Commands::Seek::Id,
};

// One piece of content the app can play. The Content Launcher reports these as its Presets
// and starts them with PlayPreset; the Media Playback cluster reports the name and the
// available commands of whichever entry is currently playing.
struct Entry
{
    uint8_t presetID;
    chip::CharSpan name;
    chip::Span<const chip::CommandId> availableCommands;
};

// Keeping both clusters on this single list is what stops the Presets attribute and the
// content that Media Playback describes from drifting apart.
inline constexpr Entry kEntries[] = {
    { 1, "Morning News"_span, chip::Span<const chip::CommandId>(kLiveCommands) },
    { 2, "Evening Playlist"_span, chip::Span<const chip::CommandId>(kOnDemandCommands) },
};

} // namespace MediaContentCatalog
