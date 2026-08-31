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

// Registers the code-driven On/Off and Level Control clusters for the tv-app Speaker
// endpoint (MA-speaker, endpoint 2) only. MA-videoplayer's On/Off (endpoint 1) is
// untouched and keeps using the legacy Ember plugin.
//
// This is phase 1 of the Speaker migration: it establishes the on/off <-> level
// coupling that the legacy Ember plugins already provided (turning the speaker off
// zeroes/restores level via the standard OnOffDelegate hook), but does not yet wire
// AudioControl into the sync -- that is a later phase.

namespace chip::app::Clusters::Speaker {

// Must be called after Server::Init() has completed (so the legacy On/Off and Level
// Control Ember plugin init callbacks for endpoint 2 have already run once, and this
// registration's Startup() calls are authoritative for the code-driven clusters'
// initial attribute values).
void InitOnOffLevelControl();

// Mirrors InitOnOffLevelControl(); safe to call even if Init did not run.
void ShutdownOnOffLevelControl();

} // namespace chip::app::Clusters::Speaker
