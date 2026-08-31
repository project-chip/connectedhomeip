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
// Also owns the SpeakerAudioCoordinator that keeps On/Off, Level Control and Audio
// Control in sync on this endpoint (OnOff.OnOff mirrors the inverse of
// AudioControl.SoftMuted; LevelControl.CurrentLevel and AudioControl.Volume track the
// same audio level, scaled between their ranges). AudioControlCluster itself is not
// constructed here -- it is code-driven via tv-app.zap's normal ZAP dispatch (Audio
// Control is on the CodeDrivenClusters list) -- the .cpp file registers the coordinator
// as its delegate at static-init time (before Server::Init(), the same requirement
// AudioControl::SetDelegate always had) and retrieves the resulting instance in
// InitOnOffLevelControl() to hand to SpeakerAudioCoordinator::SetClusters().

namespace chip::app::Clusters::Speaker {

// Must be called after Server::Init() has completed (so the legacy On/Off and Level
// Control Ember plugin init callbacks for endpoint 2 have already run once, and this
// registration's Startup() calls are authoritative for the code-driven clusters'
// initial attribute values).
void InitOnOffLevelControl();

// Mirrors InitOnOffLevelControl(); safe to call even if Init did not run.
void ShutdownOnOffLevelControl();

} // namespace chip::app::Clusters::Speaker
